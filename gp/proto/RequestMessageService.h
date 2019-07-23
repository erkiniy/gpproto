//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#ifndef GPPROTO_REQUESTMESSAGESERVICE_H
#define GPPROTO_REQUESTMESSAGESERVICE_H

#include "gp/proto/MessageService.h"

namespace gpproto
{
    class Context;
    class Proto;
    class Request;
    class StreamSlice;

    class RequestMessageService : public MessageService, public std::enable_shared_from_this<RequestMessageService> {

    public:
        explicit RequestMessageService(const std::shared_ptr<Context>& context) : context(context), MessageService() {};

        virtual void protoDidReceiveMessage(const std::shared_ptr<Proto>& proto, std::shared_ptr<IncomingMessage> message) override;
        virtual void protoTransactionsMayHaveFailed(const std::shared_ptr<Proto>& proto, std::vector<int> transactionIds) override;
        virtual void protoMessageDeliveryFailed(const std::shared_ptr<Proto>& proto, int64_t messageId) override;
        virtual void protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto>& proto, std::vector<int64_t> messages) override;
        virtual void protoErrorReceived(const std::shared_ptr<Proto>& proto, int32_t errorCode) override;

        virtual void protoWillAddService(const std::shared_ptr<Proto>& proto) override;
        virtual void protoDidAddService(const std::shared_ptr<Proto>& proto) override;
        virtual void protoWillRemoveService(const std::shared_ptr<Proto>& proto) override;
        virtual void protoDidRemoveService(const std::shared_ptr<Proto>& proto) override;

        virtual void protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto>& proto) override;

        virtual std::shared_ptr<MessageTransaction> protoMessageTransaction(const std::shared_ptr<Proto>& proto) override;

        virtual void protoDidChangeSession(const std::shared_ptr<Proto>& proto) override;
        virtual void protoServerDidChangeSession(const std::shared_ptr<Proto>& proto) override;

        virtual void protoNetworkAvailabilityChanged(const std::shared_ptr<Proto>& proto, bool isNetworkAvailable) override;
        virtual void protoConnectionStateChanged(const std::shared_ptr<Proto>& proto, bool isConnected) override;

        virtual void protoAuthTokenUpdated(const std::shared_ptr<Proto>& proto) override;

        void addRequest(std::shared_ptr<Request> request);
        void cancelRequest(int internalId);


    private:
        std::shared_ptr<Context> context;
        std::weak_ptr<Proto> proto;
        std::vector<std::shared_ptr<Request>> requests;

        void removeRequestByInternalId(int internalId);
        std::shared_ptr<StreamSlice> decorateRequestData(std::shared_ptr<Request> request, bool initializeApi);
    };
}

#endif //GPPROTO_REQUESTMESSAGESERVICE_H
