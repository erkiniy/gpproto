//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#ifndef GPPROTO_TCPTRANSPORT_H
#define GPPROTO_TCPTRANSPORT_H

#include "gp/network/Transport.h"
#include "gp/network/TcpTransportContext.h"
#include "gp/network/TcpTransportContextDelegate.h"
#include "gp/net/TcpConnection.h"
#include "gp/utils/Timer.h"

namespace gpproto
{
    class TcpTransport final : public Transport, public std::enable_shared_from_this<TcpTransport>, public ConnectionDelegate, public TcpTransportContextDelegate {
    public:
        TcpTransport(std::shared_ptr<TransportDelegate> delegate, std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress> address)
        : Transport(std::move(delegate), std::move(context), datacenterId, std::move(address)),
          transportContext(std::make_shared<TcpTransportContext>(TcpTransport::queue()))
        {
            TcpTransport::queue()->asyncForce([this] {
                transportContext->address = this->address;
                auto strongSelf = shared_from_this();
                auto contextDelegate = std::static_pointer_cast<TcpTransportContextDelegate>(strongSelf);

                transportContext->setDelegate(contextDelegate);

                transportContext->networkIsAvailable = true;
            });
        };

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.tcpTransport");
            return q;
        }

        ~TcpTransport() override;

        void stop() override;

        void reset() override;

        void setDelegate(std::shared_ptr<TransportDelegate> delegate) override;

        void setDelegateNeedsTransaction() override;

        void updateConnectionState() override;

        void tcpConnectionRequestReconnection(const TcpTransportContext& context) override;

        void connectionOpened(const Connection& connection) override;
        void connectionClosed(const Connection& connection) override;
        void connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) override;

    private:
        void requestTransactionFromDelegate();
        std::shared_ptr<TcpTransportContext> transportContext;

        void startIfNeeded();

        void connectionIsValid();
        void connectionIsInvalid();
    };
}

#endif //GPPROTO_TCPTRANSPORT_H
