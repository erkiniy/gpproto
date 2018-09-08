//
// Created by Erkiniy Jaloliddin on 7/25/18.
//

#include "gp/utils/DispatchQueue.h"
#include "gp/utils/Logging.h"

using namespace gpproto;

void DispatchQueue::async(DispatchQueue::DispatchWork work) {
    _async(std::move(work), false);
}

void DispatchQueue::asyncForce(DispatchQueue::DispatchWork work) {
    _async(std::move(work), true);
}

void DispatchQueue::sync(DispatchQueue::DispatchWork work) {
    if (this->isCurrentQueue()) {
        work();
        LOGV("Sync current %s", this->name().c_str());
    }
    else {
        LOGV("Sync %s", this->name().c_str());
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
        LOGV("Async current %s", this->name().c_str());
    }
    else
    {
        LOGV("Async %s", this->name().c_str());
        _mutex.lock();

        _jobs.push_back(work);

        _mutex.unlock();

        _asyncSemaphore.notify();
    }
}