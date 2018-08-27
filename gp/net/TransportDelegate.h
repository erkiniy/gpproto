//
// Created by Jaloliddin Erkiniy on 8/25/18.
//

#ifndef GPPROTO_TRANSPORTDELEGATE_H
#define GPPROTO_TRANSPORTDELEGATE_H

#include "gp/utils/StreamSlice.h"
#include "gp/net/Transport.h"
#include "gp/proto/Context.h"
namespace gpproto {
    class Transport;

    class TransportDelegate {
    public:
        virtual void transportNetworkAvailabilityChanged(bool networkIsAvailable) = 0;
        virtual void transportNetworkConnectionStateChanged(bool networkIsConnected) = 0;
        virtual void transportReadyForTransaction(const Transport& transport) = 0;
        virtual void transportHasIncomingData(std::shared_ptr<StreamSlice> data) = 0;
    };
}

#endif //GPPROTO_TRANSPORTDELEGATE_H
