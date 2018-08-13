//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#ifndef GPPROTO_PROTO_H
#define GPPROTO_PROTO_H

#include <stdlib.h>
#include <memory>
#include <functional>
#include "gp/net/NetworkAddress.h"

namespace gpproto {
    class Proto {
    public:
        Proto(IPv4Address& address, uint16_t port);
        ~Proto();

        void init();
        void start();
        void send(const char *data, size_t& length);
        void stop();

        struct Callbacks {
            void (*didConnect)();
            void (*didDisconnect)();
            void (*didReceiveData)(const char*, const size_t&);
        };

        Callbacks callbacks;

    protected:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

#endif //GPPROTO_PROTO_H
