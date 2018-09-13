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
    workerQueue->async([self = shared_from_this(), timer]
    {
        LOGV("RUNLOOP SCHEDULE");
        std::shared_ptr<Timer> found = nullptr;

        for (const auto & it : self->timerQueue) {
            if (it->id == timer->id) {
                found = it;
                break;
            }
        }

        if (const auto & _timer = found)
            self->invalidate(_timer);

        auto position = self->timerQueue.begin();

        for (auto it = self->timerQueue.begin(); it != self->timerQueue.end(); it++) {
            if ((*it)->timeout >= timer->timeout) {
                position = it;
                break;
            }
        }

        self->timerQueue.insert(position, timer);

        self->run();
    });
}

void Runloop::invalidate(std::shared_ptr<Timer> timer) {
    workerQueue->async([self = shared_from_this(), timer]
    {
        for (auto it = self->timerQueue.begin(); it != self->timerQueue.end(); it++)
        {
            if ((*it)->id == timer->id) {
                self->timerQueue.erase(it);
                LOGV("INVALIDATE TRUE");
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
            if (self->timerQueue.empty()) {
                self->ticks = 0;
                return;
            }

            self->ticks++;

            self->dump();

            auto closestTimer = *self->timerQueue.begin();
            auto ticks = (long long)(closestTimer->timeout / self->ratio);

            if (ticks <= self->ticks)
            {
                self->timerQueue.pop_front();
                std::shared_ptr<DispatchQueue> _queue = closestTimer->queue;

                if (_queue == nullptr)
                    _queue = self->callbackQueue;

                _queue->async([self, timer = closestTimer]
                {
                    timer->action();

                    if (timer->repeats)
                        self->schedule(timer);
                });
            }

            self->run();
        });
    });
}

void Runloop::dump() {
    LOGV("\n------Runloop-Dump------");
    LOGV("------Ticks = %d--------", ticks);
    LOGV("------Timers = %lu--------\n", timerQueue.size());
}

