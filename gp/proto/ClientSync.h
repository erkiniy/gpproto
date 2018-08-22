//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CLIENTSYNC_H
#define GPPROTO_CLIENTSYNC_H

#include <cstdlib>
#include "../net/ProtoError.h"
#include "gp/utils/StreamSlice.h"

namespace gpproto {
    class ClientSync final {
    public:
        ClientSync() = default;
        ~ClientSync() {

        }

        int send(const unsigned char *data, size_t& length);
        std::pair<StreamSlice*, ProtoError*> receive(double& timeout);
    };
}


#endif //GPPROTO_CLIENTSYNC_H
