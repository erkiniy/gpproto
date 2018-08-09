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
        virtual void networkSocketDidReadData(std::shared_ptr<NetworkSocket> socket, std::shared_ptr<StreamSlice> data, uint8_t tag) = 0;
        virtual void networkSocketDidReadPartialData(std::shared_ptr<NetworkSocket> socket, std::shared_ptr<StreamSlice> data, uint8_t tag) = 0;
        virtual void networkSocketDidSendData(std::shared_ptr<NetworkSocket> socket, size_t length, uint8_t tag) = 0;
        virtual void networkSocketDidConnectToHost(std::shared_ptr<NetworkSocket> socket, const NetworkAddress& address, uint16_t port) = 0;
        virtual void networkSocketDidDisconnectFromHost(std::shared_ptr<NetworkSocket> socket, const NetworkAddress& address, uint16_t port, uint8_t reasonCode) = 0;
    };
}
#endif //GPPROTO_NETWORKSOCKETDELEGATE_H
