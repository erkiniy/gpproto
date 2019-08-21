//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#include "gp/proto/RequestMessageService.h"
#include "gp/proto/Request.h"
#include "gp/proto/RequestContext.h"
#include "gp/proto/Proto.h"
#include "gp/proto/InternalParser.h"
#include "gp/proto/ProtoInternalMessage.h"
#include "gp/proto/MessageTransaction.h"
#include "gp/proto/PreparedMessage.h"
#include "gp/proto/gp_client_data.h"
#include "gp/utils/OutputStream.h"
#include "gp/network/IncomingMessage.h"
#include "gp/network/OutgoingMessage.h"
#include "gp/utils/DispatchQueue.h"
#include <algorithm>

using namespace gpproto;

void RequestMessageService::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto, std::shared_ptr<IncomingMessage> message) {
    if (auto rpcResultMessage = std::dynamic_pointer_cast<RpcResultMessage>(message->body)) {

        auto maybeInternalMessage = InternalParser::parseMessage(rpcResultMessage->responseData);

        for (auto it = requests.begin(); it != requests.end(); ++it)
        {
            auto request = *it;
            auto context = request->requestContext;

            if (context == nullptr || context->messageId != rpcResultMessage->requestMessageId)
                continue;

            requests.erase(it);

            if (maybeInternalMessage != nullptr) {
                if (auto rpcErrorMessage = std::dynamic_pointer_cast<RpcError>(maybeInternalMessage)) {
                    LOGE("[RequestMessageService protoDidReceiveMessage] rpcError mid = %lld, code = %d, desc = %s", message->messsageId, rpcErrorMessage->code, rpcErrorMessage->description.c_str());
                    request->failure(rpcErrorMessage->code, rpcErrorMessage->description);
                }
            }
            else {
                auto unwrappedData = InternalParser::unwrapMessage(rpcResultMessage->responseData);
                request->completion(unwrappedData);
            }

            break;
        }
    }
}

void RequestMessageService::protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto,
                                                           std::vector<int> transactionIds) {
    bool requestTransaction = false;

    for (const auto & request : requests)
    {
        if (request->requestContext != nullptr && std::find(transactionIds.begin(), transactionIds.end(), request->requestContext->transactionId) != transactionIds.end()) {
            request->requestContext = nullptr;
            requestTransaction = true;
        }
    }

    if (requestTransaction)
        proto->requestTransportTransactions();
}

void RequestMessageService::protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) {

    bool requestTransaction = false;

    for (const auto & request : requests)
    {
        if (request->requestContext != nullptr) {
            request->requestContext = nullptr;
            requestTransaction = true;
        }
    }

    if (requestTransaction)
        proto->requestTransportTransactions();
}

void RequestMessageService::protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) {
    bool requestTransaction = false;

    for (const auto & request : requests)
    {
        if (request->requestContext != nullptr && request->requestContext->messageId == messageId) {
            request->requestContext = nullptr;
            requestTransaction = true;
        }
    }

    if (requestTransaction)
        proto->requestTransportTransactions();
}

void RequestMessageService::protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto, std::vector<int64_t> messages) {

    for (const auto & mid : messages) {

        for (const auto & request : requests)
        {
            if (request->requestContext != nullptr && request->requestContext->messageId == mid) {
                request->requestContext->delivered = true;
            }
        }
    }
}

void RequestMessageService::protoWillAddService(const std::shared_ptr<Proto> &proto) {

}

void RequestMessageService::protoDidAddService(const std::shared_ptr<Proto> &proto) {
    this->proto = proto;
}

void RequestMessageService::protoWillRemoveService(const std::shared_ptr<Proto> &proto) {

}

void RequestMessageService::protoDidRemoveService(const std::shared_ptr<Proto> &proto) {

}

