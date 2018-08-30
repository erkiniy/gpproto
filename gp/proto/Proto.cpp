//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#include "Proto.h"
#include "gp/utils/Logging.h"

using namespace gpproto;

void Proto::setDelegate(std::shared_ptr<ProtoDelegate> delegate) {
    Proto::queue()->async([&, delegate] {
        this->delegate = delegate;
    });
}

void Proto::pause() {
    Proto::queue()->async([&] {
        if ((state & ProtoStatePaused) == 0)
        {
            setState(state | ProtoStatePaused);
            LOGV("Proto paused");
        }
    });
}

void Proto::resume() {
    Proto::queue()->async([&] {

        if (state & ProtoStatePaused)
        {
            setState(state & (~ProtoStatePaused));
            LOGV("Proto resumed");
        }
    });
}

void Proto::stop() {
    Proto::queue()->async([&] {
        if ((state & ProtoStateStopped) == 0)
        {
            setState(state | ProtoStateStopped);
            LOGV("Proto stopped");
        }
    });
}

void Proto::setState(int state) {

    this->state = static_cast<ProtoState>(state);

}

void Proto::setTransport(std::shared_ptr<Transport> transport) {

}

void Proto::resetTransport() {
    Proto::queue()->async([strongSelf = shared_from_this(), transport] {
        if (strongSelf->state & ProtoStateStopped)
            return;

        if (auto _transport = strongSelf->transport)
        {
            _transport->setDelegate(nullptr);
            _transport->stop();

            strongSelf->setTransport(nullptr);
        }


    });
}