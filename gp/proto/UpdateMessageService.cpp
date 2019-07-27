//
// Created by Jaloliddin Erkiniy on 7/27/19.
//

#include "gp/proto/UpdateMessageService.h"
#include "gp/proto/Proto.h"
#include "gp/network/IncomingMessage.h"

using namespace gpproto;

void UpdateMessageService::setDelegate(const std::shared_ptr<gpproto::UpdateMessageServiceDelegate> & delegate) {
    Proto::queue()->async([self = shared_from_this(), delegate] {
        self->delegate = delegate;
    });
}

void UpdateMessageService::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto,
                                                  std::shared_ptr<IncomingMessage> message) {
    uint32_t signature;

}

void UpdateMessageService::protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto,
                                                          std::vector<int> transactionIds) {
}

void UpdateMessageService::protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) {

}

void UpdateMessageService::protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto,
                                                          std::vector<int64_t> messages) {

}

void UpdateMessageService::protoErrorReceived(const std::shared_ptr<Proto> &proto, int32_t errorCode) {

}

void UpdateMessageService::protoWillAddService(const std::shared_ptr<Proto> &proto) {

}

void UpdateMessageService::protoDidAddService(const std::shared_ptr<Proto> &proto) {

}

void UpdateMessageService::protoWillRemoveService(const std::shared_ptr<Proto> &proto) {

}

void UpdateMessageService::protoDidRemoveService(const std::shared_ptr<Proto> &proto) {

}

void UpdateMessageService::protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) {

}

std::shared_ptr<MessageTransaction> UpdateMessageService::protoMessageTransaction(const std::shared_ptr<Proto> &proto) {
    return nullptr;
}

void UpdateMessageService::protoDidChangeSession(const std::shared_ptr<Proto> &proto) {

}

void UpdateMessageService::protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) {

}

void UpdateMessageService::protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) {

}

void UpdateMessageService::protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) {

}

void UpdateMessageService::protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) {

}

