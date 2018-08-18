//
// Created by ProDigital on 8/1/18.
//

#include "InputStream.h"
#include "ByteOrder.h"
#include "Logging.h"
#include <math.h>

using namespace gpproto;

void InputStream::writeBool(bool value) {
    uint32_t number = value ? StreamSlice::TLBoolTrue : StreamSlice::TLBoolFalse;
    writeUInt32(number);
}

void InputStream::writeUInt8(uint8_t number) {
    writeNumber(reinterpret_cast<const unsigned char *>(&number), sizeof(number));
}

void InputStream::writeInt8(int8_t number) {
    writeNumber(reinterpret_cast<const unsigned char *>(&number), sizeof(number));
}

void InputStream::writeInt16(int16_t number) {
    writeNumber(reinterpret_cast<const unsigned char *>(&number), sizeof(number));
}

void InputStream::writeInt32(int32_t number) {
    writeNumber(reinterpret_cast<const unsigned char *>(&number), sizeof(number));
}

void InputStream::writeUInt32(uint32_t number) {
    writeNumber(reinterpret_cast<const unsigned char *>(&number), sizeof(number));
}

void InputStream::writeInt64(int64_t number) {
    writeNumber(reinterpret_cast<const unsigned char *>(&number), sizeof(number));
}

void InputStream::writeDouble(double number) {
    writeNumber(reinterpret_cast<const unsigned char *>(&number), sizeof(number));
}

void InputStream::writeNumber(const unsigned char *number, size_t size) {
    StreamSlice s = StreamSlice(number, size, true);
    addSlice(s);
}

void InputStream::writeData(const StreamSlice &data) {
    addSlice(data);
}

void InputStream::writeRawString(const std::string &string) {
    if (string.length() == 0)
        writeInt8(0);
    else {
        const char* data = string.data();
        size_t size = strlen(data);
        writeInt32((int32_t)size);
        StreamSlice s = StreamSlice(reinterpret_cast<const unsigned char *>(data), size);
        writeData(s);
    }
}

void InputStream::writeString(const std::string &string) {
    if (string.length() == 0)
        writeInt8(0);
    else {
        const char* data = string.data();
        StreamSlice s = StreamSlice(reinterpret_cast<const unsigned char *>(data), strlen(data));
        writeBytes(s);
    }
}

void InputStream::writeBytes(const StreamSlice &data) {
    int8_t extraBytes = 0;

    if (data.size <= 253) {
        writeUInt8((uint8_t)data.size);
        extraBytes = 1;
    }
    else {
        int32_t sizeInt = ((int32_t)data.size & (0x00ffffff));
        writeUInt8((uint8_t)254);
        auto lengthData = StreamSlice(reinterpret_cast<const unsigned char*>(&sizeInt), 3, true);
        writeData(lengthData);
        extraBytes = 4;
    }

    writeData(data);

    while ((extraBytes + data.size) % 4) {
        writeInt8(0);
        extraBytes++;
    }
}

void InputStream::addSlice(const StreamSlice& slice) {

    while (remainingSize() < slice.size)
    {
        size_t targetChunks = 1 + (size_t)(ceil((double)(slice.size - remainingSize()) / kChunkSize)) + numberOfChunks;
        bytes = (unsigned char *)realloc(reinterpret_cast<void *>(bytes), kChunkSize * targetChunks);
        numberOfChunks = (int)targetChunks;
    }

    memcpy(bytes + currentSize, slice.toLittleEndian(), slice.size);

    currentSize += slice.size;
}

std::shared_ptr<StreamSlice> InputStream::currentBytes() const {
//    if (currentSize == 0)
//        return nullptr;

    LOGV("Total allocated slice is = %lu, when useful one is = %lu", kChunkSize * numberOfChunks, currentSize);

    return std::make_shared<StreamSlice>(bytes, currentSize);
}

size_t InputStream::remainingSize() const {
    return kChunkSize * numberOfChunks - currentSize;
}