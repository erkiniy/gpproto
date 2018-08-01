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
        iS->writeInt8(5);       // 0x5
        iS->writeInt32(231);    // 0x000000e7
        iS->writeBool(true);    // 0x997275b5
        iS->writeInt64(1);      // 0x0000000000000001
        iS->writeInt8(34);      // 0x22

        auto bytes = iS->currentBytes();
        auto bytesString = bytes->description();

        printf("Bytes = %s\n", bytesString.c_str());
    }
};