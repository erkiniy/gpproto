//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#ifndef GPPROTO_AUTHKEYINFO_H
#define GPPROTO_AUTHKEYINFO_H

#include <vector>
#include <gputils/gp/utils/Logging.h>

namespace gpproto
{
    class StreamSlice;
    class DatacenterSaltsetInfo;
    class AuthKeyInfo {
    public:
        AuthKeyInfo() = delete;
        AuthKeyInfo(std::shared_ptr<StreamSlice> key, int64_t id, std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltSet) :
                authKey(std::move(key)),
                authKeyId(id),
                saltSet(std::move(saltSet))
        {
            LOGV("[AuthInfo cons] saltsetCount = %d, this=%d", this->saltSet.size(), this);
        };

        ~AuthKeyInfo() = default;

        AuthKeyInfo(const AuthKeyInfo&) = default;

        AuthKeyInfo(AuthKeyInfo &&) = default;

        const std::shared_ptr<StreamSlice> authKey;
        const int64_t authKeyId;
        mutable std::vector<std::shared_ptr<DatacenterSaltsetInfo>> saltSet;

        std::shared_ptr<AuthKeyInfo> mergeSaltset(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>>& updatedSaltset, double timestamp);
        std::shared_ptr<AuthKeyInfo> replaceSaltset(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>>& updatedSaltset);

        int64_t authSaltForClientMessageId(int64_t messageId) const;
    };
}

#endif //GPPROTO_AUTHKEYINFO_H
