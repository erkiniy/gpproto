//
// Created by ProDigital on 8/1/18.
//

#define BOOST_TEST_MODULE StreamTests
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <gp/utils/InputStream.h>
#include <gp/utils/InputStreamException.h>
#include "gp/utils/Logging.h"
#include "gp/utils/OutputStream.h"

using namespace gpproto;

BOOST_AUTO_TEST_CASE(input_streams) {
    //0x62d6b459 = 1658238041
    //0xBEC9674E = -1094097074

    int32_t i1 = 0xBEC9674E;
    uint32_t i2 = 0xBEC9674E;

    LOGV("I1 = %d", i1);
    LOGV("I2 = %u", i2);

    std::shared_ptr<StreamSlice> slice;

    if (true) {
        auto iS = std::make_unique<InputStream>();
        iS->writeInt8(5);       // 0x5
        iS->writeInt32(-123421);    // 0x000000e7
        iS->writeString("ߏ");
        iS->writeBool(true);    // 0x997275b5
        iS->writeInt64(-2384792837492);      // 0x0000000000000001
        iS->writeInt8(34);      // 0x22
        iS->writeDouble(-3.14);
        iS->writeUInt8(240);

        auto bytes = iS->currentBytes();
        auto bytesString = bytes->description();

        LOGV("Bytes = %s\n", bytesString.c_str());

        slice = bytes;
    }

    LOGV("Os bytes = %lu", slice->size);

    auto oS = std::make_unique<OutputStream>(slice);

    try {
        auto five = oS->readNumber<int8_t>();
        auto tto = oS->readNumber<int32_t>();
        auto str = oS->readString();

        auto value = oS->readBool();
        auto one = oS->readNumber<int64_t>();
        auto tf = oS->readNumber<int8_t>();
        auto pi = oS->readNumber<double>();
        auto rs = oS->readNumber<uint8_t>();

        LOGV("Numbers read %d %d %d %lld %d %f %u str = %s", five, tto, value, one, tf, pi, rs, str.c_str());

    } catch (OutputStreamException &e) {
        LOGV("Exception caught, message = %s", e.message.c_str());
    }
};