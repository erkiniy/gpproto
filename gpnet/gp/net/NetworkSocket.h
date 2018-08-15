//
// Created by System Administrator on 7/17/18.
//

#ifndef MYTESTLIBRARY_SOCKET_H
#define MYTESTLIBRARY_SOCKET_H

#include <string>
#include <vector>
#include <list>
#include "NetworkSocketDelegate.h"
#include "NetworkAddress.h"
#include "gp/utils/DispatchQueue.h"
#include "gp/utils/StreamSlice.h"

namespace gpproto {

    enum NetworkProtocol {
        PROTO_UDP = 0,
        PROTO_TCP
    };

    struct NetworkPacket {
        std::shared_ptr<StreamSlice> slice;
        NetworkAddress* address;
        uint16_t port;
        NetworkProtocol protocol;
        uint8_t tag;
    };

    typedef struct NetworkPacket NetworkPacket;

class NetworkSocket : public std::enable_shared_from_this<NetworkSocket> {
    public:
        static std::shared_ptr<DispatchQueue> queue() {
            static auto q = std::make_shared<DispatchQueue>("uz.gpproto.tcpQueue");
            return q;
        }

        static std::shared_ptr<DispatchQueue> receiveQueue() {
            static auto q = std::make_shared<DispatchQueue>("uz.gpproto.tcpReceiveQueue");
            return q;
        }

        NetworkSocket(NetworkProtocol protocol, NetworkAddress* address);
        virtual ~NetworkSocket();

        void readDataWithTimeout(float timeout, size_t length, uint8_t tag);
        void sendDataWithTimeout(float timeout, const std::shared_ptr<StreamSlice>& slice, uint8_t tag);

        virtual size_t Send(NetworkPacket* packet) = 0;
        void Send(std::shared_ptr<StreamSlice> slice);

        virtual size_t Receive(NetworkPacket* packet) = 0;
        void Receive(std::shared_ptr<StreamSlice> slice);

        virtual void Open() = 0;
        virtual void Close() = 0;
        virtual uint16_t GetLocalPort() { return 0; };
        virtual void Connect(NetworkAddress* address, uint16_t port) = 0;
        virtual std::string GetLocalInterfaceInfo(IPv4Address* inet4addr) = 0;
        virtual void OnActiveInterfaceChanged(){};
        virtual NetworkAddress* GetConnectedAddress() { return nullptr; };
        virtual uint16_t GetConnectedPort() { return 0; };
        virtual void SetTimeouts(int sendTimeout, int receiveTimeout) {};

        virtual bool Connected() = 0;
        virtual bool Reading() = 0;

        virtual bool isFailed();

        static NetworkSocket* Create(NetworkProtocol protocol, NetworkAddress* address);
        static IPv4Address* ResolveDomainName(std::string name);

        void setDelegate(std::shared_ptr<NetworkSocketDelegate> delegate);

    protected:
        bool failed;
        NetworkProtocol protocol;
        NetworkAddress* address;
        std::weak_ptr<NetworkSocketDelegate> delegate;

        void maybeDequeueWrite();
        void maybeDequeueRead();

    private:
        std::list<std::shared_ptr<NetworkPacket>> readBufferQueue;
        std::list<std::shared_ptr<NetworkPacket>> sendBufferQueue;
    };

}

#endif //MYTESTLIBRARY_SOCKET_H