std::shared_ptr<MessageTransaction> RequestMessageService::protoMessageTransaction(const std::shared_ptr<Proto> &proto) {

    auto messages = std::vector<std::shared_ptr<OutgoingMessage>>();
    bool initializeApi = false;

    std::unordered_map<int, int> requestInternalIdToOutgoingMessageInternalId;

    for (auto const & request : requests)
    {
        if (request->requestContext == nullptr || (!request->requestContext->waitingForMessageId && !request->requestContext->delivered && request->requestContext->transactionId == -1)) {

            int64_t messageId = 0;
            int32 messageSeqNo = 0;

            if (auto context = request->requestContext) {
                messageId = context->messageId;
                messageSeqNo = context->messageSeqNo;
                initializeApi = context->willInitializeApi;
            }

            initializeApi = initializeApi || willInitializeApi;

            willInitializeApi = false;

            auto outgoingMessage = std::make_shared<OutgoingMessage>(messageId, messageSeqNo, true, decorateRequestData(request, initializeApi));
            messages.push_back(outgoingMessage);

            requestInternalIdToOutgoingMessageInternalId[request->internalId] = outgoingMessage->internalId;
        }
    }
    return std::make_shared<MessageTransaction>(messages, [self = shared_from_this(), requestInternalIdToOutgoingMessageInternalId](std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) { //prepared

        for (const auto & request : self->requests)
        {
            auto it = requestInternalIdToOutgoingMessageInternalId.find(request->internalId);
            if (it == requestInternalIdToOutgoingMessageInternalId.end()) continue;

            int messageInternalId = (*it).second;

            auto preparedMessageIt = messageInternalIdToPreparedMessage.find(messageInternalId);
            if (preparedMessageIt == messageInternalIdToPreparedMessage.end()) continue;

            auto preparedMessage = (*preparedMessageIt).second;

            auto requestContext = std::make_shared<RequestContext>(preparedMessage->messageId, preparedMessage->seqNo, -1);
            requestContext->willInitializeApi = false;
            requestContext->waitingForMessageId = true;

            request->requestContext = requestContext;
        }
    }, [self = shared_from_this(), requestInternalIdToOutgoingMessageInternalId] { //failed
        for (const auto & request : self->requests)
        {
            auto it = requestInternalIdToOutgoingMessageInternalId.find(request->internalId);
            if (it == requestInternalIdToOutgoingMessageInternalId.end()) continue;

            int messageInternalId = (*it).second;

            if (auto context = request->requestContext)
                context->waitingForMessageId = false;
        }
    }, [self = shared_from_this(), requestInternalIdToOutgoingMessageInternalId, initializeApi](auto messageInternalIdToTransactionId, std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) { //completed

        for (const auto & request : self->requests)
        {
            if (auto context = request->requestContext)
                context->waitingForMessageId = false;

            auto it = requestInternalIdToOutgoingMessageInternalId.find(request->internalId);
            if (it == requestInternalIdToOutgoingMessageInternalId.end()) continue;

            int messageInternalId = (*it).second;

            auto preparedMessageIt = messageInternalIdToPreparedMessage.find(messageInternalId);
            if (preparedMessageIt == messageInternalIdToPreparedMessage.end()) continue;

            auto preparedMessage = (*preparedMessageIt).second;

            auto messageTransactionIdIt = messageInternalIdToTransactionId.find(messageInternalId);
            if (messageTransactionIdIt == messageInternalIdToTransactionId.end()) continue;

            auto messageTransactionId = (*messageTransactionIdIt).second;

            auto requestContext = std::make_shared<RequestContext>(preparedMessage->messageId, preparedMessage->seqNo, messageTransactionId);
            requestContext->willInitializeApi = initializeApi;

            request->requestContext = requestContext;
        }
    });
}

void RequestMessageService::protoDidChangeSession(const std::shared_ptr<Proto> &proto) {
    willInitializeApi = true;

    LOGV("[RequestMessageService didChangeSession]");

    for (const auto & request : requests)
        request->requestContext = nullptr;

    if (!requests.empty())
        proto->requestTransportTransactions();

}

void RequestMessageService::protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) {
    willInitializeApi = true;

    for (const auto & request : requests)
        request->requestContext = nullptr;

    if (!requests.empty())
        proto->requestTransportTransactions();
}

void RequestMessageService::protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) {

}

void RequestMessageService::protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) {
    willInitializeApi = true;
}

void RequestMessageService::protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) {

}

void RequestMessageService::protoErrorReceived(const std::shared_ptr<Proto>& proto, int32_t errorCode) {

}

void RequestMessageService::addRequest(std::shared_ptr<Request> request) {
    LOGD("addRequest");
    Proto::queue()->async([self = shared_from_this(), request] {
        if (auto proto = self->proto.lock())
        {
            self->requests.push_back(request);
            proto->requestTransportTransactions();
        }
    });
}

void RequestMessageService::cancelRequest(int internalId) {
    removeRequestByInternalId(internalId);
}

void RequestMessageService::removeRequestByInternalId(int internalId) {
    LOGD("removeRequestByInternalId");
    Proto::queue()->async([self = shared_from_this(), internalId] {

        for (auto it = self->requests.begin(); it != self->requests.end(); ++it) {
            if ((*it)->internalId == internalId) {
                self->requests.erase(it);
                break;
            }
        }

        //TODO: delegate
    });
}

std::shared_ptr<StreamSlice> RequestMessageService::decorateRequestData(std::shared_ptr<Request> request, bool initializeApi) {
    auto currentData = request->payload;

    LOGV("[RequestMessageService decorateRequestData] initialize = %d, size = %zu", initializeApi, request->payload->size);

    if (!initializeApi || context->environment == nullptr)
        return currentData;

    auto environment = context->environment;

    OutputStream buffer;
    buffer.writeUInt32(0x416dda56);

    buffer.writeInt32((int32_t)environment->api_id);
    buffer.writeInt32(environment->layer);
    buffer.writeString(environment->device_model);
    buffer.writeString(environment->system_version);
    buffer.writeString(environment->app_version);
    buffer.writeString(environment->lang_code);

    buffer.writeData(*currentData);

    LOGV("[RequestMessageService decorateRequestData] Data inside initConnection %s", currentData->description().c_str());

    currentData = buffer.currentBytes();

    return currentData;
}


