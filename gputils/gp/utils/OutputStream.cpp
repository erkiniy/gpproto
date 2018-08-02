//
// Created by ProDigital on 8/2/18.
//

#include "OutputStream.h"
#include "OutputStreamException.h"

#define MIN(X, Y) ((X > Y) ? Y : X)
#define MAX(X, Y) ((X > Y) ? X : Y)

using namespace gpproto;

uint8_t OutputStream::readUInt8() {
    checkSize(1, __PRETTY_FUNCTION__);
    uint8_t value = (uint8_t)*(readSlise(1).toLittleEndian());
    return value;
}

int8_t OutputStream::readInt8() {
    checkSize(1, __PRETTY_FUNCTION__);
    int8_t value = (int8_t)*(readSlise(1).toLittleEndian());
    return value;
}

int16_t OutputStream::readInt16() {
    checkSize(2, __PRETTY_FUNCTION__);
    int16_t value = (int16_t)*(readSlise(2).toLittleEndian());
    return value;
}

uint32_t OutputStream::readUInt32() {
    checkSize(4, __PRETTY_FUNCTION__);
    uint32_t value = (uint32_t)*(readSlise(4).toLittleEndian());
    return value;
}

int32_t OutputStream::readInt32() {
    checkSize(4, __PRETTY_FUNCTION__);
    int32_t value = (int32_t)*(readSlise(4).toLittleEndian());
    return value;
}

int64_t OutputStream::readInt64() {
    checkSize(8, __PRETTY_FUNCTION__);
    int64_t value = (int64_t)*(readSlise(8).toLittleEndian());
    return value;
}

bool OutputStream::readBool() {
    checkSize(4, __PRETTY_FUNCTION__);
    uint32_t marker = readUInt32();
    return marker == StreamSlice::TLBoolTrue;
}

StreamSlice OutputStream::readSlise(size_t size) {
    StreamSlice s = StreamSlice(bytes + currentPosition, size);
    currentPosition += size;
    return s;
}

size_t OutputStream::remainingSize() const {
    return MAX(size - currentPosition, 0);
}

void OutputStream::checkSize(size_t size, std::string message) const {
    if (remainingSize() - size < 0)
        throw OutputStreamException(message, 400);
}
