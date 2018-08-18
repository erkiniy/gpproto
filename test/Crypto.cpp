//
// Created by Jaloliddin Erkiniy on 8/18/18.
//

//
// Created by Jaloliddin Erkiniy on 8/18/18.
//

#define BOOST_TEST_MODULE TimerTests
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "gp/utils/Logging.h"
#include "gp/utils/Crypto.h"
#include "gp/utils/Random.h"

using namespace gpproto;

BOOST_AUTO_TEST_CASE(random_test) {

    for (int i = 0; i < 10; ++i)
    {
        LOGV("Random int32: %d = %d", i, Random::secureInt32());
        LOGV("Random int64: %d = %lld", i, Random::secureInt64());
        LOGV("Random fastUInt32: %d = %d", i, Random::fastUInt32());
        LOGV("Random fastUInt64: %d = %llud", i, Random::fastUInt64());
        LOGV("Random fastRange: %d = %d", i, Random::fast(15, 33));
        LOGV("-----------\n");
    }
};