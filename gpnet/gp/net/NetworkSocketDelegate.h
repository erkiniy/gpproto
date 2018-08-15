//
// Created by Jaloliddin Erkiniy on 8/7/18.
//

#ifndef GPPROTO_NETWORKSOCKETDELEGATE_H
#define GPPROTO_NETWORKSOCKETDELEGATE_H

#include "NetworkSocket.h"
#include "gp/utils/StreamSlice.h"

namespace gpproto {
    class NetworkSocket;
    class NetworkAddress;

    class NetworkSocketDelegate {
    public:
        virtual void networkSocketDidReadData(const NetworkSocket& socket, std::shared_ptr<StreamSlice> data, uint8_t tag) = 0;
        virtual void networkSocketDidReadPartialData(const NetworkSocket& socket, std::shared_ptr<StreamSlice> data, uint8_t tag) = 0;
        virtual void networkSocketDidSendData(const NetworkSocket& socket, size_t length, uint8_t tag) = 0;
        virtual void networkSocketDidConnectToHost(const NetworkSocket& socket, const NetworkAddress& address, uint16_t port) = 0;
        virtual void networkSocketDidDisconnectFromHost(const NetworkSocket& socket, const NetworkAddress& address, uint16_t port, uint8_t reasonCode) = 0;
    };
}
#endif //GPPROTO_NETWORKSOCKETDELEGATE_H
