//
// Created by Jaloliddin Erkiniy on 8/6/18.
//

#include "DispatchQueuePool.h"
using namespace gpproto;

std::shared_ptr<DispatchQueue> DispatchQueuePool::getQueue() {
    std::unique_lock<std::mutex> lock(mtx);

    if (pool.empty())
    {
        static int internalId = 0;
        internalId++;

        std::string name = "_pool" + std::to_string(internalId);
        std::shared_ptr<DispatchQueue> queue = std::make_shared<DispatchQueue>(name);

        pool.push_back(queue);
    }

    auto queue = pool.front();
    pool.pop_front();

    return queue;
}

void DispatchQueuePool::releaseQueue(std::shared_ptr<DispatchQueue> queue) {
    std::unique_lock<std::mutex> lock(mtx);
    pool.push_back(queue);
}