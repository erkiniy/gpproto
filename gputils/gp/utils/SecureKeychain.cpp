//
// Created by Jaloliddin Erkiniy on 7/29/19.
//

#include "gp/utils/SecureKeychain.h"
#include "gp/utils/Crypto.h"
#include "gp/utils/StreamSlice.h"
#include "gp/utils/Logging.h"
#include "gp/utils/StringUtils.h"
#include "gp/utils/OutputStream.h"

#include "gp/utils/filesystem/NativeFileSystem.h"
#include "gp/utils/Random.h"

using namespace gpproto;
using namespace nlohmann;

SecureKeychain::SecureKeychain(const std::string & name, const std::string & documentsPath, const std::string & password) : name(name), documentsPath(documentsPath), password(password) {
    auto passDataPtr = password.c_str();

    auto passData = StreamSlice((unsigned char *)passDataPtr, strlen(passDataPtr));
    auto passHash = Crypto::sha256(passData);
    auto keyData = Crypto::sha256(*passHash);

    iv = std::make_unique<UInt128>();
    key = std::make_unique<UInt256 >();

    memcpy(iv->raw, passHash->rbegin(), 16);
    memcpy(key->raw, keyData->rbegin(), 32);

    fileSystem = std::make_shared<NativeFileSystem>(documentsPath);
    fileSystem->Initialize();
}

void SecureKeychain::setObject(const json & obj, const std::string & key, const std::string & group) {
    std::unique_lock<std::mutex> lock(mutex);

    loadKeychainIfNeeded(group);

    json groupJson;

    auto it = dictByGroup.find(group);

    if (it != dictByGroup.end())
        groupJson = *it;

    groupJson[key] = obj;

    dictByGroup[group] = groupJson;

    storeKeychain(group);
}

json SecureKeychain::getObject(const std::string & key, const std::string & group) {
    std::unique_lock<std::mutex> lock(mutex);

    loadKeychainIfNeeded(group);

    json obj;

    auto it = dictByGroup.find(group);

    if (it != dictByGroup.end())
        obj = (*it).second;
    
    return obj;
}

void SecureKeychain::removeObject(const std::string & key, const std::string & group) {
    std::unique_lock<std::mutex> lock(mutex);

    loadKeychainIfNeeded(group);

    auto it = dictByGroup.find(group);
    if (it == dictByGroup.end()) return;

    json groupJson = *it;
    groupJson.erase(key);

    dictByGroup[group] = groupJson;

    storeKeychain(group);
}

void SecureKeychain::dropGroup(const std::string &group) {
    std::unique_lock<std::mutex> lock(mutex);

    loadKeychainIfNeeded(group);

    auto it = dictByGroup.find(group);
    if (it == dictByGroup.end()) return;

    dictByGroup.erase(it);

    storeKeychain(group);
}

std::string SecureKeychain::filePathForName(const std::string & name, const std::string & group) {
    return /*documentsPath + */"/" + name + "_" + group + ".bin";
}

void SecureKeychain::loadKeychainIfNeeded(const std::string & group) {
    if (dictByGroup.find(group) != dictByGroup.end()) return;

    auto fileInfo = FileInfo(filePathForName(name, group));
    if (!fileSystem->IsFileExists(fileInfo)) return;

    auto file = fileSystem->OpenFile(fileInfo, File::FileMode::read);
    auto size = file->size();

    if (size < 8 || (size - 4) % 16 != 0) return;

    bool hashVerified = false;

    auto cypher = std::make_unique<StreamSlice>(size);
    file->Read(cypher->begin(), size);

    auto hash = cypher->prefix(4);

    auto decryptedData = Crypto::aes_cbc_decrypt(*key, &(*iv), *cypher->subData(4, cypher->size - 4));
    int decryptedSize = 0;
    memcpy(&decryptedSize, decryptedData->rbegin(), 4);

    if (decryptedSize > 0 && decryptedSize < INT_MAX && decryptedSize <= decryptedData->size - 4) {
        auto computedHash = Crypto::sha256(*decryptedData->prefix(4 + (size_t)decryptedSize))->suffix(4);
        hashVerified = *computedHash == *hash;
    }

    if (hashVerified) {
        LOGI("[SecureKeychain] verified");
        auto plainData = decryptedData->subData(4, (size_t)decryptedSize);
        json object = json::parse(plainData->rbegin());
        dictByGroup[group] = object;
    }
    else {
        LOGE("[SecureKeychain] hash not verified");
    }

    if (dictByGroup.find(group) == dictByGroup.end())
        dictByGroup[group] = json();
}

void SecureKeychain::storeKeychain(const std::string & group) {
    auto it = dictByGroup.find(group);

    if (it == dictByGroup.end()) {
        removeFileIfExists(fileSystem, filePathForName(name, group));
        return;
    }

    json obj = (*it).second;

    auto j = obj.dump();
    LOGV("[SecureKeychain dump] %s", j.c_str());

    auto data = StringUtils::toData(j);
    auto length = data->size;

    auto encryptedData = std::make_shared<StreamSlice>(4 + length);
    memcpy(encryptedData->begin(), &length, 4);
    memcpy(encryptedData->begin() + 4, data->rbegin(), length);

    auto encryptedDataWithHash = Crypto::sha256(*encryptedData)->suffix(4);

    auto encryptedDataCount = encryptedData->size;
    OutputStream os;

    while(encryptedDataCount % 16 != 0)
    {
        os.writeInt8(Random::fastUInt8());
        encryptedDataCount++;
    }

    if (os.getCurrentSize())
        encryptedData = encryptedData->appended(*os.currentBytes());

    encryptedData = Crypto::aes_cbc_encrypt(*key, &(*iv), *encryptedData);
    encryptedDataWithHash = encryptedDataWithHash->appended(*encryptedData);

    auto filePath = filePathForName(name, group);
    auto fileInfo = FileInfo(filePath);

    removeFileIfExists(fileSystem, filePath);

    auto file = fileSystem->OpenFile(fileInfo, File::FileMode::readWrite);
    file->Write(encryptedDataWithHash->rbegin(), encryptedDataWithHash->size);
    file->Close();
}

void SecureKeychain::removeFileIfExists(const std::shared_ptr<NativeFileSystem> & fs, const std::string & path) {

    auto fileInfo = FileInfo(path);

    if (fs->IsFileExists(fileInfo))
        fs->RemoveFile(fileInfo);
}