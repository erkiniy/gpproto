//
// Created by Jaloliddin Erkiniy on 9/6/18.
//

#ifndef GPPROTO_PREPAREDMESSAGE_H
#define GPPROTO_PREPAREDMESSAGE_H

#include <cstdint>
#include <memory>

namespace gpproto {
    class StreamSlice;

    class PreparedMessage {
    public:
        PreparedMessage(int internalId, int64_t messageId, int32_t seqNo, int64_t salt, std::shared_ptr<StreamSlice> data, bool requiresConfirmation, bool hasHighpriority)
                : internalId(internalId), messageId(messageId), seqNo(seqNo), salt(salt), data(std::move(data)), requiresConfirmation(requiresConfirmation), hasHighPriority(hasHighpriority) {};

        const int internalId;
        const int64_t messageId;
        const int32_t seqNo;
        const int64_t salt;
        const std::shared_ptr<StreamSlice> data;
        const bool requiresConfirmation;
        const bool hasHighPriority;
    };
}
#endif //GPPROTO_PREPAREDMESSAGE_H
