//
// Created by Jaloliddin Erkiniy on 8/24/18.
//

#ifndef GPPROTO_PROTODELEGATE_H
#define GPPROTO_PROTODELEGATE_H

#include "Proto.h"

namespace gpproto {
    class Proto;

    class ProtoDelegate {
    public:
        virtual void connectionStateChanged(const Proto& proto, int state) = 0;
    };
}

#endif //GPPROTO_PROTODELEGATE_H
