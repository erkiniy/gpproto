//
// Created by Jaloliddin Erkiniy on 8/28/18.
//

#ifndef GPPROTO_TCPTRANSPORT_H
#define GPPROTO_TCPTRANSPORT_H

#include "gp/net/Transport.h"

namespace gpproto
{
class TcpTransport final : public Transport, public std::enable_shared_from_this<TcpTransport> {
        TcpTransport(std::shared_ptr<TransportDelegate> delegate, std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress> address)
        : Transport(std::move(delegate), std::move(context), datacenterId, std::move(address))
        {};

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.tcpTransport");
            return q;
        }

        void stop();

        void reset();

        void setDelegate(std::shared_ptr<TransportDelegate> delegate);

        void setDelegateNeedsTransaction();
    };
}

#endif //GPPROTO_TCPTRANSPORT_H
