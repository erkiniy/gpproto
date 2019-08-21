//
// Created by ProDigital on 7/25/18.
//

#ifndef GPPROTO_DISPATCHQUEUE_H
#define GPPROTO_DISPATCHQUEUE_H

#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <deque>

class Semaphore;
namespace gpproto {
    class DispatchQueue {
        typedef std::function<void()> DispatchWork;

        class DispatchWorker {
        public:
            DispatchWorker(DispatchWork && work, std::shared_ptr<Semaphore> semaphore = nullptr);
            const DispatchWork work;
            const std::shared_ptr<Semaphore> syncSemaphore;

            DispatchWorker(const DispatchWorker &) = delete;
        };
    public:
        explicit DispatchQueue(std::string name);

        DispatchQueue(const DispatchQueue&) = delete;

        ~DispatchQueue();

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
        std::deque<DispatchWorker> _jobs;
        std::deque<DispatchWorker> _tempJobs;
        std::thread _thread;
        std::thread::id _threadId;
        std::recursive_mutex _mutex;
        std::atomic_bool started;

        volatile bool _finished;
        std::shared_ptr<Semaphore> _asyncSemaphore;

        void _async(DispatchWork && work, bool force);

        void threadWorker()
        {
            _threadId = std::this_thread::get_id();

            while (!_finished) {
                maybeDispatchWorker();
            }
        }

        void maybeDispatchWorker();

    };
}
#endif //GPPROTO_DISPATCHQUEUE_H
