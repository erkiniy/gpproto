//
// Created by Jaloliddin Erkiniy on 8/10/18.
//

#include "Proto.h"
#include "gp/utils/Logging.h"
#include "gp/network/TcpTransport.h"
#include "gp/network/TransportScheme.h"

using namespace gpproto;

void Proto::setDelegate(std::shared_ptr<ProtoDelegate> delegate) {
    Proto::queue()->async([&, delegate] {
        this->delegate = delegate;
    });
}

void Proto::pause() {
    Proto::queue()->async([&] {
        if ((protoState & ProtoStatePaused) == 0)
        {
            setState(protoState | ProtoStatePaused);
            LOGV("Proto paused");
        }
    });
}

void Proto::resume() {
    Proto::queue()->async([&] {

        if (protoState & ProtoStatePaused)
        {
            setState(protoState & (~ProtoStatePaused));
            LOGV("Proto resumed");
        }
    });
}

void Proto::stop() {
    Proto::queue()->async([&] {
        if ((protoState & ProtoStateStopped) == 0)
        {
            setState(protoState | ProtoStateStopped);
            LOGV("Proto stopped");
        }
    });
}

void Proto::setState(int state) {

    this->protoState = static_cast<ProtoState>(state);

}

void Proto::setTransport(std::shared_ptr<Transport> transport) {
    Proto::queue()->async([strongSelf = shared_from_this(), transport] {
        LOGV("[Proto setTransport] -> changing transport");

        strongSelf->allTransactionsMayHaveFailed();

    });
}

void Proto::resetTransport() {
    Proto::queue()->async([strongSelf = shared_from_this()] {

        if (strongSelf->protoState & ProtoStateStopped)
            return;

        if (auto _transport = strongSelf->transport)
        {
            _transport->setDelegate(nullptr);
            _transport->stop();

            strongSelf->setTransport(nullptr);
        }

        if (strongSelf->transportScheme == nullptr)
        {
            if ((strongSelf->protoState & ProtoStateAwaitingTransportScheme) == 0)
            {
                LOGV("[Proto resetTransport] -> awaitingTransportScheme");

                strongSelf->setState(strongSelf->protoState | ProtoStateAwaitingTransportScheme);
                strongSelf->context->transportSchemeForDatacenterIdRequired(strongSelf->datacenterId);
            }
        }
        if (!strongSelf->useUnauthorizedMode && strongSelf->context->getAuthKeyInfoForDatacenterId(strongSelf->datacenterId) == nullptr)
        {
            if ((strongSelf->protoState & ProtoStateAwaitingAuthorization) == 0)
            {
                LOGV("[Proto resetTransport] -> missing authorized key for datacenterId = %d", strongSelf->datacenterId);

                strongSelf->setState(strongSelf->protoState | ProtoStateAwaitingAuthorization);
                strongSelf->context->authInfoForDatacenterWithIdRequired(strongSelf->datacenterId);
            }
        }
        else
        {
            LOGV("[Proto resetTransport] -> setting created transport");
            auto transport = strongSelf->transportScheme->createTransportWithContext(strongSelf->context, strongSelf->datacenterId, strongSelf);

            strongSelf->setTransport(transport);
        }
    });
}

void Proto::allTransactionsMayHaveFailed() {
    Proto::queue()->async([strongSelf = shared_from_this()] {

    });
}