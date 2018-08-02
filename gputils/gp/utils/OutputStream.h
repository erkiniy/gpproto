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

        OutputStream(StreamSlice& data) : currentPosition(0), size(data.size) {
            LOGV("Allocating OutputStream");
            this->bytes = (char *)malloc(data.size);
            memcpy(this->bytes, data.bytes, data.size);
        }

        ~OutputStream() {
            LOGV("Dealocating OutputStream");
            if (bytes)
                free(bytes);
        }

        uint8_t readUInt8();
        int8_t readInt8();
        int16_t readInt16();
        int32_t readInt32();
        uint32_t readUInt32();
        int64_t readInt64();

        double readDouble();
        bool readBool();

        std::shared_ptr<StreamSlice> readData(size_t legth);
        std::shared_ptr<StreamSlice> readDataMaxLength(size_t legth);
        std::shared_ptr<StreamSlice> readBytes();
        std::string readString();
        std::string readStringRaw();

    private:
        char* bytes;
        const size_t size;
        size_t currentPosition;

        size_t remainingSize() const;
        StreamSlice readSlise(size_t size);

        void checkSize(size_t size, std::string message) const;
    };

}
#endif //GPPROTO_OUTPUTSTREAM_H
