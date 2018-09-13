//
// Created by Jaloliddin Erkiniy on 9/14/18.
//

#include "gp/network/Transport.h"
using namespace gpproto;

Transport::~Transport() {

}

void Transport::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto, std::shared_ptr<IncomingMessage> message) {
    MessageService::protoDidReceiveMessage(proto, message);
}

void Transport::protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto, std::vector<int> transactionIds) {
    MessageService::protoTransactionsMayHaveFailed(proto, transactionIds);
}

void Transport::protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) {
    MessageService::protoMessageDeliveryFailed(proto, messageId);
}

void Transport::protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto, std::vector<int64_t> messages) {
    MessageService::protoMessagesDeliveryConfirmed(proto, messages);
}

void Transport::protoErrorReceived(const std::shared_ptr<Proto> &proto, int32_t errorCode) {
    MessageService::protoErrorReceived(proto, errorCode);
}

void Transport::protoWillAddService(const std::shared_ptr<Proto> &proto) {
    MessageService::protoWillAddService(proto);
}

void Transport::protoDidAddService(const std::shared_ptr<Proto> &proto) {
    MessageService::protoDidAddService(proto);
}

void Transport::protoWillRemoveService(const std::shared_ptr<Proto> &proto) {
    MessageService::protoWillRemoveService(proto);
}

void Transport::protoDidRemoveService(const std::shared_ptr<Proto> &proto) {
    MessageService::protoDidRemoveService(proto);
}

void Transport::protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) {
    MessageService::protoAllTransactionsMayHaveFailed(proto);
}

std::shared_ptr<MessageTransaction> Transport::protoMessageTransaction(const std::shared_ptr<Proto> &proto) {
    return MessageService::protoMessageTransaction(proto);
}

void Transport::protoDidChangeSession(const std::shared_ptr<Proto> &proto) {
    MessageService::protoDidChangeSession(proto);
}

void Transport::protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) {
    MessageService::protoServerDidChangeSession(proto);
}

void Transport::protoNetworkAvailabilityChanged(const std::shared_ptr<Proto> &proto, bool isNetworkAvailable) {
    MessageService::protoNetworkAvailabilityChanged(proto, isNetworkAvailable);
}

void Transport::protoConnectionStateChanged(const std::shared_ptr<Proto> &proto, bool isConnected) {
    MessageService::protoConnectionStateChanged(proto, isConnected);
}

void Transport::protoAuthTokenUpdated(const std::shared_ptr<Proto> &proto) {
    MessageService::protoAuthTokenUpdated(proto);
}
