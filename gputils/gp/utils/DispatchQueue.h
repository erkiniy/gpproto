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

        DispatchQueue(std::string name) : _name(std::move(name)), _finished(false), _asyncSemaphore(), _syncSemaphore(),
                                          _runningSynchronous(false) {
            this->_thread = std::thread(&DispatchQueue::threadWorker, this);
            this->_jobs = std::list<DispatchWork>();
        }

        ~DispatchQueue() {
            _finished = true;
            _jobs.clear();
            _asyncSemaphore.notify();
            _thread.join();
        }

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
        std::list<DispatchWork> _jobs;
        std::list<DispatchWork> _tempJobs;
        std::thread _thread;
        std::thread::id _threadId;
        std::recursive_mutex _mutex;

        bool _finished;
        bool _runningSynchronous;
        Semaphore _asyncSemaphore;
        Semaphore _syncSemaphore;

        void _async(DispatchWork && work, bool force);

        void threadWorker()
        {
            _threadId = _thread.get_id();

            while (!_finished) {
                maybeDispatchWorker();
            }
        }

        void maybeDispatchWorker()
        {
            _mutex.lock();

            _tempJobs = _jobs;

            _jobs.clear();

            _mutex.unlock();

            //printf("Before invoke list number %lu of %s\n", l.size(), name().c_str());

            while (!_tempJobs.empty())
            {
                (*_tempJobs.begin())();
                _tempJobs.pop_front();
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
