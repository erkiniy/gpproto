//
// Created by ProDigital on 7/30/18.
//

#include "gp/utils/filesystem/NativeFileSystem.h"
#include <fstream>
#include "gp/utils/StringUtils.h"
#include "gp/utils/filesystem/NativeFile.h"
#include <sys/stat.h>
#include <array>

using namespace gpproto;

const uint64_t kChunkSize = 1024;

NativeFileSystem::NativeFileSystem(const std::string &basePath)
: _basePath(basePath)
, _isInitialized(false)
{
    if (!StringUtils::EndsWith(_basePath, "/"))
        _basePath += "/";

}


NativeFileSystem::~NativeFileSystem() {

}

void NativeFileSystem::Initialize() {
    if (_isInitialized)
        return;

    DIR* dir = opendir(BasePath().c_str());
    if (dir)
    {
        BuildFileList(dir, BasePath(), _fileList);
        _isInitialized = true;

        closedir(dir);
    }
    else {
        printf("Cannot open dir %d\n", errno);
    }
}

void NativeFileSystem::Shutdown() {
    _basePath = "";
    _fileList.clear();
    _isInitialized = false;
}

bool NativeFileSystem::IsInitialized() const {
    return _isInitialized;
}

const std::string& NativeFileSystem::BasePath() const {
    return _basePath;
}

const FileSystem::TFileList& NativeFileSystem::FileList() const {
    return _fileList;
}

bool NativeFileSystem::IsReadOnly() const {
    if (!IsInitialized())
        return true;

    struct stat fileStat;
    if (stat(BasePath().c_str(), &fileStat) < 0)
        return false;

    return fileStat.st_mode & S_IWGRP;
}

FilePtr NativeFileSystem::OpenFile(const FileInfo &filePath, int mode) {
    FileInfo fileInfo(BasePath(), filePath.AbsolutePath(), false);
    FilePtr file = FindFile(fileInfo);

    bool isExists = (file != nullptr);

    if (!isExists)
    {
        mode |= File::truncate;
        file.reset(new NativeFile(fileInfo));
    }

    file->Open(mode);

    if (!isExists && file->isOpened())
        _fileList.insert(file);

    return file;
}

void NativeFileSystem::CloseFile(FilePtr file) {
    if (file)
        file->Close();
}

bool NativeFileSystem::CreateFile(const FileInfo &filePath) {

    if (!IsReadOnly() && !IsFileExists(filePath))
    {
        FileInfo fileInfo(BasePath(), filePath.AbsolutePath(), false);
        FilePtr file = OpenFile(filePath, File::write | File::truncate);

        if (file)
        {
            file->Close();
            return true;
        }
    }

    return false;
}

bool NativeFileSystem::RemoveFile(const FileInfo &filePath) {
    FilePtr file = FindFile(BasePath() + filePath.Name());
    if (!IsReadOnly() && file)
    {
        FileInfo fileInfo(BasePath(), file->fileInfo().AbsolutePath(), false);
        if (remove(fileInfo.AbsolutePath().c_str()))
            _fileList.erase(file);
    }

    return true;
}

bool NativeFileSystem::CopyFile(const FileInfo &src, const FileInfo &dest) {
    if (!IsReadOnly())
    {
        FilePtr fromFile = FindFile(src);
        FilePtr toFile = OpenFile(dest, File::write);

        if (fromFile && toFile)
        {
            uint64_t size = kChunkSize;
            std::vector<uint8_t> buff((size_t)size);
            do
            {
                fromFile->Read(buff.data(), kChunkSize);
                toFile->Write(buff.data(), size);
            }
            while (size == kChunkSize);

            return true;
        }
    }

    return false;
}

bool NativeFileSystem::RenameFile(const FileInfo &src, const FileInfo &dest) {
    if (!IsReadOnly())
        return false;

    FilePtr fromFile = FindFile(src);
    FilePtr toFile = FindFile(dest);

    if (fromFile && !toFile)
    {
        FileInfo toInfo(BasePath(), dest.AbsolutePath(), false);

        if (rename(fromFile->fileInfo().AbsolutePath().c_str(), toInfo.AbsolutePath().c_str()))
        {
            _fileList.erase(fromFile);
            toFile = OpenFile(dest, File::read);

            if (toFile)
            {
                toFile->Close();
                return true;
            }
        }
    }

    return false;
}

bool NativeFileSystem::IsFileExists(const FileInfo &filePath) const {
    return (FindFile(BasePath() + filePath.Name()) != nullptr);
}

bool NativeFileSystem::IsFile(const gpproto::FileInfo &filePath) const {
    FilePtr file = FindFile(filePath);

    if (file)
        return !file->fileInfo().isDir();

    return false;
}

bool NativeFileSystem::IsDir(const FileInfo &filePath) const {
    FilePtr file = FindFile(filePath);

    if (file)
        return file->fileInfo().isDir();

    return false;
}

FilePtr NativeFileSystem::FindFile(const FileInfo &fileInfo) const {

    auto it = std::find_if(_fileList.begin(), _fileList.end(), [&](FilePtr file) {
        return file->fileInfo() == fileInfo;
    });

    if (it != _fileList.end())
        return *it;

    return nullptr;
}

void NativeFileSystem::BuildFileList(DIR *dir, std::string basePath, FileSystem::TFileList &outFileList) {

    if (!StringUtils::EndsWith(basePath, "/"))
        basePath += "/";

    struct dirent *ent;

    while ((ent = readdir(dir)) != NULL)
    {
        std::string fileName = ent->d_name;
        std::string filePath = basePath + fileName;
        DIR *childDir = opendir(filePath.c_str());

        bool isDotOrDotDot = StringUtils::EndsWith(fileName, ".") && childDir;

        if (childDir && !isDotOrDotDot)
            fileName += "/";

        FileInfo fileInfo(basePath, fileName, childDir != NULL);

        if (!FindFile(fileInfo))
        {
            FilePtr file(new NativeFile(fileInfo));
            outFileList.insert(file);
        }

        if (childDir)
        {
            if (!isDotOrDotDot)
                BuildFileList(childDir,(childDir ? filePath : basePath), outFileList);

            closedir(childDir);
        }
    }
}
