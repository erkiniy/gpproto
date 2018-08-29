//
// Created by Jaloliddin Erkiniy on 8/29/18.
//

#include "TcpTransportContext.h"
using namespace gpproto;

void TcpTransportContext::requestConnection() {
    startIfNeeded();
}

void TcpTransportContext::startIfNeeded() {
    auto strongSelf = shared_from_this();

    queue->async([strongSelf] {
        if (strongSelf->connection == nullptr)
        {
            if (auto delegate = strongSelf->delegate.lock())
                delegate->tcpConnectionRequestReconnection(*strongSelf);
        }
    });
}

void TcpTransportContext::setDelegate(std::shared_ptr<TcpTransportContextDelegate> delegate) {
    auto strongSelf = shared_from_this();

    queue->async([strongSelf, delegate] {
        strongSelf->delegate = delegate;
    });
}