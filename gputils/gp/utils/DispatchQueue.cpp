//
// Created by ProDigital on 7/25/18.
//

#include "DispatchQueue.h"
using namespace gpproto;

void DispatchQueue::async(const DispatchQueue::DispatchWork work) {
    _async(work, false);
}

void DispatchQueue::asyncForce(const DispatchQueue::DispatchWork work) {
    _async(work, true);
}

void DispatchQueue::sync(const DispatchQueue::DispatchWork &work) {
    if (this->isCurrentQueue()) {
        work();
    }
    else {
        _mutex.lock();

        _runningSynchronous = true;

        jobs.push_back(work);

        _mutex.unlock();

        _semaphore.notify();
        _syncSemaphore.wait();
    }
}

void DispatchQueue::_async(const DispatchQueue::DispatchWork work, bool force) {
    if (this->isCurrentQueue() && !force) {
        work();
    }
    else
    {
        _mutex.lock();

        jobs.push_back(work);

        _mutex.unlock();

        _semaphore.notify();
    }
}