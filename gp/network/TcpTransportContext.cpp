//
// Created by Jaloliddin Erkiniy on 8/29/18.
//

#include "TcpTransportContext.h"
using namespace gpproto;

void TcpTransportContext::requestConnection() {
    startIfNeeded();
}

void TcpTransportContext::startIfNeeded() {
    auto self = shared_from_this();
    LOGV("[TcpTransportContext -> startIfNeeded]");
    queue->async([self] {
        if (self->connection == nullptr)
        {
            if (auto delegate = self->delegate.lock())
                delegate->tcpConnectionRequestReconnection(*self);
        }
    });
}

void TcpTransportContext::setDelegate(std::shared_ptr<TcpTransportContextDelegate> delegate) {
    auto strongSelf = shared_from_this();

    LOGV("[TcpTransportContext setDelegate] %s", delegate != nullptr ? "delegate" : "nullptr");

    queue->async([strongSelf, delegate] {
        strongSelf->delegate = delegate;
    });
}

void TcpTransportContext::connectionClosed() {
    //TODO: implement reconnect timer;

    LOGV("[TcpTransportContext -> connectionClosed]");
    if (!needsReconnection)
        return;

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

    queue->async([self = shared_from_this(), timeout] {
        std::weak_ptr<TcpTransportContext> weakSelf = self;
        self->backoffTimer = Timer::make_timer((float)timeout, false, [weakSelf] {

            if (auto strongSelf_ = weakSelf.lock())
                strongSelf_->timerEvent();

        }, self->queue);
        self->backoffTimer->start();
    });

}

void TcpTransportContext::invalidateTimer() {
    auto reconnectionTimer = backoffTimer;

    backoffTimer = nullptr;

    queue->async([reconnectionTimer] {
        if (const auto & timer = reconnectionTimer)
            timer->invalidate();
    });
}

void TcpTransportContext::timerEvent() {
    invalidateTimer();

    LOGV("[TcpTransportContext timerEvent]");

    queue->async([strongSelf = shared_from_this()] {
        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->tcpConnectionRequestReconnection(*strongSelf);
    });
}

void TcpTransportContext::connectionValidDataReceived() {
    backoffCount = 0;

    invalidateTimer();
}