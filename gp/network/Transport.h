//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_TRANSPORT_H
#define GPPROTO_TRANSPORT_H

#include "gp/proto/Context.h"
#include "gp/network/TransportDelegate.h"
#include "gp/proto/DatacenterAddress.h"

#include <atomic>

namespace gpproto
{
    class Transport {
    public:
        Transport(std::shared_ptr<TransportDelegate> delegate, std::shared_ptr<Context> context, int32_t datacenterId, std::shared_ptr<DatacenterAddress> address)
                : internalId(Transport::getNextInternalId()),
                  context(std::move(context)),
                  datacenterId(datacenterId),
                  address(std::move(address)),
                  delegate(delegate)
        {};

        virtual ~Transport() {};

        virtual void stop() = 0;

        virtual void reset() = 0;

        virtual void setDelegateNeedsTransaction() = 0;

        virtual void setDelegate(std::shared_ptr<TransportDelegate> delegate) = 0;

        virtual void updateConnectionState() = 0;

        bool isEqual(const Transport& obj) const {
            return std::addressof(*this) == std::addressof(obj);
        }

        const int internalId;
        std::shared_ptr<Context> context;
        const int32_t datacenterId;
        std::shared_ptr<DatacenterAddress> address;

    protected:
        std::weak_ptr<TransportDelegate> delegate;
    private:
        static int getNextInternalId() {
            static std::atomic_int internalId = 0;
            return internalId++;
        }
    };
}

#endif //GPPROTO_TRANSPORT_H
