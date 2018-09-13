//
// Created by Jaloliddin Erkiniy on 8/31/18.
//

#ifndef GPPROTO_TRANSPORTSCHEME_H
#define GPPROTO_TRANSPORTSCHEME_H

#include <memory>

namespace gpproto {

    class DatacenterAddress;
    class TcpTransport;
    class Transport;
    class TransportDelegate;
    class Context;

    enum class TransportType {
        Tcp = 1,
        Http = 2
    };

    class TransportScheme {
    public:
        TransportScheme(TransportType type, std::shared_ptr<DatacenterAddress> address) : type(type), address(std::move(address)) {};

        std::shared_ptr<Transport> createTransportWithContext(std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<TransportDelegate> delegate) const;
        const TransportType type;
        const std::shared_ptr<DatacenterAddress> address;
    };
}

#endif //GPPROTO_TRANSPORTSCHEME_H
