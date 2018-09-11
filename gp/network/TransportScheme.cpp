//
// Created by Jaloliddin Erkiniy on 8/31/18.
//

#include "gp/network/TransportScheme.h"
using namespace gpproto;

std::shared_ptr<Transport> TransportScheme::createTransportWithContext(std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<TransportDelegate> delegate) const {
    auto transport = std::make_shared<TcpTransport>(delegate, context, datacenterId, address);
    transport->initialize();
    return transport;
}