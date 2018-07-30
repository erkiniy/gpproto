//
// Created by ProDigital on 7/30/18.
//

#include "FileInfo.h"
#include "gp/utils/StringUtils.h"

using namespace gpproto;

FileInfo::FileInfo()
{

}

FileInfo::~FileInfo() {

}


FileInfo::FileInfo(const std::string &basePath, const std::string &fileName, bool isDir) {

    Initialize(basePath, fileName, isDir);
}

FileInfo::FileInfo(const std::string &filePath) {

    std::size_t found = filePath.rfind("/");
    if (found != std::string::npos)
    {
        const std::string basePath = filePath.substr(0, found + 1);
        std::string fileName;
        if (found != filePath.length())
            fileName = filePath.substr(found + 1, filePath.length() - found - 1);

        Initialize(basePath, fileName, false);
    }
}

void FileInfo::Initialize(const std::string &basePath, const std::string &fileName, bool isDir) {

    _basePath = basePath;
    _name = fileName;
    _isDir = isDir;

    if (!StringUtils::EndsWith(_basePath, "/"))
        _basePath += "/";

    if (isDir && !StringUtils::EndsWith(_name, "/"))
        _name += "/";

    if (StringUtils::StartsWith(_name, "/"))
        _name = _name.substr(1, _name.length() - 1);

    _absolutePath = _basePath + _name;

    auto dotsNum = std::count(_name.begin(), _name.end(), '.');
    bool isDotOrDotDot = (dotsNum == _name.length() && isDir);

    if (!isDotOrDotDot)
    {
        std::size_t found = _name.rfind(".");
        if (found != std::string::npos)
        {
            _baseName = _name.substr(0, found);

            if (found < _name.length())
                _extension = _name.substr(found, _name.length() - found);
        }
    }
}

const std::string& FileInfo::Name() const {
    return _name;
}

const std::string& FileInfo::BaseName() const {
    return _baseName;
}

const std::string& FileInfo::Extension() const {
    return _extension;
}

const std::string& FileInfo::AbsolutePath() const {
    return _absolutePath;
}

const std::string& FileInfo::BasePath() const {
    return _basePath;
}

bool FileInfo::isDir() const {
    return _isDir;
}

bool FileInfo::isValid() const {
    return !_absolutePath.empty();
}





