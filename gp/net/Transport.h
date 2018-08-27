//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_TRANSPORT_H
#define GPPROTO_TRANSPORT_H

#include "gp/proto/Context.h"
#include "gp/net/TransportDelegate.h"
#include "gp/proto/DatacenterAddress.h"

namespace gpproto {
    class Transport {
    public:
        Transport(std::shared_ptr<TransportDelegate> delegate, std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress>)
                : context(context),
                  datacenterId(datacenterId),
                  address(address),
                  delegate(delegate)
        {}

        virtual void stop();

        virtual void reset();

        void setDelegate(std::shared_ptr<TransportDelegate> delegate);

        std::shared_ptr<Context> context;
        const int32_t datacenterId;
        std::shared_ptr<DatacenterAddress> address;

    private:
        std::weak_ptr<TransportDelegate> delegate;
    };
}

#endif //GPPROTO_TRANSPORT_H
