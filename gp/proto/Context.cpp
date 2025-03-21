//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include <chrono>
#include <tuple>
#include <example/android/openssl-1.1.1a-clang/include/openssl/e_os2.h>
#include "gp/proto/Context.h"
#include "gp/network/TransportScheme.h"
#include "gp/utils/Common.h"
#include "gp/utils/Logging.h"
#include "gp/utils/DispatchQueue.h"

#include "gp/proto/ProtoInternalMessage.h"
#include "gp/utils/SecureKeychain.h"
#include "gp/proto/JsonAdapter.h"
#include "gp/proto/DatacenterSaltsetInfo.h"

#include "gp/utils/third_party/nlohmann/json.hpp"

#include "gp_client_data.h"

using namespace gpproto;
using namespace nlohmann;

Context::Context(std::shared_ptr<gp_environment> & environment): environment(environment) {
    appSupportedIds.clear();

    for (int i = 0; i < environment->supported_types_count; i++)
        appSupportedIds.insert((uint32_t) environment->supported_types[i]);
}

std::shared_ptr<DispatchQueue> Context::queue() {
    static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.context");
    return q;
}

std::shared_ptr<ProtoInternalMessage> Context::parseSupportedMessage(const std::shared_ptr<StreamSlice> & data) {
    uint32_t signature;
    memcpy(&signature, data->rbegin(), 4);

    auto it = appSupportedIds.find(signature);
    if (it == appSupportedIds.end()) {
        LOGE("Cannot parse signature %x", signature);
        for(auto id : appSupportedIds) {
            LOGE("Id: %x", id);
        }
        return nullptr;
    }

    return std::make_shared<AppSupportedMessage>(data);
}

double Context::getGlobalTime() {
    return getAbsoluteSystemTime() + getGlobalTimeDifference();
}

double Context::getGlobalTimeDifference() {

    auto diffPtr = std::make_shared<double>();

    Context::queue()->sync([self = shared_from_this(), &diffPtr]() mutable {
        auto diff = self->globalTimeDifference;
        auto ptr = new double(diff);
        diffPtr.reset(ptr);
    });

    return *diffPtr;
}

void Context::setKeychain(std::shared_ptr<SecureKeychain> keychain) {
    Context::queue()->async([self = shared_from_this(), keychain] {
        self->keychain = keychain;

        self->globalTimeDifference = JsonAdapter::toTimeDifference(keychain->getObject("globalTimeDifference", "temp"));

        self->authInfoByDatacenterId = JsonAdapter::toAuthKeyInfoByDatacenterId(keychain->getObject("authInfoById", "persistent"));
    });
}

void Context::setGlobalTimeDifference(double difference) {
    Context::queue()->async([self = shared_from_this(), difference] {
        self->globalTimeDifference = difference;
    });

    keychain->setObject(JsonAdapter::fromTimeDifference(difference), "globalTimeDifference", "temp");
}

std::shared_ptr<AuthKeyInfo> Context::getAuthKeyInfoForDatacenterId(int32_t id) {
    std::shared_ptr<AuthKeyInfo> infoPtr;

    Context::queue()->sync([self = shared_from_this(), id, &infoPtr]() mutable {
        auto it = self->authInfoByDatacenterId.find(id);

        if (it != self->authInfoByDatacenterId.end())
            infoPtr.reset(new AuthKeyInfo((*self->authInfoByDatacenterId[id])));

    });

    if (infoPtr != nullptr) {
        LOGV("[Context getAuthKeyInfoForDatacenterId] -> auth info found");
        return infoPtr;
    }

    LOGI("[Context getAuthKeyInfoForDatacenterId] -> auth info not found");

    return nullptr;
}

void Context::updateAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id) {
    Context::queue()->async([self = shared_from_this(), keyInfo, id] {

        self->setAuthKeyInfoForDatacenterId(keyInfo, id);

        if (keyInfo != nullptr)
        {
            for (const auto & listener : self->changeListeners)
            {
                if (auto strongListener = listener.second)
                    strongListener->contextDatacenterAuthInfoUpdated(*self, id, keyInfo);
            }
        }
    });

}

void Context::setAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id) {

    Context::queue()->async([self = shared_from_this(), keyInfo, id] {

        if (keyInfo != nullptr)
            self->authInfoByDatacenterId[id] = keyInfo;

        self->keychain->setObject(JsonAdapter::fromAuthKeyInfoByDatacenterId(self->authInfoByDatacenterId), "authInfoById", "persistent");
    });
}

std::shared_ptr<DatacenterAddress> Context::getDatacenterAddressForDatacenterId(int32_t id) {

    std::shared_ptr<DatacenterAddress> addressPtr;

    Context::queue()->sync([self = shared_from_this(), &addressPtr, id]() mutable {
        //LOGV("[Context getDatacenterAddressForDatacenterId]");
        auto it = self->datacenterAddressByDatacenterId.find(id);

        if (it != self->datacenterAddressByDatacenterId.end())
            addressPtr.reset(new DatacenterAddress(*(self->datacenterAddressByDatacenterId[id])));

        LOGV("[Context getDatacenterAddressForDatacenterId] -> returning address %d", addressPtr != nullptr);

    });

    return addressPtr;
}

