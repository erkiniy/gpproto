//
// Created by Jaloliddin Erkiniy on 9/13/18.
//

#ifndef GPPROTO_RUNLOOP_H
#define GPPROTO_RUNLOOP_H

#include <functional>
#include <memory>
#include <list>
#include <chrono>

namespace gpproto
{
    class Timer;
    class DispatchQueue;
class Runloop : public std::enable_shared_from_this<Runloop> {
    public:
        static std::shared_ptr<Runloop> Default() {
            static std::shared_ptr<Runloop> rl = std::shared_ptr<Runloop>(new Runloop(1.0));
            return rl;
        }

        static std::shared_ptr<Runloop> HighPriority() {
            static std::shared_ptr<Runloop> rl = std::shared_ptr<Runloop>(new Runloop(0.5));
            return rl;
        }

        Runloop(const Runloop &) = delete;

        Runloop(Runloop &&) = delete;

        void schedule(std::shared_ptr<Timer> timer);

        void invalidate(std::shared_ptr<Timer> timer);

    private:

        explicit Runloop(double ratio);

        double ratio;

        int ticks = 0;

        double tickDuration = 0;

        std::chrono::microseconds microseconds;

        std::list<std::shared_ptr<Timer>> timerQueue;

        std::shared_ptr<DispatchQueue> workerQueue;

        std::shared_ptr<DispatchQueue> loopQueue;

        std::shared_ptr<DispatchQueue> callbackQueue;

        void run();

        void dump();
    };
}

#endif //GPPROTO_RUNLOOP_H
