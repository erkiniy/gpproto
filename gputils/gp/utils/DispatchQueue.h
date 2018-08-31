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

#include "gp/utils/Semaphore.h"

namespace gpproto {
    class DispatchQueue {
        typedef std::function<void()> DispatchWork;
    public:

        DispatchQueue(std::string name) : _name(name), _finished(false), _asyncSemaphore(), _syncSemaphore(),
                                          _runningSynchronous(false) {
            this->_thread = std::thread(&DispatchQueue::threadWorker, this);
            this->_threadId = _thread.get_id();
            this->_jobs = std::list<DispatchWork>();
            printf("DispatchQueue %s allocated\n", this->_name.c_str());
        }

        ~DispatchQueue() {
            printf("DispatchQueue %s deallocated\n", this->_name.c_str());
            _finished = true;
            _jobs.clear();
            _asyncSemaphore.notify();
            _thread.join();
        }

        void sync(const DispatchWork &work);

        void async(DispatchWork work);

        void asyncForce(DispatchWork work);

        bool isCurrentQueue() {
            return std::this_thread::get_id() == this->_threadId;
        }

        const std::string& name() const {
            return _name;
        }

    private:
        const std::string _name;
        std::list<DispatchWork> _jobs;
        std::thread _thread;
        std::thread::id _threadId;
        std::mutex _mutex;

        bool _finished;
        bool _runningSynchronous;
        Semaphore _asyncSemaphore;
        Semaphore _syncSemaphore;

        void _async(DispatchWork work, bool force);

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

            if (sync && _jobs.empty())
            {
                _runningSynchronous = false;
                _syncSemaphore.notify();
            }
            _mutex.unlock();

            _asyncSemaphore.wait();
        }

    };
}
#endif //GPPROTO_DISPATCHQUEUE_H
