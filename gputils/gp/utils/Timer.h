//
// Created by Jaloliddin Erkiniy on 8/4/18.
//

#ifndef GPPROTO_TIMER_H
#define GPPROTO_TIMER_H

#include <functional>
#include <atomic>
#include "DispatchQueuePool.h"

namespace gpproto {

class Timer : public std::enable_shared_from_this<Timer> {
    typedef std::function<void()> TimerAction;
    public:
        Timer(float timeout, bool repeats, const TimerAction& action, std::shared_ptr<DispatchQueue> queue = nullptr)
                : timeout(timeout),
                  repeats(repeats),
                  action(action),
                  queue(queue),
                  timerQueue(nullptr),
                  started(false),
                  timerToken(0) {}

        ~Timer() {
            if (timerQueue) {
                DispatchQueuePool::instance().releaseQueue(timerQueue);
            }
        }
        void start();
        void resetTimeout(float timeout);
        void invalidate();
        bool isScheduled() const;
        bool repeats;
        mutable std::atomic_bool started;
    private:
        float timeout; //seconds
        TimerAction action;
        std::shared_ptr<DispatchQueue> queue;
        std::shared_ptr<DispatchQueue> timerQueue;
        mutable std::atomic_int timerToken;
        std::mutex mtx;
    };
}

#endif //GPPROTO_TIMER_H
