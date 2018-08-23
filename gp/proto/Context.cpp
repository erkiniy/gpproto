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

AuthKeyInfo* Context::getAuthKeyInfoForDatacenterId(int32_t id) {
    AuthKeyInfo* info;
    Context::queue()->sync([&] {

    });

    return info;
}