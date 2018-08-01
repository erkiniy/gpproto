//
// Created by ProDigital on 8/1/18.
//

#include "StreamSlice.h"
#include "ByteOrder.h"

using namespace gpproto;

char* StreamSlice::byteSwapped() {

    char* result = (char*)malloc(size);

    for (int i = 0; i < size / 2; ++i)
        *(result + i) = *(bytes - i - 1);

    return result;
}

char* StreamSlice::toLittleEndian() const {
    char* result = bytes;

#if GP_IS_BIG_ENDIAN && !GP_IS_LITTLE_ENDIAN
    result = byteSwapped();
#endif

    return result;
}

std::string StreamSlice::description() const {
    auto s = std::string("");

    for (int i = 0; i < size; i++) {
        printf("%02x ", *(bytes + i));
    }

    return s;
}
