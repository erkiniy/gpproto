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
#include <atomic>

#include "gp/proto/ProtoDelegate.h"
#include "gp/proto/UpdateMessageService.h"

struct gp_environment;
struct gp_rx_event;

class Semaphore;

namespace gpproto
{
    class Proto;
    class Context;
    class RequestMessageService;
    class UpdateMessageService;

    class ClientSync : public std::enable_shared_from_this<ClientSync>, public ProtoDelegate, public UpdateMessageServiceDelegate {
    public:

        explicit ClientSync(std::shared_ptr<gp_environment> environment);

        ClientSync(const ClientSync&) = delete;

        ~ClientSync() = default;

        int send(const unsigned char *data, size_t length);

        gp_rx_event* receive(double& timeout);

        void pause();

        void resume();

        void stop();

        void cancel(int requestId);

        double getGlobalTime();

        const int id;

        void initialize();

        void connectionStateAvailibilityChanged(const Proto& proto, bool isNetworkAvailable) override;

        void connectionStateChanged(const Proto& proto, ProtoConnectionState state) override;

        void didReceiveUpdates(const std::shared_ptr<UpdateMessageService>& service, std::shared_ptr<StreamSlice> appData, int32_t date) override;

    private:
        std::shared_ptr<gp_rx_event> currentReceiveEvent;
        std::shared_ptr<Proto> proto;
        std::shared_ptr<RequestMessageService> requestService;
        std::shared_ptr<UpdateMessageService> updateService;

        std::mutex mutex;
        std::shared_ptr<Semaphore> semaphore;
        std::deque<std::shared_ptr<gp_rx_event>> queue;

        void push_back(const std::shared_ptr<gp_rx_event> & event);
        std::shared_ptr<gp_rx_event> pop_front();

        void cleanUpEvent(const std::shared_ptr<gp_rx_event> event);

        void notifyReceiveQueue();

        static int getNextInternalId() {
            static std::atomic_int internalId = 0;
            return internalId++;
        }
    };
}


#endif //GPPROTO_CLIENTSYNC_H
