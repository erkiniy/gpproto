//
// Created by Jaloliddin Erkiniy on 8/25/18.
//

#include "gp/proto/Session.h"
#include "gp/utils/Logging.h"

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

void Session::scheduleMessageConfirmation(int64_t messageId) {
    scheduledConfirmationMessageIds.insert(messageId);
}

bool Session::scheduledMessageConfirmationsExceedThreashold(int maxSize) {
    return scheduledConfirmationMessageIds.size() > maxSize;
}

std::vector<int64_t> Session::getScheduledConfirmationMessageIds() const {
    std::vector<int64_t> v = std::vector<int64_t>(scheduledConfirmationMessageIds.size());

    for (auto mid : scheduledConfirmationMessageIds)
        v.push_back(mid);

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
    auto messageId = (int64_t)context->getGlobalTime() + (int64_t)4294967296;

    while (messageId % 4 != 0) {
        messageId++;
    }

    return messageId;

}