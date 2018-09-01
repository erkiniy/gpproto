//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#include "TcpTransport.h"

using namespace gpproto;

void TcpTransport::reset() {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        strongSelf->transportContext->connection->stop();
    });
}

void TcpTransport::stop() {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        //TODO: report to delegate about the failure of transaction

        strongSelf->transportContext->stopped = false;
        strongSelf->transportContext->connected = false;

        if (auto delegate = strongSelf->delegate.lock())
            delegate->transportNetworkConnectionStateChanged(*strongSelf, false);

        strongSelf->transportContext->connection->setDelegate(nullptr);
        strongSelf->transportContext->connection->stop();
        strongSelf->transportContext->connection = nullptr;

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
    auto strongSelf = shared_from_this();
    LOGV("[TcpTransport -> setDelegateNeedsTransaction]");
    TcpTransport::queue()->async([strongSelf] {
        if (strongSelf->transportContext->connection == nullptr)
            strongSelf->transportContext->requestConnection();
        else
            strongSelf->requestTransactionFromDelegate();
    });
}

void TcpTransport::requestTransactionFromDelegate() {
    LOGV("[TcpTransport -> requestTransactionFromDelegate]");
}

//MARK: Delegate methods
void TcpTransport::connectionOpened(const Connection &connection) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([&, strongSelf] {

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

    TcpTransport::queue()->async([&, strongSelf] {
        if (!strongSelf->transportContext->connection)
            return;

        if (!strongSelf->transportContext->connection->isEqual(connection))
            return;

        strongSelf->transportContext->connected = false;
        strongSelf->transportContext->setDelegate(nullptr);
        strongSelf->transportContext->connection = nullptr;

        strongSelf->transportContext->connectionClosed();

        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->transportNetworkConnectionStateChanged(*strongSelf, false);

    });
}

void TcpTransport::connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([&, strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        if (!strongSelf->transportContext->connection->isEqual(connection))
            return;

        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->transportHasIncomingData(*strongSelf, slice, true, [weakSelf = weak_from_this()](bool success) {
                if (auto _strongSelf = weakSelf.lock())
                {
                    if (success)
                        _strongSelf->connectionIsValid();
                    else
                        _strongSelf->connectionIsInvalid();
                }
            });

    });
}

void TcpTransport::tcpConnectionRequestReconnection(const TcpTransportContext &context) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([&, strongSelf] {

        if (strongSelf->transportContext->stopped)
            return;

        strongSelf->startIfNeeded();
    });
}

void TcpTransport::startIfNeeded() {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        //TODO: start connection watchdog timer

        auto context = strongSelf->context;
        auto address = IPv4Address(strongSelf->address->ip);

        strongSelf->transportContext->connection = std::make_shared<TcpConnection>(address, strongSelf->address->port);

        auto strongDelegate = std::static_pointer_cast<ConnectionDelegate>(strongSelf);

        strongSelf->transportContext->connection->setDelegate(strongDelegate);

        strongSelf->transportContext->connection->start();
    });
}


void TcpTransport::updateConnectionState() {

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