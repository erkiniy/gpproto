//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#include "gp/network/TcpTransport.h"
#include "gp/proto/MessageTransaction.h"
#include "gp/network/OutgoingMessage.h"
#include "gp/proto/TransportTransaction.h"
#include "gp/utils/Common.h"
#include "gp/utils/OutputStream.h"
#include "gp/utils/Random.h"
#include "gp/proto/PreparedMessage.h"
#include "gp/proto/Context.h"
#include "gp/network/TransportDelegate.h"
#include "gp/proto/DatacenterAddress.h"
#include "gp/net/TcpConnection.h"
#include "gp/utils/Timer.h"
#include "gp/network/TcpTransportContext.h"
#include "gp/proto/ProtoInternalMessage.h"
#include "gp/network/IncomingMessage.h"

#include <cassert>

using namespace gpproto;

void TcpTransport::reset() {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        strongSelf->transportContext->connection->stop();
    });
}

void TcpTransport::initialize() {
    auto self = shared_from_this();
    TcpTransport::queue()->async([self] {
        self->transportContext->setDelegate(self);
    });
}

void TcpTransport::stop() {
    auto self = shared_from_this();
    TcpTransport::queue()->async([self] {

        if (!self->transportContext->connection)
            return;

        LOGV("[TcpTransport stop]");

        //TODO: report to delegate about the failure of transaction

        self->transportContext->stopped = false;
        self->transportContext->connected = false;
        self->transportContext->needsReconnection = false;

        if (auto delegate = self->delegate.lock())
            delegate->transportNetworkConnectionStateChanged(*self, false);

        self->transportContext->connection->setDelegate(nullptr);
        self->transportContext->connection->stop();
        self->transportContext->connection = nullptr;

        self->stopActualizationPingResendTimer();
    });
}

TcpTransport::~TcpTransport() {
    //TODO: stop connection and invalidate timers
    auto transportContext = this->transportContext;

    TcpTransport::queue()->async([transportContext] {
        if (auto connection = transportContext->connection)
        {
            transportContext->needsReconnection = false;
            connection->setDelegate(nullptr);
        }
    });
}

void TcpTransport::setDelegate(std::shared_ptr<TransportDelegate> delegate) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([delegate, strongSelf] {
        strongSelf->delegate = delegate;
    });
}

void TcpTransport::setDelegateNeedsTransaction() {
    auto self = shared_from_this();

    TcpTransport::queue()->async([self] {
        if (self->transportContext->willRequestTransactionOnNextQueuePass)
            return;

        self->transportContext->willRequestTransactionOnNextQueuePass = true;

        TcpTransport::queue()->asyncForce([self] {

            self->transportContext->willRequestTransactionOnNextQueuePass = false;

            if (self->transportContext->connection == nullptr)
                self->transportContext->requestConnection();
            else
                self->requestTransactionFromDelegate();
        });
    });
}

