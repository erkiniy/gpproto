//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CONTEXT_H
#define GPPROTO_CONTEXT_H

#include <unordered_map>
#include "gp/utils/DispatchQueue.h"
#include "gp/proto/AuthKeyInfo.h"
#include "gp/proto/DatacenterAddress.h"
#include "gp/proto/DatacenterAuthAction.h"

namespace gpproto
{
    class TransportScheme;

class Context final : public std::enable_shared_from_this<Context>, public DatacenterAuthActionDelegate {
    public:
        Context() = default;

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

        void addressSetForDatacenterIdRequired(int32_t id);

        void authInfoForDatacenterWithIdRequired(int32_t id);

        void transportSchemeForDatacenterIdRequired(int32_t id);

        void datacenterAuthActionCompleted(const DatacenterAuthAction& action) override;

private:
        double globalTimeDifference;
        std::unordered_map<int32_t, std::shared_ptr<AuthKeyInfo>> authInfoByDatacenterId;
        std::unordered_map<int32_t, std::shared_ptr<DatacenterAddress>> datacenterAddressByDatacenterId;
        std::unordered_map<int32_t, std::shared_ptr<DatacenterAddress>> datacenterSeedAddressByDatacenterId;
        std::unordered_map<int32_t, std::shared_ptr<DatacenterAuthAction>> datacenterAuthActionsByDatacenterId;
    };
}


#endif //GPPROTO_CONTEXT_H
