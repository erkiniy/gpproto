
//
// Created by Jaloliddin Erkiniy on 8/18/18.
//

#define BOOST_TEST_MODULE TimerTests
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "gp/utils/Logging.h"
#include "gp/utils/Crypto.h"
#include "gp/utils/Common.h"
#include "gp/utils/Random.h"
#include "gp/utils/InputStream.h"

using namespace gpproto;

BOOST_AUTO_TEST_CASE(random_test) {

    for (int i = 0; i < 1; ++i)
    {
        LOGV("Random int32: %d = %d", i, Random::secureInt32());
        LOGV("Random int64: %d = %lld", i, Random::secureInt64());
        LOGV("Random fastUInt32: %d = %d", i, Random::fastUInt32());
        LOGV("Random fastUInt64: %d = %llud", i, Random::fastUInt64());
        LOGV("Random fastRange: %d = %d", i, Random::fast(15, 33));
        LOGV("-----------\n");
    }
};

BOOST_AUTO_TEST_CASE(aes_test) {
    std::string ivString = "3197377f86b1329832839d395a6e65a7";
    auto iv = hexToData(ivString);

    std::string keyString = "ba128357278774412c10c6f9bbcfc9183f639b9e6888c5b78abfad81bb4e2c89";
    auto key = hexToData(keyString);

    //LOGV("IV KEY HEX = %s", iv->description().c_str());
    //LOGV("KEY HEX = %s", key->description().c_str());

    std::string plain = "JaloliddinErkini";
    const char* data = plain.data();
    size_t size = strlen(data);

    UInt256 keyInt;
    UInt128 ivInt;

    memcpy(keyInt.raw, key->begin(), 32);
    memcpy(ivInt.raw, iv->begin(), 16);

    auto plainSlice = StreamSlice(reinterpret_cast<const unsigned char *>(data), size);

    auto cypher = Crypto::aes_cbc_encrypt(keyInt, &ivInt, plainSlice);

    //LOGV("Plain data = %s", plainSlice.description().c_str());
    //LOGV("Encrypted data = %s", cypher->description().c_str());

};

