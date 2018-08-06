//
// Created by Jaloliddin Erkiniy on 8/6/18.
//

#ifndef GPPROTO_DISPATCHQUEUEPOOL_H
#define GPPROTO_DISPATCHQUEUEPOOL_H

#include <mutex>
#include <list>
#include "DispatchQueue.h"

namespace gpproto {
    class DispatchQueuePool {
    public:
        static DispatchQueuePool &instance() {
            static DispatchQueuePool instance;
            return instance;
        }

        std::shared_ptr<DispatchQueue> getQueue();
        void releaseQueue(std::shared_ptr<DispatchQueue> queue);
    private:
        DispatchQueuePool(){};
        std::list<std::shared_ptr<DispatchQueue>> pool;
        std::mutex mtx;
    public:
        DispatchQueuePool(DispatchQueuePool const&) = delete;
        void operator=(DispatchQueuePool const&) = delete;
    };
}
#endif //GPPROTO_DISPATCHQUEUEPOOL_H
