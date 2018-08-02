//
// Created by ProDigital on 8/1/18.
//

#define BOOST_TEST_MODULE StreamTests
#include <boost/test/unit_test.hpp>
#include "gp/utils/Logging.h"
#include "gp/utils/InputStream.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <gp/utils/OutputStream.h>
#include <gp/utils/OutputStreamException.h>

using namespace gpproto;

BOOST_AUTO_TEST_CASE(input_streams) {

    std::shared_ptr<StreamSlice> slice;

    {
        auto iS = std::make_unique<InputStream>();
        iS->writeInt8(5);       // 0x5
        iS->writeInt32(231);    // 0x000000e7
        iS->writeBool(true);    // 0x997275b5
        iS->writeInt64(1);      // 0x0000000000000001
        iS->writeInt8(34);      // 0x22

        //iS->writeString("Jalol Жалол");

        auto bytes = iS->currentBytes();
        auto bytesString = bytes->description();

        LOGV("Bytes = %s\n", bytesString.c_str());

        slice = bytes;

        LOGV("Os bytes = %lu", slice->size);

        auto oS = std::make_unique<OutputStream>(bytes);

        try {
            auto five = oS->readInt8();
            auto tto = oS->readInt32();
            auto value = oS->readBool();
            auto one = oS->readInt64();
            auto tf = oS->readInt8();

            LOGV("Numbers read %d %d %d", five, tto, value);

        } catch (OutputStreamException &e) {
            LOGV("Exception caught, message = %s", e.message.c_str());
        }
    }

};