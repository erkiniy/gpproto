//
// Created by Jaloliddin Erkiniy on 8/13/18.
//

#include "NetworkAddress.h"

#ifdef _WIN32
#else
#include "os/posix/NetworkSocketPosix.h"
#endif

using namespace gpproto;

bool NetworkAddress::operator == (const NetworkAddress &other) {
    IPv4Address* self4 = dynamic_cast<IPv4Address*>(this);
    IPv4Address* other4 = dynamic_cast<IPv4Address*>((NetworkAddress*)&other);

    if (self4 && other4) {
        return self4->GetAddress() == other4->GetAddress();
    }

    return false;
}

bool NetworkAddress::operator != (const NetworkAddress &other) {
    return !(*this == other);
}

std::string NetworkAddress::ToString() {
    return "";
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