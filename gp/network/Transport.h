//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_TRANSPORT_H
#define GPPROTO_TRANSPORT_H

#include "gp/proto/MessageService.h"
#include <atomic>

namespace gpproto
{
    class Context;
    class TransportDelegate;
    class DatacenterAddress;

    class Transport : public MessageService {
    public:
        Transport(std::shared_ptr<TransportDelegate> delegate, std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress> address)
                : MessageService(),
                  internalId(Transport::getNextInternalId()),
                  context(context),
                  datacenterId(datacenterId),
                  address(address),
                  delegate(delegate)
        {};

        virtual ~Transport();

        virtual void stop() = 0;

        virtual void reset() = 0;

        virtual void setDelegateNeedsTransaction() = 0;

        virtual void setDelegate(std::shared_ptr<TransportDelegate> delegate) = 0;

        virtual void updateConnectionState() = 0;

        bool isEqual(const Transport& obj) const {
            return std::addressof(*this) == std::addressof(obj);
        }

        void protoDidReceiveMessage(const std::shared_ptr<Proto> &proto, std::shared_ptr<IncomingMessage> message) override;

        void protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto, std::vector<int> transactionIds) override;

        void protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) override;

        void protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto, std::vector<int64_t> messages) override;

        void protoErrorReceived(const std::shared_ptr<Proto> &proto, int32_t errorCode) override;

        void protoWillAddService(const std::shared_ptr<Proto> &proto) override;

        void protoDidAddService(const std::shared_ptr<Proto> &proto) override;

        void protoWillRemoveService(const std::shared_ptr<Proto> &proto) override;

        void protoDidRemoveService(const std::shared_ptr<Proto> &proto) override;

        void protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) override;

        std::shared_ptr<MessageTransaction> protoMessageTransaction(const std::shared_ptr<Proto> &proto) override;

        void protoDidChangeSession(const std::shared_ptr<Proto> &proto) override;

        void protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) override;

        void protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) override;

        void protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) override;

        void protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) override;

        const int internalId;

        std::shared_ptr<Context> context;

        const int32_t datacenterId;

        std::shared_ptr<DatacenterAddress> address;

    protected:
        std::weak_ptr<TransportDelegate> delegate;
    private:
        static int getNextInternalId() {
            static std::atomic_int internalId = 0;
            return internalId++;
        }
    };
}

#endif //GPPROTO_TRANSPORT_H
