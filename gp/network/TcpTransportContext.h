//
// Created by Jaloliddin Erkiniy on 8/29/18.
//

#ifndef GPPROTO_TCPTRANSPORTCONTEXT_H
#define GPPROTO_TCPTRANSPORTCONTEXT_H

#include "gp/net/TcpConnection.h"
#include "gp/proto/DatacenterAddress.h"
#include "gp/network/TcpTransportContextDelegate.h"
#include "gp/utils/DispatchQueue.h"

namespace gpproto
{
    class TcpConnection;
    class TcpTransportContext : public std::enable_shared_from_this<TcpTransportContext> {
    public:
        TcpTransportContext(std::shared_ptr<DispatchQueue> queue) : queue(queue) {};

        std::shared_ptr<TcpConnection> connection;
        std::shared_ptr<DispatchQueue> queue;

        bool waitingForConnectionToBecomeAvailable = false;
        bool requestTransactionWhenBecomesAvailable = false;

        bool networkIsAvailable = true;

        bool stopped = false;
        bool connected = false;

        void requestConnection();
        void startIfNeeded();

        std::shared_ptr<DatacenterAddress> address;

        void setDelegate(std::shared_ptr<TcpTransportContextDelegate> delegate);

    private:
        std::weak_ptr<TcpTransportContextDelegate> delegate;
    };
}

#endif //GPPROTO_TCPTRANSPORTCONTEXT_H
