//
// Created by Jaloliddin Erkiniy on 8/9/19.
//

#ifndef GPPROTO_SCHEDULEDMESSAGECONFIRMATION_H
#define GPPROTO_SCHEDULEDMESSAGECONFIRMATION_H

#endif //GPPROTO_SCHEDULEDMESSAGECONFIRMATION_H

#include <unordered_set>

namespace gpproto
{
    class ScheduledMessageConfirmation {
    public:
        explicit ScheduledMessageConfirmation(int64_t messageId, size_t size): messageId(messageId), size(size) {};

        const int64_t messageId;
        const size_t size;
        std::unordered_set<int> transactionIds;
    };
}