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

namespace gpproto {
    class TransportScheme;

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

        explicit Proto(std::shared_ptr<Context> context, int32_t datacenterId, bool useUnauthorizedMode = false) : useUnauthorizedMode(useUnauthorizedMode), datacenterId(datacenterId), context(std::move(context)) {};

        ~Proto() = default;

        void pause();
        void resume();
        void stop();

        void setDelegate(std::shared_ptr<ProtoDelegate> delegate);

        const bool useUnauthorizedMode;

        const int32_t datacenterId;

        void transportNetworkAvailabilityChanged(const Transport& transport, bool networkIsAvailable) override;
        void transportNetworkConnectionStateChanged(const Transport& transport, bool networkIsConnected) override;
        void transportReadyForTransaction(const Transport& transport) override;
        void transportHasIncomingData(const Transport& transport, std::shared_ptr<StreamSlice> data) override;

    private:
        std::shared_ptr<Context> context;

        uint32_t protoState = 0;

        std::weak_ptr<ProtoDelegate> delegate;

        std::shared_ptr<TransportScheme> transportScheme;

        std::shared_ptr<Transport> transport;

        void setState(int state);

        void setTransport(std::shared_ptr<Transport> transport);

        void resetTransport();

        void allTransactionsMayHaveFailed();
    };
}

#endif //GPPROTO_PROTO_H
