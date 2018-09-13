//
// Created by Jaloliddin Erkiniy on 8/29/18.
//

#ifndef GPPROTO_TCPTRANSPORTCONTEXT_H
#define GPPROTO_TCPTRANSPORTCONTEXT_H

#include "gp/net/TcpConnection.h"
#include "gp/proto/DatacenterAddress.h"
#include "gp/network/TcpTransportContextDelegate.h"
#include "gp/utils/DispatchQueue.h"
#include "gp/utils/Timer.h"
namespace gpproto
{
    class TcpConnection;
    class TcpTransportContext : public std::enable_shared_from_this<TcpTransportContext> {
    public:
        explicit TcpTransportContext(std::shared_ptr<DispatchQueue> queue) : queue(std::move(queue)) {};

        std::shared_ptr<TcpConnection> connection;
        std::shared_ptr<DispatchQueue> queue;

        bool waitingForConnectionToBecomeAvailable = false;
        bool requestTransactionWhenBecomesAvailable = false;

        bool connectionIsValid = false;

        bool networkIsAvailable = true;

        bool stopped = false;
        bool connected = false;

        bool needsReconnection = true;

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
