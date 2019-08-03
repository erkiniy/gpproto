//
// Created by Jaloliddin Erkiniy on 7/31/19.
//

#include <gputils/gp/utils/Logging.h>
#include "gp/proto/JsonAdapter.h"
#include "gp/proto/DatacenterAddress.h"
#include "gp/proto/DatacenterSaltsetInfo.h"
#include "gp/proto/AuthKeyInfo.h"

#include "gp/utils/Common.h"

using namespace gpproto;

json JsonAdapter::fromDatacenterAddresses(const std::vector<std::shared_ptr<gpproto::DatacenterAddress>> & addresses) {

    json v = json::array();

    for (auto const & address : addresses)
    {
        json obj;
        obj["ip"] = address->ip;
        obj["port"] = address->port;
        obj["host"] = address->host;

        v.push_back(obj);
    }

    return v;
}

std::vector<std::shared_ptr<DatacenterAddress>> JsonAdapter::toDatacenterAddresses(json obj) {
    std::vector<std::shared_ptr<DatacenterAddress>> v;

    if (!obj.is_array()) return {};

    for (json j : obj)
    {
        try {

            std::string ip = j["ip"];
            uint16_t port = j["port"];
            std::string host = j["host"];

            v.push_back(std::make_shared<DatacenterAddress>(host, ip, port));

        } catch (std::exception & e) { continue; }
    }

    return v;
}

json JsonAdapter::fromSaltsetInfo(const std::vector<std::shared_ptr<gpproto::DatacenterSaltsetInfo>> &saltset) {
    json v = json::array();

    for (const auto & saltInfo : saltset)
    {
        json obj;
        obj["salt"] = saltInfo->salt;
        obj["first"] = saltInfo->firstValidMessageId;
        obj["last"] = saltInfo->lastValidMessageId;

        v.push_back(obj);
    }

    return v;
}

std::vector<std::shared_ptr<DatacenterSaltsetInfo>> JsonAdapter::toSaltsetInfo(json obj) {
    std::vector<std::shared_ptr<DatacenterSaltsetInfo>> v;

    if (!obj.is_array()) return {};

    for (const json & j : obj)
    {
        try
        {
            int64_t salt = j["salt"];
            int64_t first = j["first"];
            int64_t last = j["last"];

            v.push_back(std::make_shared<DatacenterSaltsetInfo>(salt, first, last));

        } catch (std::exception &e) { continue; }
    }

    return v;
}

json JsonAdapter::fromAuthKeyInfo(const std::vector<std::shared_ptr<gpproto::AuthKeyInfo>> &authInfos) {
    json v = json::array();

    for (const auto & authInfo: authInfos)
    {
        json obj;
        obj["id"] =  authInfo->authKeyId;
        obj["key"] = base64_encode(*authInfo->authKey);
        obj["salts"] = fromSaltsetInfo(authInfo->saltSet);

        v.push_back(obj);
    }

    return v;
}

std::vector<std::shared_ptr<AuthKeyInfo>> JsonAdapter::toAuthKeyInfo(json obj) {
    std::vector<std::shared_ptr<AuthKeyInfo>> v;

    if (!obj.is_array()) return {};

    for (const json & j : obj)
    {
        try {
            int64_t id = j["id"];
            std::string keyString = j["key"];
            auto salts = toSaltsetInfo(j["salts"]);

            v.push_back(std::make_shared<AuthKeyInfo>(base64_decode(keyString), id, salts));

        } catch (std::exception &e) { continue; }

    }

    return v;
}

json JsonAdapter::fromAuthKeyInfoByDatacenterId(const std::unordered_map<int, std::shared_ptr<gpproto::AuthKeyInfo>> &authInfos) {
    json v = json::array();

    for (const auto & it : authInfos)
    {
        json obj;
        obj["id"] = it.first;

        json authObj;
        authObj["id"] = it.second->authKeyId;
        authObj["key"] = base64_encode(*it.second->authKey);
        authObj["salts"] = fromSaltsetInfo(it.second->saltSet);

        obj["info"] = authObj;

        v.push_back(obj);
    }

    LOGV("[JsonAdapter authInfoByDatacenter] = %s", v.dump().c_str());

    return v;
}

std::unordered_map<int, std::shared_ptr<AuthKeyInfo>> JsonAdapter::toAuthKeyInfoByDatacenterId(json obj) {
    std::unordered_map<int, std::shared_ptr<AuthKeyInfo>> m;

    if (!obj.is_array()) return {};

    for (const json & j : obj)
    {
        try {
            int dc_id = j["id"];
            json authObj = j["info"];

            int64_t id = authObj["id"];
            std::string keyString = authObj["key"];
            auto salts = toSaltsetInfo(authObj["salts"]);

            m[dc_id] = std::make_shared<AuthKeyInfo>(base64_decode(keyString), id, salts);

        } catch (std::exception &e) { continue; }
    }
    return m;
}

double JsonAdapter::toTimeDifference(json obj) {
    double difference = 0.0;

    if (obj.is_null()) return 0.0;

    try {
        difference = obj["timeDifference"];
    } catch (std::exception &e) { }

    return difference;
}

json JsonAdapter::fromTimeDifference(double difference) {
    json obj;

    obj["timeDifference"] = difference;

    return obj;
}
