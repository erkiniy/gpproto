//
// Created by Jaloliddin Erkiniy on 7/27/19.
//

#ifndef GPPROTO_UPDATEMESSAGESERVICE_H
#define GPPROTO_UPDATEMESSAGESERVICE_H

#include <unordered_set>
#include "gp/proto/MessageService.h"

namespace gpproto
{
    class StreamSlice;
    class UpdateMessageService;

    class UpdateMessageServiceDelegate {
        virtual void didReceiveUpdates(const std::shared_ptr<UpdateMessageService>& service, std::shared_ptr<StreamSlice> bytes, int32_t date);
    };

class UpdateMessageService: public MessageService, public std::enable_shared_from_this<UpdateMessageService> {

    public:
        explicit UpdateMessageService(const std::unordered_set<uint32_t> & supportedTypes): supportedTypes(supportedTypes), MessageService() {};

        const std::unordered_set<uint32_t> supportedTypes;

        void protoDidReceiveMessage(const std::shared_ptr<Proto>& proto, std::shared_ptr<IncomingMessage> message) override;
        void protoTransactionsMayHaveFailed(const std::shared_ptr<Proto>& proto, std::vector<int> transactionIds) override;
        void protoMessageDeliveryFailed(const std::shared_ptr<Proto>& proto, int64_t messageId) override;
        void protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto>& proto, std::vector<int64_t> messages) override;
        void protoErrorReceived(const std::shared_ptr<Proto>& proto, int32_t errorCode) override;

        void protoWillAddService(const std::shared_ptr<Proto>& proto) override;
        void protoDidAddService(const std::shared_ptr<Proto>& proto) override;
        void protoWillRemoveService(const std::shared_ptr<Proto>& proto) override;
        void protoDidRemoveService(const std::shared_ptr<Proto>& proto) override;

        void protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto>& proto) override;

        std::shared_ptr<MessageTransaction> protoMessageTransaction(const std::shared_ptr<Proto>& proto) override;

        void protoDidChangeSession(const std::shared_ptr<Proto>& proto) override;
        void protoServerDidChangeSession(const std::shared_ptr<Proto>& proto) override;

        void protoNetworkAvailabilityChanged(const std::shared_ptr<Proto>& proto, bool isNetworkAvailable) override;
        void protoConnectionStateChanged(const std::shared_ptr<Proto>& proto, bool isConnected) override;

        void protoAuthTokenUpdated(const std::shared_ptr<Proto>& proto) override;

        void setDelegate(const std::shared_ptr<UpdateMessageServiceDelegate> & delegate);

    private:
        std::weak_ptr<UpdateMessageServiceDelegate> delegate;
    };
}



#endif //GPPROTO_UPDATEMESSAGESERVICE_H
