//
// Created by ProDigital on 7/25/18.
//

#include "DispatchQueue.h"

void DispatchQueue::async(const DispatchQueue::DispatchWork work) {

    _mutex.lock();

    jobs.push_back(work);

    _mutex.unlock();

    _semaphore.notify();
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