//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#include "Random.h"
#include "Logging.h"

#include <openssl/rand.h>

void gpproto::Random::secureBytes(unsigned char *ptr, size_t size) {
    int err = RAND_bytes(ptr, static_cast<int>(size));
    if (err)
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
