//
// Created by Jaloliddin Erkiniy on 7/31/19.
//

#ifndef GPPROTO_JSONADAPTER_H
#define GPPROTO_JSONADAPTER_H

#include "gp/utils/third_party/nlohmann/json.hpp"

using namespace nlohmann;

namespace gpproto
{
    class DatacenterAddress;
    class DatacenterSaltsetInfo;
    class AuthKeyInfo;

    class JsonAdapter {
    public:
        static std::vector<std::shared_ptr<DatacenterAddress>> toDatacenterAddresses(json obj);
        static json fromDatacenterAddresses(const std::vector<std::shared_ptr<DatacenterAddress>> & addresses);

        static std::vector<std::shared_ptr<DatacenterSaltsetInfo>> toSaltsetInfo(json obj);
        static json fromSaltsetInfo(const std::vector<std::shared_ptr<DatacenterSaltsetInfo>> & saltset);

        static std::vector<std::shared_ptr<AuthKeyInfo>> toAuthKeyInfo(json obj);
        static json fromAuthKeyInfo(const std::vector<std::shared_ptr<AuthKeyInfo>> & authInfos);

        static std::unordered_map<int, std::shared_ptr<AuthKeyInfo>> toAuthKeyInfoByDatacenterId(json obj);
        static json fromAuthKeyInfoByDatacenterId(const std::unordered_map<int, std::shared_ptr<AuthKeyInfo>>& authInfos);

        static double toTimeDifference(json obj);
        static json fromTimeDifference(double difference);
    };


}

#endif //GPPROTO_JSONADAPTER_H
