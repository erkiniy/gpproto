//
// Created by Jaloliddin Erkiniy on 9/7/18.
//

#ifndef GPPROTO_DATACENTERACTION_H
#define GPPROTO_DATACENTERACTION_H

#include <memory>
#include <atomic>
#include "gp/proto/DatacenterAuthMessageService.h"

namespace gpproto
{
    class Proto;
    class Context;
    class DatacenterAuthAction;
    class AuthKeyInfo;

    typedef double TimeInterval;

    class DatacenterAuthActionDelegate {
    public:
        virtual void datacenterAuthActionCompleted(const DatacenterAuthAction& action) = 0;
    };

class DatacenterAuthAction : public std::enable_shared_from_this<DatacenterAuthAction>, public DatacenterAuthMessageServiceDelegate {
    public:
        DatacenterAuthAction() : internalId(nextInternalId()) {};

        DatacenterAuthAction(const DatacenterAuthAction&) = delete;

        ~DatacenterAuthAction() {
            printf("DatacenterAuthAction dealloc");
            cleanup();
        };

        void execute(std::shared_ptr<Context> context, int32_t datacenterId);
        void cancel();
        void setDelegate(std::shared_ptr<DatacenterAuthActionDelegate> delegate) {
            this->delegate = delegate;
        }

        void authMessageServiceCompletedWithAuthKey(const DatacenterAuthMessageService& service, std::shared_ptr<AuthKeyInfo> authKey, int64_t timestamp) override;

    const int internalId;
    private:
        void fail();
        void cleanup();
        void complete();
        std::weak_ptr<DatacenterAuthActionDelegate> delegate;
        std::weak_ptr<Context> context;
        int32_t datacenterId;
        std::shared_ptr<Proto> proto;

        TimeInterval startTime;

    static int nextInternalId() {
        static std::atomic_int internalId = 0;
        return internalId++;
    }
    };
}

#endif //GPPROTO_DATACENTERACTION_H
