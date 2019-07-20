//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#ifndef GPPROTO_TIMESYNCMESSAGESERVICE_H
#define GPPROTO_TIMESYNCMESSAGESERVICE_H

#include "gp/proto/MessageService.h"

namespace gpproto
{
    class TimeSyncMessageService;
    class DatacenterSaltsetInfo;

class TimeSyncMessageServiceDelegate {
    public:
        virtual void timeSyncServiceCompleted(const TimeSyncMessageService& service, double timeDifference, std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltlist) = 0;
    };

    class TimeSyncMessageService : public MessageService, public std::enable_shared_from_this<TimeSyncMessageService> {
    public:
        explicit TimeSyncMessageService() : MessageService() {};

        virtual ~TimeSyncMessageService();

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

        void setDelegate(std::shared_ptr<TimeSyncMessageServiceDelegate> delegate);

    private:
        std::weak_ptr<TimeSyncMessageServiceDelegate> delegate;

        int64_t currentMessageId = -1;

        std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltlist;

        double currentSampleAbsoluteStartTime = 0.0;

        std::vector<double> takenSamples;

        int requiredSamplesCount = 0;
    };
}

#endif //GPPROTO_TIMESYNCMESSAGESERVICE_H
