//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#include "gp/proto/AuthKeyInfo.h"
#include "gp/proto/DatacenterSaltsetInfo.h"
#include "gp/utils/Logging.h"
#include "gp/utils/StreamSlice.h"

#include <unordered_set>
using namespace gpproto;

std::shared_ptr<AuthKeyInfo> AuthKeyInfo::mergeSaltset(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>> &updatedSaltset,
                                                       double timestamp) {
    auto referenceMessageId = (int64_t)(timestamp * 4294967296.0);

    LOGV("[AuthKeyInfo mergeSaltset]");

    std::vector<std::shared_ptr<DatacenterSaltsetInfo>> mergedSaltset;

    for (auto & saltInfo : saltSet)
    {
        if (saltInfo->isValidFutureSaltForMessageId(referenceMessageId))
            mergedSaltset.push_back(saltInfo);
    }

    for (auto & saltInfo : updatedSaltset)
    {
        bool alreadyExists = false;

        for (auto & existingSaltInfo : mergedSaltset)
        {
            if (existingSaltInfo->firstValidMessageId == saltInfo->firstValidMessageId) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists)
        {
            if (saltInfo->isValidFutureSaltForMessageId(referenceMessageId))
                mergedSaltset.push_back(saltInfo);
        }
    }

    return std::make_shared<AuthKeyInfo>(this->authKey, this->authKeyId, mergedSaltset);
}

std::shared_ptr<AuthKeyInfo> AuthKeyInfo::replaceSaltset(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>> &updatedSaltset) {
    return std::make_shared<AuthKeyInfo>(this->authKey, this->authKeyId, updatedSaltset);
}

int64_t AuthKeyInfo::authSaltForClientMessageId(int64_t messageId) const {
    int64_t bestSalt = 0;

    std::unordered_set<int> saltsToRemove;
    int index = -1;

    for (const auto &salt : saltSet)
    {
        index++;

        if (messageId >= salt->firstValidMessageId && messageId <= salt->lastValidMessageId)
        {
            bestSalt = salt->salt;
            break;
        }

        saltsToRemove.insert(index);
    }

    for (int i = (int)saltSet.size() - 1; i >= 0; i--)
    {
        if (saltsToRemove.find(i) != saltsToRemove.end())
            saltSet.erase(saltSet.begin() + i);
    }

    LOGV("[AuthKeyInfo] remaining salts count = %zu", saltSet.size())

    return bestSalt;
}