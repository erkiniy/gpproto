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

        //TODO: report to delegate about failure of transaction

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
}

void TcpTransport::setDelegate(std::shared_ptr<TransportDelegate> delegate) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([delegate, strongSelf] {
        strongSelf->delegate = delegate;
    });
}

void TcpTransport::setDelegateNeedsTransaction() {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([strongSelf] {
        if (strongSelf->transportContext->connection == nullptr)
        strongSelf->requestTransactionFromDelegate();
    });
}

void TcpTransport::requestTransactionFromDelegate() {

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

}

void TcpTransport::connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) {
    auto strongSelf = shared_from_this();
    TcpTransport::queue()->async([&, strongSelf] {

        if (!strongSelf->transportContext->connection)
            return;

        if (!strongSelf->transportContext->connection->isEqual(connection))
            return;

        if (auto strongDelegate = strongSelf->delegate.lock())
        {
            strongDelegate->transportHasIncomingData(*strongSelf, slice);
        }
    });
}

void TcpTransport::tcpConnectionRequestReconnection(const TcpTransportContext &context) {

}
