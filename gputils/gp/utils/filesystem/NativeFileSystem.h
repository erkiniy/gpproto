//
// Created by ProDigital on 7/30/18.
//

#ifndef GPPROTO_NATIVEFILESYSTEM_H
#define GPPROTO_NATIVEFILESYSTEM_H

#include "gp/utils/filesystem/FileSystem.h"
#include <dirent.h>

namespace gpproto {

    class NativeFileSystem final : public FileSystem {
    public:
        NativeFileSystem(const std::string& basePath);
        ~NativeFileSystem();

        virtual void Initialize();

        virtual void Shutdown();

        virtual bool IsInitialized() const;

        virtual const std::string& BasePath() const;

        virtual const TFileList& FileList() const;

        virtual bool IsReadOnly() const;

        virtual FilePtr OpenFile(const FileInfo& filePath, int mode);

        virtual void CloseFile(FilePtr file);

        virtual bool CreateFile(const FileInfo& filePath);

        virtual bool RemoveFile(const FileInfo& filePath);

        virtual bool CopyFile(const FileInfo& src, const FileInfo& dest);

        virtual bool RenameFile(const FileInfo& src, const FileInfo& dest);

        virtual bool IsFileExists(const FileInfo& filePath) const;

        virtual bool IsFile(const FileInfo& filePath) const;

        virtual bool IsDir(const FileInfo& filePath) const;

    private:
        FilePtr FindFile(const FileInfo& fileInfo) const;
        void BuildFileList(DIR* dir, std::string basePath, TFileList& outFileList);

    private:
        std::string _basePath;
        bool _isInitialized;
        TFileList _fileList;
    };
}



#endif //GPPROTO_NATIVEFILESYSTEM_H
