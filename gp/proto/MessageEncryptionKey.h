//
// Created by Jaloliddin Erkiniy on 9/1/18.
//

#ifndef GPPROTO_MESSAGEENCRYPTIONKEY_H
#define GPPROTO_MESSAGEENCRYPTIONKEY_H

#include <memory>
#include "gp/utils/Common.h"

namespace gpproto {
    class StreamSlice;
    class MessageEncryptionKey {
    public:
        static std::shared_ptr<MessageEncryptionKey> messageEncryptionKeyForAuthKey(std::shared_ptr<StreamSlice> authKey, std::shared_ptr<StreamSlice> messageKey);
        MessageEncryptionKey(UInt256 && aes_key, UInt128 && aes_iv) : aes_key(std::move(aes_key)), aes_iv(std::move(aes_iv)) {};

        const UInt256 aes_key;
        UInt128 aes_iv;
    };
}

#endif //GPPROTO_MESSAGEENCRYPTIONKEY_H
