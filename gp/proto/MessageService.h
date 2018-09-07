//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#ifndef GPPROTO_MESSAGESERVICE_H
#define GPPROTO_MESSAGESERVICE_H

#include <memory>
#include <vector>
#include <cstdint>
#include <atomic>

namespace gpproto
{
    class Proto;
    class IncomingMessage;
    class MessageTransaction;

    class MessageService {
    public:

        explicit MessageService() : internalId(MessageService::getNextInternalId()) {};

        const int internalId;

        virtual void protoDidReceiveMessage(const std::shared_ptr<Proto>& proto, std::shared_ptr<IncomingMessage> message) = 0;
        virtual void protoTransactionsMayHaveFailed(const std::shared_ptr<Proto>& proto, std::vector<int> transactionIds) {};
        virtual void protoMessageDeliveryFailed(const std::shared_ptr<Proto>& proto, int64_t messageId) {};
        virtual void protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto>& proto, std::vector<int64_t> messages) {};
        virtual void protoErrorReceived(const std::shared_ptr<Proto>& proto, int32_t errorCode) {};

        virtual void protoWillAddService(const std::shared_ptr<Proto>& proto) {};
        virtual void protoDidAddService(const std::shared_ptr<Proto>& proto) {};
        virtual void protoWillRemoveService(const std::shared_ptr<Proto>& proto) {};
        virtual void protoDidRemoveService(const std::shared_ptr<Proto>& proto) {};

        virtual void protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto>& proto) {};

        virtual std::shared_ptr<MessageTransaction> protoMessageTransaction(const std::shared_ptr<Proto>& proto) { return nullptr; };

        virtual void protoDidChangeSession(const std::shared_ptr<Proto>& proto) {};
        virtual void protoServerDidChangeSession(const std::shared_ptr<Proto>& proto) {};

        virtual void protoNetworkAvailabilityChanged(const std::shared_ptr<Proto>& proto, bool isNetworkAvailable) {};
        virtual void protoConnectionStateChanged(const std::shared_ptr<Proto>& proto, bool isConnected) {};

        virtual void protoAuthTokenUpdated(const std::shared_ptr<Proto>& proto) {};

        virtual ~MessageService() = default;

    private:
        static int getNextInternalId() {
            static std::atomic_int sharedInternalId = 0;
            return sharedInternalId++;
        }
    };
}
#endif //GPPROTO_MESSAGESERVICE_H
