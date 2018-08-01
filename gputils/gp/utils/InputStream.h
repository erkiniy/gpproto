//
// Created by ProDigital on 8/1/18.
//

#ifndef GPPROTO_INPUTSTREAM_H
#define GPPROTO_INPUTSTREAM_H

#include <stdint.h>
#include <string>
#include "Buffer.h"
#include "StreamSlice.h"

namespace gpproto {

    class InputStream final {

    public:
        static const size_t kChunkSize = 12;

        InputStream() : currentSize(0), numberOfChunks(0) {
            bytes = (char*)malloc(1);
            printf("Input Stream allocated\n");
        }

        ~InputStream() {
            currentSize = 0;
            if (bytes)
                free(bytes);
        }

        InputStream(const InputStream&) = delete;

        void writeBool(bool value);

        void writeInt8(int8_t number);
        void writeInt16(int16_t number);
        void writeInt32(int32_t number);
        void writeUInt32(uint32_t number);
        void writeInt64(int64_t number);

        void writeDouble(double number);

        void writeData(const Buffer& data);
        void writeBytes(const Buffer& data);

        void writeRawString(const std::string& string);
        void writeString(const std::string& string);

        std::shared_ptr<StreamSlice> currentBytes() const;

    private:
        size_t currentSize;
        int numberOfChunks;
        char* bytes;

        void writeNumber(char* number, size_t size);

        void addSlice(const StreamSlice& slice);
        size_t remainingSize() const;
    };

}


#endif //GPPROTO_INPUTSTREAM_H
