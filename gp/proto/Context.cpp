//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include "Context.h"
#include <chrono>

using namespace gpproto;

double Context::getGlobalTime() {
    auto timestamp = std::chrono::seconds(std::time(nullptr)).count();
    return static_cast<double>(timestamp) + getGlobalTimeDifference();
}

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
#warning implement storing to keychain
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
    auto info = std::make_shared<AuthKeyInfo>(std::move(keyInfo));

    Context::queue()->async([&, info, id] {
        authInfoByDatacenterId[id] = info;
    });
#warning implement storing to keychain

}

std::shared_ptr<DatacenterAddress> Context::getDatacenterAddressForDatacenterId(int32_t id) {
    std::shared_ptr<DatacenterAddress> address = nullptr;

    Context::queue()->sync([&, id] {
        auto it = datacenterAddressByDatacenterId.find(id);

        if (it != datacenterAddressByDatacenterId.end())
            address = datacenterAddressByDatacenterId[id];
    });

    return address;
}

void Context::setDatacenterAddressForDatacenterId(DatacenterAddress&& address, int32_t id) {
    auto addr = std::make_shared<DatacenterAddress>(std::move(address));

    Context::queue()->async([&, addr] {
        datacenterAddressByDatacenterId[id] = addr;
    });
#warning implement storing to keychain
}