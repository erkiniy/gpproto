//
// Created by Jaloliddin Erkiniy on 9/13/18.
//

#ifndef GPPROTO_RUNLOOP_H
#define GPPROTO_RUNLOOP_H

#include <functional>
#include <memory>
#include <list>
#include <chrono>
#include <tuple>

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

        long long ticks = 0;

        double tickDuration = 0;

        std::chrono::microseconds microseconds;

        std::shared_ptr<DispatchQueue> workerQueue;

        std::shared_ptr<DispatchQueue> loopQueue;

        std::shared_ptr<DispatchQueue> callbackQueue;

        std::list<std::pair<long long, std::shared_ptr<Timer>>> queue;

        long long timeoutToTicks(double timeout);

        long long targetTicks(double timeout);

        void run();

        void dump();
    };
}

#endif //GPPROTO_RUNLOOP_H
