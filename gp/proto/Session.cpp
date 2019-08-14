//
// Created by Jaloliddin Erkiniy on 8/25/18.
//

#include "gp/proto/Session.h"
#include "gp/utils/Logging.h"
#include "gp/proto/ScheduledMessageConfirmation.h"

using namespace gpproto;

int64_t Session::generateClientMessageId(bool &monotonityViolated) {
    auto messageId = (int64_t)(context->getGlobalTime() * 4294967296.0);
    LOGV("[Session generateClientMessageId] -> messageId = %lld, lastClientMessageId = %lld", messageId, lastClientMessageId);

    if (messageId < lastClientMessageId)
        monotonityViolated = true;

    if (messageId == lastClientMessageId)
        messageId = lastClientMessageId + 1;

    while (messageId % 4 != 0)
        messageId++;

    lastClientMessageId = messageId;

    return messageId;
}

int32_t Session::generateClientSeqNo(bool messageIdIsMeaningful) {
    int32_t seqNo = 0;

    if (messageIdIsMeaningful)
    {
        seqNo = this->seqNo + 1;
        this->seqNo += 2;
    }
    else {
        seqNo = this->seqNo;
    }

    return seqNo;
}

void Session::setMessageProcessed(int64_t messageId) {
    processedMessageIds.insert(messageId);
}

bool Session::messageIdProcessed(int64_t messageId) {
    auto it = processedMessageIds.find(messageId);
    return it != processedMessageIds.end();
}

void Session::scheduleMessageConfirmation(int64_t messageId, size_t size) {
    bool found = false;

    for (const auto & item : scheduledConfirmationMessageIds)
    {
        if (item->messageId == messageId) {
            found = true;
            break;
        }
    }

    if (!found)
        scheduledConfirmationMessageIds.push_back(std::make_shared<ScheduledMessageConfirmation>(messageId, size));
}

bool Session::scheduledMessageConfirmationsExceedThreashold(int maxSize) {
    return scheduledConfirmationMessageIds.size() > maxSize;
}

std::vector<int64_t> Session::getScheduledConfirmationMessageIds() const {
    std::vector<int64_t> v;

    for (const auto & item : scheduledConfirmationMessageIds)
        v.push_back(item->messageId);

    return v;
}

void Session::removeScheduledConfirmationMessageIds() {
    scheduledConfirmationMessageIds.clear();
}

void Session::addContainerMessageIdMapping(int64_t containerMessageId, std::vector<int64_t> childMessageIds) {
    containerMessageIdsMapping[containerMessageId] = std::move(childMessageIds);
}

std::vector<int64_t> Session::messageIdsInContainer(int64_t containerMessageId) const {
    auto it = containerMessageIdsMapping.find(containerMessageId);

    if (it != containerMessageIdsMapping.end())
        return it->second;

    return {};
}

int64_t Session::actualClientMessageId() const {
    auto messageId = (int64_t)(context->getGlobalTime() * 4294967296.0);

    while (messageId % 4 != 0) {
        messageId++;
    }

    return messageId;

}

void Session::assignTransactionIdToScheduledMessageConfirmationdIds(int id, std::vector<int64_t> mids) {

    for (const auto & confirmation : scheduledConfirmationMessageIds)
    {
        for (const auto & mid : mids)
        {
            if (mid == confirmation->messageId) {
                confirmation->transactionIds.insert(id);
                break;
            }
        }
    }
}

void Session::removeScheduledConfirmationWithTransactionId(int transactionId) {
    std::vector<std::shared_ptr<ScheduledMessageConfirmation>> newScheduledMessageConfirmations;

    for (const auto & confirmation : scheduledConfirmationMessageIds)
    {
        if (confirmation->transactionIds.find(transactionId) == confirmation->transactionIds.end()) {
            newScheduledMessageConfirmations.push_back(confirmation);
        }
    }

    scheduledConfirmationMessageIds = newScheduledMessageConfirmations;

    LOGV("[Session] Remaining messageConfirmationIds count = %d", scheduledConfirmationMessageIds.size());
}