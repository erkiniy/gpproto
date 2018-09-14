//
// Created by Jaloliddin Erkiniy on 9/13/18.
//

#include "gp/utils/Runloop.h"
#include "gp/utils/DispatchQueue.h"
#include "gp/utils/Timer.h"
#include "gp/utils/Logging.h"

using namespace gpproto;

Runloop::Runloop(double ratio) {
    this->ratio = ratio;
    this->workerQueue = std::make_shared<DispatchQueue>("uz.gpproto.runloop.defaultWorker");
    this->loopQueue = std::make_shared<DispatchQueue>("uz.gpproto.runloop.loop");
    this->callbackQueue = std::make_shared<DispatchQueue>("uz.gpproto.runloop.callback");
    this->microseconds = std::chrono::microseconds((long long)(1000000 * ratio));
}

void Runloop::schedule(std::shared_ptr<Timer> timer) {
    typedef std::pair<long long, std::shared_ptr<Timer>> TimerPair;

    workerQueue->async([self = shared_from_this(), timer]
    {
        std::shared_ptr<Timer> found = nullptr;

        for (const auto & it : self->queue) {
            if (*it.second == *timer) {
                found = it.second;
                return;
            }
        }

        if (const auto & _timer = found)
            self->invalidate(_timer);

        auto targetTicks = self->targetTicks(timer->timeout);

        auto pair = std::make_pair(targetTicks, timer);

        auto it = std::lower_bound(self->queue.begin(), self->queue.end(), pair, [](const TimerPair & lhs, const TimerPair & rhs) -> bool {
            return lhs.first < rhs.first;
        });

        LOGV("[Runloop schedule] inserting timer with %lld ticks", targetTicks);

        self->queue.insert(it, std::make_pair(targetTicks, timer));

        self->run();
    });
}

void Runloop::invalidate(std::shared_ptr<Timer> timer) {
    workerQueue->async([self = shared_from_this(), timer]
    {
        for (auto it = self->queue.begin(); it != self->queue.end(); it++)
        {
            if (*(*it).second == *timer) {
                self->queue.erase(it);
                break;
            }
        }
    });
}

void Runloop::run() {
    loopQueue->async([self = shared_from_this()]
    {
        std::this_thread::sleep_for(self->microseconds);

        self->workerQueue->async([self]
        {
            if (self->queue.empty()) {
                self->ticks = 0;
                return;
            }

            self->ticks++;

            self->dump();

            auto closestPair = *self->queue.begin();
            auto closestTimer = closestPair.second;

            if (closestPair.first <= self->ticks)
            {
                self->queue.pop_front();
                std::shared_ptr<DispatchQueue> _queue = closestTimer->queue;

                if (_queue == nullptr)
                    _queue = self->callbackQueue;

                _queue->async([self, timer = closestTimer]
                {
                    timer->action();

                    if (timer->repeats)
                        timer->start();
                });
            }

            self->run();
        });
    });
}

long long Runloop::timeoutToTicks(double timeout) {
    return (long long)(timeout / ratio);
}

long long Runloop::targetTicks(double timeout) {
    return timeoutToTicks(timeout) + this->ticks;
}

void Runloop::dump() {
    LOGV("------Runloop-Dump------ %zu timers", queue.size());
    for (const auto & it : queue)
        LOGV("-------- Tick = %lld ------ %lld", ticks, it.first);
    LOGV("------Runloop-Dump------ %zu timers\n", queue.size());

}

