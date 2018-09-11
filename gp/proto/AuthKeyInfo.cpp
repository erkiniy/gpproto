//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#include "gp/proto/AuthKeyInfo.h"
#include "gp/proto/DatacenterSaltsetInfo.h"
#include "gp/utils/Logging.h"
#include "gp/utils/StreamSlice.h"

using namespace gpproto;

std::shared_ptr<AuthKeyInfo> AuthKeyInfo::mergeSaltset(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>> &updatedSaltset,
                                                       double timestamp) {
    auto referenceMessageId = (int64_t)(timestamp * 4294967296.0);

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
    std::vector<std::shared_ptr<DatacenterSaltsetInfo>> validSalts;

    LOGV("[AuthKeyInfo authSaltForClientMessageId]");

    for (const auto &salt : saltSet)
    {
        if (messageId >= salt->firstValidMessageId && messageId <= salt->lastValidMessageId)
        {
            if (bestSalt == 0)
                bestSalt = salt->salt;

            validSalts.push_back(salt);
        }
    }

    saltSet = validSalts;

    LOGV("[AuthKeyInfo authSaltForClientMessageId] -> saltSet count %lu", saltSet.size());

    return bestSalt;
}