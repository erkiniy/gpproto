//
// Created by ProDigital on 7/28/18.
//

#ifndef GPPROTO_FILE_H
#define GPPROTO_FILE_H

#include "FileInfo.h"

namespace gpproto {

class File {
public:

    enum Origin {
        begin,
        end,
        set
    };

    enum FileMode {
        read = 0x1,
        write = 0x2,
        readWrite = read | write,
        append = 0x4,
        truncate = 0x8
    };


public:

    File() = default;
    ~File() = default;

    virtual const FileInfo& fileInfo() const = 0;

    virtual const uint64_t size() = 0;

    virtual bool isReadOnly() const = 0;

    virtual void Open(int mode) = 0;

    virtual void Close() = 0;

    virtual bool isOpened() const = 0;

    virtual uint64_t Seek(uint64_t offset, Origin origin) = 0;

    virtual uint64_t Tell() = 0;

    virtual uint64_t Read(uint8_t* buffer, uint64_t size) = 0;

    virtual uint64_t Write(const uint8_t* buffer, uint64_t size) = 0;

    template<typename T>
    bool Read(T& value) {
        return Read(&value, sizeof(value)) == sizeof(value);
    }

    template<typename T>
    bool Write(const T& value) {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

};


inline bool operator == (const File& _0, const File& _1) {
    return _0.fileInfo() == _1.fileInfo();
}


inline bool operator == (std::shared_ptr<File> _0, std::shared_ptr<File> _1) {
    if (!_0 || !_1)
        return false;

    return _0->fileInfo() == _1->fileInfo();
}

}
#endif //GPPROTO_FILE_H
