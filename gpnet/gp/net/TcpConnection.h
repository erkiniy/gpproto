//
// Created by Jaloliddin Erkiniy on 8/7/18.
//

#ifndef GPPROTO_TCPCONNECTION_H
#define GPPROTO_TCPCONNECTION_H

#include <list>
#include "Connection.h"
#include "NetworkSocket.h"
#include "gp/utils/Timer.h"
#include "gp/utils/Logging.h"

namespace gpproto {
class TcpConnection final : public Connection, public NetworkSocketDelegate, public std::enable_shared_from_this<TcpConnection> {
    public:

        enum class TcpPacketReadTag : uint8_t {
            shortLength = 0,
            longLength = 1,
            body = 2,
            head = 33
        };

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.tcpConnection");
            return q;
        }

        TcpConnection(IPv4Address& address, uint16_t port) : socket(nullptr), address(std::move(address)), port(port) {
            LOGV("TcpConnection initialized");
            closed = false;
        }

        ~TcpConnection() {
            LOGV("TcpConnection deinitialized");
        }

        void start() override;
        void stop() override;

        void setDelegate(std::shared_ptr<ConnectionDelegate> delegate) override;
        void sendDatas(std::list<std::shared_ptr<StreamSlice>> datas) const override;
        void closeAndNotify() override;

        void networkSocketDidReadData(const NetworkSocket& socket, std::shared_ptr<StreamSlice> data, uint8_t tag) override;
        void networkSocketDidReadPartialData(const NetworkSocket& socket, std::shared_ptr<StreamSlice> data, uint8_t tag) override;
        void networkSocketDidSendData(const NetworkSocket& socket, size_t length, uint8_t tag) override;
        void networkSocketDidConnectToHost(const NetworkSocket& socket, const NetworkAddress& address, uint16_t port) override;
        void networkSocketDidDisconnectFromHost(const NetworkSocket& socket, const NetworkAddress& address, uint16_t port, uint8_t reasonCode) override;

    private:
        std::shared_ptr<NetworkSocket> socket;
        std::weak_ptr<ConnectionDelegate> delegate;
        IPv4Address address;
        uint16_t port;
    };
}


#endif //GPPROTO_TCPCONNECTION_H
