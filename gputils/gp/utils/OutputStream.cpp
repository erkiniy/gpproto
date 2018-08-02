//
// Created by ProDigital on 8/2/18.
//

#include "OutputStream.h"
#include "OutputStreamException.h"

#define MIN(X, Y) ((X > Y) ? Y : X)
#define MAX(X, Y) ((X > Y) ? X : Y)

using namespace gpproto;

uint8_t OutputStream::readUInt8() const {
    checkSize(1, __PRETTY_FUNCTION__);
    return(uint8_t)*(readSlice(1, true)->bytes);
}

int8_t OutputStream::readInt8() const {
    checkSize(1, __PRETTY_FUNCTION__);
    return (int8_t)*(readSlice(1, true)->bytes);
}

int16_t OutputStream::readInt16() const {
    checkSize(2, __PRETTY_FUNCTION__);
    return (int16_t)*(readSlice(2, true)->toSystemEndian());
}

uint32_t OutputStream::readUInt32() const {
    checkSize(4, __PRETTY_FUNCTION__);
    return (uint32_t)*(readSlice(4, true)->toSystemEndian());
}

int32_t OutputStream::readInt32() const {
    checkSize(4, __PRETTY_FUNCTION__);
    return (int32_t)*(readSlice(4, true)->toSystemEndian());
}

int64_t OutputStream::readInt64() const {
    checkSize(8, __PRETTY_FUNCTION__);
    return (int64_t)*(readSlice(8, true)->toSystemEndian());
}

bool OutputStream::readBool() const {
    checkSize(4, __PRETTY_FUNCTION__);
    uint32_t marker = readUInt32();
    return marker == StreamSlice::TLBoolTrue;
}

double OutputStream::readDouble() const {
    checkSize(8, __PRETTY_FUNCTION__);
    return (double)*(readSlice(8, true)->toSystemEndian());
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
    return slice->size == 0 ? "" : std::string(slice->bytes, slice->size);
}

std::string OutputStream::readString() const {
    try {
        auto slice = readBytes();
        return slice->size ==0 ? "" : std::string(slice->bytes, slice->size);
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
        length = (int32_t)(readUInt8());
        length |= (((int32_t)readUInt8()) >> 8);
        length |= (((int32_t)readUInt8()) >> 16);
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
    LOGV("Read slice %s", s->description().c_str());
    currentPosition += size;
    return s;
}

size_t OutputStream::remainingSize() const {
    return MAX(size - currentPosition, 0);
}

void OutputStream::checkSize(size_t size, std::string message) const {
    if (remainingSize() < size)
        throw OutputStreamException(message, 400);
}
