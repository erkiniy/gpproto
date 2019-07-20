//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#include "gp/proto/TimeSyncMessageService.h"
#include "gp/proto/DatacenterSaltsetInfo.h"
#include "gp/proto/Proto.h"
#include "gp/network/OutgoingMessage.h"
#include "gp/utils/OutputStream.h"
#include "gp/utils/Common.h"
#include "gp/proto/MessageTransaction.h"
#include "gp/proto/PreparedMessage.h"
#include "gp/proto/ProtoInternalMessage.h"
#include "gp/network/IncomingMessage.h"

#include <unordered_set>

using namespace gpproto;


TimeSyncMessageService::~TimeSyncMessageService() {
    LOGV("TimeSyncMessageService deinitialized");
}

std::shared_ptr<MessageTransaction> TimeSyncMessageService::protoMessageTransaction(const std::shared_ptr<Proto> &proto) {
    if (currentMessageId < 0)
    {
        OutputStream getFutureSaltsBuffer;

        getFutureSaltsBuffer.writeUInt32(0xb921bd04);
        getFutureSaltsBuffer.writeInt32(saltlist.size() != 0 ? 1 : 32);

        auto outgoingMessage = std::make_shared<OutgoingMessage>(0, 0, true, getFutureSaltsBuffer.currentBytes());

        return std::make_shared<MessageTransaction>(std::initializer_list<std::shared_ptr<OutgoingMessage>>{outgoingMessage},
                [self = shared_from_this(), internalId = outgoingMessage->internalId](std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {
                auto it = messageInternalIdToPreparedMessage.find(internalId);
                if (it != messageInternalIdToPreparedMessage.end()) {
                    self->currentMessageId = messageInternalIdToPreparedMessage[internalId]->messageId;
                    LOGV("[TimeSyncMessageService protoMessageTransaction] preparedMessageId = %lld", self->currentMessageId);
                }
            },
                []{}, [self = shared_from_this(), internalId = outgoingMessage->internalId](auto messageInternalIdToTransactionId, std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {
                    self->currentSampleAbsoluteStartTime = getAbsoluteSystemTime();
             });
    }

    return nullptr;
}

void TimeSyncMessageService::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto, std::shared_ptr<IncomingMessage> message) {
    if (const auto & futureSaltsMessage = std::dynamic_pointer_cast<FutureSaltsMessage>(message->body))
    {
        if (futureSaltsMessage->requestMessageId != currentMessageId)
            return;

        currentMessageId = -1;

        for (const auto & salt : futureSaltsMessage->salts)
            saltlist.emplace_back(std::make_shared<DatacenterSaltsetInfo>(salt->salt, (int64_t)salt->validSince * 4294967296, (int64_t)salt->validUntil * 4294967296));

        TimeInterval timeDifference = message->messsageId / 4294967296.0 - getAbsoluteSystemTime();
        takenSamples.push_back(timeDifference);

        LOGV("[TimeSyncMessageService protoDidReceiveMessage] Time sync message received timestamp difference %lf", timeDifference);
        bool requestTransaction = false;

        if (requiredSamplesCount == 0)
        {
            if (fabs(getAbsoluteSystemTime() - currentSampleAbsoluteStartTime) > 1)
            {
                LOGV("[TimeSyncMessageService protoDidReceiveMessage] Time interval for getting future salts is more than 1 secs");
                requiredSamplesCount = 6;
                requestTransaction = true;
            }
        }

        if (takenSamples.size() >= requiredSamplesCount)
        {
            LOGV("[TimeSyncMessageService protoDidReceiveMessage] Received required %zu samples", takenSamples.size());

            TimeInterval maxSampleAbs = 0;
            int maxSampleIndex = -1;

            TimeInterval minSampleAbs = 0;
            int minSampleIndex = -1;

            int index = -1;

            for (const auto & sample : takenSamples)
            {
                index++;

                if (maxSampleIndex == -1 || abs(sample) > maxSampleAbs)
                {
                    maxSampleAbs = abs(sample);
                    maxSampleIndex = index;
                }

                if (minSampleIndex == -1 || abs(sample) < minSampleAbs)
                {
                    minSampleAbs = abs(sample);
                    minSampleIndex = index;
                }
            }

            std::unordered_set<int> indexesToRemove;

            if (maxSampleIndex != -1)
                indexesToRemove.insert(maxSampleIndex);

            if (minSampleIndex != -1)
                indexesToRemove.insert(minSampleIndex);

            TimeInterval totalTimeDifference = 0;
            std::vector<TimeInterval> newSamples;

            index = -1;
            for (const auto & sample : takenSamples)
            {
                index++;

                if (indexesToRemove.find(index) == indexesToRemove.end())
                    newSamples.push_back(sample);
            }

            if (takenSamples.size() > 2)
                takenSamples = newSamples;

            if (!takenSamples.empty())
            {
                TimeInterval differenceSum = 0.0;
                for (const auto & sample : takenSamples)
                    differenceSum += sample;

                totalTimeDifference = differenceSum / (TimeInterval)takenSamples.size();
            }
            else {
                totalTimeDifference = timeDifference;
            }

            if (auto strongDelegate = delegate.lock())
                strongDelegate->timeSyncServiceCompleted(*this, totalTimeDifference, std::move(saltlist));
        }
        else
            requestTransaction = true;

        if (requestTransaction)
            proto->requestTransportTransactions();
    }
}

void TimeSyncMessageService::protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto, std::vector<int> transactionIds) {

}

void TimeSyncMessageService::protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) {
    LOGE("[TimeSyncMessageService protoMessageDeliveryFailed] time sync failed with messageId = %lld, my messageId = %lld", messageId, currentMessageId);

    if (currentMessageId == messageId)
    {
        currentMessageId = -1;
        currentSampleAbsoluteStartTime = 0.0;
        proto->requestTransportTransactions();
    }
}

void TimeSyncMessageService::protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto,
                                                            std::vector<int64_t> messages) {
    LOGV("[TimeSyncMessageService protoMessagesDeliveryConfirmed] delivery confirmed");
}

void TimeSyncMessageService::protoErrorReceived(const std::shared_ptr<Proto> &proto, int32_t errorCode) {

}

void TimeSyncMessageService::protoWillAddService(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoDidAddService(const std::shared_ptr<Proto> &proto) {
    proto->requestTransportTransactions();
}

void TimeSyncMessageService::protoWillRemoveService(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoDidRemoveService(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) {
    currentMessageId = -1;
    proto->requestTransportTransactions();
}

void TimeSyncMessageService::protoDidChangeSession(const std::shared_ptr<Proto> &proto) {
    currentMessageId = -1;
    proto->requestTransportTransactions();
}

void TimeSyncMessageService::protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) {
    currentMessageId = -1;
    proto->requestTransportTransactions();
}

void TimeSyncMessageService::protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) {

}

void TimeSyncMessageService::protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) {

}

void TimeSyncMessageService::protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::setDelegate(std::shared_ptr<TimeSyncMessageServiceDelegate> delegate) {
    this->delegate = delegate;
}
