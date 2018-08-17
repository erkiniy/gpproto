//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#ifndef GPPROTO_RANDOM_H
#define GPPROTO_RANDOM_H

#include <cstdlib>
#include "Common.h"

namespace gpproto::Random {
    void secureBytes(unsigned char *ptr, size_t size);
    int32 secureInt32();
    int64 secureInt64();

    uint32 fastUInt32();
    uint64 fastUInt64();

    int fast(int min, int max);

}
#endif //GPPROTO_RANDOM_H
