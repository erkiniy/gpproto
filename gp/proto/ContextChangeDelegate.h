//
// Created by Jaloliddin Erkiniy on 8/31/18.
//

#ifndef GPPROTO_CONTEXTCHANGEDELEGATE_H
#define GPPROTO_CONTEXTCHANGEDELEGATE_H

#include <cstdint>
#include <memory>


namespace gpproto
{
    class Context;
    class TransportScheme;
    class DatacenterAddress;
    class AuthKeyInfo;

    class ContextChangeDelegate {
    public:
        virtual void contextDatacenterTransportSchemeUpdated(std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress> address) = 0;
        virtual void contextDatacenterAuthInfoUpdated(std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<AuthKeyInfo> authKey) = 0;
    };
}

#endif //GPPROTO_CONTEXTCHANGEDELEGATE_H
