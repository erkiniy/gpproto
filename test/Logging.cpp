//
// Created by ProDigital on 7/27/18.
//

#define BOOST_TEST_MODULE MyTests
#include <boost/test/unit_test.hpp>
#include "gp/utils/Logging.h"

BOOST_AUTO_TEST_CASE(log_verbose) {
    //LOGV("Verbose log with number %d = 5", 23);
    BOOST_ASSERT(true);
};

BOOST_AUTO_TEST_CASE(log_debug) {
    //LOGD("Debug log with %d number = 5", 5);
    BOOST_ASSERT(true);
};

BOOST_AUTO_TEST_CASE(log_info) {
    //LOGI("Info log with %d number = 5", 5);
    BOOST_ASSERT(true);
};

BOOST_AUTO_TEST_CASE(log_warn) {
    //LOGW("Warn log with %d number = 5", 5);

    BOOST_ASSERT(true);
};

BOOST_AUTO_TEST_CASE(log_error) {
    //LOGW("Error log with %d number = 5", 5);
    BOOST_ASSERT(true);
};