//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_AUTHKEYINFO_H
#define GPPROTO_AUTHKEYINFO_H

#include "gp/utils/StreamSlice.h"
#include "DatacenterSaltsetInfo.h"
#include <vector>

namespace gpproto
{
    class DatacenterSaltsetInfo;
    class AuthKeyInfo {
    public:
        AuthKeyInfo() = delete;
        AuthKeyInfo(std::shared_ptr<StreamSlice> key, int64_t id, const std::vector<std::shared_ptr<DatacenterSaltsetInfo>>& saltSet) :
                authKey(key),
                authKeyId(id),
                saltSet(saltSet)
        {};

        ~AuthKeyInfo() = default;

        AuthKeyInfo(const AuthKeyInfo&) = delete;

        AuthKeyInfo(AuthKeyInfo &&) = default;

        const std::shared_ptr<StreamSlice> authKey;
        const int64_t authKeyId;
        const std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltSet;

        std::shared_ptr<AuthKeyInfo> mergeSaltset(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>>& updatedSaltset, double timestamp);
        std::shared_ptr<AuthKeyInfo> replaceSaltset(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>>& updatedSaltset);
    };
}

#endif //GPPROTO_AUTHKEYINFO_H
