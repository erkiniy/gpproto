//
// Created by ProDigital on 7/30/18.
//

#ifndef GPPROTO_FILESYSTEM_H
#define GPPROTO_FILESYSTEM_H

#include <set>
#include "File.h"

namespace gpproto
{
    CLASS_PTR(File);

    class FileSystem {
    public:
        typedef std::set<FilePtr> TFileList;

        FileSystem() = default;
        ~FileSystem() = default;

        virtual void Initialize() = 0;

        virtual void Shutdown() = 0;

        virtual bool IsInitialized() const = 0;

        virtual const std::string& BasePath() const = 0;

        virtual const TFileList& FileList() const = 0;

        virtual bool IsReadOnly() const = 0;

        virtual FilePtr OpenFile(const FileInfo& filePath, int mode) = 0;

        virtual void CloseFile(FilePtr file) = 0;

        virtual bool CreateFile(const FileInfo& filePath) = 0;

        virtual bool RemoveFile(const FileInfo& filePath) = 0;

        virtual bool CopyFile(const FileInfo& src, const FileInfo& dest) = 0;

        virtual bool RenameFile(const FileInfo& src, const FileInfo& dest) = 0;

        virtual bool IsFileExists(const FileInfo& filePath) const = 0;

        virtual bool IsFile(const FileInfo& filePath) const = 0;

        virtual bool IsDir(const FileInfo& filePath) const = 0;

    };
}

#endif //GPPROTO_FILESYSTEM_H
