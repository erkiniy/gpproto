//
// Created by ProDigital on 8/1/18.
//

#ifndef GPPROTO_STREAMSLICE_H
#define GPPROTO_STREAMSLICE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Logging.h"
#include "CustomStringConvertable.h"

namespace gpproto {
    struct StreamSlice: public CustomStringConvertable {

    public:
        char *bytes;
        size_t size;
        bool number;

        static const uint32_t TLBoolTrue = 0x997275b5;
        static const uint32_t TLBoolFalse = 0xbc799737;

        StreamSlice() = default;

        StreamSlice(const char *bytes, size_t size, bool number = false) {
            this->size = size;
            this->bytes = (char *)malloc(size);
            this->number = number;
            memcpy(this->bytes, bytes, size);
        }

        ~StreamSlice() {
            LOGV("Destructed StreamSlice with size %lu\n", this->size);
            if (bytes)
                free(bytes);
        }

        virtual std::string description() const;

        char* toLittleEndian() const;
        char* toSystemEndian() const;
        char* byteSwapped() const;
    };

    bool operator == (const StreamSlice &s1, const StreamSlice &s2) {
        return s1.size == s2.size && !memcmp(s1.bytes, s2.bytes, s1.size);
    }
}

#endif //GPPROTO_STREAMSLICE_H
