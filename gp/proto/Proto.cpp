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
#include "gp/proto/InternalParser.h"
#include "gp/network/IncomingMessage.h"
#include "gp/proto/TimeFixContext.h"
#include "gp/proto/MessageService.h"
#include "gp/proto/TimeSyncMessageService.h"
#include "gp/proto/TransportTransaction.h"

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

bool Proto::canAskTransactions() const {
    return (protoState & (ProtoStateAwaitingAuthorization | ProtoStateAwaitingTimeFixAndSalts | ProtoStateStopped)) == 0;
}

bool Proto::canAskServiceTransactions() const {
    return (protoState & (ProtoStateAwaitingAuthorization | ProtoStateStopped)) == 0;
}

bool Proto::timeFixAndSaltMissing() const {
    return (protoState & ProtoStateAwaitingTimeFixAndSalts) != 0;
}

void Proto::setState(uint32_t state) {
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
        if (strongSelf->isStopped())
            return;

        for (auto service : strongSelf->messageServices)
            service.second->protoAllTransactionsMayHaveFailed(strongSelf);
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
            decryptedData = strongSelf->decryptIncomingTransportData(data);

        if (decryptedData)
        {

            int64_t dataMessageId = 0;
            bool parseError = false;

            auto parsedMessages = parseIncomingMessages(decryptedData, dataMessageId, parseError);

            decodeResult(!parseError);

            if (parseError) {
                LOGE("[Proto transportHasIncomingData] -> parse error");
                return;
            }

            for (auto incomingMessage : parsedMessages)
                processIncomingMessage(incomingMessage);
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

std::vector<std::shared_ptr<IncomingMessage>> Proto::parseIncomingMessages(std::shared_ptr<StreamSlice> data,
                                                                           int64_t &dataMessageId, bool &parseError) {
    InputStream is = InputStream(std::move(data));
    try {

        int64_t embeddedMessageId = 0;
        int32_t embeddedSeqNo = 0;
        int64_t embeddedSalt = 0;
        int32_t topMessageSize = 0;

        if (useUnauthorizedMode)
        {
            int64_t authKeyId = is.readInt64();
            if (authKeyId != 0)
            {
                parseError = true;
                return {};
            }
            embeddedMessageId = is.readInt64();

            topMessageSize = is.readInt32();
            if (topMessageSize < 4) {

                parseError = true;
                return {};
            }

            dataMessageId = embeddedMessageId;
        }
        else
        {
            embeddedSalt = is.readInt64();

            int64_t embeddedSessionId = is.readInt64();

            if (embeddedSessionId != sessionInfo->id)
            {
                parseError = true;
                return {};
            }

            embeddedMessageId = is.readInt64();

            embeddedSeqNo = is.readInt32();

            topMessageSize = is.readInt32();

        }

        auto topMessageData = is.readRemainingData();
        auto topMessage = parseMessage(topMessageData);

        if (topMessage == nullptr)
        {
            parseError = true;
            return {};
        }

#warning check message id

        std::vector<std::shared_ptr<IncomingMessage>> messages;
        auto timestamp = (int32_t)(embeddedMessageId / 4294967296);

        if (auto containerMessage = std::dynamic_pointer_cast<Container>(topMessage))
        {
            for (const auto& subMessage : containerMessage->messages)
            {
                if (auto subObject = parseMessage(subMessage->body))
                {
                    auto subMessageId = subMessage->messageId;
                    auto subMessageSeqNo = subMessage->seqNo;
                    auto subMessageLength = subMessage->body->size;

                    messages.push_back(std::make_shared<IncomingMessage>(subMessageId, subMessageSeqNo, timestamp, subMessageLength, subObject));
                }
            }
        }
        else {
            messages.push_back(std::make_shared<IncomingMessage>(embeddedMessageId, embeddedSeqNo, timestamp, topMessageSize, topMessage));
        }

        return messages;
    }
    catch (const InputStreamException& e) {
        LOGE("[Proto parseIncomingMessages] -> parse error %s", e.message.c_str());
        return {};
    }
}

std::shared_ptr<ProtoInternalMessage> Proto::parseMessage(const std::shared_ptr<StreamSlice>& data) {
    auto unwrappedData = InternalParser::unwrapMessage(data);
    auto internalMessage = InternalParser::parseMessage(unwrappedData);
    return internalMessage;
}

void Proto::processIncomingMessage(const std::shared_ptr<IncomingMessage> &message) {
    LOGV("[Proto processIncomingMessage] -> processing incoming message with id: %lld", message->messsageId);

    if (sessionInfo->messageIdProcessed(message->messsageId))
    {
        LOGV("[Proto processIncomingMessage] -> duplicate message received %lld", message->messsageId);
        forceAcks = true;
        sessionInfo->scheduleMessageConfirmation(message->messsageId);

        requestTransportTransactions();
        return;
    }

    sessionInfo->setMessageProcessed(message->messsageId);

    auto strongSelf = shared_from_this();

    if (auto badMessageNotificationMessage = std::dynamic_pointer_cast<BadMsgNotificationMessage>(message->body))
    {
        if (useUnauthorizedMode)
            return;

        auto badMessageId = badMessageNotificationMessage->badMessageId;
        auto containerMessageIds = sessionInfo->messageIdsInContainer(badMessageId);

        if (auto badSaltNotificationMessage = std::dynamic_pointer_cast<BadServerSaltNotificationMessage>(badMessageNotificationMessage))
        {
            LOGE("[Proto processIncomingMessage] -> badSaltMessageNotification received %lld", message->messsageId);

            if (timeFixContext && badSaltNotificationMessage->badMessageId == timeFixContext->messageId)
            {
                auto validSalt = badSaltNotificationMessage->validServerSalt;
                auto timeDifference = (double)(message->messsageId / 4294967296) - getAbsoluteSystemTime();

                setState(protoState & ~ProtoStateAwaitingTimeFixAndSalts);

                std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltSet = {};
                saltSet.push_back(std::make_shared<DatacenterSaltsetInfo>(validSalt, message->messsageId, message->messsageId + (int64_t)(4294967296.0 * 0.9 * 60.0)));

                timeSyncInfoChanged(timeDifference, saltSet, true);

                timeFixContext = nullptr;
                resetSessionInfo();
            }
            else
                initiateTimeSync();

        }
        else {
            switch (badMessageNotificationMessage->errorCode) {
                case 16:
                case 17: {
                    LOGE("[Proto processIncomingMessage] -> initiating timeSync due to bad message %d", badMessageNotificationMessage->errorCode);
                    initiateTimeSync();
                }
                case 32:
                case 33: {
                    LOGE("[Proto processIncomingMessage] -> initiating timeSync due to bad message %d", badMessageNotificationMessage->errorCode);
                    resetSessionInfo();
                    initiateTimeSync();
                }
                case 48: {
                    LOGE("[Proto processIncomingMessage] -> initiating timeSync due to bad message %d", badMessageNotificationMessage->errorCode);
                    initiateTimeSync();
                }
                default: {
                    LOGE("[Proto processIncomingMessage] -> received BadMessageNotification code=%d, messageId=%lld", badMessageNotificationMessage->errorCode, badMessageNotificationMessage->badMessageId);
                    resetSessionInfo();
                }
            }
        }

        for (auto it : messageServices)
        {
            auto service = it.second;
            service->protoMessageDeliveryFailed(strongSelf, badMessageNotificationMessage->badMessageId);

            for (auto msgId : containerMessageIds)
                service->protoMessageDeliveryFailed(strongSelf, msgId);
        }
    }
    else if (auto msgsAckMessage = std::dynamic_pointer_cast<MsgsAckMessage>(message->body)) {
        auto messageIds = msgsAckMessage->messageIds;

        for (auto it : messageServices)
        {
            auto service = it.second;
            service->protoMessagesDeliveryConfirmed(strongSelf, messageIds);
        }
    }
    else {
        if (!useUnauthorizedMode && message->seqNo % 2 != 0)
        {
            sessionInfo->scheduleMessageConfirmation(message->messsageId);

            if (sessionInfo->scheduledMessageConfirmationsExceedThreashold(MaxUnacknowlegedMessagesCount))
                requestTransportTransactions();

            for (auto it : messageServices)
            {
                auto service = it.second;
                service->protoDidReceiveMessage(strongSelf, message);
            }
        }
    }
}

void Proto::requestTransportTransactions() {
    Proto::queue()->async([strongSelf = shared_from_this()] {

        if (strongSelf->willRequestTransactionOnNextQueuePass)
            return;

        strongSelf->willRequestTransactionOnNextQueuePass = true;
        Proto::queue()->asyncForce([strongSelf] {
            strongSelf->willRequestTransactionOnNextQueuePass = false;

            if (!strongSelf->isStopped() && !strongSelf->isPaused())
            {
                if (strongSelf->transport == nullptr)
                    strongSelf->resetTransport();

                strongSelf->transport->setDelegateNeedsTransaction();
            }
        });
    });
}


void Proto::initiateTimeSync() {
    Proto::queue()->async([strongSelf = shared_from_this()] {
        strongSelf->setState(strongSelf->protoState | ProtoStateAwaitingTimeFixAndSalts);
        strongSelf->requestTimeResync();
    });
}

void Proto::completeTimeSync() {
    Proto::queue()->async([strongSelf = shared_from_this()] {
        strongSelf->setState(strongSelf->protoState & (~ProtoStateAwaitingTimeFixAndSalts));

        for (auto service : strongSelf->messageServices)
        {
            if (auto timeSyncService = std::dynamic_pointer_cast<TimeSyncMessageService>(service.second))
            {
                timeSyncService->setDelegate(nullptr);
                strongSelf->removeMessageService(timeSyncService);
            }
        }
    });
}

void Proto::requestTimeResync() {
    Proto::queue()->async([strongSelf = shared_from_this()] {
        bool alreadySyncing = false;

        for (auto& service : strongSelf->messageServices) {
            if (std::dynamic_pointer_cast<TimeSyncMessageService>(service.second)) {
                alreadySyncing = true;
                break;
            }
        }

        if (!alreadySyncing) {
            LOGV("[Proto requestTimeResync] -> begin time sync");
            auto timeSyncService = std::make_shared<TimeSyncMessageService>();
            timeSyncService->setDelegate(strongSelf);

            strongSelf->addMessageService(timeSyncService);
        }
    });
}

void Proto::resetSessionInfo() {
    Proto::queue()->async([strongSelf = shared_from_this()] {
        if (strongSelf->resetSessionInfoLock != 0)
        {
            if (getAbsoluteSystemTime() > strongSelf->resetSessionInfoLock + 1)
                strongSelf->resetSessionInfoLock = 0;
            else { return; }
        }
        else {
            strongSelf->resetSessionInfoLock = getAbsoluteSystemTime();
        }

        strongSelf->sessionInfo = std::make_unique<Session>(strongSelf->context);

        for (auto service : strongSelf->messageServices)
            service.second->protoDidChangeSession(strongSelf);

        strongSelf->resetTransport();
    });
}

void Proto::addMessageService(const std::shared_ptr<MessageService>& service) {
    Proto::queue()->async([strongSelf = shared_from_this(), service] {
        auto it = strongSelf->messageServices.find(service->internalId);

        if (it == strongSelf->messageServices.end()) {
            service->protoWillAddService(strongSelf);
            strongSelf->messageServices[service->internalId] = service;
        }
    });
}

void Proto::removeMessageService(const std::shared_ptr<MessageService>& service) {
    Proto::queue()->async([strongSelf = shared_from_this(), service] {
        auto it = strongSelf->messageServices.find(service->internalId);

        if (it != strongSelf->messageServices.end()) {
            service->protoWillRemoveService(strongSelf);
            strongSelf->messageServices.erase(it);
        }
    });
}

void Proto::transportNetworkAvailabilityChanged(const Transport &transport, bool networkIsAvailable) {
    Proto::queue()->async([self = shared_from_this(), networkIsAvailable] {
        for (auto service : self->messageServices)
            service.second->protoNetworkAvailabilityChanged(self, networkIsAvailable);

        if (auto strongDelegate = self->delegate.lock())
            strongDelegate->connectionStateAvailibilityChanged(*self, networkIsAvailable);
    });
}

void Proto::transportNetworkConnectionStateChanged(const Transport &transport, bool networkIsConnected) {
    Proto::queue()->async([self = shared_from_this(), networkIsConnected] {
        for (auto service : self->messageServices)
            service.second->protoConnectionStateChanged(self, networkIsConnected);
    });
}

void Proto::timeSyncInfoChanged(double timeDifference, const std::vector<std::shared_ptr<DatacenterSaltsetInfo>> &saltlist,
                           bool replace) {
    context->setGlobalTimeDifference(timeDifference);

    if (!saltlist.empty())
    {
        auto updatedAuthInfo = replace ? authInfo->replaceSaltset(saltlist) : authInfo->mergeSaltset(saltlist, context->getGlobalTime());
        authInfo = updatedAuthInfo;

        if (canAskTransactions() || canAskServiceTransactions())
            requestTransportTransactions();
    }
}

void Proto::timeSyncServiceCompleted(const TimeSyncMessageService &service, double timeDifference,
                                     std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltlist) {
    Proto::queue()->async([self = shared_from_this(), &service, timeDifference, saltlist = std::move(saltlist)] {
        auto it = self->messageServices.find(service.internalId);

        if (it == self->messageServices.end())
            return;

        self->completeTimeSync();

        self->messageServices.erase(it);

        self->timeSyncInfoChanged(timeDifference, saltlist, false);
    });
}

void Proto::transportReadyForTransaction(const Transport &transport,
                                         std::shared_ptr<MessageTransaction> transportSpecificTransaction,
                                         std::function<void(
                                                 std::vector<TransportTransaction>)> transactionsReady) {

}
