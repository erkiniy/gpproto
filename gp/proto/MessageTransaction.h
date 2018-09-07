//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#ifndef GPPROTO_MESSAGETRANSACTION_H
#define GPPROTO_MESSAGETRANSACTION_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>

namespace gpproto
{
    class OutgoingMessage;
    class PreparedMessage;

    class MessageTransaction {
    public:
        MessageTransaction(std::vector<std::shared_ptr<OutgoingMessage>> payload,
                           std::function<void(std::unordered_map<int, std::shared_ptr<PreparedMessage>>)> prepared,
                           std::function<void()> failed,
                           std::function<void(std::unordered_map<int, std::shared_ptr<PreparedMessage>>)> completed)
                : internalId(MessageTransaction::getNextInternalId()),
                  payload(std::move(payload)),
                  prepared(std::move(prepared)),
                  failed(std::move(failed)),
                  completed(std::move(completed)) {};

        MessageTransaction(const MessageTransaction&) = delete;
        MessageTransaction(MessageTransaction&&) = default;

        const int internalId;
        const std::vector<std::shared_ptr<OutgoingMessage>> payload;
        const std::function<void(std::unordered_map<int, std::shared_ptr<PreparedMessage>>)> prepared;
        const std::function<void()> failed;
        const std::function<void(std::unordered_map<int, std::shared_ptr<PreparedMessage>>)> completed;

    private:
        static int getNextInternalId() {
            static std::atomic_int internalId = 0;
            return internalId++;
        }
    };
}
#endif //GPPROTO_MESSAGETRANSACTION_H
