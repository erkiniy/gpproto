//
// Created by ProDigital on 7/30/18.
//

#include <ios>
#include <fstream>
#include "NativeFile.h"

using namespace gpproto;

NativeFile::NativeFile(const FileInfo &fileInfo)
: _fileInfo(fileInfo)
, _isReadOnly(true)
, _mode(static_cast<FileMode>(0))
{

}

NativeFile::~NativeFile() {
    Close();
}

const FileInfo& NativeFile::fileInfo() const {
    return _fileInfo;
}

const uint64_t NativeFile::size() {
    if(isOpened())
    {
        uint64_t curPos = Tell();
        Seek(0, end);
        uint64_t size = Tell();
        Seek(curPos, begin);

        return size;
    }

    return 0;
}

bool NativeFile::isReadOnly() const {
    return _isReadOnly;
}

void NativeFile::Open(int mode)
{
    if (isOpened() && _mode == mode)
    {
        Seek(0, begin);
        return;;
    }

    _mode = mode;
    _isReadOnly = true;

    auto openMode = (std::ios_base::open_mode)0x00;

    if (mode & File::read)
        openMode |= std::fstream::in;

    if (mode & File::write) {
        openMode |= std::fstream::out;
        _isReadOnly = false;
    }

    if (mode & File::append) {
        openMode |= std::fstream::app;
        _isReadOnly = false;
    }

    if (mode & File::truncate)
        openMode |= std::fstream::trunc;


    _stream.open(fileInfo().AbsolutePath().c_str(), openMode);

}

void NativeFile::Close() {
    _stream.close();
}

bool NativeFile::isOpened() const {
    return _stream.is_open();
}

uint64_t NativeFile::Seek(uint64_t offset, File::Origin origin)
{
    if (!isOpened())
        return 0;

    std::ios_base::seek_dir way;
    if (origin == begin)
        way = std::ios_base::beg;
    else if (origin == end)
        way = std::ios_base::end;
    else
        way = std::ios_base::cur;

    _stream.seekg(offset, way);
    _stream.seekp(offset, way);

    return Tell();
}

uint64_t NativeFile::Tell() {
    return static_cast<uint64_t>(_stream.tellg());
}

uint64_t NativeFile::Read(uint8_t *buffer, uint64_t size) {

    if (!isOpened())
        return 0;

    _stream.read(reinterpret_cast<char*>(buffer), (std::streamsize)size);

    if (_stream)
        return size;

    auto s = static_cast<uint64_t>(_stream.gcount());

    return s;
}

uint64_t NativeFile::Write(const uint8_t *buffer, uint64_t size) {

    if (!isOpened() || isReadOnly())
        return 0;

    _stream.write(reinterpret_cast<const char*>(buffer), (std::streamsize)size);
    if (_stream)
        return size;

    return static_cast<uint64_t>(_stream.gcount());
}