void TcpTransport::requestTransactionFromDelegate() {

    if (transportContext == nullptr)
        return;

    auto self = shared_from_this();

    if (transportContext->waitingForConnectionToBecomeAvailable)
    {
        if (!transportContext->didSendActualizationPingAfterConnection)
        {
            LOGV("[TcpTransport unlocking transaction processing due to connection context update task]");

            transportContext->waitingForConnectionToBecomeAvailable = true;
            transportContext->transactionLockTime = 0.0;
        }
        else if (getAbsoluteSystemTime() > transportContext->transactionLockTime + 1.0)
        {
            LOGV("[TcpTransport unlocking transaction processing due to timeout]");

            transportContext->waitingForConnectionToBecomeAvailable = true;
            transportContext->transactionLockTime = 0.0;
        }
        else
        {
            LOGV("[TcpTransport locking transaction processing]");

            transportContext->requestTransactionWhenBecomesAvailable = true;
            return;
        }
    }

    if (auto strongDelegate = delegate.lock())
    {
        LOGV("[TcpTransport -> requestTransactionFromDelegate]");

        transportContext->waitingForConnectionToBecomeAvailable = true;
        transportContext->transactionLockTime = getAbsoluteSystemTime();

        std::shared_ptr<MessageTransaction> transportSpecificTransactions;
        std::vector<std::shared_ptr<OutgoingMessage>> transportTransactionOutgoingMessages;

        std::shared_ptr<OutgoingMessage> outgoingPingMessage;
        std::shared_ptr<OutgoingMessage> outgoingPongMessage;

        if (!transportContext->didSendActualizationPingAfterConnection)
        {
            transportContext->didSendActualizationPingAfterConnection = true;
            auto randomId = Random::secureInt64();

            OutputStream pingBuffer;
            pingBuffer.writeUInt32(0x7abe77ec);
            pingBuffer.writeInt64(randomId);

            outgoingPingMessage = std::make_shared<OutgoingMessage>(0, 0, false, pingBuffer.currentBytes());

            transportTransactionOutgoingMessages.push_back(outgoingPingMessage);
        }

        if (transportContext->currentServerPingId != 0 && transportContext->currentServerPingMessageId != 0)
        {
            LOGV("[TcpTransport sending pong for pingId:%lld, messageId:%lld]", transportContext->currentServerPingId, transportContext->currentServerPingMessageId);

            OutputStream pongBuffer;
            pongBuffer.writeUInt32(0x347773c5);
            pongBuffer.writeInt64(transportContext->currentServerPingMessageId);
            pongBuffer.writeInt64(transportContext->currentServerPingId);

            outgoingPongMessage = std::make_shared<OutgoingMessage>(0, 0, false, pongBuffer.currentBytes());

            transportTransactionOutgoingMessages.push_back(outgoingPongMessage);
        }

        if (!transportTransactionOutgoingMessages.empty())
        {
            transportSpecificTransactions = std::make_shared<MessageTransaction>(transportTransactionOutgoingMessages,
                    [](auto messageInternalIdToPreparedMessage) {},
                    []{}, [context = transportContext, outgoingPingMessage, outgoingPongMessage](auto messageInternalIdToTransactionId, std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {
                if (const auto & pingMessage = outgoingPingMessage)
                {
                    auto it = messageInternalIdToPreparedMessage.find(pingMessage->internalId);
                    if (it != messageInternalIdToPreparedMessage.end())
                        context->currentActualizationPingMessageId = messageInternalIdToPreparedMessage[pingMessage->internalId]->messageId;
                }

                if (const auto & pongMessage = outgoingPongMessage)
                {
                    auto it = messageInternalIdToPreparedMessage.find(pongMessage->internalId);
                    if (it != messageInternalIdToPreparedMessage.end()) {
                        context->currentServerPingMessageId = 0;
                        context->currentServerPingId = 0;
                    }
                }
            });
        }

        strongDelegate->transportReadyForTransaction(*self, transportSpecificTransactions, [weakSelf = weak_from_this()](std::vector<std::shared_ptr<TransportTransaction>> readyTransactions){
            if (auto strongSelf = weakSelf.lock())
            {
                for (const auto & transaction : readyTransactions)
                {
                    if (transaction->data->size != 0)
                    {
                        if (strongSelf->transportContext != nullptr && strongSelf->transportContext->connection != nullptr) {
                            strongSelf->transportContext->connection->sendDatas({transaction->data});
                            transaction->completion(true, strongSelf->internalId);
                        }
                        else {
                            transaction->completion(false, strongSelf->internalId);
                        }
                    }
                }
            }
        });

        TcpTransport::queue()->async([self = shared_from_this()] {
            self->transportContext->waitingForConnectionToBecomeAvailable = false;
            if (self->transportContext->requestTransactionWhenBecomesAvailable)
            {
                self->transportContext->requestTransactionWhenBecomesAvailable = false;
                self->requestTransactionFromDelegate();
            }
        });
    }
}

//MARK: Delegate methods
void TcpTransport::connectionOpened(const Connection &connection) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([&connection, strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        if (!strongSelf->transportContext->connection->isEqual(connection))
            return;

        strongSelf->transportContext->connected = true;

        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->transportNetworkConnectionStateChanged(*strongSelf, true);

        strongSelf->transportContext->didSendActualizationPingAfterConnection = false;
        strongSelf->transportContext->currentActualizationPingMessageId = 0;

        strongSelf->requestTransactionFromDelegate();
    });
}

