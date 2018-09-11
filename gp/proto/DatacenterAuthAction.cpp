//
// Created by Jaloliddin Erkiniy on 9/7/18.
//

#include "gp/proto/DatacenterAuthAction.h"
#include "gp/proto/Context.h"
#include "gp/proto/Proto.h"
#include "gp/proto/DatacenterAuthMessageService.h"
#include "gp/utils/Common.h"

using namespace gpproto;

void DatacenterAuthAction::execute(std::shared_ptr<Context> context, int32_t datacenterId) {
    LOGV("[DatacenterAuthAction execute]");

    this->startTime = getAbsoluteSystemTime();

    LOGV("Start execution of Auth action at %lf", startTime);

    this->datacenterId = datacenterId;
    this->context = context;

    if (datacenterId != 0 && context != nullptr)
    {
        if (context->getAuthKeyInfoForDatacenterId(datacenterId) != nullptr)
            complete();
        else {
            proto = std::make_shared<Proto>(context, datacenterId, true);
            proto->initialize();

            auto authService = std::make_shared<DatacenterAuthMessageService>(context);
            authService->setDelegate(shared_from_this());
            LOGV("[DatacenterAuthAction execute] after setting delegate");
            proto->addMessageService(authService);
        }
    } else {
        fail();
    }
}

void DatacenterAuthAction::authMessageServiceCompletedWithAuthKey(const DatacenterAuthMessageService &service,
                                                                  std::shared_ptr<AuthKeyInfo> authKey,
                                                                  int64_t timestamp) {
    if (auto strongContext = context.lock()) {
        strongContext->updateAuthKeyInfoForDatacenterId(authKey, datacenterId);
        complete();
        LOGV("[DatacenterAuthAction authMessageServiceCompletedWithAuthKey] auth key fetch action finished with keyId %lld", authKey->authKeyId);
    }
}

void DatacenterAuthAction::complete() {
    LOGV("[DatacenterAuthAction complete] aut key exchange completed in %lf", getAbsoluteSystemTime() - startTime);
    if (auto strongDelegate = delegate.lock())
        strongDelegate->datacenterAuthActionCompleted(*this);

    cleanup();
}

void DatacenterAuthAction::fail() {
    LOGV("DatacenterAuthAction fail");
    if (auto strongDelegate = delegate.lock())
        strongDelegate->datacenterAuthActionCompleted(*this);
}

void DatacenterAuthAction::cancel() {
    cleanup();
    fail();
}

void DatacenterAuthAction::cleanup() {
    LOGV("[DatacenterAuthAction cleanup]");
    if (proto)
    {
        proto->stop();
        proto = nullptr;
    }
}
