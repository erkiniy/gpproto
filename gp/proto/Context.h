//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CONTEXT_H
#define GPPROTO_CONTEXT_H

#include "gp/utils/DispatchQueue.h"
#include "gp/proto/AuthKeyInfo.h"
#include "gp/proto/DatacenterAddress.h"
#include "gp/proto/DatacenterAuthAction.h"

#include <unordered_map>
#include <mutex>

struct gp_environment;

namespace gpproto
{
    class TransportScheme;

    class ContextChangeListener {
    public:
        ContextChangeListener() : internalId(nextInternalId()) {};

        const int internalId;

        virtual void contextDatacenterAddressSetUpdated(const Context& context, int32_t datacenterId, std::vector<std::shared_ptr<DatacenterAddress>> addressSet) = 0;
        virtual void contextDatacenterAuthInfoUpdated(const Context& context, int32_t datacenterId, std::shared_ptr<AuthKeyInfo> authInfo) = 0;
        virtual void contextDatacenterTransportSchemeUpdated(const Context& context, int32_t datacenterId, std::shared_ptr<TransportScheme> scheme) = 0;

    private:
        static int nextInternalId() {
            static std::atomic_int id;
            return id++;
        }
    };

class Context final : public std::enable_shared_from_this<Context>, public DatacenterAuthActionDelegate {
    public:
        Context(std::shared_ptr<gp_environment> & environment): environment(std::move(environment)) {};

        ~Context() = default;

        Context(const Context&) = delete;

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.context");
            return q;
        }

        double getGlobalTime();

        double getGlobalTimeDifference();

        void setGlobalTimeDifference(double difference);

        std::shared_ptr<AuthKeyInfo> getAuthKeyInfoForDatacenterId(int32_t id);

        void setAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id);

        void updateAuthKeyInfoForDatacenterId(std::shared_ptr<AuthKeyInfo> keyInfo, int32_t id);

        std::shared_ptr<DatacenterAddress> getDatacenterAddressForDatacenterId(int32_t id);

        void setDatacenterAddressForDatacenterId(DatacenterAddress&& address, int32_t id);

        std::shared_ptr<DatacenterAddress> getDatacenterSeedAddressForDatacenterId(int32_t id);

        void setDatacenterSeedAddress(DatacenterAddress&& address, int32_t id);

        std::shared_ptr<TransportScheme> transportSchemeForDatacenterId(int32_t id);

        void updateTransportSchemeForDatacenterId(std::shared_ptr<TransportScheme> scheme, int32_t datacenterId);

        void addressSetForDatacenterIdRequired(int32_t id);

        void authInfoForDatacenterWithIdRequired(int32_t id);

        void transportSchemeForDatacenterIdRequired(int32_t id);

        void datacenterAuthActionCompleted(const DatacenterAuthAction& action) override;

        void addChangeListener(std::shared_ptr<ContextChangeListener> listener);
        void removeChangeListener(std::shared_ptr<ContextChangeListener> listener);

private:
        double globalTimeDifference = 0.0;
        std::unordered_map<int32_t, std::shared_ptr<AuthKeyInfo>> authInfoByDatacenterId;
        std::unordered_map<int32_t, std::shared_ptr<DatacenterAddress>> datacenterAddressByDatacenterId;
        std::unordered_map<int32_t, std::shared_ptr<DatacenterAddress>> datacenterSeedAddressByDatacenterId;
        std::unordered_map<int32_t, std::shared_ptr<DatacenterAuthAction>> datacenterAuthActionsByDatacenterId;
        std::unordered_map<int, std::shared_ptr<ContextChangeListener>> changeListeners;
        std::shared_ptr<gp_environment> environment;
    };
}


#endif //GPPROTO_CONTEXT_H