void Context::setDatacenterAddressForDatacenterId(DatacenterAddress&& address, int32_t id) {
    auto addr = std::make_shared<DatacenterAddress>(std::move(address));

    Context::queue()->async([self = shared_from_this(), addr, id] {
        self->datacenterAddressByDatacenterId[id] = addr;
    });
#warning implement storing to keychain
}

std::shared_ptr<DatacenterAddress> Context::getDatacenterSeedAddressForDatacenterId(int32_t id) {
    std::shared_ptr<DatacenterAddress> addressPtr;

    Context::queue()->sync([self = shared_from_this(), &addressPtr, id]() mutable {
        LOGV("[Context getDatacenterSeedAddressForDatacenterId]");
        auto it = self->datacenterSeedAddressByDatacenterId.find(id);

        if (it != self->datacenterSeedAddressByDatacenterId.end()) {
            addressPtr.reset(new DatacenterAddress(*(self->datacenterSeedAddressByDatacenterId[id])));
        }
        if (addressPtr == nullptr)
            LOGV("[Context getDatacenterSeedAddressForDatacenterId] -> addressSeed not found");
    });

    if (addressPtr != nullptr) {
        LOGV("[Context getDatacenterSeedAddressForDatacenterId] -> address found");
        return addressPtr;
    }

    return nullptr;
}

void Context::setDatacenterSeedAddress(DatacenterAddress && address, int32_t id) {

    auto addr = std::make_shared<DatacenterAddress>(std::move(address));

    Context::queue()->async([self = shared_from_this(), addr, id] {
        self->datacenterSeedAddressByDatacenterId[id] = addr;
    });

}

std::shared_ptr<TransportScheme> Context::transportSchemeForDatacenterId(int32_t id) {

    LOGV("[Context transportSchemeForDatacenterId] for datacenterId %d", id);

    std::shared_ptr<DatacenterAddress> address = getDatacenterAddressForDatacenterId(id);

    if (address == nullptr)
        address = getDatacenterSeedAddressForDatacenterId(id);

    if (address == nullptr)
        addressSetForDatacenterIdRequired(id);
    else {
        LOGV("[Context transportSchemeForDatacenterId] returning scheme");
        return std::make_shared<TransportScheme>(TransportType::Tcp, address);
    }

    return nullptr;
}

void Context::transportSchemeForDatacenterIdRequired(int32_t id) {
    Context::queue()->async([self = shared_from_this(), id] {
        auto scheme = self->transportSchemeForDatacenterId(id);

        self->updateTransportSchemeForDatacenterId(scheme, id);
    });
}

void Context::addressSetForDatacenterIdRequired(int32_t id) {
#warning Implement getConfig
    LOGV("[Context addressSetForDatacenterIdRequired]");
}

void Context::authInfoForDatacenterWithIdRequired(int32_t id) {
    Context::queue()->async([self = shared_from_this(), id] {
        LOGV("[Context authInfoForDatacenterWithIdRequired]");

        auto it = self->datacenterAuthActionsByDatacenterId.find(id);

        if (it == self->datacenterAuthActionsByDatacenterId.end())
        {
            auto authAction = std::make_shared<DatacenterAuthAction>();
            authAction->setDelegate(self);
            self->datacenterAuthActionsByDatacenterId.insert({id, authAction});

            authAction->execute(self, id);
        }
        else {
            LOGV("[Context authInfoForDatacenterWithIdRequired] -> action found");
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

void Context::updateTransportSchemeForDatacenterId(std::shared_ptr<TransportScheme> scheme, int32_t datacenterId) {
    Context::queue()->async([self = shared_from_this(), scheme, datacenterId] {
        if (scheme != nullptr && datacenterId != 0)
        {
            for (const auto& listener : self->changeListeners)
            {
                if (auto strongListener = listener.second)
                    strongListener->contextDatacenterTransportSchemeUpdated(*self, datacenterId, scheme);
            }
        }
    });
}

void Context::addChangeListener(std::shared_ptr<ContextChangeListener> listener) {
    Context::queue()->async([self = shared_from_this(), listener] {
        auto it = self->changeListeners.find(listener->internalId);

        if (it == self->changeListeners.end())
            self->changeListeners[listener->internalId] = listener;

    });
}

void Context::removeChangeListener(std::shared_ptr<ContextChangeListener> listener) {
    Context::queue()->sync([self = shared_from_this(), listener] {
        auto it = self->changeListeners.find(listener->internalId);

        if (it != self->changeListeners.end())
            self->changeListeners.erase(it);
    });

}