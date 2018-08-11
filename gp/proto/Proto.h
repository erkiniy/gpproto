//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#ifndef GPPROTO_PROTO_H
#define GPPROTO_PROTO_H

#include <stdlib.h>
#include <memory>

namespace gpproto {
    class Proto {
    public:
        Proto();
        ~Proto();


        void start();
        void send(char *data, size_t length);
        void stop();

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

#endif //GPPROTO_PROTO_H
