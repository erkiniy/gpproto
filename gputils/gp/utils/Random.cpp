//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#include "Random.h"
#include "Logging.h"
#include <random>
#include <limits>
#include <openssl/rand.h>

void gpproto::Random::secureBytes(unsigned char *ptr, size_t size) {
    RAND_bytes(ptr, static_cast<int>(size));
}

int32 gpproto::Random::secureInt32() {
    int32 val = 0;
    secureBytes(reinterpret_cast<unsigned char *>(&val), sizeof(int32));
    return val;
}
int64_t gpproto::Random::secureInt64() {
    int64 val = 0;
    secureBytes(reinterpret_cast<unsigned char *>(&val), sizeof(int64));
    return val;
}

static unsigned int rand_device_helper() {
    static std::random_device rd;
    return rd();
}

uint32 gpproto::Random::fastUInt32() {
    auto &rg = rand_device_helper;
    std::seed_seq seq{ rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg() };
    std::mt19937 gen(seq);
    return static_cast<uint32>(gen());
}
uint64 gpproto::Random::fastUInt64() {
    auto &rg = rand_device_helper;
    std::seed_seq seq{ rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg(), rg() };
    std::mt19937_64 gen(seq);
    return static_cast<uint64>(gen());
}

int gpproto::Random::fast(int min, int max) {
    if (min == std::numeric_limits<int>::min() && max == std::numeric_limits<int>::max())
        min++;

    return static_cast<int>(min + fastUInt32() % (max - min + 1));
}
