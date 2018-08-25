//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#ifndef GPPROTO_PROTO_H
#define GPPROTO_PROTO_H

#include <string>
#include "gp/utils/DispatchQueue.h"
#include "ProtoDelegate.h"
#include "gp/net/Transport.h"

namespace gpproto {

    typedef enum {
        ProtoStateAwaitingAuthorization = 1,
        ProtoStateAwaitingTimeFixAndSalts = 2,
        ProtoStateStopped = 4,
        ProtoStatePaused = 8
    } ProtoState;

    class Proto final : public std::enable_shared_from_this<Proto> {
    public:

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.manager");
            return q;
        }

        ~Proto() {
            auto _delegate = delegate;
            Proto::queue()->async([&] {
                _delegate.reset();
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
        std::shared_ptr<Transport> transport;
        void setTransport(std::shared_ptr<Transport> transport);
        void resetTransport();
    };
}

#endif //GPPROTO_PROTO_H
