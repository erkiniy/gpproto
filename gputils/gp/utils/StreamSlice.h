//
// Created by ProDigital on 8/1/18.
//

#ifndef GPPROTO_STREAMSLICE_H
#define GPPROTO_STREAMSLICE_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>

#include "gp/utils/Logging.h"
#include "gp/utils/CustomStringConvertable.h"

namespace gpproto {
    struct StreamSlice: public CustomStringConvertable {

    public:
        unsigned char *bytes;
        size_t size;
        bool number;

        static const uint32_t TLBoolTrue = 0x997275b5;
        static const uint32_t TLBoolFalse = 0xbc799737;

        StreamSlice() = delete;

        StreamSlice(const unsigned char *bytes, size_t size, bool number = false) {
            this->size = size;
            this->bytes = (unsigned char *)malloc(size);
            this->number = number;
            memcpy(this->bytes, bytes, size);
        }

        StreamSlice(const size_t& size) : bytes((unsigned char *)malloc(size)), size(size), number(false) {}

        ~StreamSlice() {
            //LOGV("Destructed StreamSlice with size %lu\n", this->size);
            if (bytes)
                delete bytes;
        }

        virtual std::string description() const;

        unsigned char* toLittleEndian() const;
        unsigned char* toSystemEndian() const;
        unsigned char* byteSwapped() const;

        std::shared_ptr<StreamSlice> subData(int index, size_t length);

        unsigned char* begin() {
            return bytes;
        }

        unsigned char* rbegin() const {
            return bytes;
        }

        unsigned char* end() {
            return bytes + size;
        }

        void removeLast() {
            if (size > 0)
                size--;
            //TODO: realloc
        }

//        inline StreamSlice operator + (const StreamSlice& s1, const StreamSlice& s2) {
//            auto s = StreamSlice(s1.size + s2.size);
//            memcpy(s.begin(), s1.rbegin(), s1.size);
//            memcpy(s.begin() + s1.size, s2.rbegin(), s2.size);
//            return s;
//        }
//
//        inline StreamSlice operator + (const StreamSlice& s1) const {
//            auto s = StreamSlice(this->size + s1.size);
//            memcpy(s.begin(), this->rbegin(), this->size);
//            memcpy(s.begin() + this->size, s1.rbegin(), s1.size);
//            return s;
//        }
    };

    inline bool operator == (const StreamSlice &s1, const StreamSlice &s2) {
        return s1.size == s2.size && (memcmp(s1.bytes, s2.bytes, s1.size) == 0);
    }

    inline bool operator != (const StreamSlice &s1, const StreamSlice &s2) {
        return !(s1 == s2);
    }


}

#endif //GPPROTO_STREAMSLICE_H