void TcpTransport::connectionClosed(const Connection &connection) {
    auto strongSelf = shared_from_this();

    LOGV("[TcpTransport connectionClosed]");

    TcpTransport::queue()->async([&connection, strongSelf] {
        if (!strongSelf->transportContext->connection)
            return;

        if (!strongSelf->transportContext->connection->isEqual(connection))
            return;

        strongSelf->transportContext->connected = false;
        strongSelf->transportContext->connection->setDelegate(nullptr);
        strongSelf->transportContext->connection = nullptr;

        strongSelf->transportContext->connectionClosed();

        strongSelf->transportContext->didSendActualizationPingAfterConnection = false;
        strongSelf->transportContext->currentActualizationPingMessageId = 0;

        strongSelf->transportContext->currentServerPingId = 0;
        strongSelf->transportContext->currentServerPingMessageId = 0;

        if (auto strongDelegate = strongSelf->delegate.lock()) {
            strongDelegate->transportNetworkConnectionStateChanged(*strongSelf, false);
            strongDelegate->transportTransactionsMayHaveFailed(*strongSelf, strongSelf->internalId);
        }
    });
}

void TcpTransport::connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) {
    auto self = shared_from_this();
    TcpTransport::queue()->async([&connection, slice, self] {

        LOGD("[TcpTransport connectionDidReceiveData] -> bytes = %zu", slice->size);

        if (!self->transportContext->connection)
            return;

        if (!self->transportContext->connection->isEqual(connection))
            return;

        if (self->transportContext->currentActualizationPingMessageId != 0 && self->transportContext->actualizationPingResendTimer == nullptr)
            self->startActualizationPingResendTimer();

        if (auto strongDelegate = self->delegate.lock())
        {
            strongDelegate->transportHasIncomingData(*self, slice, true, [weakSelf = self->weak_from_this()](bool success) {
                 if (auto _self = weakSelf.lock()) {
                     if (success)
                         _self->connectionIsValid();
                     else
                         _self->connectionIsInvalid();
                 }
             });
        }
    });
}

void TcpTransport::tcpConnectionRequestReconnection(const TcpTransportContext &context) {
    auto self = shared_from_this();
    TcpTransport::queue()->async([&context, self] {

        LOGV("[TcpTransport tcpConnectionRequestReconnection]");

        if (self->transportContext->stopped)
            return;

        self->startIfNeeded();
    });
}

void TcpTransport::startIfNeeded() {
    LOGV("[TcpTransport startIfNeeded]");
    TcpTransport::queue()->async([self = shared_from_this()] {
        //LOGV("[TcpTransport startIfNeeded] connection is %s", self->transportContext->connection == nullptr ? "nil" : "not nil");
        if (self->transportContext->connection)
            return;

        //TODO: start connection watchdog timer

        auto context = self->context;
        auto address = IPv4Address(self->address->ip);

        self->transportContext->connection = std::make_shared<TcpConnection>(address, self->address->port);

        self->transportContext->connection->setDelegate(self);

        self->transportContext->connection->start();
    });
}


void TcpTransport::updateConnectionState() {
    TcpTransport::queue()->async([self = shared_from_this()] {

        if (self->transportContext == nullptr)
            return;

        if (auto strongDelegate = self->delegate.lock())
        {
            strongDelegate->transportNetworkAvailabilityChanged(*self, self->transportContext->networkIsAvailable);
            strongDelegate->transportNetworkConnectionStateChanged(*self, self->transportContext->connected);
        }
    });
}

void TcpTransport::connectionIsValid() {
    TcpTransport::queue()->async([strongSelf = shared_from_this()] {
        strongSelf->transportContext->connectionIsValid = true;
        strongSelf->transportContext->connectionValidDataReceived();
    });

}

