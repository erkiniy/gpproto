//
// Created by ProDigital on 7/30/18.
//

#ifndef GPPROTO_NATIVEFILE_H
#define GPPROTO_NATIVEFILE_H

#include "File.h"
#include <fstream>

namespace gpproto {
    class NativeFile final : public File {
    public:
        NativeFile(const FileInfo& fileInfo);
        ~NativeFile();

        const FileInfo& fileInfo() const override;

        const uint64_t size() override;

        bool isReadOnly() const override;

        void Open(int mode) override;

        void Close() override;

        bool isOpened() const override;

        uint64_t Seek(uint64_t offset, Origin origin) override;

        uint64_t Tell() override;

        uint64_t Read(uint8_t* buffer, uint64_t size) override;

        uint64_t Write(const uint8_t* buffer, uint64_t size) override;

    private:
        FileInfo _fileInfo;
        std::fstream _stream;
        bool _isReadOnly;
        int _mode;
    };
}

#endif //GPPROTO_NATIVEFILE_H
