//
// Created by Jaloliddin Erkiniy on 8/6/18.
//

#define BOOST_TEST_MODULE TimerTests
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "gp/utils/Timer.h"
#include "gp/utils/Logging.h"
#include "gp/utils/Semaphore.h"

using namespace gpproto;

BOOST_AUTO_TEST_CASE(timer_first) {
    auto s = std::make_shared<Semaphore>();

//        auto timer = std::shared_ptr<Timer>(new Timer(5.0, false, [s]{
//            LOGV("Timer fired");
//            s->notify();
//        }));
//
//        timer->start();
//        //timer->invalidate();
//        LOGV("Timer started");
//
//        auto timer2 = std::make_shared<Timer>(2.0, true, [&] {
//            LOGV("Timer2 fired");
//
//        });
//
//        timer2->start();
//
//    s->wait();
};