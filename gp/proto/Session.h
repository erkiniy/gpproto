//
// Created by Jaloliddin Erkiniy on 8/25/18.
//

#ifndef GPPROTO_SESSION_H
#define GPPROTO_SESSION_H

#include <cstdint>
#include <unordered_set>
#include "gp/utils/Random.h"
#include "Context.h"

namespace gpproto {
    class Session final {
    public:
        explicit Session(std::shared_ptr<Context> context)
                : id(Random::secureInt64()),
                  context(context),
                  seqNo(0),
                  lastClientMessageId(0),
                  lastServerMessageId(0)
        {};

        Session(const Session&) = delete;

        Session(Session &&) = default;

        ~Session() = default;
        const int64_t id;

        int64_t generateClientMessageId(bool& monotonityViolated);
        int32_t generateClientSeqNo(bool messageIdIsMeaningful);

        void setMessageProcessed(int64_t messageId);
        bool messageIdProcessed(int64_t messageId);

        void scheduleMessageConfirmation(int64_t messageId);
        bool scheduledMessageConfirmationsExceedThreashold(int maxSize);
        std::vector<int64_t> getScheduledConfirmationMessageIds() const;
        void removeScheduledConfirmationMessageIds();

        void addContainerMessageIdMapping(int64_t containerMessageId, std::vector<int64_t> childMessageIds);
        std::vector<int64_t> messageIdsInContainer(int64_t containerMessageId) const;

        int64_t actualClientMessageId() const;

    private:
        std::shared_ptr<Context> context;
        int32_t seqNo = 0;
        int64_t lastClientMessageId = 0;
        int64_t lastServerMessageId = 0;
        std::unordered_set<int64_t> processedMessageIds;
        std::unordered_set<int64_t> scheduledConfirmationMessageIds;
        std::unordered_map<int64_t, std::vector<int64_t>> containerMessageIdsMapping;
    };
}


#endif //GPPROTO_SESSION_H
