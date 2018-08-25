//
// Created by Jaloliddin Erkiniy on 8/4/18.
//

#ifndef GPPROTO_TIMER_H
#define GPPROTO_TIMER_H

#include <functional>
#include <atomic>
#include <unordered_map>
#include "DispatchQueuePool.h"

namespace gpproto {

class Timer : public std::enable_shared_from_this<Timer> {
    typedef std::function<void()> TimerAction;
    public:
        Timer(float timeout, bool repeats, const TimerAction& action, std::shared_ptr<DispatchQueue> queue = nullptr)
                : timeout(timeout),
                  action(action),
                  repeats(repeats),
                  queue(queue),
                  timerQueue(nullptr),
                  started(false),
                  timerToken(0),
                  invalidatedQueues() {}

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
        std::unordered_map<std::string, std::shared_ptr<DispatchQueue>> invalidatedQueues;
        std::mutex mtx;

        void releaseInvalidatedQueue(const std::string& name);
    };
}

#endif //GPPROTO_TIMER_H
