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

    if (true) {
        auto iS = std::make_unique<InputStream>();
//        iS->writeInt8(5);       // 0x5
//        iS->writeInt32(0x7fffffff);    // 0x000000e7
//        iS->writeBool(true);    // 0x997275b5
        iS->writeInt64(9223372036854775807);      // 0x0000000000000001
//        iS->writeInt8(34);      // 0x22
        iS->writeDouble(3.14);

        //iS->writeString("Jalol Жалол");

        auto bytes = iS->currentBytes();
        auto bytesString = bytes->description();

        LOGV("Bytes = %s\n", bytesString.c_str());

        slice = bytes;

        LOGV("Os bytes = %lu", slice->size);

        auto oS = std::make_unique<OutputStream>(bytes);

        try {
//            auto five = oS->readInt8();
//            auto tto = oS->readInt32();
//            auto value = oS->readBool();
            auto one = oS->readInt64();
//            auto tf = oS->readInt8();
            auto pi = oS->readDouble();

            LOGV("PI is = %f, one is %lld", pi, one);
            //LOGV("Numbers read %d %d %d %lld %d %f", five, tto, value, one, tf, pi);

        } catch (OutputStreamException &e) {
            LOGV("Exception caught, message = %s", e.message.c_str());
        }
    }

//    unsigned char bytes[4] = { 0x0, 0x0, 0x0, 0xff };
//
//    int32_t value = (int32_t)(bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24);
//
//    LOGV("Value %d", value);
};