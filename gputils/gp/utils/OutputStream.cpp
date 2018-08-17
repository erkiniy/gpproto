//
// Created by ProDigital on 8/2/18.
//

#include "OutputStream.h"
#include "OutputStreamException.h"

#define MIN(X, Y) ((X > Y) ? Y : X)
#define MAX(X, Y) ((X > Y) ? X : Y)

using namespace gpproto;

uint8_t OutputStream::readUInt8() const {
    uint8_t result;
    memcpy(&result, readSlice(1, true)->toSystemEndian(), 1);
    return result;
}

int8_t OutputStream::readInt8() const {
    checkSize(1, __PRETTY_FUNCTION__);
    int8_t result;
    memcpy(&result, readSlice(1, true)->toSystemEndian(), 1);
    return result;
}

int16_t OutputStream::readInt16() const {
    checkSize(2, __PRETTY_FUNCTION__);
    int16_t result;
    memcpy(&result, readSlice(2, true)->toSystemEndian(), 2);
    return result;
}

uint32_t OutputStream::readUInt32() const {
    checkSize(4, __PRETTY_FUNCTION__);
    uint32_t result;
    memcpy(&result, readSlice(4, true)->toSystemEndian(), 4);
    return result;
}

int32_t OutputStream::readInt32() const {
    checkSize(4, __PRETTY_FUNCTION__);
    int32_t result;
    memcpy(&result, readSlice(4, true)->toSystemEndian(), 4);
    return result;
}

int64_t OutputStream::readInt64() const {
    checkSize(8, __PRETTY_FUNCTION__);
    int64_t result;
    memcpy(&result, readSlice(8, true)->toSystemEndian(), 8);
    return result;
}

bool OutputStream::readBool() const {
    checkSize(4, __PRETTY_FUNCTION__);
    uint32_t marker = readUInt32();
    return marker == StreamSlice::TLBoolTrue;
}

double OutputStream::readDouble() const {
    checkSize(8, __PRETTY_FUNCTION__);
    auto s = readSlice(8, true);
    auto bytes = s->toSystemEndian();
    double result;
    memcpy(&result, bytes, 8);
    return result;
}

std::shared_ptr<StreamSlice> OutputStream::readData(size_t length) const {
    checkSize(length, __PRETTY_FUNCTION__);
    return readSlice(length, false);
}

std::shared_ptr<StreamSlice> OutputStream::readDataMaxLength(size_t length) const noexcept {
    if (remainingSize() > length)
        return readSlice(length, false);

    return readSlice(remainingSize(), false);
}

std::string OutputStream::readStringRaw() const {
    checkSize(4, __PRETTY_FUNCTION__);
    int32_t length = readInt32();

    if (length == 0)
        return "";

    if (length < 0)
        throw OutputStreamException("Negative length marker for readBytes()", 400);

    size_t size = length;

    checkSize(size, __PRETTY_FUNCTION__);

    auto slice = readSlice(size, false);
    return slice->size == 0 ? "" : std::string(reinterpret_cast<char *>(slice->bytes), slice->size);
}

std::string OutputStream::readString() const {
    try {
        auto slice = readBytes();
        return slice->size == 0 ? "" : std::string(reinterpret_cast<char *>(slice->bytes), slice->size);
    }
    catch (OutputStreamException& e) {
        throw e;
    }
}

std::shared_ptr<StreamSlice> OutputStream::readBytes() const {
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
        throw OutputStreamException("Wrong length marker for readBytes()", 400);
    }

    if (length < 0)
        throw OutputStreamException("Negative length marker for readBytes()", 400);

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

std::shared_ptr<StreamSlice> OutputStream::readSlice(size_t size, bool number) const {
    auto s = std::make_shared<StreamSlice>(bytes + currentPosition, size, number);
    currentPosition += size;
    return s;
}

size_t OutputStream::remainingSize() const {
    return MAX(size - currentPosition, 0);
}

void OutputStream::checkSize(size_t size, std::string message) const {
    if (remainingSize() < size)
        throw OutputStreamException("Size not enough when revoking " + message, 400);
}
