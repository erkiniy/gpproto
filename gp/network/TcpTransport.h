//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#ifndef GPPROTO_TCPTRANSPORT_H
#define GPPROTO_TCPTRANSPORT_H

#include "gp/network/Transport.h"
#include "gp/network/TcpTransportContextDelegate.h"
#include "gp/net/ConnectionDelegate.h"

namespace gpproto
{
    class TcpTransport final : public Transport, public std::enable_shared_from_this<TcpTransport>, public ConnectionDelegate, public TcpTransportContextDelegate {
    public:
        TcpTransport(std::shared_ptr<TransportDelegate> delegate, std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress> address)
        : Transport(delegate, context, datacenterId, address),
          transportContext(std::make_shared<TcpTransportContext>(TcpTransport::queue()))
        {
            transportContext->address = this->address;
            transportContext->networkIsAvailable = true;
        };

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.tcpTransport");
            return q;
        }

        ~TcpTransport() override;

        void initialize();

        void stop() override;

        void reset() override;

        void setDelegate(std::shared_ptr<TransportDelegate> delegate) override;

        void setDelegateNeedsTransaction() override;

        void updateConnectionState() override;

        void tcpConnectionRequestReconnection(const TcpTransportContext& context) override;

        void connectionOpened(const Connection& connection) override;
        void connectionClosed(const Connection& connection) override;
        void connectionDidReceiveData(const Connection& connection, std::shared_ptr<StreamSlice> slice) override;

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

        std::shared_ptr <MessageTransaction> protoMessageTransaction(const std::shared_ptr<Proto> &proto) override;

        void protoDidChangeSession(const std::shared_ptr<Proto> &proto) override;

        void protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) override;

        void protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) override;

        void protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) override;

        void protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) override;

    private:
        void requestTransactionFromDelegate();
        std::shared_ptr<TcpTransportContext> transportContext;

        void startIfNeeded();

        void connectionIsValid();
        void connectionIsInvalid();

        void startActualizationPingResendTimer();
        void stopActualizationPingResendTimer();
        void resendActualizationPing();

    };
}

#endif //GPPROTO_TCPTRANSPORT_H
