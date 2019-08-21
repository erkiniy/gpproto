//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#ifndef GPPROTO_PROTO_H
#define GPPROTO_PROTO_H

#include "gp/proto/ProtoDelegate.h"
#include "gp/network/Transport.h"
#include "gp/proto/Context.h"
#include "gp/proto/Session.h"
#include "gp/proto/TimeSyncMessageService.h"
#include "gp/network/TransportDelegate.h"
#include "gp/utils/Logging.h"

#include <string>
#include <unordered_set>
#include <unordered_map>

namespace gpproto
{
    class TransportScheme;
    class IncomingMessage;
    class PreparedMessage;
    class ProtoInternalMessage;
    class InputStream;
    class MessageService;
    class OutputStream;
    class DispatchQueue;
    struct TimeFixContext;

    typedef enum : uint32_t {
        ProtoStateAwaitingTransportScheme = 1,
        ProtoStateAwaitingAuthorization = 2,
        ProtoStateAwaitingTimeFixAndSalts = 4,
        ProtoStateStopped = 8,
        ProtoStatePaused = 16
    } ProtoState;

    class Proto final : public std::enable_shared_from_this<Proto>, public TransportDelegate, public TimeSyncMessageServiceDelegate, public ContextChangeListener {
    public:

        static std::shared_ptr<DispatchQueue> queue();

        static const int MaxUnacknowlegedMessagesCount = 2;

        explicit Proto(std::shared_ptr<Context> context, int32_t datacenterId, bool useUnauthorizedMode = false);

        ~Proto() {
            LOGV("[Proto deallocating]");
            auto strongSelf = shared_from_this();
            this->context->removeChangeListener(strongSelf);
        };

        void initialize();

        void pause();
        void resume();
        void stop();

        double getGlobalTime();

        void setDelegate(std::shared_ptr<ProtoDelegate> delegate);

        const bool useUnauthorizedMode;

        const int32_t datacenterId;

        void contextDatacenterAddressSetUpdated(const Context& context, int32_t datacenterId, std::vector<std::shared_ptr<DatacenterAddress>> addressSet) override;
        void contextDatacenterAuthInfoUpdated(const Context& context, int32_t datacenterId, std::shared_ptr<AuthKeyInfo> authInfo) override;
        void contextDatacenterTransportSchemeUpdated(const Context& context, int32_t datacenterId, std::shared_ptr<TransportScheme> scheme) override;

        void transportNetworkAvailabilityChanged(const Transport& transport, bool networkIsAvailable) override;
        void transportNetworkConnectionStateChanged(const Transport& transport, bool networkIsConnected) override;
        void transportReadyForTransaction(const Transport& transport, std::shared_ptr<MessageTransaction> transportSpecificTransaction, std::function<void(std::vector<std::shared_ptr<TransportTransaction>>)> transactionsReady) override;

        void transportHasIncomingData(const Transport& transport, std::shared_ptr<StreamSlice> data, bool requestTransactionAfterProcessing, std::function<void(bool)> decodeResult) override;

        void timeSyncServiceCompleted(const TimeSyncMessageService& service, double timeDifference, std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltlist) override;

        void requestTransportTransactions();

        void addMessageService(std::shared_ptr<MessageService> service);
        void removeMessageService(const std::shared_ptr<MessageService>& service);

    private:
        std::shared_ptr<Context> context;
        std::shared_ptr<AuthKeyInfo> authInfo;
        std::shared_ptr<Session> sessionInfo;

        uint32_t protoState = 0;

        std::unordered_map<int, std::shared_ptr<MessageService>> messageServices;

        std::weak_ptr<ProtoDelegate> delegate;

        std::shared_ptr<TransportScheme> transportScheme;

        std::shared_ptr<Transport> transport;

        bool forceAcks = false;

        bool willRequestTransactionOnNextQueuePass = false;

        double resetSessionInfoLock = 0;

        std::shared_ptr<TimeFixContext> timeFixContext;

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

        std::shared_ptr<StreamSlice> decryptIncomingTransportData(const std::shared_ptr<StreamSlice>& data) const;
        std::vector<std::shared_ptr<IncomingMessage>> parseIncomingMessages(std::shared_ptr<StreamSlice> data,
                                                                            int64_t &dataMessageId, bool &parseError);
        std::shared_ptr<ProtoInternalMessage> parseMessage(const std::shared_ptr<StreamSlice>& data);

        std::shared_ptr<StreamSlice> dataForEncryptedMessage(const std::shared_ptr<PreparedMessage>& message, const std::shared_ptr<Session>& session) const;
        std::shared_ptr<StreamSlice> dataForEncryptedContainer(const std::vector<std::shared_ptr<PreparedMessage>>& messages, const std::shared_ptr<Session>& session) const;
        std::shared_ptr<StreamSlice> dataForPlainMessage(const std::shared_ptr<PreparedMessage>& message) const;
        void paddedData(OutputStream& os) const;

        void processIncomingMessage(const std::shared_ptr<IncomingMessage>& message);

        void initiateTimeSync();
        void completeTimeSync();
        void requestTimeResync();

        void resetSessionInfo();

        void timeSyncInfoChanged(double timeDifference, const std::vector<std::shared_ptr<DatacenterSaltsetInfo>>& saltlist, bool replace);

        void transportTransactionsSucceeded(int transcationId);

    };
}

#endif //GPPROTO_PROTO_H
