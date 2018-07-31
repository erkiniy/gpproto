//
// Created by ProDigital on 7/25/18.
//

#ifndef GPPROTO_DISPATCHQUEUE_H
#define GPPROTO_DISPATCHQUEUE_H

#include <functional>
#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

#include "Semaphore.h"

namespace gpproto {
    class DispatchQueue {
        typedef std::function<void()> DispatchWork;
    public:

        DispatchQueue(std::string name) : _name(name), _finished(false), _semaphore(), _syncSemaphore(),
                                          _runningSynchronous(false), _thread(&DispatchQueue::threadWorker, this) {
            this->_jobs = std::list<DispatchWork>();
            this->_threadId = _thread.get_id();
            printf("DispatchQueue %s allocated\n", this->_name.c_str());
        }

        ~DispatchQueue() {
            printf("DispatchQueue %s deallocated\n", this->_name.c_str());
            _finished = true;
            _jobs.clear();
            _semaphore.notify();
            _thread.join();
        }

        void sync(const DispatchWork &work);

        void async(const DispatchWork work);

        void asyncForce(const DispatchWork work);

        bool isCurrentQueue() {
            return std::this_thread::get_id() == this->_threadId;
        }

    private:
        std::string _name;
        std::list<DispatchWork> _jobs;
        std::thread _thread;
        std::thread::id _threadId;
        std::mutex _mutex;

        bool _finished;
        bool _runningSynchronous;
        Semaphore _semaphore;
        Semaphore _syncSemaphore;

        void _async(const DispatchWork work, bool force);

        void threadWorker() {

            while (!_finished) {
                maybeDispatchWorker();
            }

        }

        void maybeDispatchWorker() {
            auto l = std::list<DispatchWork>();

            _mutex.lock();
            do {
                if (!_jobs.empty())
                {
                    auto f = _jobs.front();
                    _jobs.pop_front();
                    l.push_back(f);
                }
            }
            while (!_jobs.empty());

            _mutex.unlock();

            while (!l.empty())
            {
                auto f = l.front();
                l.pop_front();
                f();
            }

            bool sync = false;

            _mutex.lock();
            sync = _runningSynchronous;
            _runningSynchronous = false;
            _mutex.unlock();

            if (sync) {
                _syncSemaphore.notify();
            }

            _semaphore.wait();
        }

    };
}
#endif //GPPROTO_DISPATCHQUEUE_H
