//
// Created by Erkiniy Jaloliddin on 7/25/18.
//

#include "gp/utils/DispatchQueue.h"
#include "gp/utils/Logging.h"

using namespace gpproto;

DispatchQueue::~DispatchQueue() {
    _mutex.lock();
    _finished = true;
    _jobs.clear();
    _mutex.unlock();
    _asyncSemaphore.notify();

    _thread.join();
}

void DispatchQueue::async(DispatchQueue::DispatchWork && work) {
    _async(std::move(work), false);
}

void DispatchQueue::asyncForce(DispatchQueue::DispatchWork && work) {
    _async(std::move(work), true);
}

void DispatchQueue::sync(DispatchQueue::DispatchWork && work) {
    if (this->isCurrentQueue()) {
        //LOGV("<--------------> Dispatched %s SYNC", this->name().c_str());
        work();
    }
    else {
        //LOGV("<--------------> Dispatched %s SYNC", this->name().c_str());
        _mutex.lock();

        auto syncSemaphore = std::make_shared<Semaphore>();

        if (!_finished)
            _jobs.emplace_back(std::move(work), syncSemaphore);

        _mutex.unlock();

        _asyncSemaphore.notify();

        if (!_finished)
            syncSemaphore->wait();
    }
}

void DispatchQueue::_async(DispatchQueue::DispatchWork && work, bool force) {
    if (this->isCurrentQueue() && !force) {
        //LOGV("<--------------> Dispatched %s ASYNC-CURRENT", this->name().c_str());
        work();
    }
    else
    {
        _mutex.lock();

        //LOGV("<--------------> Dispatched %s ASYNC. Jobs.count = %d", this->name().c_str(), _jobs.size());

        if (!_finished)
            _jobs.emplace_back(std::move(work));

        _mutex.unlock();

        _asyncSemaphore.notify();
    }
}