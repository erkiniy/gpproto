//
// Created by ProDigital on 8/2/18.
//

#ifndef GPPROTO_OUTPUTSTREAM_H
#define GPPROTO_OUTPUTSTREAM_H

#include <cstdint>
#include <string>
#include "StreamSlice.h"
#include "Logging.h"

namespace gpproto {
    class OutputStream final {
    public:

        OutputStream(std::shared_ptr<StreamSlice> data) : currentPosition(0), size(data->size) {
            LOGV("Allocating OutputStream");
            this->bytes = (unsigned char *)malloc(data->size);
            memcpy(this->bytes, data->bytes, data->size);
        }

        ~OutputStream() {
            LOGV("Dealocating OutputStream");
            if (bytes)
                free(bytes);
        }

        uint8_t readUInt8() const;
        int8_t readInt8() const;
        int16_t readInt16() const;
        int32_t readInt32() const;
        uint32_t readUInt32() const;
        int64_t readInt64() const;

        template <class T> T readNumber() const {
            T result;
            memcpy(&result, readSlice(sizeof(T), true)->toSystemEndian(), sizeof(T));
            return result;
        }

        double readDouble() const;
        bool readBool() const;

        std::shared_ptr<StreamSlice> readData(size_t length) const;
        std::shared_ptr<StreamSlice> readDataMaxLength(size_t length) const noexcept;
        std::shared_ptr<StreamSlice> readBytes() const;
        std::string readString() const;
        std::string readStringRaw() const;

    private:
        unsigned char* bytes;
        const size_t size;
        mutable size_t currentPosition;

        size_t remainingSize() const;
        std::shared_ptr<StreamSlice> readSlice(size_t size, bool number) const;

        void checkSize(size_t size, std::string message) const;
    };


}
#endif //GPPROTO_OUTPUTSTREAM_H
