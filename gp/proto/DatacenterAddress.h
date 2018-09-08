//
// Created by Jaloliddin Erkiniy on 8/25/18.
//

#ifndef GPPROTO_DATACENTERADDRESS_H
#define GPPROTO_DATACENTERADDRESS_H

#include <string>

namespace gpproto {
    class DatacenterAddress {
    public:
        DatacenterAddress(std::string host, std::string ip, uint16_t port) : host(std::move(host)), ip(std::move(ip)), port(port) {}

        std::string host;
        std::string ip;
        uint16_t port;

        DatacenterAddress(const DatacenterAddress&) = default;
    };

    inline bool operator == (const DatacenterAddress& lhs, const DatacenterAddress& rhs) {
        return lhs.ip == rhs.ip && lhs.port == rhs.port;
    }
}
#endif //GPPROTO_DATACENTERADDRESS_H
