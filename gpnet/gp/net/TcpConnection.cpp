//
// Created by Jaloliddin Erkiniy on 8/7/18.
//

#include "TcpConnection.h"
using namespace gpproto;

void TcpConnection::start() {
    auto strongSelf = shared_from_this();
    TcpConnection::queue()->async([strongSelf] {
        auto socketDelegate = std::static_pointer_cast<NetworkSocketDelegate>(strongSelf);

        LOGV("Starting TcpConnection");
        if (strongSelf->socket)
            return;

        strongSelf->socket = std::shared_ptr<NetworkSocket>(NetworkSocket::Create(NetworkProtocol::PROTO_TCP, &strongSelf->address));
        strongSelf->socket->setDelegate(socketDelegate);
        strongSelf->socket->Connect(&strongSelf->address, strongSelf->port);
        strongSelf->socket->readDataWithTimeout(10.0, 1, (uint8_t)TcpConnection::TcpPacketReadTag::shortLength);
    });
}

void TcpConnection::stop() {
    auto strongSelf = shared_from_this();
    TcpConnection::queue()->async([strongSelf] {
        if (strongSelf->closed) {
            LOGV("Stopping the socket");
            strongSelf->closeAndNotify();
        }
    });
}

void TcpConnection::sendDatas(std::list<std::shared_ptr<StreamSlice>> datas) const {
    if (datas.empty())
        return;
    auto strongSelf = shared_from_this();

    TcpConnection::queue()->async([strongSelf, datas = std::move(datas)] {
         for (auto const& i : datas)
         {
             size_t length = i->size;

             if (length == 0)
                 continue;

             if (length % 4) {
                 LOGE("TcpConnection: attempt to send a packet with length %lu. Should be divisble by 4", length);
                 return;
             }

             if (strongSelf->closed || !strongSelf->socket) {
                 LOGE("TcpConnection: attempt to send a packet on closed or invalid socket");
                 return;
             }

             size_t quarterLength = length / 4;
             size_t quarterLengthMarker = quarterLength;

             size_t headerLength = 0;

             auto packetData = (unsigned char *)malloc(4 + i->size);

             if (quarterLength <= 0x7e) {
                 memcpy(packetData, &quarterLengthMarker, 1);
                 headerLength += 1;
             }
             else {
                 quarterLengthMarker = 0x7f;
                 memcpy(packetData, &quarterLengthMarker, 1);
                 headerLength += 1;
                 memcpy(packetData + (uint8_t)headerLength, &quarterLength, 3);
                 headerLength += 3;
             }

             memcpy(packetData + (uint8_t)headerLength, i->bytes, headerLength + length);

             auto finalData = std::make_shared<StreamSlice>(packetData, headerLength + length, false);

             strongSelf->socket->sendDataWithTimeout(10.0, finalData, 0);

             free(packetData);
         }
    });
}

void TcpConnection::setDelegate(std::shared_ptr<ConnectionDelegate> delegate) {
    auto strongSelf = shared_from_this();
    TcpConnection::queue()->async([strongSelf, delegate] {
        strongSelf->delegate = delegate;
    });
}

void TcpConnection::closeAndNotify() {
    auto strongSelf = shared_from_this();
    TcpConnection::queue()->async([strongSelf] {
        if (!strongSelf->closed) {
            strongSelf->closed = true;

            strongSelf->socket->setDelegate(nullptr);
            strongSelf->socket->Close();
            strongSelf->socket = nullptr;

            if (auto strongDelegate = strongSelf->delegate.lock())
                strongDelegate->connectionClosed(*strongSelf);

            LOGV("Close and notify");
        }
    });
}

void TcpConnection::networkSocketDidConnectToHost(const NetworkSocket& socket,
                                                  const NetworkAddress &address, uint16_t port) {
    LOGV("TcpConnection did connect to host");
    auto strongSelf = shared_from_this();
    TcpConnection::queue()->async([strongSelf] {
        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->connectionOpened(*strongSelf);
    });
}

void TcpConnection::networkSocketDidDisconnectFromHost(const NetworkSocket& socket,
                                                       const NetworkAddress &address, uint16_t port,
                                                       uint8_t reasonCode) {
    LOGV("TcpConnection did disconnect from host");
    auto strongSelf = shared_from_this();
    TcpConnection::queue()->async([strongSelf] {
        if (auto strongDelegate = strongSelf->delegate.lock())
            strongDelegate->connectionClosed(*strongSelf);
    });
}

void TcpConnection::networkSocketDidReadData(const NetworkSocket& socket,
                                             std::shared_ptr<StreamSlice> data, uint8_t tag) {
    auto strongSelf = shared_from_this();
    TcpConnection::queue()->async([strongSelf, data, tag] {
        LOGV("TcpConnection did read data with size = %zu bytes and tag %u", data->size, tag);
        if (tag == (uint8_t)TcpPacketReadTag::shortLength && data->size == 1)
        {
            uint8_t lengthMarker = 0;
            memcpy(&lengthMarker, data->bytes, 1);

            if (lengthMarker >= 0x1 && lengthMarker <= 0x7e)
            {
                lengthMarker <<= 2;
                strongSelf->socket->readDataWithTimeout(10.0, lengthMarker, (uint8_t)TcpConnection::TcpPacketReadTag::body);
            }
            else if (lengthMarker == 0x7f)
                strongSelf->socket->readDataWithTimeout(10.0, 3, (uint8_t)TcpConnection::TcpPacketReadTag::longLength);
            else {
                LOGE("TcpConnection: Received wrong packet length %u", lengthMarker);
                strongSelf->closeAndNotify();
            }
        }
        else if (tag == (uint8_t)TcpPacketReadTag::longLength && data->size == 3)
        {
            uint32_t lengthMarker = 0;
            memcpy(&lengthMarker, data->bytes, 3);

            lengthMarker &= 0x00ffffff;
            lengthMarker <<= 2;

            if (lengthMarker > 0 && lengthMarker < 4 * 1024 * 1024)
                strongSelf->socket->readDataWithTimeout(10.0, lengthMarker, (uint8_t)TcpConnection::TcpPacketReadTag::body);
            else {
                LOGE("TcpConnection: Received wrong packet length %u", lengthMarker);
                strongSelf->closeAndNotify();
            }
        }
        else if (tag == (uint8_t)TcpPacketReadTag::body && data->size > 0)
        {
            if (auto strongDelegate = strongSelf->delegate.lock())
                strongDelegate->connectionDidReceiveData(*strongSelf, data);

            strongSelf->socket->readDataWithTimeout(10.0, 1, (uint8_t)TcpConnection::TcpPacketReadTag::shortLength);
        }
    });
}


void TcpConnection::networkSocketDidSendData(const NetworkSocket& socket, size_t length,
                                             uint8_t tag) {
    LOGV("TcpConnection did send data with %zu and tag %u", length, tag);
}

void TcpConnection::networkSocketDidReadPartialData(const NetworkSocket& socket,
                                                    std::shared_ptr<StreamSlice> data, uint8_t tag) {

}