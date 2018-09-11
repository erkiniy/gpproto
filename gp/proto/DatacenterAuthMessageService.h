//
// Created by Jaloliddin Erkiniy on 9/7/18.
//

#ifndef GPPROTO_DATACENTERAUTHMESSAGESERVICE_H
#define GPPROTO_DATACENTERAUTHMESSAGESERVICE_H

#include "gp/proto/MessageService.h"
#include <unordered_map>
#include <string>

namespace gpproto
{
    class AuthKeyInfo;
    class DatacenterAuthMessageService;
    class Context;
    class StreamSlice;

    class DatacenterAuthMessageServiceDelegate {
    public:
        virtual void authMessageServiceCompletedWithAuthKey(const DatacenterAuthMessageService& service, std::shared_ptr<AuthKeyInfo> authKey, int64_t timestamp) = 0;
    };

    enum class DatacenterAuthStage {
        pq = 1,
        reqDh = 2,
        keyVerification = 3,
        done = 4
    };

class DatacenterAuthMessageService : public MessageService, public std::enable_shared_from_this<DatacenterAuthMessageService> {
    public:
        explicit DatacenterAuthMessageService(const std::shared_ptr<Context>& context);
        void setDelegate(const std::shared_ptr<DatacenterAuthMessageServiceDelegate>& delegate) {
            this->delegate = delegate;
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

    private:
        std::weak_ptr<DatacenterAuthMessageServiceDelegate> delegate;

        void reset(std::shared_ptr<Proto> proto);

        int64_t currentStateMessageId = 0;
        int32_t currentStateMessageSeqNo = 0;
        int currentStateTransactionId = 0;

        std::shared_ptr<StreamSlice> nonce;
        std::shared_ptr<StreamSlice> serverNonce;
        std::shared_ptr<StreamSlice> newNonce;

        std::shared_ptr<StreamSlice> dhP;
        std::shared_ptr<StreamSlice> dhQ;
        uint64_t publicKeyFingerprint;

        std::shared_ptr<StreamSlice> dhEncryptedData;
        std::shared_ptr<AuthKeyInfo> authKey;

        std::shared_ptr<StreamSlice> encryptedClientData;
        std::unordered_map<uint64_t, std::string> publicKeys;

        DatacenterAuthStage stage = DatacenterAuthStage::pq;
    };
}

#endif //GPPROTO_DATACENTERAUTHMESSAGESERVICE_H
