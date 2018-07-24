//
// Created by System Administrator on 7/17/18.
//

#ifndef MYTESTLIBRARY_SOCKET_H
#define MYTESTLIBRARY_SOCKET_H

#include "string"
#include "vector"

namespace gpproto {

    enum NetworkProtocol {
        PROTO_UDP = 0,
        PROTO_TCP
    };

    class NetworkAddress {
    public:
        virtual std::string ToString() = 0;
        virtual ~NetworkAddress() = default;
        bool operator == (const NetworkAddress& other);
        bool operator != (const NetworkAddress& other);
    };

    class IPv4Address : public NetworkAddress {
    public:
        IPv4Address(std::string addr);
        IPv4Address(uint32_t addr);
        virtual std::string ToString();
        virtual bool isEmpty();
        uint32_t GetAddress();
    private:
        uint32_t address;
    };

    struct NetworkPacket {
        unsigned char* data;
        size_t length;
        NetworkAddress* address;
        uint16_t port;
        NetworkProtocol protocol;
    };

    typedef struct  NetworkPacket NetworkPacket;


    class NetworkSocket {
    public:
        NetworkSocket(NetworkProtocol protocol, NetworkAddress* address);
        virtual ~NetworkSocket();

        virtual void Send(NetworkPacket* packet) = 0;
        size_t Send(unsigned char* buffer, size_t len);

        virtual void Receive(NetworkPacket* packet) = 0;
        size_t Receive(unsigned char* buffer, size_t len);

        virtual void Open() = 0;
        virtual void Close() = 0;
        virtual uint16_t GetLocalPort() { return 0; };
        virtual void Connect(NetworkAddress* address, uint16_t port) = 0;
        virtual std::string GetLocalInterfaceInfo(IPv4Address* inet4addr) = 0;
        virtual void OnActiveInterfaceChanged(){};
        virtual NetworkAddress* GetConnectedAddress() { return NULL; };
        virtual uint16_t GetConnectedPort() { return 0; };
        virtual void SetTimeouts(int sendTimeout, int receiveTimeout) {};

        virtual bool isFailed();

        static NetworkSocket* Create(NetworkProtocol protocol, NetworkAddress* address);
        static IPv4Address* ResolveDomainName(std::string name);

    protected:
        bool failed;
        NetworkProtocol protocol;
        NetworkAddress* address;
    };

}

#endif //MYTESTLIBRARY_SOCKET_H
