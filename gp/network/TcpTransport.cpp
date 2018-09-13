//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#include "gp/network/TcpTransport.h"
#include "gp/proto/MessageTransaction.h"
#include "gp/proto/TransportTransaction.h"

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

        //TODO: stop timers;
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
    LOGV("[TcpTransport -> setDelegateNeedsTransaction]");
    TcpTransport::queue()->async([self] {
        if (self->transportContext->connection == nullptr)
            self->transportContext->requestConnection();
        else
            self->requestTransactionFromDelegate();
    });
}

void TcpTransport::requestTransactionFromDelegate() {

    if (transportContext == nullptr)
        return;

    auto self = shared_from_this();

    if (auto strongDelegate = delegate.lock())
    {
        LOGV("[TcpTransport -> requestTransactionFromDelegate]");

        strongDelegate->transportReadyForTransaction(*self, nullptr, [weakSelf = weak_from_this()](std::vector<std::shared_ptr<TransportTransaction>> readyTransactions){
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

        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->transportNetworkConnectionStateChanged(*strongSelf, false);

    });
}

void TcpTransport::connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([&connection, slice, strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        if (!strongSelf->transportContext->connection->isEqual(connection))
            return;

        if (auto strongDelegate = strongSelf->delegate.lock())
        {
            strongDelegate->transportHasIncomingData(*strongSelf, slice, true, [weakSelf = strongSelf->weak_from_this()](bool success) {
                 if (auto _strongSelf = weakSelf.lock()) {
                     if (success)
                         _strongSelf->connectionIsValid();
                     else
                         _strongSelf->connectionIsInvalid();
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
        LOGV("[TcpTransport startIfNeeded] connection is %s", self->transportContext->connection == nullptr ? "nil" : "not nil");
        if (self->transportContext->connection)
            return;

        //TODO: start connection watchdog timer

        auto context = self->context;
        auto address = IPv4Address(self->address->ip);

        self->transportContext->connection = std::make_shared<TcpConnection>(address, self->address->port);

        //auto strongDelegate = std::static_pointer_cast<ConnectionDelegate>(strongSelf);

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