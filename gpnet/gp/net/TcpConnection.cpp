//
// Created by Jaloliddin Erkiniy on 8/7/18.
//

#include "TcpConnection.h"
using namespace gpproto;

void TcpConnection::start() {
    TcpConnection::queue()->async([&] {
        LOGV("Starting TcpConnection");
        if (socket)
            return;

        socket = std::shared_ptr<NetworkSocket>(NetworkSocket::Create(NetworkProtocol::PROTO_TCP, &address));
        socket->Connect(&address, port);
        socket->setDelegate(shared_from_this());
        LOGV("Tcp connection start after setting delegate");
        socket->readDataWithTimeout(10.0, 1, (uint8_t)TcpConnection::TcpPacketReadTag::shortLength);
    });
}

void TcpConnection::stop() {
    TcpConnection::queue()->async([&]{
        if (closed) {
            LOGV("Stopping the socket");
            closeAndNotify();
        }
    });
}

void TcpConnection::sendDatas(std::list<std::shared_ptr<StreamSlice>> datas) const {
    if (datas.empty())
        return;

    TcpConnection::queue()->async([&, datas] {
         for (auto const& i : datas)
         {
             if (socket)
                 socket->sendDataWithTimeout(10, i, 0);
         }
    });
}

void TcpConnection::setDelegate(std::shared_ptr<ConnectionDelegate> delegate) {
    this->delegate = delegate;
}

void TcpConnection::closeAndNotify() {
    TcpConnection::queue()->async([&]{
        if (!closed) {
            closed = true;

            LOGV("Close and notify");
        }
    });
}

void TcpConnection::networkSocketDidConnectToHost(std::shared_ptr<NetworkSocket> socket,
                                                  const NetworkAddress &address, uint16_t port) {
    LOGV("TcpConnection did connect to host");

}

void TcpConnection::networkSocketDidDisconnectFromHost(std::shared_ptr<NetworkSocket> socket,
                                                       const NetworkAddress &address, uint16_t port,
                                                       uint8_t reasonCode) {
    LOGV("TcpConnection did disconnect from host");
}

void TcpConnection::networkSocketDidReadData(std::shared_ptr<NetworkSocket> socket,
                                             std::shared_ptr<StreamSlice> data, uint8_t tag) {
    LOGV("TcpConnection did read data with %zu and tag %u", data->size, tag);
    socket->readDataWithTimeout(10.0, 1, (uint8_t)TcpConnection::TcpPacketReadTag::shortLength);
}


void TcpConnection::networkSocketDidSendData(std::shared_ptr<NetworkSocket> socket, size_t length,
                                             uint8_t tag) {
    LOGV("TcpConnection did send data with %zu and tag %u", length, tag);
}

void TcpConnection::networkSocketDidReadPartialData(std::shared_ptr<NetworkSocket> socket,
                                                    std::shared_ptr<StreamSlice> data, uint8_t tag) {

}