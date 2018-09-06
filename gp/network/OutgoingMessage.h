//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#ifndef GPPROTO_OUTGOINGMESSAGE_H
#define GPPROTO_OUTGOINGMESSAGE_H

#include <cstdint>
#include <cstdlib>

namespace gpproto
{
    class StreamSlice;
    class OutgoingMessage {
    public:
        OutgoingMessage(int64_t messageId, int32_t seqNo, bool requiresConfirmation, std::shared_ptr<StreamSlice> data)
                : internalId(OutgoingMessage::getNextInternalId()), messsageId(messageId), seqNo(seqNo), data(std::move(data)), requiresConfirmation(requiresConfirmation) {};

        const int internalId;
        const int64_t messsageId;
        const int32_t seqNo;
        const std::shared_ptr<StreamSlice> data;
        const bool requiresConfirmation;

    private:
        static int getNextInternalId() {
            static int id = 0;
            return id++;
        }
    };
}

#endif //GPPROTO_OUTGOINGMESSAGE_H
