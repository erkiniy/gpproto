//
// Created by Jaloliddin Erkiniy on 8/4/18.
//

#ifndef GPPROTO_TIMER_H
#define GPPROTO_TIMER_H

#include <functional>
#include <atomic>
#include <unordered_map>
#include <memory>

namespace gpproto {
    class DispatchQueue;

class Timer : public std::enable_shared_from_this<Timer> {

    typedef std::function<void()> TimerAction;

    public:
        static std::shared_ptr<Timer> make_timer(double timeout, bool repeats, TimerAction&& action, std::shared_ptr<DispatchQueue> queue = nullptr) {
            return std::shared_ptr<Timer>(new Timer(timeout, repeats, std::move(action), queue));
        }

        ~Timer() = default;

        const int id;

        mutable double timeout; //seconds

        const bool repeats;

        mutable std::atomic_bool started;

        std::shared_ptr<DispatchQueue> queue;

        const TimerAction action;

        void start();

        void resetTimeout(double timeout);

        void invalidate();

        bool isScheduled() const;

    private:
        Timer(double timeout, bool repeats, TimerAction&& action, std::shared_ptr<DispatchQueue> queue = nullptr)
                : id(Timer::nextInternalId()),
                  timeout(timeout),
                  action(std::move(action)),
                  repeats(repeats),
                  queue(queue),
                  started(false) {};


        static int nextInternalId() {
            static std::atomic_int id = 0;
            return id++;
        }
    };

    inline bool operator == (const Timer& lhs, const Timer& rhs) {
        return lhs.id == rhs.id;
    }
}

#endif //GPPROTO_TIMER_H
