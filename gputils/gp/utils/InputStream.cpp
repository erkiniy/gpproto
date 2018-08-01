//
// Created by ProDigital on 8/1/18.
//

#include "InputStream.h"
#include "ByteOrder.h"

using namespace gpproto;

void InputStream::writeBool(bool value) {
    uint32_t number = value ? StreamSlice::TLBoolTrue : StreamSlice::TLBoolFalse;
    writeUInt32(number);
}

void InputStream::writeInt8(int8_t number) {
    writeNumber(reinterpret_cast<char *>(&number), sizeof(number));
}

void InputStream::writeInt16(int16_t number) {
    writeNumber(reinterpret_cast<char *>(&number), sizeof(number));
}

void InputStream::writeInt32(int32_t number) {
    writeNumber(reinterpret_cast<char *>(&number), sizeof(number));
}

void InputStream::writeUInt32(uint32_t number) {
    writeNumber(reinterpret_cast<char *>(&number), sizeof(number));
}

void InputStream::writeInt64(int64_t number) {
    writeNumber(reinterpret_cast<char *>(&number), sizeof(number));
}

void InputStream::writeNumber(char *number, size_t size) {
    StreamSlice s = StreamSlice(number, size);
    addSlice(s);
}

void InputStream::addSlice(const StreamSlice& slice) {

    while (remainingSize() < slice.size)
        bytes = (char *)realloc(reinterpret_cast<void *>(bytes), kChunkSize * (++numberOfChunks));

    memcpy(bytes + currentSize, slice.toLittleEndian(), slice.size);

    currentSize += slice.size;
}

std::shared_ptr<StreamSlice> InputStream::currentBytes() const {
    if (currentSize == 0)
        return nullptr;

    size_t size = currentSize;
    char* result = (char*)malloc(size);

    memcpy(result, bytes, size);

    return std::make_shared<StreamSlice>(result, size);
}

size_t InputStream::remainingSize() const {
    return kChunkSize * numberOfChunks - currentSize;
}