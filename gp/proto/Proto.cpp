//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#include "gp/proto/Proto.h"

#include "gp/utils/Logging.h"
#include "gp/utils/Crypto.h"
#include "gp/utils/OutputStream.h"
#include "gp/utils/InputStream.h"
#include "gp/network/TcpTransport.h"
#include "gp/network/TransportScheme.h"
#include "gp/network/IncomingMessage.h"
#include "gp/network/OutgoingMessage.h"

#include "gp/proto/MessageEncryptionKey.h"
#include "gp/proto/InternalParser.h"
#include "gp/proto/TimeFixContext.h"
#include "gp/proto/MessageService.h"
#include "gp/proto/TimeSyncMessageService.h"
#include "gp/proto/TransportTransaction.h"
#include "gp/proto/PreparedMessage.h"
#include "gp/proto/MessageTransaction.h"
#include "gp/proto/DatacenterSaltsetInfo.h"

#include <unordered_map>

using namespace gpproto;

void Proto::initialize() {
    auto self = shared_from_this();
    context->addChangeListener(self);
}

void Proto::setDelegate(std::shared_ptr<ProtoDelegate> delegate) {
    Proto::queue()->async([self = shared_from_this(), delegate] {
        self->delegate = delegate;
    });
}

void Proto::pause() {
    Proto::queue()->async([self = shared_from_this()] {
        LOGV("Pausing proto with old state %d", self->protoState);
        if ((self->protoState & ProtoStatePaused) == 0)
        {
            self->setState(self->protoState | ProtoStatePaused);
            LOGV("Proto paused");

            self->setTransport(nullptr);
        }
    });
}

void Proto::resume() {
    Proto::queue()->async([self = shared_from_this()] {

        if (self->protoState & ProtoStatePaused)
        {
            self->setState(self->protoState & (~ProtoStatePaused));
            LOGV("Proto resumed");

            self->resetTransport();
            self->requestTransportTransactions();
        }
    });
}

void Proto::stop() {
    Proto::queue()->async([self = shared_from_this()] {
        if ((self->protoState & ProtoStateStopped) == 0)
        {
            self->setState(self->protoState | ProtoStateStopped);
            LOGV("Proto stopped");

            if (self->transport)
            {
                self->transport->setDelegate(nullptr);
                self->transport->stop();
                self->setTransport(nullptr);
            }
        }
    });
}

void Proto::contextDatacenterTransportSchemeUpdated(const Context &context, int32_t datacenterId,
                                                    std::shared_ptr<TransportScheme> scheme) {
    Proto::queue()->async([self = shared_from_this(), datacenterId, scheme] {
        if (datacenterId == self->datacenterId && !self->isStopped() && self->transportScheme == nullptr)
        {
            self->transportScheme = scheme;
            if (self->protoState & ProtoStateAwaitingTransportScheme)
                self->setState(self->protoState & (~ProtoStateAwaitingTransportScheme));

            self->resetTransport();
            self->requestTransportTransactions();
        }
    });
}

void Proto::contextDatacenterAuthInfoUpdated(const Context &context, int32_t datacenterId,
                                             std::shared_ptr<AuthKeyInfo> authInfo) {
    Proto::queue()->async([self = shared_from_this(), datacenterId, authInfo] {
        if (!self->useUnauthorizedMode && datacenterId == self->datacenterId && authInfo != nullptr)
        {
            self->authInfo = authInfo;
            if (self->protoState & ProtoStateAwaitingAuthorization)
            {
                self->setState(self->protoState & (~ProtoStateAwaitingAuthorization));
                self->resetTransport();
                self->requestTransportTransactions();
            }
        }
    });
}

void Proto::contextDatacenterAddressSetUpdated(const gpproto::Context &context, int32_t datacenterId,
                                               std::vector<std::shared_ptr<DatacenterAddress>> addressSet) {}

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
    Proto::queue()->async([self = shared_from_this(), transport] {
        LOGV("[Proto setTransport] -> changing transport from %s to %s", (self->transport == nullptr ? "nullptr" : "tcp_transport"), (transport == nullptr ? "nullptr" : "tcp_transport"));

        self->allTransactionsMayHaveFailed();

        auto previousTransport = self->transport;

        if (previousTransport)
            self->removeMessageService(previousTransport);

        self->transport = transport;

        if (previousTransport)
            previousTransport->stop();

        self->updateConnectionState();

        if (transport)
            self->addMessageService(transport);
    });
}

