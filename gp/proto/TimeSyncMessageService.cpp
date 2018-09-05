//
// Created by Jaloliddin Erkiniy on 9/5/18.
//

#include "gp/proto/TimeSyncMessageService.h"
using namespace gpproto;

void TimeSyncMessageService::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto,
                                                    std::shared_ptr<IncomingMessage> message) {

}

void TimeSyncMessageService::protoTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto,
                                                            std::vector<int> transactionIds) {

}

void TimeSyncMessageService::protoMessageDeliveryFailed(const std::shared_ptr<Proto> &proto, int64_t messageId) {

}

void TimeSyncMessageService::protoMessagesDeliveryConfirmed(const std::shared_ptr<Proto> &proto,
                                                            std::vector<int64_t> messages) {
}

void TimeSyncMessageService::protoErrorReceived(const std::shared_ptr<Proto> &proto, int32_t errorCode) {

}

void TimeSyncMessageService::protoWillAddService(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoDidAddService(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoWillRemoveService(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoDidRemoveService(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoAllTransactionsMayHaveFailed(const std::shared_ptr<Proto> &proto) {

}

std::shared_ptr<MessageTransaction> TimeSyncMessageService::protoMessageTransaction(const std::shared_ptr<Proto> &proto) {
    return nullptr;
}

void TimeSyncMessageService::protoDidChangeSession(const std::shared_ptr<Proto> &proto) {

}

void TimeSyncMessageService::protoServerDidChangeSession(const std::shared_ptr<Proto> &proto) {

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
