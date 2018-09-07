//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include <chrono>
#include "gp/proto/Context.h"
#include "gp/network/TransportScheme.h"
#include <tuple>

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

void Context::updateAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id) {
    setAuthKeyInfoForDatacenterId(keyInfo, id);

    //TODO: notify listeners;
}

void Context::setAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id) {

    Context::queue()->async([&, keyInfo, id] {
        authInfoByDatacenterId[id] = keyInfo;
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

    Context::queue()->async([strongSelf = shared_from_this(), addr, id] {
        strongSelf->datacenterAddressByDatacenterId[id] = addr;
    });
#warning implement storing to keychain
}

std::shared_ptr<DatacenterAddress> Context::getDatacenterSeedAddressForDatacenterId(int32_t id) {
    std::shared_ptr<DatacenterAddress> address = nullptr;

    Context::queue()->sync([&, id] {
        auto it = datacenterSeedAddressByDatacenterId.find(id);

        if (it != datacenterSeedAddressByDatacenterId.end())
            address = datacenterSeedAddressByDatacenterId[id];
    });

    return address;
}

void Context::setDatacenterSeedAddress(gpproto::DatacenterAddress &&address, int32_t id) {

    auto addr = std::make_shared<DatacenterAddress>(std::move(address));

    Context::queue()->async([strongSelf = shared_from_this(), addr, id] {
        strongSelf->datacenterSeedAddressByDatacenterId[id] = addr;
    });
}

std::shared_ptr<TransportScheme> Context::transportSchemeForDatacenterId(int32_t id) {
    auto strongSelf = shared_from_this();
    std::shared_ptr<TransportScheme> scheme = nullptr;

    Context::queue()->sync([&, strongSelf, id] {
        std::shared_ptr<DatacenterAddress> address = strongSelf->getDatacenterAddressForDatacenterId(id);

        if (address == nullptr)
            address = strongSelf->getDatacenterSeedAddressForDatacenterId(id);

        if (address == nullptr)
            strongSelf->addressSetForDatacenterIdRequired(id);
        else {
            scheme = std::make_shared<TransportScheme>(TransportType::Tcp, address);
        }
    });

    return scheme;
}

void Context::transportSchemeForDatacenterIdRequired(int32_t id) {

}

void Context::addressSetForDatacenterIdRequired(int32_t id) {
#warning Implement getConfig
}

void Context::authInfoForDatacenterWithIdRequired(int32_t id) {
    Context::queue()->async([self = shared_from_this(), id] {
        auto it = self->datacenterAuthActionsByDatacenterId.find(id);

        if (it == self->datacenterAuthActionsByDatacenterId.end())
        {
            auto authAction = std::make_shared<DatacenterAuthAction>();
            authAction->setDelegate(self);
            self->datacenterAuthActionsByDatacenterId[id] = authAction;

            authAction->execute(self, id);
        }
    });
}

void Context::datacenterAuthActionCompleted(const DatacenterAuthAction &action) {
    Context::queue()->async([self = shared_from_this(), &action] {

        for (auto it = self->datacenterAuthActionsByDatacenterId.begin(); it != self->datacenterAuthActionsByDatacenterId.end(); )
        {
            if (it->second->internalId == action.internalId)
                self->datacenterAuthActionsByDatacenterId.erase(it++);
            else
                ++it;
        }
    });
}