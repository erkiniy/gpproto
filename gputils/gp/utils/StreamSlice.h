//
// Created by ProDigital on 8/1/18.
//

#ifndef GPPROTO_STREAMSLICE_H
#define GPPROTO_STREAMSLICE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "CustomStringConvertable.h"

namespace gpproto {
    struct StreamSlice: public CustomStringConvertable {

    public:
        char *bytes;
        size_t size;

        static const uint32_t TLBoolTrue = 0x997275b5;
        static const uint32_t TLBoolFalse = 0xbc799737;

        StreamSlice(const char *bytes, size_t size) {
            this->size = size;
            this->bytes = (char *)malloc(size);
            memcpy(this->bytes, bytes, size);
        }

        ~StreamSlice() {
            printf("Destructed StreamSlice with size %lu\n", this->size);
            if (bytes)
                free(bytes);
        }

        virtual std::string description() const;

        char* toLittleEndian() const;

    private:
        char* byteSwapped() const;
        };
}

#endif //GPPROTO_STREAMSLICE_H
