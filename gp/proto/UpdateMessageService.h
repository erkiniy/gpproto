//
// Created by Jaloliddin Erkiniy on 7/27/19.
//

#ifndef GPPROTO_UPDATEMESSAGESERVICE_H
#define GPPROTO_UPDATEMESSAGESERVICE_H

#include <unordered_set>
#include "gp/proto/MessageService.h"

namespace gpproto
{
    class StreamSlice;
    class UpdateMessageService;

    class UpdateMessageServiceDelegate {
    public:
        virtual void didReceiveUpdates(const std::shared_ptr<UpdateMessageService>& service, std::shared_ptr<StreamSlice> appData, int32_t date) = 0;
    };

class UpdateMessageService final: public MessageService, public std::enable_shared_from_this<UpdateMessageService> {

    public:
        UpdateMessageService() = default;

        void protoDidReceiveMessage(const std::shared_ptr<Proto>& proto, std::shared_ptr<IncomingMessage> message) override;

        void setDelegate(const std::shared_ptr<UpdateMessageServiceDelegate> & delegate);

    private:
        std::weak_ptr<UpdateMessageServiceDelegate> delegate;
    };
}



#endif //GPPROTO_UPDATEMESSAGESERVICE_H
