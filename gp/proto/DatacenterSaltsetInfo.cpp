//
// Created by Jaloliddin Erkiniy on 8/23/18.
//

#include "gp/proto/DatacenterSaltsetInfo.h"
using namespace gpproto;

bool DatacenterSaltsetInfo::isValidFutureSaltForMessageId(int64_t messageId) const {
    return lastValidMessageId > messageId;
}
