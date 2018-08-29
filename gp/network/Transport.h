//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_TRANSPORT_H
#define GPPROTO_TRANSPORT_H

#include "gp/proto/Context.h"
#include "gp/network/TransportDelegate.h"
#include "gp/proto/DatacenterAddress.h"

namespace gpproto {
    class Transport {
    public:
        Transport(std::shared_ptr<TransportDelegate> delegate, std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress> address)
                : context(std::move(context)),
                  datacenterId(datacenterId),
                  address(std::move(address)),
                  delegate(delegate)
        {};

        virtual void stop() = 0;

        virtual void reset() = 0;

        virtual void setDelegateNeedsTransaction() = 0;

        virtual void setDelegate(std::shared_ptr<TransportDelegate> delegate) = 0;

        std::shared_ptr<Context> context;
        const int32_t datacenterId;
        std::shared_ptr<DatacenterAddress> address;

    protected:
        std::weak_ptr<TransportDelegate> delegate;
    };
}

#endif //GPPROTO_TRANSPORT_H
