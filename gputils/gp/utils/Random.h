//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#ifndef GPPROTO_RANDOM_H
#define GPPROTO_RANDOM_H

#include <cstdlib>

namespace gpproto::Random {
    void secureBytes(unsigned char *ptr, size_t size);
    int32_t secureInt32();
    int64_t secureInt64();

    uint8_t fastUInt8();
    uint32_t fastUInt32();
    uint64_t fastUInt64();

    int fast(int min, int max);
}
#endif //GPPROTO_RANDOM_H
