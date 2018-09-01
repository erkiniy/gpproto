//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#include "Proto.h"
#include "gp/utils/Logging.h"
#include "gp/network/TcpTransport.h"
#include "gp/network/TransportScheme.h"
#include "gp/utils/Crypto.h"
#include "gp/utils/OutputStream.h"
#include "gp/utils/InputStream.h"
#include "gp/proto/MessageEncryptionKey.h"

using namespace gpproto;

void Proto::setDelegate(std::shared_ptr<ProtoDelegate> delegate) {
    Proto::queue()->async([&, delegate] {
        this->delegate = delegate;
    });
}

void Proto::pause() {
    Proto::queue()->async([&] {
        if ((protoState & ProtoStatePaused) == 0)
        {
            setState(protoState | ProtoStatePaused);
            LOGV("Proto paused");
        }
    });
}

void Proto::resume() {
    Proto::queue()->async([&] {

        if (protoState & ProtoStatePaused)
        {
            setState(protoState & (~ProtoStatePaused));
            LOGV("Proto resumed");
        }
    });
}

void Proto::stop() {
    Proto::queue()->async([&] {
        if ((protoState & ProtoStateStopped) == 0)
        {
            setState(protoState | ProtoStateStopped);
            LOGV("Proto stopped");
        }
    });
}

void Proto::setState(int state) {

    protoState = static_cast<ProtoState>(state);

}

bool Proto::isStopped() {
    return (protoState & ProtoStateStopped) != 0;
}

bool Proto::isPaused() {
    return (protoState & ProtoStatePaused) != 0;
}

void Proto::setTransport(std::shared_ptr<Transport> transport) {
    Proto::queue()->async([strongSelf = shared_from_this(), transport] {
        LOGV("[Proto setTransport] -> changing transport");

        strongSelf->allTransactionsMayHaveFailed();

        auto previousTransport = strongSelf->transport;

        strongSelf->transport = transport;
        previousTransport->stop();

        strongSelf->updateConnectionState();
    });
}

void Proto::resetTransport() {
    Proto::queue()->async([strongSelf = shared_from_this()] {

        if (strongSelf->protoState & ProtoStateStopped)
            return;

        if (auto _transport = strongSelf->transport)
        {
            _transport->setDelegate(nullptr);
            _transport->stop();

            strongSelf->setTransport(nullptr);
        }

        if (strongSelf->transportScheme == nullptr)
        {
            if ((strongSelf->protoState & ProtoStateAwaitingTransportScheme) == 0)
            {
                LOGV("[Proto resetTransport] -> awaitingTransportScheme");

                strongSelf->setState(strongSelf->protoState | ProtoStateAwaitingTransportScheme);
                strongSelf->context->transportSchemeForDatacenterIdRequired(strongSelf->datacenterId);
            }
        }
        if (!strongSelf->useUnauthorizedMode && strongSelf->context->getAuthKeyInfoForDatacenterId(strongSelf->datacenterId) == nullptr)
        {
            if ((strongSelf->protoState & ProtoStateAwaitingAuthorization) == 0)
            {
                LOGV("[Proto resetTransport] -> missing authorized key for datacenterId = %d", strongSelf->datacenterId);

                strongSelf->setState(strongSelf->protoState | ProtoStateAwaitingAuthorization);
                strongSelf->context->authInfoForDatacenterWithIdRequired(strongSelf->datacenterId);
            }
        }
        else
        {
            LOGV("[Proto resetTransport] -> setting created transport");
            auto transport = strongSelf->transportScheme->createTransportWithContext(strongSelf->context, strongSelf->datacenterId, strongSelf);

            strongSelf->setTransport(transport);
        }
    });
}

void Proto::allTransactionsMayHaveFailed() {
    Proto::queue()->async([strongSelf = shared_from_this()] {

    });
}

void Proto::updateConnectionState() {
    Proto::queue()->async([strongSelf = shared_from_this()] {

        if (strongSelf->transport)
            strongSelf->transport->updateConnectionState();
        else
        {
            if (auto delegate = strongSelf->delegate.lock())
            {
                delegate->connectionStateAvailibilityChanged(*strongSelf, false);
                delegate->connectionStateChanged(*strongSelf, ProtoConnectionState::ProtoConnectionStateConnecting);
            }
        }
    });
}

void Proto::transportHasIncomingData(const Transport &transport, std::shared_ptr<StreamSlice> data,
                                     bool requestTransactionAfterProcessing, std::function<void(bool)> decodeResult) {
    Proto::queue()->async([&, strongSelf = shared_from_this(), data, requestTransactionAfterProcessing, decodeResult] {

        if (strongSelf->transport == nullptr || !strongSelf->transport->isEqual(transport) || strongSelf->isStopped())
            return;

        if (data->size <= 4 + 15)
        {
            LOGE("[Proto transportHasIncomingData] -> received data with size less than 19 bytes, dropping %lu bytes", data->size);
            decodeResult(false);

            //strongSelf->resetTransport();
            return;
        }

        std::shared_ptr<StreamSlice> decryptedData = nullptr;

        if (strongSelf->useUnauthorizedMode)
            decryptedData = data;
        else
            decryptedData = decryptIncomingTransportData(data);

        if (decryptedData)
        {
            decodeResult(true);


        }
    });
}

std::shared_ptr<StreamSlice> Proto::decryptIncomingTransportData(const std::shared_ptr<StreamSlice> &data) {

    if (data->size < 24 + 36) {
        LOGE("[Proto decryptIncomingTransportData] -> received less data than 24 + 36 bytes");
        return nullptr;
    }

    if (authInfo == nullptr) {
        LOGE("[Proto decryptIncomingTransportData] -> missing authkKey");
        return nullptr;
    }


    int64_t authKeyId = 0;
    memcpy(&authKeyId, data->rbegin(), 8);

    if (authKeyId != authInfo->authKeyId) {
        LOGE("[Proto decryptIncomingTransportData] -> received message with wrong authKey id from expected");
        return nullptr;
    }

    auto remainingData = data->subData(24, data->size - 24);

    auto embeddedMessageKey = data->subData(8, 16);

    auto encryptionKey = MessageEncryptionKey::messageEncryptionKeyForAuthKey(authInfo->authKey, embeddedMessageKey);

    auto decryptedData = Crypto::aes_cbc_decrypt(encryptionKey->aes_key, &encryptionKey->aes_iv, *remainingData);

    if (decryptedData->size < 32) {
        LOGE("[Proto decryptIncomingTransportData] -> decrypted data is less than 32 bytes");
        return nullptr;
    }

    size_t messageDataLength = 0;
    auto messageLengthData = decryptedData->subData(28, 32);
    memcpy(&messageDataLength, messageLengthData->rbegin(), 4);

    if (messageDataLength > decryptedData->size - 32) {
        LOGE("[Proto decryptIncomingTransportData] -> wrong message length %lu while decrypted message length is %lu", messageDataLength, decryptedData->size);
        return nullptr;
    }

    auto messageKeyFull = Crypto::sha256Subdata(*decryptedData, 0, 32 + messageDataLength);
    auto messageKey = messageKeyFull->subData(16, 16);

    if (*messageKey != *embeddedMessageKey) {
        LOGE("[Proto decryptIncomingTransportData] -> received message key is different from computed");
        return nullptr;
    }

    return decryptedData;
}