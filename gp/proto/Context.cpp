//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#include <chrono>
#include <tuple>
#include "gp/proto/Context.h"
#include "gp/network/TransportScheme.h"

using namespace gpproto;

double Context::getGlobalTime() {
    auto timestamp = std::chrono::seconds(std::time(nullptr)).count();
    return static_cast<double>(timestamp) + getGlobalTimeDifference();
}

double Context::getGlobalTimeDifference() {
    double *diffPtr = nullptr;

    mutex.lock();
    auto diff = globalTimeDifference;
    mutex.unlock();
    diffPtr = new double(diff);

    double difference = *diffPtr;
    delete diffPtr;

    return difference;
}

void Context::setGlobalTimeDifference(double difference) {
    Context::queue()->async([self = shared_from_this(), difference] {
        self->globalTimeDifference = difference;
    });
#warning implement storing to keychain
}

std::shared_ptr<AuthKeyInfo> Context::getAuthKeyInfoForDatacenterId(int32_t id) {
    AuthKeyInfo* infoPtr = nullptr;

    mutex.lock();
    auto it = authInfoByDatacenterId.find(id);

    if (it != authInfoByDatacenterId.end())
        infoPtr = new AuthKeyInfo((*authInfoByDatacenterId[id]));
    mutex.unlock();

    if (infoPtr != nullptr) {
        LOGV("[Context getAuthKeyInfoForDatacenterId] -> auth info found");
        return std::shared_ptr<AuthKeyInfo>(infoPtr);
    }

    return nullptr;
}

void Context::updateAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id) {
    Context::queue()->async([self = shared_from_this(), keyInfo, id] {
        self->setAuthKeyInfoForDatacenterId(keyInfo, id);

        if (keyInfo != nullptr && id != 0)
        {
            LOGV("[Context updateAuthKeyInfoForDatacenterId] -> auth info updated");
            //TODO: keychain

            for (const auto & listener : self->changeListeners)
            {
                if (auto strongListener = listener.second.lock())
                    strongListener->contextDatacenterAuthInfoUpdated(*self, id, keyInfo);
            }
        }
    });

}

void Context::setAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id) {

    Context::queue()->async([self = shared_from_this(), keyInfo, id] {
        if (keyInfo != nullptr && id != 0)
            self->mutex.lock();
            self->authInfoByDatacenterId[id] = keyInfo;
            self->mutex.unlock();
    });
#warning implement storing to keychain

}

std::shared_ptr<DatacenterAddress> Context::getDatacenterAddressForDatacenterId(int32_t id) {

    DatacenterAddress *addressPtr = nullptr;

    LOGV("[Context getDatacenterAddressForDatacenterId]");
    mutex.lock();
    auto it = datacenterAddressByDatacenterId.find(id);

    if (it != datacenterAddressByDatacenterId.end())
        addressPtr = new DatacenterAddress(*(datacenterAddressByDatacenterId[id]));
    mutex.unlock();

    LOGV("[Context getDatacenterAddressForDatacenterId] -> returning address %d", addressPtr != nullptr);

    if (addressPtr != nullptr)
        return std::shared_ptr<DatacenterAddress>(addressPtr);

    return nullptr;
}

void Context::setDatacenterAddressForDatacenterId(DatacenterAddress&& address, int32_t id) {
    auto addr = std::make_shared<DatacenterAddress>(std::move(address));

    Context::queue()->async([self = shared_from_this(), addr, id] {
        self->mutex.lock();
        self->datacenterAddressByDatacenterId[id] = addr;
        self->mutex.unlock();
    });
#warning implement storing to keychain
}

std::shared_ptr<DatacenterAddress> Context::getDatacenterSeedAddressForDatacenterId(int32_t id) {
    DatacenterAddress *addressPtr = nullptr;

    LOGV("[Context getDatacenterSeedAddressForDatacenterId]");
    mutex.lock();
    auto it = datacenterSeedAddressByDatacenterId.find(id);

    if (it != datacenterSeedAddressByDatacenterId.end()) {
        LOGV("AddressSeed found");
        addressPtr = new DatacenterAddress(*(datacenterSeedAddressByDatacenterId[id]));
    }
    mutex.unlock();

    if (addressPtr != nullptr) {
        LOGV("[Context getDatacenterSeedAddressForDatacenterId] -> address found");
        return std::shared_ptr<DatacenterAddress>(addressPtr);
    }

    LOGV("[Context getDatacenterSeedAddressForDatacenterId] -> addressSeed not found");

    return nullptr;
}

void Context::setDatacenterSeedAddress(DatacenterAddress &&address, int32_t id) {

    auto addr = std::make_shared<DatacenterAddress>(std::move(address));

    Context::queue()->async([self = shared_from_this(), addr, id] {
        self->mutex.lock();
        self->datacenterSeedAddressByDatacenterId[id] = addr;
        self->mutex.unlock();
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

        self->mutex.lock();

        auto it = self->datacenterAuthActionsByDatacenterId.find(id);

        if (it == self->datacenterAuthActionsByDatacenterId.end())
        {
            auto authAction = std::make_shared<DatacenterAuthAction>();
            LOGV("[Context authInfoForDatacenterWithIdRequired] -> createdAuthAction");
            authAction->setDelegate(self);
            LOGV("[Context authInfoForDatacenterWithIdRequired] -> setDelegate");
            self->datacenterAuthActionsByDatacenterId[id] = authAction;
            LOGV("[Context authInfoForDatacenterWithIdRequired] -> insert");

            authAction->execute(self, id);
        }
        else {
            LOGV("[Context authInfoForDatacenterWithIdRequired] -> action found");
        }
        self->mutex.unlock();
    });
}

void Context::datacenterAuthActionCompleted(const DatacenterAuthAction &action) {
    Context::queue()->async([self = shared_from_this(), &action] {

        self->mutex.lock();
        for (auto it = self->datacenterAuthActionsByDatacenterId.begin(); it != self->datacenterAuthActionsByDatacenterId.end(); )
        {
            if (it->second->internalId == action.internalId)
                self->datacenterAuthActionsByDatacenterId.erase(it++);
            else
                ++it;
        }
        self->mutex.unlock();

    });
}

void Context::updateTransportSchemeForDatacenterId(std::shared_ptr<TransportScheme> scheme, int32_t datacenterId) {
    Context::queue()->async([self = shared_from_this(), scheme, datacenterId] {
        if (scheme != nullptr && datacenterId != 0)
        {
            for (const auto& listener : self->changeListeners)
            {
                if (auto strongListener = listener.second.lock())
                    strongListener->contextDatacenterTransportSchemeUpdated(*self, datacenterId, scheme);
            }
        }
    });
}

void Context::addChangeListener(std::shared_ptr<ContextChangeListener> listener) {
    Context::queue()->async([self = shared_from_this(), listener] {
        auto it = self->changeListeners.find(listener->internalId);

        LOGV("Inside");
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