//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CLIENTSYNC_H
#define GPPROTO_CLIENTSYNC_H

#include <cstdlib>
#include "gp/network/ProtoError.h"
#include "gp/utils/StreamSlice.h"
#include "gp_client_data.h"

namespace gpproto
{
    class Proto;
    class Context;

    class ClientSync final {
    public:
        //struct gp_rx_event;

        ClientSync();
        ClientSync(const ClientSync&) = delete;

        ~ClientSync() = default;

        int send(const unsigned char *data, size_t length);
        gp_rx_event* receive(double& timeout);
        void pause();
        void resume();
        void reset();

    private:
        gp_rx_event *currentOutputEvent;
        std::shared_ptr<Proto> proto;
    };
}


#endif //GPPROTO_CLIENTSYNC_H
