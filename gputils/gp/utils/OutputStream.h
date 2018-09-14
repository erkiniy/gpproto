//
// Created by ProDigital on 8/1/18.
//

#ifndef GPPROTO_INPUTSTREAM_H
#define GPPROTO_INPUTSTREAM_H

#include <cstdint>
#include <string>
#include "gp/utils/StreamSlice.h"

namespace gpproto {

    class OutputStream final {

    public:
        static const size_t kChunkSize = 16;

        OutputStream() : currentSize(0), numberOfChunks(1) {
            bytes = (unsigned char *)malloc(kChunkSize);
        }

        ~OutputStream() {
            currentSize = 0;
            if (bytes)
                free(bytes);
        }

        OutputStream(const OutputStream&) = delete;

        void writeBool(bool value);

        void writeUInt8(uint8_t number);
        void writeInt8(int8_t number);
        void writeInt16(int16_t number);
        void writeInt32(int32_t number);
        void writeUInt32(uint32_t number);
        void writeInt64(int64_t number);
        void writeUInt64(uint64_t number);

        void writeDouble(double number);

        void writeData(const StreamSlice& data);
        void writeBytes(const StreamSlice& data);

        void writeRawString(const std::string& string);
        void writeString(const std::string& string);

        std::shared_ptr<StreamSlice> currentBytes() const;
        size_t getCurrentSize() const;

    private:
        size_t currentSize;
        int numberOfChunks;
        unsigned char* bytes;

        void writeNumber(const unsigned char* number, size_t size);

        void addSlice(const StreamSlice& slice);
        size_t remainingSize() const;
    };

}


#endif //GPPROTO_INPUTSTREAM_H
