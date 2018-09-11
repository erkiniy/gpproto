//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#include "gp/utils/Common.h"
#include <chrono>

unsigned char hexToChar(const char& c) {
    if (c >= '0' && c <= '9')
        return (unsigned char)(c - '0');
    if (c >= 'A' && c <= 'F')
        return (unsigned char)(c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (unsigned char)(c - 'a' + 10);
    return 0; //throw std::invalid_argument("Invalid input string");
}

std::shared_ptr<gpproto::StreamSlice> hexToData(std::string hex) {
    auto s = std::make_shared<gpproto::StreamSlice>(hex.length() / 2 + hex.length() % 2);

    auto data = s->begin();
    auto dataPtr = data;


    if (hex.length() % 2)
        hex.insert(0, 1, '0');

    auto it = hex.begin();

    for(; it != hex.end(); it++)
        *dataPtr++ = hexToChar(*it++) << 4 | hexToChar(*it);

    return s;
}

double getAbsoluteSystemTime() {
    //return (double)(std::chrono::microseconds(std::time(nullptr)).count()) / 1000000.0;
    //return (double)(std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1));
    return (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
}

void byteSwapUInt64(uint64_t& number) {
    uint64_t result = 0;

    for (uint8_t i = 0; i < sizeof(number); ++i)
        result |= number & ((uint64_t)0xff << (i * 8));

    number = result;
}

void byteSwapUInt32(uint32_t& number) {
    uint32_t result = 0;

    for (uint8_t i = 0; i < sizeof(number); ++i)
        result |= number & ((uint32_t)0xff << (i * 8));

    number = result;
}

void byteSwapInt32(int32_t& number) {
    int32_t result = 0;

    for (uint8_t i = 0; i < sizeof(number); ++i)
        result |= number & ((int32_t)0xff << (i * 8));

    number = result;
}


