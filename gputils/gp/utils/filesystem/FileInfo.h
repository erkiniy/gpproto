//
// Created by ProDigital on 7/28/18.
//

#ifndef GPPROTO_FILEINFO_H
#define GPPROTO_FILEINFO_H

#include <string>

#define CLASS_PTR(_class) typedef std::shared_ptr<class _class> _class##Ptr;\
                          typedef std::weak_ptr<class _class> _class##Weak;

namespace gpproto {

    class FileInfo final {
    public:
        FileInfo();

        ~FileInfo();

        FileInfo(const std::string &filePath);

        FileInfo(const std::string &basePath, const std::string &fileName, bool isDir);

        void Initialize(const std::string &basePath, const std::string &fileName, bool isDir);

        const std::string &Name() const;

        const std::string &BaseName() const;

        const std::string &Extension() const;

        const std::string &AbsolutePath() const;

        const std::string &BasePath() const;

        bool isDir() const;

        bool isValid() const;

    private:
        std::string _name;
        std::string _baseName;
        std::string _extension;
        std::string _absolutePath;
        std::string _basePath;
        bool _isDir;
    };

    inline bool operator == (const FileInfo& _0, const FileInfo& _1) {
        return _0.AbsolutePath() == _1.AbsolutePath();
    }

    inline bool operator < (const FileInfo& _0, const FileInfo& _1) {
        return _0.AbsolutePath() < _1.AbsolutePath();
    }
}
#endif //GPPROTO_FILEINFO_H
