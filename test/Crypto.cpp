
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
#include "gp/utils/OutputStream.h"

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
    /*
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
     */
};

BOOST_AUTO_TEST_CASE(gzip_test) {
    std::string hex = "1f8b08000000000000007d503b0e824010bdca4b2c68d403783a81024dd4c4560baf80c8465970b9c29b1bf9062db4b1d8cfccbcdf2e2f4c6c2d676fa56d58f3c9b4028f0cea3dac988351881b5bd6d016d84cc0c80784488cb6666d5bad8315c838a8d7bba65607abc4f0b2d1193357480eb22a5b8267f9b5ee08d96d19a0bb5c4619e7b6935498e33d6ee036bc2b886b2555010bd8de0a1f8fe2abc3ab4f247c1236e2934def700ac4f6ec953f4bdc77d40eff132b40fd05556e517f94ac745b0f36fdd5643cbadda0e8618999c69fcf7c0177413d6f6d010000";
    auto data = hexToData(hex);

    LOGV("Hex length %lu", data->size);

    auto unzippedData = Crypto::gzip_unzip(data);

    if (unzippedData)
    {
        auto str = std::string(reinterpret_cast<char *>(unzippedData->begin()), unzippedData->size);
        LOGV("UNZIPPED STRING is %s", str.c_str());
    }
}

