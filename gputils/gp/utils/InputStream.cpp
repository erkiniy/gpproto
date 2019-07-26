//
// Created by ProDigital on 8/2/18.
//

#include "gp/utils/InputStream.h"
#include "gp/utils/StreamSlice.h"
#include "gp/utils/InputStreamException.h"

#define MIN(X, Y) ((X > Y) ? Y : X)
#define MAX(X, Y) ((X > Y) ? X : Y)

using namespace gpproto;

InputStream::InputStream(std::shared_ptr<gpproto::StreamSlice> data) : currentPosition(0), size(data->size) {
    this->bytes = (unsigned char *)malloc(data->size);
    memcpy(this->bytes, data->bytes, data->size);
}

uint8_t InputStream::readUInt8() const {
    uint8_t result;
    memcpy(&result, readSlice(1, true)->toSystemEndian(), 1);
    return result;
}

int8_t InputStream::readInt8() const {
    checkSize(1, __PRETTY_FUNCTION__);
    int8_t result;
    memcpy(&result, readSlice(1, true)->toSystemEndian(), 1);
    return result;
}

int16_t InputStream::readInt16() const {
    checkSize(2, __PRETTY_FUNCTION__);
    int16_t result;
    memcpy(&result, readSlice(2, true)->toSystemEndian(), 2);
    return result;
}

uint32_t InputStream::readUInt32() const {
    checkSize(4, __PRETTY_FUNCTION__);
    uint32_t result;
    memcpy(&result, readSlice(4, true)->toSystemEndian(), 4);
    return result;
}

int32_t InputStream::readInt32() const {
    checkSize(4, __PRETTY_FUNCTION__);
    int32_t result;
    memcpy(&result, readSlice(4, true)->toSystemEndian(), 4);
    return result;
}

int64_t InputStream::readInt64() const {
    checkSize(8, __PRETTY_FUNCTION__);
    int64_t result;
    memcpy(&result, readSlice(8, true)->toSystemEndian(), 8);
    return result;
}

uint64_t InputStream::readUInt64() const {
    checkSize(8, __PRETTY_FUNCTION__);
    uint64_t result;
    memcpy(&result, readSlice(8, true)->toSystemEndian(), 8);
    return result;
}

bool InputStream::readBool() const {
    checkSize(4, __PRETTY_FUNCTION__);
    uint32_t marker = readUInt32();
    return marker == StreamSlice::TLBoolTrue;
}

double InputStream::readDouble() const {
    checkSize(8, __PRETTY_FUNCTION__);
    auto s = readSlice(8, true);
    auto bytes = s->toSystemEndian();
    double result;
    memcpy(&result, bytes, 8);
    return result;
}

std::shared_ptr<StreamSlice> InputStream::readRemainingData() const {
    return readDataMaxLength(INT32_MAX);
}

std::shared_ptr<StreamSlice> InputStream::readData(size_t length) const {
    checkSize(length, __PRETTY_FUNCTION__);
    return readSlice(length, false);
}

std::shared_ptr<StreamSlice> InputStream::readDataMaxLength(size_t length) const {
    if (length <= remainingSize())
        return readSlice(length, false);

    return readSlice(remainingSize(), false);
}

std::string InputStream::readStringRaw() const {
    checkSize(4, __PRETTY_FUNCTION__);
    int32_t length = readInt32();

    if (length == 0)
        return "";

    if (length < 0)
        throw InputStreamException("Negative length marker for readBytes()", 400);

    size_t size = (size_t)length;

    checkSize(size, __PRETTY_FUNCTION__);

    auto slice = readSlice(size, false);
    return slice->size == 0 ? "" : std::string(reinterpret_cast<char *>(slice->bytes), slice->size);
}

std::string InputStream::readString() const {
    try {
        auto slice = readBytes();
        return slice->size == 0 ? "" : std::string(reinterpret_cast<char *>(slice->bytes), slice->size);
    }
    catch (InputStreamException& e) {
        throw e;
    }
}

std::shared_ptr<StreamSlice> InputStream::readBytes() const {
    checkSize(1, __PRETTY_FUNCTION__);
    uint8_t lengthMarker = readUInt8();
    int32_t length = lengthMarker;

    int8_t extraBytes = 0;

    if (lengthMarker == 0)
        return std::make_shared<StreamSlice>(nullptr, 0, false);

    if (lengthMarker <= 253)
        extraBytes = 1;
    else if (lengthMarker == 254) {
        checkSize(3, __PRETTY_FUNCTION__);
        length = 0;
        memcpy(&length, readSlice(3, true)->toSystemEndian(), 3);
        extraBytes = 4;
    }
    else {
        throw InputStreamException("Wrong length marker for readBytes()", 400);
    }

    if (length < 0)
        throw InputStreamException("Negative length marker for readBytes()", 400);

    size_t size = length;

    checkSize(size, __PRETTY_FUNCTION__);

    auto slice = readSlice(size, false);

    while ((extraBytes + size) % 4) {
        checkSize(1, __PRETTY_FUNCTION__);
        readUInt8();
        extraBytes++;
    }

    return slice;
}

std::shared_ptr<StreamSlice> InputStream::readSlice(size_t size, bool number) const {
    auto s = std::make_shared<StreamSlice>(bytes + currentPosition, size, number);
    currentPosition += size;
    return s;
}

size_t InputStream::remainingSize() const {
    return MAX(size - currentPosition, 0);
}

void InputStream::checkSize(size_t size, std::string message) const {
    if (remainingSize() < size)
        throw InputStreamException("Size not enough when revoking " + message, 400);
}
