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
#include <condition_variable>

#include "Semaphore.h"

class DispatchQueue {
    typedef std::function<void()> DispatchWork;
public:

    DispatchQueue(std::string name) : _name(name), _finished(false), _semaphore(), _thread(&DispatchQueue::threadWorker, this) {
        this->jobs = std::list<DispatchWork>();
        this->_threadId = _thread.get_id();
    }

    ~DispatchQueue() {
        printf("DispatchQueue %s deallocated", this->_name.c_str());
        _finished = true;
        jobs.clear();
        _semaphore.notify();
        _thread.join();
    }

    void sync(const DispatchWork& work);
    void async(const DispatchWork work);

    bool isCurrentQueue() {
        return std::this_thread::get_id() == this->_threadId;
    }

private:
    std::string _name;
    std::list<DispatchWork> jobs;
    std::thread _thread;
    std::thread::id _threadId;
    std::mutex _mutex;

    bool _finished;
    Semaphore _semaphore;

    void threadWorker() {
        printf("DispatchQueue began");

        while (!_finished)
        {
            maybeDispatchWorker();
        }

        printf("DispatchQueue finished");
    }

    void maybeDispatchWorker() {
        printf("maybeDispatchWorker");

        _mutex.lock();

        while (!jobs.empty())
        {
            auto f = jobs.front();
            f();
            jobs.pop_front();
        }

        _mutex.unlock();

        _semaphore.wait();
    }

};
#endif //GPPROTO_DISPATCHQUEUE_H
