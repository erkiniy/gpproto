//
// Created by Jaloliddin Erkiniy on 8/4/18.
//

#include "Timer.h"
#include <chrono>
#include "Logging.h"

using namespace gpproto;

void Timer::start() {
    invalidate();

    LOGV("Starting timer");

    if (!this->timerQueue)
        this->timerQueue = DispatchQueuePool::instance().getQueue();

    int token = this->timerToken.load();

    mtx.lock();
    auto duration = (long long)(1000000 * timeout);
    mtx.unlock();

    std::weak_ptr<Timer> weakSelf = shared_from_this();

    this->timerQueue->async([duration, token, weakSelf] {

        std::this_thread::sleep_for(std::chrono::microseconds(duration));

        auto strongSelf = weakSelf.lock();
        if (!strongSelf)
            return;

        if (token != strongSelf->timerToken) {
            LOGV("Timer tokens are different");
            return;
        }
        else {
            if (strongSelf->queue)
                strongSelf->queue->async(strongSelf->action);
            else
                strongSelf->action();

            if (strongSelf->repeats)
                strongSelf->start();
        }
    });

    started = true;
}

bool Timer::isScheduled() const {
    return started;
}

void Timer::invalidate() {
    if (!started)
        return;

    timerToken ++;
    started = false;
 }

 void Timer::resetTimeout(float timeout) {
    invalidate();

    mtx.lock();
    this->timeout = timeout;
    mtx.unlock();

    start();
}