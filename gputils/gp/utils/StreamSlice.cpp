//
// Created by ProDigital on 8/1/18.
//

#include "StreamSlice.h"
#include "ByteOrder.h"

using namespace gpproto;

char* StreamSlice::byteSwapped() const {

    char temp;
    for (int i = 0; i < size / 2 && size > 1; ++i)
    {
        char* left = (bytes + i);
        char* right = (bytes + (size - i) - 1);
        temp = *left;
        *left = *right;
        *right = temp;
    }

    return bytes;
}

char* StreamSlice::toLittleEndian() const {
    char* result = bytes;

#if GP_IS_BIG_ENDIAN && !GP_IS_LITTLE_ENDIAN
    if (number)
        result = byteSwapped();
#endif

    return result;
}

std::string StreamSlice::description() const {
    auto s = std::string("");

    static char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    for( int i = 0; i < size; ++i )
    {
        char const byte = bytes[i];

        s += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        s += hex_chars[ ( byte & 0x0F ) >> 0 ];
        s += " ";
    }

    return s;
}
