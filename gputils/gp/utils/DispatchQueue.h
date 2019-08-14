//
// Created by ProDigital on 7/25/18.
//

#ifndef GPPROTO_DISPATCHQUEUE_H
#define GPPROTO_DISPATCHQUEUE_H

#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <deque>

#include "gp/utils/Semaphore.h"

namespace gpproto {
    class DispatchQueue {
        typedef std::function<void()> DispatchWork;

        class DispatchWorker {
        public:
            DispatchWorker(DispatchWork && work, bool sync): work(std::move(work)), sync(sync) {};
            const DispatchWork work;
            const bool sync;

            DispatchWorker(const DispatchWorker &) = delete;
        };
    public:
        explicit DispatchQueue(std::string name) : _name(std::move(name)),
                                          _finished(false),
                                          _asyncSemaphore(),
                                          _syncSemaphore() {
            this->_thread = std::thread(&DispatchQueue::threadWorker, this);
        }

        DispatchQueue(const DispatchQueue&) = delete;

        ~DispatchQueue();

        void sync(DispatchWork && work);

        void async(DispatchWork && work);

        void asyncForce(DispatchWork && work);

        bool isCurrentQueue() const {
            return std::this_thread::get_id() == this->_threadId;
        }

        const std::string& name() const {
            return _name;
        }

    private:
        const std::string _name;
        std::deque<DispatchWorker> _jobs;
        std::deque<DispatchWorker> _tempJobs;
        std::thread _thread;
        std::thread::id _threadId;
        std::recursive_mutex _mutex;
        std::atomic_bool started;

        volatile bool _finished;
        Semaphore _asyncSemaphore;
        Semaphore _syncSemaphore;

        void _async(DispatchWork && work, bool force);

        void threadWorker()
        {
            _threadId = std::this_thread::get_id();

            while (!_finished) {
                maybeDispatchWorker();
            }

            printf("Thread worker finished");
        }

        void maybeDispatchWorker()
        {
            _mutex.lock();

            _tempJobs = std::move(_jobs);

            _jobs.clear();

            _mutex.unlock();

            if (_finished) return;

            while (!_tempJobs.empty())
            {
                auto worker = _tempJobs.begin();
                worker->work();

                if (worker->sync)
                    _syncSemaphore.notify();

                _tempJobs.pop_front();
            }

            _asyncSemaphore.wait();
        }

    };
}
#endif //GPPROTO_DISPATCHQUEUE_H
