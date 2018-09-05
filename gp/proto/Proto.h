//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#ifndef GPPROTO_PROTO_H
#define GPPROTO_PROTO_H

#include "gp/utils/DispatchQueue.h"
#include "gp/proto/ProtoDelegate.h"
#include "gp/network/Transport.h"
#include "gp/proto/Context.h"
#include "gp/proto/Session.h"
#include "gp/proto/TimeSyncMessageService.h"

#include <string>
#include <unordered_set>
#include <unordered_map>

namespace gpproto {
    class TransportScheme;
    class IncomingMessage;
    class ProtoInternalMessage;
    class InputStream;
    class MessageService;
    struct TimeFixContext;

    typedef enum : uint32_t {
        ProtoStateAwaitingTransportScheme = 1,
        ProtoStateAwaitingAuthorization = 2,
        ProtoStateAwaitingTimeFixAndSalts = 4,
        ProtoStateStopped = 8,
        ProtoStatePaused = 16
    } ProtoState;

    class Proto final : public TransportDelegate, public std::enable_shared_from_this<Proto>, public TimeSyncMessageServiceDelegate {
    public:

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.manager");
            return q;
        }

        static const int MaxUnacknowlegedMessagesCount = 2;

        explicit Proto(std::shared_ptr<Context> context, int32_t datacenterId, bool useUnauthorizedMode = false)
                : useUnauthorizedMode(useUnauthorizedMode),
                  datacenterId(datacenterId),
                  context(std::move(context)), sessionInfo(std::make_unique<Session>(context)) {};

        //~Proto() = default;

        void pause();
        void resume();
        void stop();

        void setDelegate(std::shared_ptr<ProtoDelegate> delegate);

        const bool useUnauthorizedMode;

        const int32_t datacenterId;

        void transportNetworkAvailabilityChanged(const Transport& transport, bool networkIsAvailable) override;
        void transportNetworkConnectionStateChanged(const Transport& transport, bool networkIsConnected) override;
        void transportReadyForTransaction(const Transport& transport) override;

        void transportHasIncomingData(const Transport& transport, std::shared_ptr<StreamSlice> data, bool requestTransactionAfterProcessing, std::function<void(bool)> decodeResult) override;

        void timeSyncServiceCompleted(const TimeSyncMessageService& service, double timeDifference, std::vector<DatacenterSaltsetInfo> saltlist) override;

        void requestTransportTransactions();

        void addMessageService(const std::shared_ptr<MessageService>& service);
        void removeMessageService(const std::shared_ptr<MessageService>& service);

    private:
        std::shared_ptr<Context> context;
        std::shared_ptr<AuthKeyInfo> authInfo;
        std::unique_ptr<Session> sessionInfo;

        uint32_t protoState = 0;

        std::unordered_map<int, std::shared_ptr<MessageService>> messageServices;

        std::weak_ptr<ProtoDelegate> delegate;

        std::shared_ptr<TransportScheme> transportScheme;

        std::shared_ptr<Transport> transport;

        bool forceAcks = false;

        bool willRequestTransactionOnNextQueuePass = false;

        double resetSessionInfoLock = 0;

        std::unique_ptr<TimeFixContext> timeFixContext;

        void setState(uint32_t state);

        void setTransport(std::shared_ptr<Transport> transport);

        void resetTransport();

        void allTransactionsMayHaveFailed();

        void updateConnectionState();

        bool isStopped();
        bool isPaused();

        bool canAskTransactions() const;
        bool canAskServiceTransactions() const;
        bool timeFixAndSaltMissing() const;

        std::shared_ptr<StreamSlice> decryptIncomingTransportData(const std::shared_ptr<StreamSlice>& data);
        std::vector<std::shared_ptr<IncomingMessage>> parseIncomingMessages(std::shared_ptr<StreamSlice> data,
                                                                            int64_t &dataMessageId, bool &parseError);
        std::shared_ptr<ProtoInternalMessage> parseMessage(const std::shared_ptr<StreamSlice>& data);

        void processIncomingMessage(const std::shared_ptr<IncomingMessage>& message);

        void initiateTimeSync();
        void completeTimeSync();
        void requestTimeResync();

        void resetSessionInfo();

        void timeSyncInfoChanged(double timeDifference, const std::vector<std::shared_ptr<DatacenterSaltsetInfo>>& saltlist, bool replace);
    };
}

#endif //GPPROTO_PROTO_H
