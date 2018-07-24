//
// Created by System Administrator on 7/17/18.
//

#include "NetworkSocket.h"
#if defined(_WIN32)
#include "../os/windows/NetworkSocketWinsock.h"
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

std::string NetworkSocket::GetLocalInterfaceInfo(gpproto::IPv4Address *inet4addr) {
    std::string info = "not implemented";
    return info;
}

bool NetworkSocket::isFailed() {
    return this->failed;
}


NetworkSocket* NetworkSocket::Create(gpproto::NetworkProtocol protocol, NetworkAddress* address) {
#ifdef _WIN32
    return NetworkSocketWinsock
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

size_t NetworkSocket::Receive(unsigned char *buffer, size_t len) {
    NetworkPacket pkt = { 0 };
    pkt.data = buffer;
    pkt.length = len;
    Receive(&pkt);
    return pkt.length;
}

size_t NetworkSocket::Send(unsigned char *buffer, size_t len) {
    NetworkPacket pkt = { 0 };
    pkt.data = buffer;
    pkt.length = len;
    Send(&pkt);
    return pkt.length;
}

bool NetworkAddress::operator == (const gpproto::NetworkAddress &other) {
    IPv4Address* self4 = dynamic_cast<IPv4Address*>(this);
    IPv4Address* other4 = dynamic_cast<IPv4Address*>((NetworkAddress*)&other);

    if (self4 && other4) {
        return self4->GetAddress() == other4->GetAddress();
    }

    return false;
}

bool NetworkAddress::operator != (const gpproto::NetworkAddress &other) {
    return !(*this == other);
}


IPv4Address::IPv4Address(std::string addr) {
#ifdef _WIN32
#else
    this->address = NetworkSocketPosix::StringToV4Adress(addr);
#endif
}

IPv4Address::IPv4Address(uint32_t addr) {
    this->address = addr;
}

uint32_t IPv4Address::GetAddress() {
    return this->address;
}

bool IPv4Address::isEmpty() {
    return this->address == 0;
}

std::string IPv4Address::ToString() {
#ifdef _WIN32
#else
    return NetworkSocketPosix::V4AddressToString(this->address);
#endif
}

