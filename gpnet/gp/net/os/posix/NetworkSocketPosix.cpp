//
// Created by System Administrator on 7/17/18.
//

#include "NetworkSocketPosix.h"
#include <vector>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "gp/utils/Logging.h"

using namespace gpproto;

NetworkSocketPosix::NetworkSocketPosix(NetworkProtocol protocol, NetworkAddress* address) : NetworkSocket(protocol, address) {
    fd = -1;
    tcpConnectedAddress = nullptr;
    tcpConnectedPort = 0;

    LOGV("NetworkSocketPosix allocated");
}

NetworkSocketPosix::~NetworkSocketPosix() {
    if (tcpConnectedAddress) {
        delete tcpConnectedAddress;
    }

    LOGV("NetworkSocketPosix deallocated");

}

void NetworkSocketPosix::Open() {

    std::weak_ptr<NetworkSocketPosix> weakSelf = shared_from_this();

    NetworkSocket::queue()->async([weakSelf]{
        auto strongSelf = weakSelf.lock();

        if (!strongSelf)
            return;

        if (strongSelf->protocol == PROTO_UDP) {
            return;
        }

        strongSelf->fd = socket(AF_INET, SOCK_STREAM, 0);
        strongSelf->reading = false;

        timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        if (setsockopt(strongSelf->fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            LOGE("setsockopt error SO_SNDTIMEO");
            return;
        }

        timeout.tv_sec = 60;
        if (setsockopt(strongSelf->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            LOGE("setsockopt error SO_RCVTIMEO");
            return;
        }

        int nosigpipe = 1;
        setsockopt(strongSelf->fd, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe));

        int flag = 1;
        if (setsockopt(strongSelf->fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
            LOGE("setsockopt error SO_REUSEADDR");
            return;
        }

        if (setsockopt(strongSelf->fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
            LOGE("setsockopt error SO_KEEPALIVE");
            return;
        }

        if (setsockopt(strongSelf->fd, SOL_SOCKET, TCP_NODELAY, reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
            LOGE("setsockopt error TCP_NODELAY");
            return;
        }
    });
}

void NetworkSocketPosix::Close() {
    std::weak_ptr<NetworkSocketPosix> weakSelf = shared_from_this();

    NetworkSocket::queue()->async([weakSelf] {
        auto strongSelf = weakSelf.lock();

        if (!strongSelf)
            return;

        strongSelf->closing = true;
        strongSelf->failed = true;
        strongSelf->reading = false;

        if (strongSelf->fd >= 0)
        {
            shutdown(strongSelf->fd, SHUT_RDWR);
            close(strongSelf->fd);
            strongSelf->fd = -1;

            auto delegate = strongSelf->delegate.lock();
            if (delegate)
                delegate->networkSocketDidDisconnectFromHost(strongSelf, *strongSelf->tcpConnectedAddress, strongSelf->tcpConnectedPort, 0);
        }
    });
}

bool NetworkSocketPosix::Connected() {
    bool connected;

    NetworkSocket::queue()->sync([&] {
        connected = fd >= 0;
    });

    return connected;
}

bool NetworkSocketPosix::Reading() {
    bool reading_;
    reading_ = reading.load();
    return reading_;
}

size_t NetworkSocketPosix::Send(NetworkPacket *packet) {

    if (!packet || (protocol == PROTO_UDP && !packet->address)) {
        LOGE("Tried to send null packet");
        return 0;
    }

    if (protocol == PROTO_UDP) {

    }
    else {
        ssize_t res = send(fd, packet->slice->bytes, packet->slice->size, 0);

        if (res <= 0) {
            LOGE("Error sending packet");
            Close();
        }
        else {
            LOGV("Did send packet with length %d bytes", (int)packet->slice->size);
            maybeDequeueWrite();
        }
    }

    return 0;
}

size_t NetworkSocketPosix::Receive(NetworkPacket *packet) {
    if (protocol == PROTO_UDP) {

    }
    else {
        reading = true;
        ssize_t res = recv(fd, packet->slice->bytes, packet->slice->size, 0);
        reading = false;
        if (res <= 0) {
            LOGE("Error receiving TCP packet");
            Close();
            return 0;
        }
        else {
            packet->slice->size = (size_t)res;
            packet->address = tcpConnectedAddress;
            packet->port = tcpConnectedPort;
            packet->protocol = protocol;

            maybeDequeueRead();

            return (size_t)res;
        }
    }

    return 0;
}

void NetworkSocketPosix::Connect(NetworkAddress *address, uint16_t port) {
    std::weak_ptr<NetworkSocketPosix> weakSelf = shared_from_this();

    NetworkSocket::queue()->async([weakSelf, address, port] {
        auto strongSelf = weakSelf.lock();

        if (!strongSelf)
            return;

        strongSelf->failed = false;
        strongSelf->closing = false;

        auto v4address = dynamic_cast<IPv4Address*>(address);
        sockaddr_in v4;
        sockaddr* addr = nullptr;
        size_t addrLen = 0;

        if (v4address) {
            v4.sin_family = AF_INET;
            v4.sin_addr.s_addr = v4address->GetAddress();
            v4.sin_port = htons(port);
            addr = reinterpret_cast<sockaddr*>(&v4);
            addrLen = sizeof(v4);
        }

        if (strongSelf->fd < 0) {
            strongSelf->Open();
        }

        if (strongSelf->fd < 0) {
            LOGE("Socket TCP create error");
            return;
        }

        int result = connect(strongSelf->fd, (const sockaddr*) addr, addrLen);
        if (result != 0) {
            LOGE("Connect error %d", result);
            close(strongSelf->fd);
            strongSelf->failed = true;
            return;
        }

        strongSelf->tcpConnectedAddress = (NetworkAddress*)new IPv4Address(*v4address);
        strongSelf->tcpConnectedPort = port;

        LOGV("Posix socket sucessfully connected");

        auto delegate = strongSelf->delegate.lock();
        if (delegate)
            delegate->networkSocketDidConnectToHost(strongSelf, *address, port);

        strongSelf->maybeDequeueRead();
        strongSelf->maybeDequeueWrite();
    });
}

uint16_t NetworkSocketPosix::GetLocalPort() {
    return 0;
}

std::string NetworkSocketPosix::GetLocalInterfaceInfo(gpproto::IPv4Address *inet4addr) {
    return "";
}

void NetworkSocketPosix::OnActiveInterfaceChanged() {

}

NetworkAddress* NetworkSocketPosix::GetConnectedAddress() {
    return nullptr;
}

uint16_t NetworkSocketPosix::GetConnectedPort() {
    return this->tcpConnectedPort;
}

void NetworkSocketPosix::SetTimeouts(int sendTimeout, int receiveTimeout) {
    timeval timeout;
    timeout.tv_sec = sendTimeout;
    timeout.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    timeout.tv_sec = receiveTimeout;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

int NetworkSocketPosix::GetDescriptorFromSocket(gpproto::NetworkSocket *socket) {
    return 0;
}

std::string NetworkSocketPosix::V4AddressToString(uint32_t address) {
    char buf[INET_ADDRSTRLEN];
    in_addr addr;
    addr.s_addr = address;
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return std::string(buf);
}

uint32_t NetworkSocketPosix::StringToV4Adress(std::string address) {
    in_addr addr;
    inet_pton(AF_INET, address.c_str(), &addr);
    return addr.s_addr;
}

IPv4Address* NetworkSocketPosix::ResolveDomainName(std::string name) {
    addrinfo* addr0;
    IPv4Address* ret = NULL;

    int res = getaddrinfo(name.c_str(), NULL, NULL, &addr0);
    if (res != 0) {
        LOGE("Error resolving address:%s", name.c_str());
    }
    else {
        addrinfo* addrPtr;
        for(addrPtr = addr0; addrPtr; addrPtr = addrPtr->ai_next) {
            if (addrPtr->ai_family == AF_INET) {
                sockaddr_in* addr = (sockaddr_in *)addrPtr->ai_addr;
                ret = new IPv4Address(addr->sin_addr.s_addr);
            }
        }
        freeaddrinfo(addr0);
    }

    return ret;
}