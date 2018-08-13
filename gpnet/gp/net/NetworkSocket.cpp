//
// Created by System Administrator on 7/17/18.
//

#include "NetworkSocket.h"
#if defined(_WIN32)
#include "os/windows/NetworkSocketWinsock.h"
#include "winsock2.h"
#else
#include "os/posix/NetworkSocketPosix.h"
#endif


using namespace gpproto;

NetworkSocket::NetworkSocket(NetworkProtocol protocol, NetworkAddress* address) : protocol(protocol), address(address) {
    this->failed = false;
}

NetworkSocket::~NetworkSocket() {

}

std::string NetworkSocket::GetLocalInterfaceInfo(IPv4Address *inet4addr) {
    std::string info = "Not implemented";
    return info;
}

void NetworkSocket::setDelegate(std::shared_ptr<NetworkSocketDelegate> delegate) {
    std::weak_ptr<NetworkSocket> weakSelf = shared_from_this();

    NetworkSocket::queue()->async([weakSelf, delegate] {
        auto strongSelf = weakSelf.lock();
        if (strongSelf)
            strongSelf->delegate = delegate;
    });
}

bool NetworkSocket::isFailed() {
    return this->failed;
}

NetworkSocket* NetworkSocket::Create(NetworkProtocol protocol, NetworkAddress* address) {
#ifdef _WIN32
    return NetworkSocketWinsock()
#else
    return new NetworkSocketPosix(protocol, address);
#endif
}

IPv4Address* NetworkSocket::ResolveDomainName(std::string name) {
#ifdef _WIN32
#else
#endif
    return NetworkSocketPosix::ResolveDomainName(name);
}

void NetworkSocket::Receive(std::shared_ptr<StreamSlice> slice) {
    NetworkPacket packet = { 0 };
    packet.slice = slice;
    Receive(&packet);
}

void NetworkSocket::Send(std::shared_ptr<StreamSlice> slice) {
    NetworkPacket packet = { 0 };
    packet.slice = slice;
    Send(&packet);
}

void NetworkSocket::readDataWithTimeout(float timeout, size_t length, uint8_t tag) {
    std::weak_ptr<NetworkSocket> weakSelf = shared_from_this();
    NetworkSocket::queue()->async([weakSelf, tag, length] {
        auto strongSelf = weakSelf.lock();

        if (!strongSelf)
            return;

        auto slice = std::make_shared<StreamSlice>();
        slice->size = length;
        slice->bytes = (char *)malloc(length);

        auto readBuffer = std::make_shared<NetworkPacket>();
        readBuffer->tag = tag;
        readBuffer->slice = slice;
        strongSelf->readBufferQueue.push_back(readBuffer);

        strongSelf->maybeDequeueRead();
    });
}

void NetworkSocket::sendDataWithTimeout(float timeout, const std::shared_ptr<StreamSlice>& slice, uint8_t tag) {
    std::weak_ptr<NetworkSocket> weakSelf = shared_from_this();
    NetworkSocket::queue()->async([weakSelf, slice, tag] {
        auto strongSelf = weakSelf.lock();

        if (!strongSelf)
            return;

        auto writeBuffer = std::make_shared<NetworkPacket>();
        writeBuffer->tag = tag;
        writeBuffer->slice = slice;
        strongSelf->sendBufferQueue.push_back(writeBuffer);

        strongSelf->maybeDequeueWrite();
    });
}

void NetworkSocket::maybeDequeueWrite() {
    std::weak_ptr<NetworkSocket> weakSelf = shared_from_this();

    NetworkSocket::queue()->asyncForce([weakSelf] {
        auto strongSelf = weakSelf.lock();

        if (!strongSelf)
            return;

        if (!strongSelf->sendBufferQueue.empty() && strongSelf->Connected())
        {
            auto packet = strongSelf->sendBufferQueue.front();
            strongSelf->sendBufferQueue.pop_front();

            if (!strongSelf->Send(packet.get()))
            {
                auto weakDelegate = strongSelf->delegate;
                auto strongDelegate = weakDelegate.lock();

                if (strongDelegate)
                    strongDelegate->networkSocketDidSendData(strongSelf, packet->slice->size, packet->tag);
            }
        }

        if (strongSelf->isFailed())
            strongSelf->sendBufferQueue.clear();
    });
}

void NetworkSocket::maybeDequeueRead() {
    std::weak_ptr<NetworkSocket> weakSelf = shared_from_this();

    NetworkSocket::queue()->asyncForce([weakSelf] {
        auto strongSelf = weakSelf.lock();

        if (!strongSelf)
            return;

        if (!strongSelf->readBufferQueue.empty() && strongSelf->Connected() && !strongSelf->Reading())
        {
            auto packet = strongSelf->readBufferQueue.front();
            strongSelf->readBufferQueue.pop_front();

            NetworkSocket::receiveQueue()->async([packet, weakSelf] {
                auto strongSelf_ = weakSelf.lock();

                if (!strongSelf_)
                    return;

                if (!strongSelf_->Receive(packet.get()))
                {
                    NetworkSocket::queue()->async([packet, weakSelf] {

                        auto strongSelf__ = weakSelf.lock();
                        if (!strongSelf__)
                            return;

                        auto weakDelegate = strongSelf__->delegate;
                        auto strongDelegate = weakDelegate.lock();

                        if (strongDelegate)
                            strongDelegate->networkSocketDidReadData(strongSelf__, packet->slice, packet->tag);
                    });
                }
            });
        }

        if (strongSelf->isFailed())
            strongSelf->readBufferQueue.clear();
    });
}

