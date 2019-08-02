//
// Created by Jaloliddin Erkiniy on 7/29/19.
//

#ifndef GPPROTO_SECUREKEYCHAIN_H
#define GPPROTO_SECUREKEYCHAIN_H


#include "gp/utils/third_party/nlohmann/json.hpp"
#include "gp/utils/Common.h"

#include <string>

using json = nlohmann::json;

namespace gpproto
{
    class StreamSlice;
    class NativeFileSystem;

    class SecureKeychain final {
    public:
        explicit SecureKeychain(const std::string & name, const std::string & documentsPath, const std::string & password);

        SecureKeychain() = delete;
        SecureKeychain(const SecureKeychain&) = delete;

        const std::string name;

        void setObject(const json & obj, const std::string & key, const std::string & group);
        json getObject(const std::string & key, const std::string & group);
        void removeObject(const std::string & key, const std::string & group);

        void dropGroup(const std::string & group);

    private:
        const std::string documentsPath;
        const std::string password;

        std::unordered_map<std::string, json> dictByGroup;

        UInt256 key;
        UInt128 iv;

        std::mutex mutex;

        std::shared_ptr<NativeFileSystem> fileSystem;

        void loadKeychainIfNeeded(const std::string & group);
        void storeKeychain(const std::string & group);

        void removeFileIfExists(const std::shared_ptr<NativeFileSystem> & fs, const std::string & path);

        std::string filePathForName(const std::string & name, const std::string & group);
    };
};



#endif //GPPROTO_SECUREKEYCHAIN_H
