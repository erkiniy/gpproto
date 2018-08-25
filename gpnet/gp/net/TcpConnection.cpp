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
             size_t length = i->size;

             if (length == 0)
                 continue;

             if (length % 4) {
                 LOGE("TcpConnection: attempt to send a packet with length %lu. Should be divisble by 4", length);
                 return;
             }

             if (closed || !socket) {
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

             socket->sendDataWithTimeout(10.0, finalData, 0);

             free(packetData);
         }
    });
}

void TcpConnection::setDelegate(std::shared_ptr<ConnectionDelegate> delegate) {
    TcpConnection::queue()->async([&, delegate] {
        this->delegate = delegate;
    });
}

void TcpConnection::closeAndNotify() {
    TcpConnection::queue()->async([&] {
        if (!closed) {
            closed = true;

            socket->setDelegate(nullptr);
            socket->Close();
            socket = nullptr;

            if (auto strongDelegate = delegate.lock())
                strongDelegate->connectionClosed(*this);

            LOGV("Close and notify");
        }
    });
}

void TcpConnection::networkSocketDidConnectToHost(const NetworkSocket& socket,
                                                  const NetworkAddress &address, uint16_t port) {
    LOGV("TcpConnection did connect to host");
    TcpConnection::queue()->async([&] {
        if (auto strongDelegate = delegate.lock())
            strongDelegate->connectionOpened(*this);
    });
}

void TcpConnection::networkSocketDidDisconnectFromHost(const NetworkSocket& socket,
                                                       const NetworkAddress &address, uint16_t port,
                                                       uint8_t reasonCode) {
    LOGV("TcpConnection did disconnect from host");
    TcpConnection::queue()->async([&] {
        if (auto strongDelegate = delegate.lock())
            strongDelegate->connectionClosed(*this);
    });
}

void TcpConnection::networkSocketDidReadData(const NetworkSocket& socket,
                                             std::shared_ptr<StreamSlice> data, uint8_t tag) {
    TcpConnection::queue()->async([&, data, tag] {
        LOGV("TcpConnection did read data with size = %zu bytes and tag %u", data->size, tag);
        if (tag == (uint8_t)TcpPacketReadTag::shortLength && data->size == 1)
        {
            uint8_t lengthMarker = 0;
            memcpy(&lengthMarker, data->bytes, 1);

            if (lengthMarker >= 0x1 && lengthMarker <= 0x7e)
            {
                lengthMarker <<= 2;
                this->socket->readDataWithTimeout(10.0, lengthMarker, (uint8_t)TcpConnection::TcpPacketReadTag::body);
            }
            else if (lengthMarker == 0x7f)
                this->socket->readDataWithTimeout(10.0, 3, (uint8_t)TcpConnection::TcpPacketReadTag::longLength);
            else {
                LOGE("TcpConnection: Received wrong packet length %u", lengthMarker);
                closeAndNotify();
            }
        }
        else if (tag == (uint8_t)TcpPacketReadTag::longLength && data->size == 3)
        {
            uint32_t lengthMarker = 0;
            memcpy(&lengthMarker, data->bytes, 3);

            lengthMarker &= 0x00ffffff;
            lengthMarker <<= 2;

            if (lengthMarker > 0 && lengthMarker < 4 * 1024 * 1024)
                this->socket->readDataWithTimeout(10.0, lengthMarker, (uint8_t)TcpConnection::TcpPacketReadTag::body);
            else {
                LOGE("TcpConnection: Received wrong packet length %u", lengthMarker);
                closeAndNotify();
            }
        }
        else if (tag == (uint8_t)TcpPacketReadTag::body && data->size > 0)
        {
            if (auto strongDelegate = delegate.lock())
                strongDelegate->connectionDidReceiveData(*this, data);

            this->socket->readDataWithTimeout(10.0, 1, (uint8_t)TcpConnection::TcpPacketReadTag::shortLength);
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