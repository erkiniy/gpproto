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
            this->_jobs = std::list<DispatchWork>();
            //printf("DispatchQueue %s allocated\n", this->_name.c_str());
        }

        ~DispatchQueue() {
            //printf("DispatchQueue %s deallocated\n", this->_name.c_str());
            _finished = true;
            _jobs.clear();
            _asyncSemaphore.notify();
            _thread.join();
        }

        void sync(DispatchWork && work);

        void async(DispatchWork && work);

        void asyncForce(DispatchWork && work);

        bool isCurrentQueue() const {
            bool current = false;
            _threadIdMutex.lock();
            current = std::this_thread::get_id() == this->_threadId;
            _threadIdMutex.unlock();
            return current;
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
        mutable std::mutex _threadIdMutex;

        bool _finished;
        bool _runningSynchronous;
        Semaphore _asyncSemaphore;
        Semaphore _syncSemaphore;

        void _async(DispatchWork work, bool force);

        void threadWorker()
        {
            _threadIdMutex.lock();
            _threadId = _thread.get_id();
            _threadIdMutex.unlock();

            while (!_finished) {
                maybeDispatchWorker();
            }
        }

        void maybeDispatchWorker()
        {
            auto l = std::list<DispatchWork>();

            _mutex.lock();

            while (!_jobs.empty())
            {
                auto f = _jobs.front();
                _jobs.pop_front();
                l.push_back(f);
            }

            _mutex.unlock();

            //printf("Before invoke list number %lu of %s\n", l.size(), name().c_str());

            while (!l.empty())
            {
                (*l.begin())();
                l.pop_front();
            }

            bool sync = false;

            _mutex.lock();
            sync = _runningSynchronous;

            if (sync)
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
