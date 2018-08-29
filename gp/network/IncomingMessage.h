//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#ifndef GPPROTO_INCOMINGMESSAGE_H
#define GPPROTO_INCOMINGMESSAGE_H

#include <cstdint>
#include <cstdlib>
#include "gp/utils/StreamSlice.h"

namespace gpproto
{
    class IncomingMessage {
    public:
        IncomingMessage(int64_t messageId, int32_t seqNo, size_t length, std::shared_ptr<StreamSlice> data)
        : messsageId(messageId), seqNo(seqNo), length(length), data(std::move(data)) {};

        const int64_t messsageId;
        const int32_t seqNo;
        const size_t length;
        const std::shared_ptr<StreamSlice> data;
    };
}

#endif //GPPROTO_INCOMINGMESSAGE_H
