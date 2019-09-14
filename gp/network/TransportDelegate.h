//
// Created by Jaloliddin Erkiniy on 8/25/18.
//

#ifndef GPPROTO_TRANSPORTDELEGATE_H
#define GPPROTO_TRANSPORTDELEGATE_H

#include "gp/utils/StreamSlice.h"
#include "gp/network/Transport.h"
#include "gp/proto/Context.h"

namespace gpproto
{
    class Transport;
    class MessageTransaction;
    class TransportTransaction;

    class TransportDelegate {
    public:
        virtual void transportNetworkAvailabilityChanged(const Transport& transport, bool networkIsAvailable) = 0;
        virtual void transportNetworkConnectionStateChanged(const Transport& transport, bool networkIsConnected) = 0;
        virtual void transportReadyForTransaction(const Transport& transport, std::shared_ptr<MessageTransaction> transportSpecificTransaction, std::function<void(std::vector<std::shared_ptr<TransportTransaction>>)> transactionsReady) = 0;
        virtual void transportHasIncomingData(const Transport& transport, std::shared_ptr<StreamSlice> data, bool requestTransactionAfterProcessing, std::function<void(bool)> decodeResult) = 0;
        virtual void transportTransactionsMayHaveFailed(const Transport& transport, int transactionId) = 0;
    };
}

#endif //GPPROTO_TRANSPORTDELEGATE_H
