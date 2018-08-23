//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_AUTHKEYINFO_H
#define GPPROTO_AUTHKEYINFO_H

#include "gp/utils/StreamSlice.h"
#include "DatacenterSaltsetInfo.h"
#include <vector>

namespace gpproto {
    class AuthKeyInfo {
    public:
        AuthKeyInfo() = delete;
        AuthKeyInfo(std::shared_ptr<StreamSlice> key, int64_t id, const std::vector<DatacenterSaltsetInfo>& saltSet) :
                authKey(key),
                authKeyId(id),
                saltSet(saltSet)
        {};

        ~AuthKeyInfo() = default;
        AuthKeyInfo(const AuthKeyInfo&) = default;

        const std::shared_ptr<StreamSlice> authKey;
        const int64_t authKeyId;
        const std::vector<DatacenterSaltsetInfo> saltSet;
    };
}

#endif //GPPROTO_AUTHKEYINFO_H
