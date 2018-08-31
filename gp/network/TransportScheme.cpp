//
// Created by Jaloliddin Erkiniy on 8/31/18.
//

#include "TransportScheme.h"
using namespace gpproto;

std::shared_ptr<Transport> TransportScheme::createTransportWithContext(std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<TransportDelegate> delegate) const {
    return std::make_shared<TcpTransport>(delegate, context, datacenterId, address);
}