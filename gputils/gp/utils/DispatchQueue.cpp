//
// Created by ProDigital on 7/25/18.
//

#include "DispatchQueue.h"
#include "gp/utils/Logging.h"

using namespace gpproto;

void DispatchQueue::async(DispatchQueue::DispatchWork work) {
    printf("Starting Async\n");
    _async(std::move(work), false);
}

void DispatchQueue::asyncForce(DispatchQueue::DispatchWork work) {
    _async(std::move(work), true);
}

void DispatchQueue::sync(const DispatchQueue::DispatchWork &work) {
    if (this->isCurrentQueue()) {
        work();
    }
    else {
        _mutex.lock();

        _runningSynchronous = true;

        _jobs.push_back(work);

        _mutex.unlock();

        _asyncSemaphore.notify();
        _syncSemaphore.wait();
    }
}

void DispatchQueue::_async(DispatchQueue::DispatchWork work, bool force) {
    if (this->isCurrentQueue() && !force) {
        work();
    }
    else
    {
        _mutex.lock();

        _jobs.push_back(work);

        _mutex.unlock();

        _asyncSemaphore.notify();
    }
}