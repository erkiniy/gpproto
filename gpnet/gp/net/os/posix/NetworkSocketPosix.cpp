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

using namespace gpproto;

NetworkSocketPosix::NetworkSocketPosix(NetworkProtocol protocol, NetworkAddress* address) : NetworkSocket(protocol, address) {
    fd = -1;
    tcpConnectedAddress = NULL;
    tcpConnectedPort = 0;

    printf("NetworkSocketPosix allocated\n");
}

NetworkSocketPosix::~NetworkSocketPosix() {
    if (tcpConnectedAddress) {
        delete tcpConnectedAddress;
    }

    printf("NetworkSocketPosix deallocated\n");

}

void NetworkSocketPosix::Open() {

    if (protocol == PROTO_UDP) {
        return;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);

    timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        printf("setsockopt error SO_SNDTIMEO\n");
        return;
    }

    timeout.tv_sec=60;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        printf("setsockopt error SO_RCVTIMEO\n");
        return;
    }

    int nosigpipe = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe));

    int flag = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
        printf("setsockopt error SO_REUSEADDR\n");
        return;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
        printf("setsockopt error SO_KEEPALIVE\n");
        return;
    }

    if (setsockopt(fd, SOL_SOCKET, TCP_NODELAY, reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
        printf("setsockopt error TCP_NODELAY\n");
        return;
    }

}

void NetworkSocketPosix::Close() {
    closing = true;
    failed = true;

    if (fd >= 0) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
        fd = -1;
    }
}

void NetworkSocketPosix::Send(NetworkPacket *packet) {
    if (!packet || (protocol == PROTO_UDP && !packet->address)) {
        printf("Tried to send null packet\n");
        return;
    }

    int res = -1;

    if (protocol == PROTO_UDP) {

    }
    else {
        res = send(fd, packet->data, packet->length, 0);
    }

    if (res < 0) {
        printf("Error sending packet\n");
    }
    else {
        printf("Did send packet with length %d\n", (int)packet->length);
    }

}

void NetworkSocketPosix::Receive(NetworkPacket *packet) {
    if (protocol == PROTO_UDP) {

    }
    else {
        int res = recv(fd, packet->data, packet->length, 0);
        if (res <= 0) {
            printf("Error receiving TCP packet\n");
            failed = true;
        }
        else {
            packet->length =  (size_t)res;
            packet->address = tcpConnectedAddress;
            packet->port = tcpConnectedPort;
            packet->protocol = protocol;
        }
    }
}

void NetworkSocketPosix::Connect(NetworkAddress *address, uint16_t port) {
    IPv4Address* v4address = dynamic_cast<IPv4Address*>(address);
    sockaddr_in v4;
    sockaddr* addr = NULL;
    size_t addrLen = 0;

    if (v4address) {
        v4.sin_family = AF_INET;
        v4.sin_addr.s_addr = v4address->GetAddress();
        v4.sin_port = htons(port);
        addr = reinterpret_cast<sockaddr*>(&v4);
        addrLen = sizeof(v4);
    }

    if (fd < 0) {
        this->Open();
    }

    if (fd < 0) {
        printf("Socket TCP create error\n");
        return;
    }

    int result = connect(fd, (const sockaddr*) addr, addrLen);
    if (result != 0) {
        printf("Connect error\n");
        close(fd);
        this->failed = true;
        return;
    }

    tcpConnectedAddress = (NetworkAddress*)new IPv4Address(*v4address);
    tcpConnectedPort = port;

    printf("Sucessfully connected\n");
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
        printf("Error resolving address:%s", name.c_str());
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