//
// Created by ProDigital on 8/1/18.
//

#define BOOST_TEST_MODULE StreamTests
#include <boost/test/unit_test.hpp>
#include "gp/utils/Logging.h"
#include "gp/utils/InputStream.h"
#include <boost/filesystem.hpp>
#include <iostream>

using namespace gpproto;

BOOST_AUTO_TEST_CASE(input_streams) {

    {
        auto iS = std::make_unique<InputStream>();
        printf("First\n");
        iS->writeInt8(5);
        printf("second\n");
        iS->writeInt32(231);
        iS->writeBool(true);
        iS->writeInt64(1);
        iS->writeInt8(34);

        auto bytes = iS->currentBytes();
        bytes->description();
    }
};