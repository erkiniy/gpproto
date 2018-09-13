//
// Created by Jaloliddin Erkiniy on 8/29/18.
//

#ifndef GPPROTO_TCPTRANSPORTCONTEXT_H
#define GPPROTO_TCPTRANSPORTCONTEXT_H

#include <memory>
namespace gpproto
{
    class TcpConnection;
    class TcpTransportContextDelegate;
    class DispatchQueue;
    class DatacenterAddress;
    class Timer;

    class TcpTransportContext : public std::enable_shared_from_this<TcpTransportContext> {
    public:
        explicit TcpTransportContext(std::shared_ptr<DispatchQueue> queue) : queue(std::move(queue)) {};

        std::shared_ptr<TcpConnection> connection;
        std::shared_ptr<DispatchQueue> queue;

        bool waitingForConnectionToBecomeAvailable = false;

        bool requestTransactionWhenBecomesAvailable = false;

        bool willRequestTransactionOnNextQueuePass = false;

        bool didSendActualizationPingAfterConnection = false;

        bool connectionIsValid = false;

        bool networkIsAvailable = true;

        bool stopped = false;

        bool connected = false;

        bool needsReconnection = true;

        double transactionLockTime = 0.0;

        int64_t currentServerPingId = 0.0;
        int64_t currentServerPingMessageId = 0.0;

        int64_t currentActualizationPingMessageId = 0.0;

        std::shared_ptr<Timer> actualizationPingResendTimer;

        void requestConnection();
        void startIfNeeded();

        std::shared_ptr<DatacenterAddress> address;

        void setDelegate(std::shared_ptr<TcpTransportContextDelegate> delegate);

        void connectionClosed();

        void invalidateTimer();

        void connectionValidDataReceived();

    private:
        std::weak_ptr<TcpTransportContextDelegate> delegate;
        int backoffCount = 0;

        void startTimer(double timeout);

        void timerEvent();

        std::shared_ptr<Timer> backoffTimer;
    };
}

#endif //GPPROTO_TCPTRANSPORTCONTEXT_H
