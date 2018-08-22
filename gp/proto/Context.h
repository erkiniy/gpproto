//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CONTEXT_H
#define GPPROTO_CONTEXT_H

#include "gp/utils/DispatchQueue.h"

namespace gpproto {
    class Context {
    public:
        static std::unique_ptr<DispatchQueue> queue() {
            static std::unique_ptr<DispatchQueue> q = std::make_unique<DispatchQueue>("uz.gpproto.context");
            return q;
        }
    };
}


#endif //GPPROTO_CONTEXT_H
