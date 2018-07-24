//
// Created by System Administrator on 7/17/18.
//

#ifndef MYTESTLIBRARY_NETWORKSOCKETPOSIX_H
#define MYTESTLIBRARY_NETWORKSOCKETPOSIX_H

#include "../../NetworkSocket.h"


namespace gpproto {
    class NetworkSocketPosix : public NetworkSocket {
    public:
        NetworkSocketPosix(NetworkProtocol protocol, NetworkAddress* address);
        virtual ~NetworkSocketPosix();
        virtual void Send(NetworkPacket* packet);
        virtual void Receive(NetworkPacket* packet);

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

    private:
        static int GetDescriptorFromSocket(NetworkSocket* socket);
        int fd;
        bool closing;
        NetworkAddress* tcpConnectedAddress;
        uint16_t tcpConnectedPort;
    };
}

#endif //MYTESTLIBRARY_NETWORKSOCKETPOSIX_H
