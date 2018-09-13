//
// Created by Jaloliddin Erkiniy on 8/4/18.
//

#include "gp/utils/Timer.h"
#include "gp/utils/Logging.h"
#include "gp/utils/Runloop.h"

#include <chrono>

using namespace gpproto;

void Timer::start() {

    LOGV("Starting timer");
    auto self = shared_from_this();
    Runloop::Default()->schedule(self);

    started = true;
}

bool Timer::isScheduled() const {
    return started;
}

void Timer::invalidate() {
    auto self = shared_from_this();
    Runloop::Default()->invalidate(self);
 }

void Timer::resetTimeout(float timeout) {
    invalidate();

    this->timeout = timeout;

    start();
}
