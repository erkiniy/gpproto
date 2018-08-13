//
// Created by Jaloliddin Erkiniy on 8/13/18.
//

#ifndef GPPROTO_NETWORKADDRESS_H
#define GPPROTO_NETWORKADDRESS_H

#include <stdlib.h>
#include <string>
#include <memory>

namespace gpproto {
    class NetworkAddress {
    public:
        virtual std::string ToString();
        virtual ~NetworkAddress() {};
        bool operator == (const NetworkAddress& other);
        bool operator != (const NetworkAddress& other);
    };

    class IPv4Address : public NetworkAddress {
    public:
        IPv4Address(std::string addr);
        IPv4Address(uint32_t addr);
        std::string ToString();
        virtual bool isEmpty();
        uint32_t GetAddress();
    private:
        uint32_t address;
    };
}
#endif //GPPROTO_NETWORKADDRESS_H
