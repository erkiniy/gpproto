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
}
#endif //GPPROTO_RANDOM_H
