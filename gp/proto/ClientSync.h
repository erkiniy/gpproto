//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CLIENTSYNC_H
#define GPPROTO_CLIENTSYNC_H

#include <cstdlib>
#include "gp/network/ProtoError.h"
#include "gp/utils/StreamSlice.h"
#include "gp_client_data.h"

namespace gpproto {
    class ClientSync final {
    public:
        //struct gp_rx_event;

        ClientSync() = default;
        ~ClientSync() {}

        int send(const unsigned char *data, size_t length);
        gp_rx_event* receive(double& timeout);
        void pause();
        void resume();
        void reset();

    private:
        gp_rx_event *currentOutputEvent;
    };
}


#endif //GPPROTO_CLIENTSYNC_H
