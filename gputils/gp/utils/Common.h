//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#ifndef GPPROTO_COMMON_H
#define GPPROTO_COMMON_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <memory>
#include <type_traits>
#include "gp/utils/StreamSlice.h"

#define MIN(X, Y) ((X > Y) ? Y : X)
#define MAX(X, Y) ((X > Y) ? X : Y)

using int8 = std::int8_t;
using int16 = std::int16_t;
using uint16 = std::uint16_t;
using int32 = std::int32_t;
using uint32 = std::uint32_t;
using int64 = std::int64_t;
using uint64 = std::uint64_t;

static_assert(sizeof(std::uint8_t) == sizeof(unsigned char), "Unsigned char expected to be 8-bit");
using uint8 = unsigned char;

static_assert(static_cast<char>(128) == -128 || static_cast<char>(128) == 128,
              "Unexpected cast to char implementation-defined behaviour");
static_assert(static_cast<char>(256) == 0, "Unexpected cast to char implementation-defined behaviour");
static_assert(static_cast<char>(-256) == 0, "Unexpected cast to char implementation-defined behaviour");

// From integer type to integer type
template <typename to, typename from>
constexpr typename std::enable_if<std::is_integral<from>::value && std::is_integral<to>::value, to>::type
narrow_cast(const from& value)
{
    return static_cast<to>(value & (static_cast<typename std::make_unsigned<from>::type>(-1)));
}


#if !GP_WINDOWS
using size_t = std::size_t;
#endif

template <size_t size>
struct UInt {
    static_assert(size % 8 == 0, "size should be divisible by 8");
    uint8 raw[size / 8];

    UInt<size>() = default;
    UInt<size>(UInt<size> &&) noexcept = default;
};

template <size_t size>
inline bool operator==(const UInt<size> &a, const UInt<size> &b) {
    return std::memcmp(a.raw, b.raw, sizeof(a.raw)) == 0;
}

template <size_t size>
inline bool operator!=(const UInt<size> &a, const UInt<size> &b) {
    return !(a == b);
}

using UInt128 = UInt<128>;
using UInt256 = UInt<256>;

std::shared_ptr<gpproto::StreamSlice> hexToData(std::string hex);
double getAbsoluteSystemTime();
int64_t getAbsoluteSystemTimeInMillis();

void byteSwapUInt64(uint64_t& number);
void byteSwapUInt32(uint32_t& number);
void byteSwapInt32(int32_t& number);

std::string base64_encode(const gpproto::StreamSlice& input);

std::shared_ptr<gpproto::StreamSlice> base64_decode(const std::string& base64);

#endif //GPPROTO_COMMON_H
