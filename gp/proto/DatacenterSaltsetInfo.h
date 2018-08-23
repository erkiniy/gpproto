//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_DATACENTERSALTSETINFO_H
#define GPPROTO_DATACENTERSALTSETINFO_H

#include <cstdint>

namespace gpproto {
    class DatacenterSaltsetInfo {
    public:
        DatacenterSaltsetInfo(int64_t salt, int64_t firstValidMessageId, int64_t lastValidMessageId) :
                salt(salt),
                firstValidMessageId(firstValidMessageId),
                lastValidMessageId(lastValidMessageId)
        {};

        ~DatacenterSaltsetInfo() = default;

        DatacenterSaltsetInfo(const DatacenterSaltsetInfo& info) = default;

        const int64_t salt;
        const int64_t firstValidMessageId;
        const int64_t lastValidMessageId;
    };
}

#endif //GPPROTO_DATACENTERSALTSETINFO_H
