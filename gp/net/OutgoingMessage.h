//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#ifndef GPPROTO_OUTGOINGMESSAGE_H
#define GPPROTO_OUTGOINGMESSAGE_H

#include <cstdint>
#include <cstdlib>
#include "gp/utils/StreamSlice.h"

namespace gpproto
{
    class OutgoingMessage {
    public:
        OutgoingMessage(int64_t messageId, int32_t seqNo, std::shared_ptr<StreamSlice> data)
                : messsageId(messageId), seqNo(seqNo), data(std::move(data)) {};

        int64_t messsageId;
        int32_t seqNo;
        const std::shared_ptr<StreamSlice> data;
    };
}

#endif //GPPROTO_OUTGOINGMESSAGE_H