void Proto::resetTransport() {
    Proto::queue()->async([self = shared_from_this()] {
        if (self->protoState & ProtoStateStopped)
            return;

        LOGV("[Proto resetTransport] -> unauthorized %d", self->useUnauthorizedMode);

        if (auto _transport = self->transport)
        {
            _transport->setDelegate(nullptr);
            _transport->stop();

            self->setTransport(nullptr);
        }

        self->transportScheme = self->context->transportSchemeForDatacenterId(self->datacenterId);
        LOGV("[Proto resetTransport] -> scheme returned %d, unauthorized=%d", self->transportScheme != nullptr, self->useUnauthorizedMode);

        if (self->transportScheme == nullptr)
        {
            if ((self->protoState & ProtoStateAwaitingTransportScheme) == 0)
            {
                LOGV("[Proto resetTransport] -> awaitingTransportScheme");

                self->setState(self->protoState | ProtoStateAwaitingTransportScheme);
                self->context->transportSchemeForDatacenterIdRequired(self->datacenterId);
            }
        }
        else if (!self->useUnauthorizedMode && self->context->getAuthKeyInfoForDatacenterId(self->datacenterId) == nullptr)
        {
            if ((self->protoState & ProtoStateAwaitingAuthorization) == 0)
            {
                LOGV("[Proto resetTransport] -> missing authorized key for datacenterId = %d", self->datacenterId);

                self->setState(self->protoState | ProtoStateAwaitingAuthorization);
                self->context->authInfoForDatacenterWithIdRequired(self->datacenterId);
            }
        }
        else {
            LOGV("[Proto resetTransport] -> setting created transport with address %s", self->transportScheme->address->ip.c_str());
            auto transport = self->transportScheme->createTransportWithContext(self->context, self->datacenterId, self);
            self->setTransport(transport);
        }

    });
}

void Proto::allTransactionsMayHaveFailed() {
    Proto::queue()->async([self = shared_from_this()] {
        if (self->isStopped())
            return;

        for (const auto & service : self->messageServices)
            service.second->protoAllTransactionsMayHaveFailed(self);
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
    Proto::queue()->async([&transport, strongSelf = shared_from_this(), data, requestTransactionAfterProcessing, decodeResult] {

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

            auto parsedMessages = strongSelf->parseIncomingMessages(decryptedData, dataMessageId, parseError);

            decodeResult(!parseError);

            if (parseError) {
                LOGE("[Proto transportHasIncomingData] -> parse error");
                return;
            }

            for (const auto & incomingMessage : parsedMessages)
                strongSelf->processIncomingMessage(incomingMessage);
        }
    });
}

std::shared_ptr<StreamSlice> Proto::decryptIncomingTransportData(const std::shared_ptr<StreamSlice> &data) const {

    if (data->size < 24 + 36) {
        LOGE("[Proto decryptIncomingTransportData] -> received less data than 24 + 36 bytes");
        return nullptr;
    }

    if (authInfo == nullptr) {
        LOGE("[Proto decryptIncomingTransportData] -> missing authKey");
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
    LOGV("[Proto processIncomingMessage] -> processing incoming message with id: %lld, length: %zu, class = %s", message->messsageId, message->length,
         typeid(*message->body).name());

    if (sessionInfo->messageIdProcessed(message->messsageId))
    {
        LOGV("[Proto processIncomingMessage] -> duplicate message received %lld", message->messsageId);
        forceAcks = true;
        sessionInfo->scheduleMessageConfirmation(message->messsageId);

        requestTransportTransactions();
        return;
    }

    sessionInfo->setMessageProcessed(message->messsageId);

    auto self = shared_from_this();

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
                auto timeDifference = (message->messsageId / 4294967296.0) - getAbsoluteSystemTime();

                setState(protoState & ~ProtoStateAwaitingTimeFixAndSalts);

                std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltSet;
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
            service->protoMessageDeliveryFailed(self, badMessageNotificationMessage->badMessageId);

            for (auto msgId : containerMessageIds)
                service->protoMessageDeliveryFailed(self, msgId);
        }
    }
    else if (auto msgsAckMessage = std::dynamic_pointer_cast<MsgsAckMessage>(message->body)) {
        auto messageIds = msgsAckMessage->messageIds;

        for (auto it : messageServices)
        {
            auto service = it.second;
            service->protoMessagesDeliveryConfirmed(self, messageIds);
        }
    }
    else {
        if (!useUnauthorizedMode && message->seqNo % 2 != 0)
        {
            sessionInfo->scheduleMessageConfirmation(message->messsageId);

            if (sessionInfo->scheduledMessageConfirmationsExceedThreashold(MaxUnacknowlegedMessagesCount))
                requestTransportTransactions();

        }

        for (auto it : messageServices)
            it.second->protoDidReceiveMessage(self, message);

    }
}

