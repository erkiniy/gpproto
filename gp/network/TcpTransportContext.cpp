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
    LOGV("[TcpTransportContext -> startIfNeeded]");
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

void TcpTransportContext::connectionClosed() {
    //TODO: implement reconnect timer;
    if (!needsReconnection)
        return;

    LOGV("[TcpTransportContext -> connectionClosed]");

    backoffCount++;

    if (backoffCount == 1)
        timerEvent();
    else
    {
        double delay = 1.0;
        if (backoffCount <= 5)
            delay = 1.0;
        else if (backoffCount <= 20)
            delay = 4.0;
        else
            delay = 8.0;

        startTimer(delay);
    }
}

void TcpTransportContext::startTimer(double timeout) {

    invalidateTimer();

    auto strongSelf = shared_from_this();

    queue->async([s = strongSelf, timeout] {
        std::weak_ptr<TcpTransportContext> weakSelf = s;
        s->backoffTimer = std::make_shared<Timer>(timeout, false, [weakSelf] {

            if (auto strongSelf_ = weakSelf.lock())
                strongSelf_->timerEvent();

        }, s->queue);
    });

}

void TcpTransportContext::invalidateTimer() {
    auto reconnectionTimer = backoffTimer;

    backoffTimer = nullptr;

    queue->async([reconnectionTimer] {
        if (auto timer = reconnectionTimer)
            timer->invalidate();
    });
}

void TcpTransportContext::timerEvent() {
    invalidateTimer();

    queue->async([strongSelf = shared_from_this()] {
        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->tcpConnectionRequestReconnection(*strongSelf);
    });
}