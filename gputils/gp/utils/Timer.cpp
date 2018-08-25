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

    auto queueName = timerQueue->name();

    this->timerQueue->async([duration, token, weakSelf, queueName] {

        std::this_thread::sleep_for(std::chrono::microseconds(duration));

        auto strongSelf = weakSelf.lock();
        if (!strongSelf)
            return;

        if (token != strongSelf->timerToken) {
            LOGV("Timer tokens are different");
            strongSelf->releaseInvalidatedQueue(queueName);
            return;
        }
        else {
            if (auto queue = strongSelf->queue)
                queue->async(strongSelf->action);
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

    mtx.lock();
    if (timerQueue)
        invalidatedQueues[timerQueue->name()] = timerQueue;
    mtx.unlock();

    timerQueue = nullptr;
 }

 void Timer::resetTimeout(float timeout) {
    invalidate();

    mtx.lock();
    this->timeout = timeout;
    mtx.unlock();

    start();
}

void Timer::releaseInvalidatedQueue(const std::string &name) {
    mtx.lock();
    auto it = invalidatedQueues.find(name);

    if (it != invalidatedQueues.end())
    {
        auto queue = it->second;
        invalidatedQueues.erase(it);
        DispatchQueuePool::instance().releaseQueue(queue);
        LOGV("Releasing queue to the pool with name %s", name.c_str());
    }
    mtx.unlock();
}