void Proto::requestTransportTransactions() {
    Proto::queue()->async([self = shared_from_this()] {

        if (self->willRequestTransactionOnNextQueuePass)
            return;

        LOGV("[Proto requestTransportTransactions] ~> willRequestTransactionOnNextQueuePass = %d", self->willRequestTransactionOnNextQueuePass);

        self->willRequestTransactionOnNextQueuePass = true;
        Proto::queue()->asyncForce([self] {
            self->willRequestTransactionOnNextQueuePass = false;

            if (!self->isStopped() && !self->isPaused())
            {
                if (self->transport == nullptr) {//NOTE: not waiting on unauthorized
                    LOGV("[Proto requestTransportTransactions] transport is null and unauthorized = %d", self->useUnauthorizedMode);
                    self->resetTransport();
                    //LOGV("[Proto requestTransportTransactions] transport should be %s if unauthorized=%d", self->transport == nullptr ? "null" : "tcp_transport", self->useUnauthorizedMode);
                }
                LOGV("[Proto requestTransportTransactions] is ProtoQueue = %d", Proto::queue()->isCurrentQueue());
                LOGV("[Proto requestTransportTransactions] after resetting transport = %s, unauthorized=%d", self->transport == nullptr ? "nullptr" : "tcp_transport", self->useUnauthorizedMode);
                if (self->transport)
                    self->transport->setDelegateNeedsTransaction();
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
                LOGV("[Proto completeTimeSync] ~> removed timeSyncService");
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

        if (!alreadySyncing)
        {
            LOGV("[Proto requestTimeResync] -> begin time sync");
            auto timeSyncService = std::make_shared<TimeSyncMessageService>();
            timeSyncService->setDelegate(strongSelf);

            strongSelf->addMessageService(timeSyncService);
        }
        else {
            LOGV("[Proto requestTimeResync] already syncing");
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

void Proto::addMessageService(std::shared_ptr<MessageService> service) {
    Proto::queue()->async([self = shared_from_this(), service] {
        auto it = self->messageServices.find(service->internalId);
        if (it == self->messageServices.end()) {
            service->protoWillAddService(self);
            self->messageServices[service->internalId] = service;
            service->protoDidAddService(self);
        }
    });
}

void Proto::removeMessageService(const std::shared_ptr<MessageService>& service) {
    Proto::queue()->async([strongSelf = shared_from_this(), service] {
        auto it = strongSelf->messageServices.find(service->internalId);

        if (it != strongSelf->messageServices.end())
        {
            service->protoWillRemoveService(strongSelf);

            strongSelf->messageServices.erase(it);

            service->protoDidRemoveService(strongSelf);
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
    LOGV("[Proto transportNetworkConnectionStateChanged] %d", networkIsConnected);

    Proto::queue()->async([self = shared_from_this(), networkIsConnected] {
        for (auto service : self->messageServices)
            service.second->protoConnectionStateChanged(self, networkIsConnected);
    });
}

void Proto::timeSyncInfoChanged(double timeDifference, const std::vector<std::shared_ptr<DatacenterSaltsetInfo>> &saltlist,
                           bool replace) {
    LOGV("[Proto timeSyncInfoChanged] difference = %f, salts count %zu", timeDifference, saltlist.size());

    //std::this_thread::sleep_for(std::chrono::microseconds((long long)(5000000 * 1.0)));

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

        self->timeSyncInfoChanged(timeDifference, saltlist, false);
    });
}

void Proto::transportReadyForTransaction(const Transport &transport,
                                         std::shared_ptr<MessageTransaction> transportSpecificTransaction,
                                         std::function<void(
                                                 std::vector<std::shared_ptr<TransportTransaction>>)> transactionsReady) {

    Proto::queue()->async([self = shared_from_this(), &transport, transportSpecificTransaction, transactionsReady] {
        LOGV("[Proto transportReadyForTransaction] -> when transport is %s", self->transport == nullptr ? "nullptr" : "transport");
        if (self->transport == nullptr || !self->transport->isEqual(transport)) {
            LOGE("[Proto transportReadyForTransaction] -> when transport id is %d", self->transport == nullptr ? -1 : self->transport->internalId);
            LOGE("[Proto transportReadyForTransaction] -> when received transport id is %d", self->transport == nullptr ? -1 : transport.internalId);
            transactionsReady({});
            return;
        }

        LOGV("[Proto transportReadyForTransaction] -> canAskTransactions = %d", self->canAskTransactions());

        if (self->canAskTransactions())
        {
            std::vector<std::shared_ptr<MessageTransaction>> messageTransactions;
            std::vector<std::shared_ptr<TransportTransaction>> transportTransactions;

            if (auto& transaction = transportSpecificTransaction) {
                if (!self->useUnauthorizedMode)
                    messageTransactions.push_back(transaction);
            }

            std::vector<std::shared_ptr<PreparedMessage>> preparedMessages;

            std::vector<std::shared_ptr<StreamSlice>> transactionPayloadList;

            bool hasHighPriorityMessages = false;
            bool transactionExpectsDataInResponse = false;

            LOGV("[Proto transportReadyForTransaction] -> messageServices count = %d", (int)self->messageServices.size());

            for (auto& service : self->messageServices)
            {
                if (auto messageTransaction = service.second->protoMessageTransaction(self))
                    messageTransactions.push_back(messageTransaction);
            }

            if (self->sessionInfo->scheduledMessageConfirmationsExceedThreashold(MaxUnacknowlegedMessagesCount) || self->forceAcks) {
                self->forceAcks = false;

                const auto scheduledMessageConfirmations = self->sessionInfo->getScheduledConfirmationMessageIds();

                LOGV("[Proto transportReadyForTransaction] -> sending message acks");

                auto msgsAckStream = std::make_shared<OutputStream>();
                msgsAckStream->writeUInt32(0x62d6b459);
                msgsAckStream->writeUInt32(0x1cb5c415); //vector
                msgsAckStream->writeInt32(static_cast<int32_t>(scheduledMessageConfirmations.size()));

                for (auto &id : scheduledMessageConfirmations)
                    msgsAckStream->writeInt64(id);

                auto outgoingMessage = std::make_shared<OutgoingMessage>(0, 0, false, msgsAckStream->currentBytes());
                auto messageTransaction = std::make_shared<MessageTransaction>(
                        std::initializer_list<std::shared_ptr<OutgoingMessage>>{outgoingMessage},
                        [](std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {},
                        [] {},
                        [](std::unordered_map<int, int> messageInternalIdToTransactionId, std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {

                        });

                messageTransactions.push_back(std::move(messageTransaction));
            }

            LOGV("[Proto transportReadyForTransaction] -> message transactions count = %d", (int)messageTransactions.size());

            std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage;
            std::unordered_map<int, int> preparedMessageInternalIdToMessageInternalId;

            bool saltSetEmpty = false;
            bool monotonicityViolated = false;

            int64_t messageSalt = 0;
            if (!self->useUnauthorizedMode) {
                messageSalt = self->authInfo->authSaltForClientMessageId(self->sessionInfo->actualClientMessageId());
                saltSetEmpty = messageSalt == 0;
            }

            for (const auto & transaction : messageTransactions)
            {
                for (const auto & outgoingMessage : transaction->payload)
                {
                    if (outgoingMessage->requiresConfirmation)
                        transactionExpectsDataInResponse = true;

                    int64_t messageId = 0;
                    int32_t seqNo = 0;

                    if (outgoingMessage->messsageId == 0)
                    {
                        seqNo = self->sessionInfo->generateClientSeqNo(outgoingMessage->requiresConfirmation);
                        messageId = self->sessionInfo->generateClientMessageId(monotonicityViolated);
                    } else {
                        seqNo = outgoingMessage->seqNo;
                        messageId = outgoingMessage->messsageId;
                    }

                    if (!monotonicityViolated || self->useUnauthorizedMode)
                    {
                        auto preparedMessage = std::make_shared<PreparedMessage>(outgoingMessage->internalId, messageId, seqNo, messageSalt, outgoingMessage->data, outgoingMessage->requiresConfirmation, false);
                        messageInternalIdToPreparedMessage[outgoingMessage->internalId] = preparedMessage;
                        preparedMessageInternalIdToMessageInternalId[preparedMessage->internalId] = outgoingMessage->internalId;

                        preparedMessages.push_back(preparedMessage);
                    }
                }
            }

            for (const auto & transaction : messageTransactions)
                transaction->prepared(messageInternalIdToPreparedMessage);

            if (monotonicityViolated || saltSetEmpty)
            {
                for (const auto & transaction : messageTransactions)
                {
                    messageInternalIdToPreparedMessage.clear();
                    transaction->completed({}, {});
                }
                transactionsReady({});

                if (monotonicityViolated)
                {
                    LOGE("[Proto transportReadyForTransaction] -> client message id monotonicity violated");
                    self->resetSessionInfo();
                    self->requestTransportTransactions();
                }
                else {
                    LOGE("[Proto transportReadyForTransaction] -> initiating time sync due to salt set is empty");
                    self->initiateTimeSync();
                }
                return;
            }

            if (!preparedMessages.empty())
            {
                if (!self->useUnauthorizedMode)
                {
                    if (preparedMessages.size() == 1)
                    {
                        auto preparedMessage = preparedMessages[0];
                        auto messageData = self->dataForEncryptedMessage(preparedMessage, self->sessionInfo);

                        transactionPayloadList.push_back(messageData);
                    }
                    else {
                        std::sort(preparedMessages.begin(), preparedMessages.end(), [](const std::shared_ptr<PreparedMessage> & $0, const std::shared_ptr<PreparedMessage> & $1) -> bool {
                            return $0->messageId < $1->messageId && $0->hasHighPriority && !$1->hasHighPriority;
                        });
                        auto containerData = self->dataForEncryptedContainer(preparedMessages, self->sessionInfo);

                        transactionPayloadList.push_back(containerData);
                    }
                } else {
                    for (const auto & preparedMessage : preparedMessages)
                    {
                        auto messageData = self->dataForPlainMessage(preparedMessage);
                        transactionPayloadList.push_back(messageData);
                    }
                }

                if (!transactionPayloadList.empty())
                {
                    for (const auto & payload : transactionPayloadList)
                    {
                        auto transportTransaction = std::make_shared<TransportTransaction>(payload, [messageTransactions = std::move(messageTransactions), messageInternalIdToPreparedMessage = std::move(messageInternalIdToPreparedMessage)](bool success, int transactionId) mutable {
                            Proto::queue()->async([messageTransactions = std::move(messageTransactions), messageInternalIdToPreparedMessage = std::move(messageInternalIdToPreparedMessage), success, transactionId] {
                                if (success)
                                {

                                    for (const auto & transaction : messageTransactions)
                                    {
                                        std::unordered_map<int, int> messageInternalIdToTransactionId;

                                        for (const auto & outgoingMessage : transaction->payload)
                                            messageInternalIdToTransactionId[outgoingMessage->internalId] = transactionId;

                                        transaction->completed(messageInternalIdToTransactionId, messageInternalIdToPreparedMessage);
                                    }
                                }
                                else
                                {
                                    for (const auto & transaction : messageTransactions)
                                        transaction->failed();
                                }
                            });

                            }, transactionExpectsDataInResponse);

                        transportTransactions.push_back(transportTransaction);
                    }

                    transactionsReady(std::move(transportTransactions));
                }
                else {
                    transactionsReady({});
                    return;
                }
            }
            else {
                transactionsReady({});
                return;
            }
        }
        else if (self->canAskServiceTransactions() && self->timeFixAndSaltMissing() && self->timeFixContext == nullptr) {

            LOGV("[Proto transportReadyForTransaction] -> initiating time sync");
            bool monotonicityViolated = false;

            auto timeFixMessageId = self->sessionInfo->generateClientMessageId(monotonicityViolated);
            auto timeFixSeqNo = self->sessionInfo->generateClientSeqNo(false);

            LOGV("[Proto transportReadyForTransaction] -> Time fix message_id %lld", timeFixMessageId);

            int64_t randomId = Random::secureInt64();

            LOGV("[Proto transportReadyForTransaction] -> Time fix random_id %lld", randomId);

            OutputStream pingBuffer;
            pingBuffer.writeUInt32(0x7abe77ec);
            pingBuffer.writeInt64(randomId);

            auto messageData = pingBuffer.currentBytes();

            OutputStream decryptedOs;
            decryptedOs.writeInt64(0); //salt = 0 to receive bad messageNotification
            decryptedOs.writeInt64(self->sessionInfo->id);
            decryptedOs.writeInt64(timeFixMessageId);
            decryptedOs.writeInt32(timeFixSeqNo);

            decryptedOs.writeInt32((int32_t)messageData->size);
            decryptedOs.writeData(*messageData);

            auto currentBytes = decryptedOs.currentBytes();
            auto messageKey = Crypto::sha256(*currentBytes)->subData(16, 16);

            self->paddedData(decryptedOs);

            auto encryptionKey = MessageEncryptionKey::messageEncryptionKeyForAuthKey(self->authInfo->authKey, messageKey);

            OutputStream encryptedOs;
            encryptedOs.writeInt64(self->authInfo->authKeyId);
            encryptedOs.writeData(*messageKey);

            auto paddedData = decryptedOs.currentBytes();

            auto encryptedData = Crypto::aes_cbc_encrypt(encryptionKey->aes_key, &encryptionKey->aes_iv, *paddedData);
            encryptedOs.writeData(*encryptedData);

            auto transportTransaction = std::make_shared<TransportTransaction>(encryptedOs.currentBytes(), [self, timeFixMessageId, timeFixSeqNo](bool success, int transactionId) {
                Proto::queue()->async([self, timeFixMessageId, timeFixSeqNo, success] {
                    if (success)
                    {
                        LOGV("[Proto transportReadyForTransaction] -> Time fix transaction success with message_id %lld", timeFixMessageId);
                        self->timeFixContext = std::make_shared<TimeFixContext>(timeFixMessageId, timeFixSeqNo, getAbsoluteSystemTime());
                    }
                    else {
                        self->requestTransportTransactions();
                    }
                });

            }, true);

            transactionsReady({transportTransaction});
            return;
        }
        else {
            transactionsReady({});
            return;
        }
    });
}

std::shared_ptr<StreamSlice> Proto::dataForEncryptedMessage(const std::shared_ptr<PreparedMessage> &message,
                                                            const std::shared_ptr<Session> &session) const {
    OutputStream decryptedOs;

    decryptedOs.writeInt64(message->salt);
    decryptedOs.writeInt64(session->id);
    decryptedOs.writeInt64(message->messageId);
    decryptedOs.writeInt32(message->seqNo);

    //TODO: maybe compress with gzip in protocol layer
    decryptedOs.writeInt32(static_cast<int32_t>(message->data->size));
    decryptedOs.writeData(*message->data);

    auto currentBytes = decryptedOs.currentBytes();
    auto messageKey = Crypto::sha256(*currentBytes)->subData(16, 16);

    paddedData(decryptedOs);

    auto paddedData = decryptedOs.currentBytes();

    auto encryptionKey = MessageEncryptionKey::messageEncryptionKeyForAuthKey(authInfo->authKey, messageKey);

    OutputStream encryptedOs;
    encryptedOs.writeInt64(authInfo->authKeyId);
    encryptedOs.writeData(*messageKey);

    auto encryptedData = Crypto::aes_cbc_encrypt(encryptionKey->aes_key, &encryptionKey->aes_iv, *paddedData);
    encryptedOs.writeData(*encryptedData);

    return encryptedOs.currentBytes();
}

std::shared_ptr<StreamSlice> Proto::dataForEncryptedContainer(const std::vector<std::shared_ptr<PreparedMessage>> &messages, const std::shared_ptr<Session> &session) const {
    static const uint32_t containerSignature = 0x73f1f8dc;
    LOGV("Generating container with %d", (int)messages.size());
    OutputStream containerOs;

    containerOs.writeUInt32(containerSignature);
    containerOs.writeInt32((int32_t)messages.size());

    std::vector<int64_t> containerMessageIds;
    containerMessageIds.reserve(messages.size());

    int64_t salt = 0;
    for (const auto & preparedMessage : messages)
    {
        salt = preparedMessage->salt;

        containerOs.writeInt64(preparedMessage->messageId);
        containerOs.writeInt32(preparedMessage->seqNo);
        containerOs.writeInt32(static_cast<int32>(preparedMessage->data->size));
        containerOs.writeData(*preparedMessage->data);

        containerMessageIds.push_back(preparedMessage->messageId);
    }

    auto containerData = containerOs.currentBytes();
    //TODO: maybe compress

    OutputStream decryptedOs;
    bool monotonityViolated = false;

    auto containerMessageId = session->generateClientMessageId(monotonityViolated);

    if (!containerMessageIds.empty())
        session->addContainerMessageIdMapping(containerMessageId, containerMessageIds);

    auto containerSeqNo = session->generateClientSeqNo(false);

    decryptedOs.writeInt64(salt);
    decryptedOs.writeInt64(session->id);
    decryptedOs.writeInt64(containerMessageId);
    decryptedOs.writeInt32(containerSeqNo);
    decryptedOs.writeInt32(static_cast<int32_t>(containerData->size));

    decryptedOs.writeData(*containerData);

    auto currentBytes = decryptedOs.currentBytes();
    auto messageKey = Crypto::sha256(*currentBytes)->subData(16, 16);

    paddedData(decryptedOs);

    auto paddedData = decryptedOs.currentBytes();

    auto encryptionKey = MessageEncryptionKey::messageEncryptionKeyForAuthKey(authInfo->authKey, messageKey);

    OutputStream encryptedOs;
    encryptedOs.writeInt64(authInfo->authKeyId);
    encryptedOs.writeData(*messageKey);

    auto encryptedData = Crypto::aes_cbc_encrypt(encryptionKey->aes_key, &encryptionKey->aes_iv, *paddedData);
    encryptedOs.writeData(*encryptedData);

    return encryptedOs.currentBytes();
}

std::shared_ptr<StreamSlice> Proto::dataForPlainMessage(const std::shared_ptr<PreparedMessage> &message) const {
    OutputStream os;

    os.writeInt64(0);
    os.writeInt64(message->messageId);
    os.writeInt32(static_cast<int32_t>(message->data->size));
    os.writeData(*message->data);

    return os.currentBytes();
}

void Proto::paddedData(OutputStream &os) const {
    unsigned char randomBuffer[16];
    Random::secureBytes(randomBuffer, 16);

    int index = 0;
    while (os.getCurrentSize() % 16 != 0)
    {
        os.writeUInt8(static_cast<uint8_t>(randomBuffer[index]));
        index++;
    }
}
