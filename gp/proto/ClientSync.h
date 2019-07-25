//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CLIENTSYNC_H
#define GPPROTO_CLIENTSYNC_H

#include <cstdlib>
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>

struct gp_environment;
struct gp_rx_event;

namespace gpproto
{
    class Proto;
    class Context;
    class RequestMessageService;

    class ClientSync : public std::enable_shared_from_this<ClientSync> {
    public:

        ClientSync(std::shared_ptr<gp_environment> environment);
        ClientSync(const ClientSync&) = delete;

        ~ClientSync() = default;

        int send(const unsigned char *data, size_t length);
        gp_rx_event* receive(double& timeout);
        void pause();
        void resume();
        void reset();

    private:
        std::shared_ptr<gp_rx_event> lastReceiveEvent;
        std::shared_ptr<Proto> proto;
        std::shared_ptr<RequestMessageService> requestService;

        std::mutex mutex;
        std::condition_variable cond;

        std::deque<std::shared_ptr<gp_rx_event>> queue;

        void push_back(const std::shared_ptr<gp_rx_event> & event);
        std::shared_ptr<gp_rx_event> pop_front();
    };
}


#endif //GPPROTO_CLIENTSYNC_H
