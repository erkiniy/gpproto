//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#include "gp/utils/Common.h"

#include <chrono>

using namespace gpproto;

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
    //return (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / (double)1000.0;
    return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t getAbsoluteSystemTimeInMillis() {
    return (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void byteSwapUInt64(uint64_t& number) {
    auto value = number;
    void *pv = &value;

    size_t n = sizeof(number);
    auto p = static_cast<char*>(pv);

    size_t lo, hi;
    for(lo=0, hi=n-1; hi>lo; lo++, hi--)
    {
        char tmp=p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
    number = value;
}

void byteSwapUInt32(uint32_t& number) {
    auto value = number;
    void *pv = &value;

    size_t n = sizeof(number);
    auto p = static_cast<char*>(pv);

    size_t lo, hi;
    for(lo=0, hi=n-1; hi>lo; lo++, hi--)
    {
        char tmp=p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
    number = value;
}

void byteSwapInt32(int32_t& number) {
    auto value = number;
    void *pv = &value;

    size_t n = sizeof(number);
    auto p = static_cast<char*>(pv);

    size_t lo, hi;
    for(lo=0, hi=n-1; hi>lo; lo++, hi--)
    {
        char tmp=p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
    number = value;
}


static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table() {

    decoding_table = (char *)malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}


void base64_cleanup() {
    free(decoding_table);
}

char *_base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = (char *)malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}


unsigned char *_base64_decode(const char *data, size_t input_length, size_t *output_length) {
    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = (unsigned char *)malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
                          + (sextet_b << 2 * 6)
                          + (sextet_c << 1 * 6)
                          + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

std::string base64_encode(const gpproto::StreamSlice& input) {
    using namespace gpproto;
    size_t outputLength;

    auto encodedStringBytes = _base64_encode(input.rbegin(), input.size, &outputLength);

    auto encodedString = std::string(encodedStringBytes, outputLength);

    free(encodedStringBytes);

    return encodedString;
}

std::shared_ptr<gpproto::StreamSlice> base64_decode(const std::string& base64) {
    using namespace gpproto;
    const char *data = base64.data();

    size_t outputLength = 0;

    unsigned char *decodedData = _base64_decode(data, (size_t)(strlen(data)), &outputLength);

    std::shared_ptr<StreamSlice> s = std::make_shared<StreamSlice>(decodedData, outputLength);

    free(decodedData);

    return s;
}
