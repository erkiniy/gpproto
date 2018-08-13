//
// Created by System Administrator on 7/17/18.
//

#ifndef MYTESTLIBRARY_NETWORKSOCKETPOSIX_H
#define MYTESTLIBRARY_NETWORKSOCKETPOSIX_H

#include "../../NetworkSocket.h"

namespace gpproto {
    class NetworkSocketPosix : public NetworkSocket {
    public:
        std::shared_ptr<NetworkSocketPosix> shared_from_this() {
            return std::static_pointer_cast<NetworkSocketPosix>(NetworkSocket::shared_from_this());
        }
        NetworkSocketPosix(NetworkProtocol protocol, NetworkAddress* address);
        virtual ~NetworkSocketPosix();
        virtual size_t Send(NetworkPacket* packet);
        virtual size_t Receive(NetworkPacket* packet);

        virtual void Open();
        virtual void Close();
        virtual uint16_t GetLocalPort();
        virtual void Connect(NetworkAddress* address, uint16_t port);
        virtual std::string GetLocalInterfaceInfo(IPv4Address* inet4addr);
        virtual void OnActiveInterfaceChanged();
        virtual NetworkAddress* GetConnectedAddress();
        virtual uint16_t GetConnectedPort();
        virtual void SetTimeouts(int sendTimeout, int receiveTimeout);

        static std::string V4AddressToString(uint32_t address);
        static uint32_t StringToV4Adress(std::string address);

        static IPv4Address* ResolveDomainName(std::string name);

        virtual bool Connected();
        virtual bool Reading();

    private:
        static int GetDescriptorFromSocket(NetworkSocket* socket);
        int fd;
        bool closing;
        std::atomic_bool reading;
        NetworkAddress* tcpConnectedAddress;
        uint16_t tcpConnectedPort;
    };
}

#endif //MYTESTLIBRARY_NETWORKSOCKETPOSIX_H
