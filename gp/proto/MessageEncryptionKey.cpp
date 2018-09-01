//
// Created by Jaloliddin Erkiniy on 9/1/18.
//

#include "gp/proto/MessageEncryptionKey.h"
#include "gp/utils/Crypto.h"

using namespace gpproto;

std::shared_ptr<MessageEncryptionKey> MessageEncryptionKey::messageEncryptionKeyForAuthKey(
        std::shared_ptr<StreamSlice> authKey, std::shared_ptr<StreamSlice> messageKey) {

    auto sha256_aData = StreamSlice(16 + 36);
    memcpy(sha256_aData.begin(), messageKey->begin(), 16);
    memcpy(sha256_aData.begin() + 16, authKey->subData(0, 36)->begin(), 36);

    auto sha256_a = Crypto::sha256(sha256_aData);

    auto sha256_bData = StreamSlice(16 + 36);
    memcpy(sha256_bData.begin(), authKey->subData(40, 36)->begin(), 36);
    memcpy(sha256_bData.begin() + 36, messageKey->begin(), 16);

    auto sha256_b = Crypto::sha256(sha256_bData);

    auto a1 = sha256_a->subData(0, 8);
    auto a2 = sha256_a->subData(8, 8);
    auto a4 = sha256_a->subData(24, 8);

    auto b2 = sha256_b->subData(8, 8);
    auto b3 = sha256_b->subData(16, 8);
    auto b4 = sha256_b->subData(24, 8);

    UInt256 key;
    UInt128 iv;

    //auto keyValue = *a1 + *b2 + *b3 + *a4;
    //auto ivValue = *a2 + *b4;

    memcpy(key.raw, a1->rbegin(), 8);
    memcpy(key.raw + 8, b2->rbegin(), 8);
    memcpy(key.raw + 16, b3->rbegin(), 8);
    memcpy(key.raw + 24, a4->rbegin(), 8);

    memcpy(iv.raw, a2->rbegin(), 8);
    memcpy(iv.raw + 8, b4->rbegin(), 8);

    return std::make_shared<MessageEncryptionKey>(std::move(key), std::move(iv));
}