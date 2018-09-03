//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#ifndef GPPROTO_PROTO_H
#define GPPROTO_PROTO_H

#include <string>
#include "gp/utils/DispatchQueue.h"
#include "gp/proto/ProtoDelegate.h"
#include "gp/network/Transport.h"
#include "gp/proto/Context.h"
#include "gp/proto/Session.h"

namespace gpproto {
    class TransportScheme;
    class IncomingMessage;
    class ProtoInternalMessage;
    class InputStream;

    typedef enum {
        ProtoStateAwaitingTransportScheme = 1,
        ProtoStateAwaitingAuthorization = 2,
        ProtoStateAwaitingTimeFixAndSalts = 4,
        ProtoStateStopped = 8,
        ProtoStatePaused = 16
    } ProtoState;

    class Proto final : public TransportDelegate, public std::enable_shared_from_this<Proto> {
    public:

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.manager");
            return q;
        }

        explicit Proto(std::shared_ptr<Context> context, int32_t datacenterId, bool useUnauthorizedMode = false)
                : useUnauthorizedMode(useUnauthorizedMode),
                  datacenterId(datacenterId),
                  context(std::move(context)), sessionInfo(std::move(Session(context))) { };

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

    private:
        std::shared_ptr<Context> context;
        std::shared_ptr<AuthKeyInfo> authInfo;
        Session sessionInfo;

        uint32_t protoState = 0;

        std::weak_ptr<ProtoDelegate> delegate;

        std::shared_ptr<TransportScheme> transportScheme;

        std::shared_ptr<Transport> transport;

        void setState(int state);

        void setTransport(std::shared_ptr<Transport> transport);

        void resetTransport();

        void allTransactionsMayHaveFailed();

        void updateConnectionState();

        bool isStopped();
        bool isPaused();

        std::shared_ptr<StreamSlice> decryptIncomingTransportData(const std::shared_ptr<StreamSlice>& data);
        std::shared_ptr<IncomingMessage> parseIncomingMessage(std::shared_ptr<StreamSlice> data, int64_t& dataMessageId, bool& parseError);

        std::shared_ptr<ProtoInternalMessage> parseMessage(const std::shared_ptr<StreamSlice>& data);
    };
}

#endif //GPPROTO_PROTO_H
