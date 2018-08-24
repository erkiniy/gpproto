//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "Context.h"
using namespace gpproto;

double Context::getGlobalTimeDifference() {
    double difference = 0;
    Context::queue()->sync([&] {
        difference = globalTimeDifference;
    });
    return difference;
}

void Context::setGlobalTimeDifference(double difference) {
    Context::queue()->async([&, difference] {
        globalTimeDifference = difference;
    });
}

std::shared_ptr<AuthKeyInfo> Context::getAuthKeyInfoForDatacenterId(int32_t id) {
    std::shared_ptr<AuthKeyInfo> info = nullptr;

    Context::queue()->sync([&, id] {
        auto it = authInfoByDatacenterId.find(id);

        if (it != authInfoByDatacenterId.end())
            info = authInfoByDatacenterId[id];
    });

    return info;
}

void Context::setAuthKeyInfoForDatacenterId(AuthKeyInfo&& keyInfo, int32_t id) {
    std::shared_ptr<AuthKeyInfo> info = std::make_shared<AuthKeyInfo>(std::move(keyInfo));
    Context::queue()->async([&, info, id] {
        authInfoByDatacenterId[id] = info;
    });
}