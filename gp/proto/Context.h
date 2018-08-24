//
// Created by Jaloliddin Erkiniy on 8/22/18.
//

#ifndef GPPROTO_CONTEXT_H
#define GPPROTO_CONTEXT_H

#include <unordered_map>
#include "gp/utils/DispatchQueue.h"
#include "AuthKeyInfo.h"

namespace gpproto {
    class Context final : std::enable_shared_from_this<Context> {
    public:
        Context() = default;

        ~Context() = default;

        Context(const Context&) = delete;

        static std::shared_ptr<DispatchQueue> queue() {
            static std::shared_ptr<DispatchQueue> q = std::make_shared<DispatchQueue>("uz.gpproto.context");
            return q;
        }

        double getGlobalTimeDifference();

        void setGlobalTimeDifference(double difference);

        std::shared_ptr<AuthKeyInfo> getAuthKeyInfoForDatacenterId(int32_t id);

        void setAuthKeyInfoForDatacenterId(AuthKeyInfo&& keyInfo, int32_t id);

    private:
        double globalTimeDifference;
        std::unordered_map<int32_t, std::shared_ptr<AuthKeyInfo>> authInfoByDatacenterId;
    };
}


#endif //GPPROTO_CONTEXT_H
