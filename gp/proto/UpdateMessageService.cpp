//
// Created by Jaloliddin Erkiniy on 7/27/19.
//

#include "gp/proto/UpdateMessageService.h"
#include "gp/proto/Proto.h"
#include "gp/network/IncomingMessage.h"
#include "gp/proto/ProtoInternalMessage.h"

using namespace gpproto;

void UpdateMessageService::setDelegate(const std::shared_ptr<gpproto::UpdateMessageServiceDelegate> & delegate) {
    Proto::queue()->async([self = shared_from_this(), delegate] {
        self->delegate = delegate;
    });
}

void UpdateMessageService::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto,
                                                  std::shared_ptr<IncomingMessage> message) {
    if (auto supportedMessage = std::dynamic_pointer_cast<AppSupportedMessage>(message->body))
    {
        if (auto strongDelegate = delegate.lock())
            strongDelegate->didReceiveUpdates(shared_from_this(), supportedMessage->data, message->timestamp);
    }
}