void TcpTransport::connectionIsInvalid() {
    TcpTransport::queue()->async([strongSelf = shared_from_this()] {
        //TODO: invalidate current transport scheme
    });
}

void TcpTransport::startActualizationPingResendTimer() {
    TcpTransport::queue()->async([self = shared_from_this()] {
        auto context = self->transportContext;
        if (auto timer = context->actualizationPingResendTimer)
            timer->invalidate();

        LOGV("[TcpTransport startActualizationPingResendTimer] setting timer ping message id %lld", context->currentActualizationPingMessageId);
        context->actualizationPingResendTimer = Timer::make_timer(20.0, true, [weak_self = self->weak_from_this()] {
            if (auto strongSelf = weak_self.lock())
                strongSelf->resendActualizationPing();
            else {
                LOGV("[TcpTransport is weak]");
            }

        }, TcpTransport::queue());
        context->actualizationPingResendTimer->start();
    });
}

void TcpTransport::stopActualizationPingResendTimer() {
    TcpTransport::queue()->async([self = shared_from_this()] {
        auto context = self->transportContext;

        if (auto timer = context->actualizationPingResendTimer)
            timer->invalidate();

        LOGV("[TcpTransport stopActualizationPingResendTimer]");

        context->actualizationPingResendTimer = nullptr;
    });
}

void TcpTransport::resendActualizationPing() {
    TcpTransport::queue()->async([self = shared_from_this()] {
        auto context = self->transportContext;

        if (context->currentActualizationPingMessageId != 0)
        {
            context->didSendActualizationPingAfterConnection = false;
            context->currentActualizationPingMessageId = 0;

            self->requestTransactionFromDelegate();
        }
    });
}

std::shared_ptr<MessageTransaction> TcpTransport::protoMessageTransaction(const std::shared_ptr<Proto> &proto) {
    return nullptr;
}

void TcpTransport::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto, std::shared_ptr<IncomingMessage> message) {
    if (auto pingMessage = std::dynamic_pointer_cast<PingMessage>(message->body))
    {
        TcpTransport::queue()->async([pingMessage, message, self = shared_from_this()] {
            LOGV("[TcpTransport PING received]");
            self->transportContext->currentServerPingMessageId = message->messsageId;
            self->transportContext->currentServerPingId = pingMessage->pingId;

            self->requestTransactionFromDelegate();
        });
    }
}

void TcpTransport::protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto, std::vector<int> transactionIds) {

}

void TcpTransport::protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) {
    TcpTransport::queue()->async([self = shared_from_this(), messageId] {
        auto transportContext = self->transportContext;
        if (transportContext->currentActualizationPingMessageId != 0 && transportContext->currentActualizationPingMessageId == messageId)
        {
            self->stopActualizationPingResendTimer();
            transportContext->currentActualizationPingMessageId = 0;
        }
    });
}

void TcpTransport::protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto, std::vector<int64_t> messages) {

}

void TcpTransport::protoErrorReceived(const std::shared_ptr<Proto> &proto, int32_t errorCode) {

}

void TcpTransport::protoWillAddService(const std::shared_ptr<Proto> &proto) {

}

void TcpTransport::protoDidAddService(const std::shared_ptr<Proto> &proto) {

}

void TcpTransport::protoWillRemoveService(const std::shared_ptr<Proto> &proto) {

}

void TcpTransport::protoDidRemoveService(const std::shared_ptr<Proto> &proto) {

}

void TcpTransport::protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) {

}

void TcpTransport::protoDidChangeSession(const std::shared_ptr<Proto> &proto) {
    TcpTransport::queue()->async([self = shared_from_this()] {
        self->stopActualizationPingResendTimer();
        self->transportContext->currentActualizationPingMessageId = 0;
    });
}

void TcpTransport::protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) {

}

void TcpTransport::protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) {

}

void TcpTransport::protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) {

}

void TcpTransport::protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) {

}
