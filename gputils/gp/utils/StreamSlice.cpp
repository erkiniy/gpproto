//
// Created by ProDigital on 8/1/18.
//

#include "gp/utils/StreamSlice.h"
#include "gp/utils/ByteOrder.h"
#include "gp/utils/Logging.h"
#include "gp/utils/Common.h"

using namespace gpproto;

unsigned char* StreamSlice::byteSwapped() const {

    unsigned char temp;
    for (int i = 0; i < size / 2 && size > 1; ++i)
    {
        unsigned char* left = (bytes + i);
        unsigned char* right = (bytes + (size - i) - 1);
        temp = *left;
        *left = *right;
        *right = temp;
    }

    return bytes;
}

unsigned char* StreamSlice::toLittleEndian() const {
    unsigned char* result = bytes;

#if GP_IS_BIG_ENDIAN && !GP_IS_LITTLE_ENDIAN
    if (number)
        result = byteSwapped();
#endif

    return result;
}

unsigned char* StreamSlice::toSystemEndian() const {
    unsigned char* result = bytes;

#if GP_IS_BIG_ENDIAN && !GP_IS_LITTLE_ENDIAN
    if (number)
        result = byteSwapped();
#endif

    return result;
}

std::shared_ptr<StreamSlice> StreamSlice::subData(int index, size_t length) {
    if (size < index + length)
        return nullptr;

    auto slice = std::make_shared<StreamSlice>(length);
    memcpy(slice->begin(), (bytes + index), length);

    return slice;
}

std::string StreamSlice::description() const {
    auto s = std::string("");

    static unsigned char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    for( int i = 0; i < size; ++i )
    {
        unsigned char const byte = bytes[i];

        s += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        s += hex_chars[ ( byte & 0x0F ) >> 0 ];
        s += " ";
    }

    return s;
}

std::shared_ptr<StreamSlice> StreamSlice::prefix(size_t length) {
    return std::make_shared<StreamSlice>(rbegin(), MIN(length, size));
}

std::shared_ptr<StreamSlice> StreamSlice::suffix(size_t length) {
    return std::make_shared<StreamSlice>(rbegin() + MAX(0, size - length), MIN(length, size));
}

std::shared_ptr<StreamSlice> StreamSlice::appended(const gpproto::StreamSlice &slice) {
    auto data = std::make_shared<StreamSlice>(size + slice.size);
    memcpy(data->begin(), rbegin(), size);
    memcpy(data->begin() + size, slice.rbegin(), slice.size);
    return data;
}
