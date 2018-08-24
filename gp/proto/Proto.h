//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#ifndef GPPROTO_PROTO_H
#define GPPROTO_PROTO_H

#include <string>
#include "gp/utils/DispatchQueue.h"
#include "ProtoDelegate.h"

namespace gpproto {

    typedef enum {
        ProtoStateAwaitingAuthorization = 1,
        ProtoStateAwaitingTimeFixAndSalts = 2,
        ProtoStateStopped = 4,
        ProtoStatePaused = 8
    } ProtoState;

    typedef enum {
        ProtoConnectionStateConnected = 0,
        ProtoConnectionStateConnecting = 1,
        ProtoConnectionStateWaiting = 2
    } ProtoConnectionState;

    class Proto final : std::enable_shared_from_this<Proto> {
    public:

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.manager");
            return q;
        }

        ~Proto() {
            auto _delegate = delegate;
            Proto::queue()->async([&] {
               // _delegate.reset();
            });
        }

        void pause();
        void resume();
        void stop();

        void setDelegate(std::shared_ptr<ProtoDelegate> delegate);

    private:
        ProtoState state;
        std::weak_ptr<ProtoDelegate> delegate;
        void setState(int state);
    };
}

#endif //GPPROTO_PROTO_H
