//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#include "gp/proto/RequestMessageService.h"
#include "Proto.h"

using namespace gpproto;

void RequestMessageService::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto,
                                                   std::shared_ptr<IncomingMessage> message) {

}

void RequestMessageService::protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto,
                                                           std::vector<int> transactionIds) {

}

void RequestMessageService::protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) {

}

void RequestMessageService::protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto, std::vector<int64_t> messages) {

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

void RequestMessageService::protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) {

}

std::shared_ptr<MessageTransaction> RequestMessageService::protoMessageTransaction(const std::shared_ptr<Proto> &proto) {
    return std::shared_ptr<MessageTransaction>();
}

void RequestMessageService::protoDidChangeSession(const std::shared_ptr<Proto> &proto) {

}

void RequestMessageService::protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) {

}

void RequestMessageService::protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) {

}

void RequestMessageService::protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) {

}

void RequestMessageService::protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) {

}

void RequestMessageService::protoErrorReceived(const std::shared_ptr<Proto>& proto, int32_t errorCode) {

}

void RequestMessageService::addRequest(std::shared_ptr<Request> request) {
    Proto::queue()->async([self = shared_from_this()] {
        if (auto proto = this->proto.lock())
        {
            requests.push_back(request);
            proto->requestTransportTransactions();
        }
    });
}

void RequestMessageService::cancelRequest(int internalId) {
    removeRequestByInternalId(internalId);
}

void RequestMessageService::removeRequestByInternalId(int internalId) {
    Proto::queue()->async([self = shared_from_this()] {

    });
}


