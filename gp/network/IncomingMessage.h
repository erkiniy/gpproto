//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#ifndef GPPROTO_INCOMINGMESSAGE_H
#define GPPROTO_INCOMINGMESSAGE_H

#include <cstdint>
#include <cstdlib>
#include <memory>

namespace gpproto
{
    class ProtoInternalMessage;
    class IncomingMessage {
    public:
        IncomingMessage(int64_t messageId, int32_t seqNo, int32_t timestamp, size_t length, std::shared_ptr<ProtoInternalMessage> body)
        : messsageId(messageId), seqNo(seqNo), timestamp(timestamp), length(length), body(std::move(body)) {};

        const int64_t messsageId;
        const int32_t seqNo;
        const int32_t timestamp;
        const size_t length;
        const std::shared_ptr<ProtoInternalMessage> body;
    };
}

#endif //GPPROTO_INCOMINGMESSAGE_H
