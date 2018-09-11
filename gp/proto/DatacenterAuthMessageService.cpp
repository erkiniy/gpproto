//
// Created by Jaloliddin Erkiniy on 9/7/18.
//

#include "gp/proto/DatacenterAuthMessageService.h"
#include "gp/proto/Proto.h"
#include "gp/proto/PreparedMessage.h"
#include "gp/proto/MessageTransaction.h"
#include "gp/proto/InternalParser.h"
#include "gp/proto/DatacenterSaltsetInfo.h"
#include "gp/network/IncomingMessage.h"
#include "gp/proto/ProtoInternalMessage.h"
#include "gp/network/OutgoingMessage.h"
#include "gp/utils/Crypto.h"
#include "gp/utils/Random.h"
#include "gp/utils/StreamSlice.h"
#include "gp/utils/OutputStream.h"
#include "gp/utils/BigNum.h"

using namespace gpproto;

static std::unordered_map<uint64_t, std::string> defaultServerPublicKeys() {
    static std::unordered_map<uint64_t, std::string> keys;
    keys[0x3996e1558cf963eb] = "-----BEGIN PUBLIC KEY-----MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1lk6awlsxW8cq2JqYxX+vakmdX+uOdTyf3YDhUPfTVs0u5uDCdO01hqbXbUNGZ+p32rSnEn/x/qXehVjSSG2vkzz15Vsu855qEYtRqkLNmhhjBakUNXGWs+rvz67HErthSU+8TgEwflisPUUYx3j8X58PkRW74i50+yI3laoqcIKwsSkmcTvEzQE+CO6csB4n7cni7ivg11famZ1W+ohvIzsShtINiUbxGQbU01adNIZrXU4pXHRgvF/KPYgOgjQQOPzvlggH+kjDf9oxBErUYbImIlxnLEe/JCgZI7wUs6VCzGxuWDO7Bc6ivd2R1m2iqlzQeYB2yS0BAlYNbM6AwIDAQAB-----END PUBLIC KEY-----";
    keys[0xbf47247de570f86f] = "-----BEGIN PUBLIC KEY-----MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz/WKS9yffzkfulcE4zFJkoSi6DNvr4lYtL6KFzTeVErWbySCQ15alapMVRWUQLIYL12LxXv0pEZYEVq3cGkVOZXO8d9L/NnGwimvYopw92WN+ZYEVguuCJiiQAOTTUM2QcIdP7/K0Gg49A3sg5cymNCeHFEWYZhDxghSYGuSYbk/a88De31oibwBjulOm/+wBXk5O3+Ii3KjMw8CpwVSR0AHe+nkZyjH/4FLHSA5ZWnEgPHZiYoqO9kwTrFYilGZxfldtlTkZwbkrDSHRWV3su8jXXawN/Zxp/lpr7gvox/VQJik0iV6XbmEQb8TheHltPbA3faQlNn1jfyu9WNquwIDAQAB-----END PUBLIC KEY-----";
    keys[0xc7ef29b0ddd8ca23] = "-----BEGIN PUBLIC KEY-----MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAslann35muUWMa8UsjgByxdHYbuXRDyuH+qtWuVe6VjR31fmF/bW8jGo6N8q1c8hmESRP4Di+O5U5t/UQ8sdgzjAqCCtX8LrgZaT8LBzY2iFhKBibyCxk2V4Wyq8ALZnk3MMfQRrBaQmLbDcSRz72KARW/h/jh54/AT8f/FZMxQipgePHJuWws3y/levrP3pm2Rv9nX63WtkmA+fK/lVvdgj7Bj5eaf+4tS9IIRT2OtUtDi/PkEO1mTPofb1D+wmketsUknGLlLg+UCYr+Zc4FAUBCOUds8QrKhyViUZ/1zTf22RWKXBCYRSXO996/aE+3/XE8wzEAsaR8F4arrMT2wIDAQAB-----END PUBLIC KEY-----";
    keys[0xea182ae529ffce57] = "-----BEGIN PUBLIC KEY-----MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA6LQIemJwh9wnTZSoONiShXa1CK7yIukZW5z4gcp6dDJuIeA+TUj7lmF1mDKZZ8cDScofYAblLOXWLzbIPPao4h97hykX7c90wY22LMxWtiH0ma0nxv5vPAXheqK/Vg+qbA4QC40YQSJ7BZdA62xXB/UttIey4QHnoWsoAmjO6mPiu4jFI2sDqWm+dW0vjWiuZkvQ0U8Y7zDgmGvblKerCOVC5NZ79M+s8NtOi5kGtJsKXV2FxGiWR2Ik965DSQ4D/f/Met78BPCfBrTfO5EWtoLXuA0xNKe/H24D6BBoo3c0FgXy5hHSK0B9NN43kNpcUn1pdygAomQK63JTIT3aXQIDAQAB-----END PUBLIC KEY-----";
    return keys;
};


DatacenterAuthMessageService::DatacenterAuthMessageService(const std::shared_ptr<gpproto::Context> &context) {

}


std::shared_ptr<gpproto::MessageTransaction> gpproto::DatacenterAuthMessageService::protoMessageTransaction(const std::shared_ptr<gpproto::Proto> &proto) {

    LOGV("[DatacenterAuthMessageService protoMessageTransaction] -> sending transaction with stage = %d, currentTransactionId = %d", (int)stage, currentStateTransactionId);

    if (currentStateTransactionId != 0)
        return nullptr;

    switch (stage)
    {
        case DatacenterAuthStage::pq: {
            if (nonce == nullptr)
            {
                nonce = std::make_shared<StreamSlice>(16);
                Random::secureBytes(nonce->begin(), 16);
            }

            OutputStream reqPQBuffer;
            reqPQBuffer.writeUInt32(0xfbf22646);
            reqPQBuffer.writeData(*nonce);

            auto message = std::make_shared<OutgoingMessage>(0, 0, true, reqPQBuffer.currentBytes());

            LOGV("[DatacenterAuthMessageService protoMessageTransaction] -> message with size %zu", message->data->size);
            return std::make_shared<MessageTransaction>(std::initializer_list<std::shared_ptr<OutgoingMessage>>{message},
                    [self = shared_from_this(), message](std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {
                        LOGV("[DatacenterAuthMessageService protoMessageTransaction] -> message prepared with count=%zu", messageInternalIdToPreparedMessage.size());
                        if (self->stage == DatacenterAuthStage::pq)
                {
                    auto it = messageInternalIdToPreparedMessage.find(message->internalId);
                    if (it != messageInternalIdToPreparedMessage.end())
                    {
                        LOGV("[DatacenterAuthMessageService protoMessageTransaction] -> message prepared");
                        auto preparedMessage = messageInternalIdToPreparedMessage[message->internalId];
                        self->currentStateMessageId = preparedMessage->messageId;
                        self->currentStateMessageSeqNo = preparedMessage->seqNo;
                        self->currentStateTransactionId = 1;
                    }
                }
            }, [] {},
            [](auto messageInternalIdToTransactionId, auto messageInternalIdToPreparedMessage) {});

        }
        case DatacenterAuthStage::reqDh: {

            OutputStream reqDhBuffer;
            reqDhBuffer.writeUInt32(0x92471d27);
            reqDhBuffer.writeData(*nonce);
            reqDhBuffer.writeData(*serverNonce);
            reqDhBuffer.writeData(*dhP);
            reqDhBuffer.writeData(*dhQ);
            reqDhBuffer.writeUInt64(publicKeyFingerprint);
            reqDhBuffer.writeBytes(*dhEncryptedData);

            auto message = std::make_shared<OutgoingMessage>(0, 0, true, reqDhBuffer.currentBytes());

            return std::make_shared<MessageTransaction>(std::initializer_list<std::shared_ptr<OutgoingMessage>>{message},
                    [](std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {}, [] {},
                    [self = shared_from_this(), message](std::unordered_map<int, int> messageInternalIdToTransactionId, std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {
                if (self->stage == DatacenterAuthStage::reqDh)
                {
                    auto it = messageInternalIdToPreparedMessage.find(message->internalId);
                    if (it != messageInternalIdToPreparedMessage.end())
                    {
                        auto preparedMessage = messageInternalIdToPreparedMessage[message->internalId];
                        self->currentStateMessageId = preparedMessage->messageId;
                        self->currentStateMessageSeqNo = preparedMessage->seqNo;
                        self->currentStateTransactionId = 1;
                    }
                }
            });
        }
        case DatacenterAuthStage::keyVerification: {

            OutputStream setDhParamsBuffer;
            setDhParamsBuffer.writeUInt32(0x92471d27);
            setDhParamsBuffer.writeData(*nonce);
            setDhParamsBuffer.writeData(*serverNonce);
            setDhParamsBuffer.writeData(*encryptedClientData);

            auto message = std::make_shared<OutgoingMessage>(0, 0, true, setDhParamsBuffer.currentBytes());

            return std::make_shared<MessageTransaction>(std::initializer_list<std::shared_ptr<OutgoingMessage>>{message},
                    [](std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {}, [] {},
                    [self = shared_from_this(), message](std::unordered_map<int, int> messageInternalIdToTransactionId, std::unordered_map<int, std::shared_ptr<PreparedMessage>> messageInternalIdToPreparedMessage) {
                if (self->stage == DatacenterAuthStage::reqDh)
                {
                    auto it = messageInternalIdToPreparedMessage.find(message->internalId);
                    if (it != messageInternalIdToPreparedMessage.end())
                    {
                        auto preparedMessage = messageInternalIdToPreparedMessage[message->internalId];
                        self->currentStateMessageId = preparedMessage->messageId;
                        self->currentStateMessageSeqNo = preparedMessage->seqNo;
                        self->currentStateTransactionId = 1;
                    }
                }
            });
        }
        case DatacenterAuthStage::done: {
            return nullptr;
        }
    }
}

void gpproto::DatacenterAuthMessageService::protoDidReceiveMessage(const std::shared_ptr<Proto> &proto, std::shared_ptr<IncomingMessage> message) {
    LOGV("[DatacenterAuthMessageService protoDidReceiveMessage] -> message received");

    if (stage == DatacenterAuthStage::pq && message->body != nullptr)
    {
        if (auto resPqMessage = std::dynamic_pointer_cast<ResPqMessage>(message->body))
        {
            if (nonce == nullptr) { return; }
            if (*nonce != *resPqMessage->nonce) { return; }

            auto publicKeyIt = publicKeys.find(resPqMessage->serverPublicKeyFingerprints);
            if (publicKeyIt == publicKeys.end())
            {
                LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> publickey not found");
                reset(proto);
                return;
            }

            publicKeyFingerprint = resPqMessage->serverPublicKeyFingerprints;
            LOGV("Fingerprint is %lld", publicKeyFingerprint);
            LOGV("pqBytes %s", resPqMessage->pq->description().c_str());

            auto pqBytes = resPqMessage->pq;
            uint64_t pq = 0;

            for (int i = 0; i < pqBytes->size; i++) {
                pq <<= 8;
                pq |= static_cast<uint64_t>(*(pqBytes->rbegin() + i));
            }

            uint64_t factP = 0;
            uint64_t factQ = 0;

            double factorizeStartTime = getAbsoluteSystemTime();

            factP = Crypto::pq_factorize(pq);
            if (factP == 0)
            {
                LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> cannot factorize number");
                reset(proto);
                return;
            }

            factQ = pq / factP;

            LOGV("Factorized P %llud", factP);
            LOGV("Factorized Q %llud", factQ);


            byteSwapUInt64(factP);
            byteSwapUInt64(factQ);

            {
                OutputStream ps;
                ps.writeUInt64(factP);
                dhP = ps.currentBytes();
            }

            {
                OutputStream qs;
                qs.writeUInt64(factP);
                dhQ = qs.currentBytes();
            }

            auto nonceBytes = std::make_shared<StreamSlice>(32);
            Random::secureBytes(nonceBytes->begin(), 32);

            serverNonce = resPqMessage->serverNonce;
            newNonce = nonceBytes;

            OutputStream innerDataBuffer;
            innerDataBuffer.writeUInt32(0xeb59a005);
            innerDataBuffer.writeBytes(*pqBytes);
            innerDataBuffer.writeBytes(*dhP);
            innerDataBuffer.writeBytes(*dhQ);
            innerDataBuffer.writeData(*nonce);
            innerDataBuffer.writeData(*serverNonce);
            innerDataBuffer.writeData(*newNonce);

            auto innerDataBytes = innerDataBuffer.currentBytes();
            auto hash = Crypto::sha1(*innerDataBytes);

            OutputStream dataWithHashStream;
            dataWithHashStream.writeData(*hash);
            dataWithHashStream.writeData(*innerDataBytes);

            auto dataWithHash = dataWithHashStream.currentBytes();

            auto encryptedData = Crypto::rsa_encrypt_pkcs1(publicKeyIt->second, *dataWithHash);

            if (encryptedData == nullptr)
            {
                LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> rsa encryption failed");
                return;
            }

            dhEncryptedData = encryptedData;

            stage = DatacenterAuthStage::reqDh;
            currentStateMessageId = 0;
            currentStateMessageSeqNo = 0;
            currentStateTransactionId = 0;

            proto->requestTransportTransactions();
        }
    }
    else if (stage == DatacenterAuthStage::reqDh && message->body != nullptr)
    {
        if (auto serverDhParamsMessage = std::dynamic_pointer_cast<ServerDhParamsMessage>(message->body))
        {
            if (nonce != serverDhParamsMessage->nonce || serverNonce != serverDhParamsMessage->serverNonce)
                return;

            if (auto serverDhParamsOkMessage = std::dynamic_pointer_cast<ServerDhParamsOkMessage>(serverDhParamsMessage))
            {
                auto newNonceAndServerNonce = std::make_shared<StreamSlice>(nonce->size + newNonce->size);
                memcpy(newNonceAndServerNonce->begin(), nonce->rbegin(), nonce->size);
                memcpy(newNonceAndServerNonce->begin() + nonce->size, newNonce->rbegin(), newNonce->size);
                auto newNonceAndServerNonceHash = Crypto::sha1(*newNonceAndServerNonce);

                auto serverNonceAndNewNonce = std::make_shared<StreamSlice>(newNonce->size + nonce->size);
                memcpy(serverNonceAndNewNonce->begin(), newNonce->rbegin(), newNonce->size);
                memcpy(serverNonceAndNewNonce->begin() + newNonce->size, nonce->rbegin(), nonce->size);
                auto serverNonceAndNewNonceHash = Crypto::sha1(*serverNonceAndNewNonce);

                auto newNonceAndNewNonce = std::make_shared<StreamSlice>(2 * newNonce->size);
                memcpy(newNonceAndNewNonce->begin(), newNonce->rbegin(), newNonce->size);
                memcpy(newNonceAndNewNonce->begin() + newNonce->size, newNonce->rbegin(), newNonce->size);
                auto newNonceAndNewNonceHash = Crypto::sha1(*newNonceAndNewNonce);

                auto tempAesKey = std::make_shared<StreamSlice>(newNonceAndServerNonceHash->size + 12);
                memcpy(tempAesKey->begin(), newNonceAndServerNonceHash->rbegin(), newNonceAndServerNonceHash->size);
                auto serverNonceNewNonceSubdata = serverNonceAndNewNonceHash->subData(0, 12);
                memcpy(tempAesKey->begin() + newNonceAndServerNonceHash->size, serverNonceNewNonceSubdata->begin(), 12);

                auto tempAesIV = std::make_shared<StreamSlice>(16);
                auto serverNonceAndNewNonceHashSubdata = serverNonceAndNewNonce->subData(12, 20);
                auto newNonceAndNewNonceHashSubdata = newNonceAndNewNonceHash->subData(12, 20);
                memcpy(tempAesIV->begin(), serverNonceAndNewNonceHashSubdata->rbegin(), 8);
                memcpy(tempAesIV->begin() + 8, newNonceAndNewNonceHashSubdata->rbegin(), 8);

                UInt256 aesKey;
                UInt128 aesIV;

                memcpy(aesKey.raw, tempAesKey->rbegin(), tempAesKey->size);
                memcpy(aesIV.raw, tempAesIV->rbegin(), tempAesIV->size);

                auto answerWithHash = Crypto::aes_cbc_decrypt(aesKey, &aesIV, *serverDhParamsOkMessage->encryptedResponse);

                if (answerWithHash == nullptr || answerWithHash->size < 20 + 16) {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> aes decryption error");
                    return;
                }

                auto answerHash = answerWithHash->subData(0, 20);
                auto answerData = answerHash->subData(20, answerWithHash->size);

                bool hashVerified = false;
                for (int i = 0; i < 16; ++i)
                {
                    auto computedHash = Crypto::sha1(*answerData);
                    if (*computedHash == *answerHash)
                    {
                        hashVerified = true;
                        break;
                    }
                    answerData->removeLast();
                }

                if (!hashVerified)
                {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> hash not verified");
                    reset(proto);
                    return;
                }

                auto dhInnerDataRaw = InternalParser::parseMessage(answerData);
                if (dhInnerDataRaw == nullptr || std::dynamic_pointer_cast<ServerDhInnerDataMessage>(dhInnerDataRaw) == nullptr)
                {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> cannot be parsed");
                    reset(proto);
                    return;
                }

                auto dhInnerData = std::dynamic_pointer_cast<ServerDhInnerDataMessage>(dhInnerDataRaw);

                if (nonce != dhInnerData->nonce) {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> invalid nonce");
                    reset(proto);
                    return;
                }

                if (serverNonce != dhInnerData->serverNonce) {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> invalid server nonce");
                    reset(proto);
                    return;
                }

                auto innerDataG = dhInnerData->g;

                if (!Crypto::isSafeG(innerDataG))
                {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> g is not safe. G = %d", innerDataG);
                    reset(proto);
                    return;
                }

                auto innerDataGa = dhInnerData->ga;
                auto innerDataDhPrime = dhInnerData->dhPrime;

                //TODO: is safe Ga;

                auto b = std::make_shared<StreamSlice>(256);
                Random::secureBytes(b->begin(), 256);

                b->begin()[0] &= 0x7f;

                auto tempG = innerDataG;
                byteSwapInt32(tempG);

                auto g = std::make_shared<StreamSlice>(4);
                memcpy(g->begin(), &tempG, 4);

                auto startTime = getAbsoluteSystemTime();
                auto g_b = Crypto::mod_exp(g, b, innerDataDhPrime);

                LOGV("[DatacenterAuthMessageService protoDidReceiveMessage] -> mod_pow lasted %lf seconds", getAbsoluteSystemTime() - startTime);

                auto authKey = Crypto::mod_exp(innerDataGa, b, innerDataDhPrime);
                auto authKeyIdHash = Crypto::sha256(*authKey)->subData(24, 32);

                int64_t authKeyId = 0;
                memcpy(&authKeyId, authKeyIdHash->rbegin(), 8);

                auto serverSaltData = std::make_shared<StreamSlice>(8);

                if (newNonce->size < 8 || serverNonce->size < 8)
                {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> new nonce and server nonce length must be at least 8 bytes");
                    reset(proto);
                    return;
                }

                for (int i = 0; i < 8; ++i)
                {
                    auto nN = newNonce->begin()[i];
                    auto sN = serverNonce->begin()[i];
                    auto sS = nN ^ sN;

                    memcpy(serverSaltData->begin() + i, &sS, 1);
                }

                int64_t serverSalt = 0;
                memcpy(&serverSalt, serverSaltData->rbegin(), 8);
                this->authKey = std::make_shared<AuthKeyInfo>(authKey, authKeyId, std::initializer_list<std::shared_ptr<DatacenterSaltsetInfo>>{std::make_shared<DatacenterSaltsetInfo>(serverSalt, (int64_t)(message->timestamp) * 4294967296, (int64_t)(message->timestamp + 60) * 4294967296)});

                OutputStream clientDhInnerDataBuffer;
                clientDhInnerDataBuffer.writeUInt32(0x706fb308);
                clientDhInnerDataBuffer.writeData(*nonce);
                clientDhInnerDataBuffer.writeData(*serverNonce);
                clientDhInnerDataBuffer.writeInt64(0);
                clientDhInnerDataBuffer.writeBytes(*g_b);

                auto clientInnerDataBytes = clientDhInnerDataBuffer.currentBytes();
                OutputStream clientDataWithHashStream;

                auto clientDataBytesHash = Crypto::sha1(*clientInnerDataBytes);
                clientDataWithHashStream.writeData(*clientDataBytesHash);
                clientDataWithHashStream.writeData(*clientInnerDataBytes);

                while (clientDataWithHashStream.getCurrentSize() % 16 != 0)
                {
                    unsigned char randomByte;
                    Random::secureBytes(&randomByte, 1);
                    clientDataWithHashStream.writeUInt8(reinterpret_cast<uint8_t>(randomByte));
                }

                auto clientDataWithHash = clientDataWithHashStream.currentBytes();

                encryptedClientData = Crypto::aes_cbc_encrypt(aesKey, &aesIV, *clientDataWithHash);
                stage = DatacenterAuthStage::keyVerification;
                currentStateMessageId = 0;
                currentStateMessageSeqNo = 0;
                currentStateTransactionId = 0;

                proto->requestTransportTransactions();
            }
            else
            {
                LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> couldn't set dh params");
                reset(proto);
                return;
            }
        }
    }
    else if (stage == DatacenterAuthStage::keyVerification && message->body != nullptr)
    {
        if (auto setClientDhParamsResponseMessage = std::dynamic_pointer_cast<ClientDhParamsResponseMessage>(message->body))
        {
            if (nonce != setClientDhParamsResponseMessage->serverNonce || serverNonce != setClientDhParamsResponseMessage->serverNonce)
                return;

            auto authKeyAuxHashFull = Crypto::sha1(*authKey->authKey);
            auto authKeyAuxHash = authKeyAuxHashFull->subData(0, 8);

            std::shared_ptr<StreamSlice> newNonce1;
            {
                OutputStream newNonce1s;
                newNonce1s.writeData(*newNonce);
                uint8_t tmp1 = 1;
                newNonce1s.writeUInt8(1);
                newNonce1s.writeData(*authKeyAuxHash);

                newNonce1 = newNonce1s.currentBytes();
            }

            auto newNonceHash1Full = Crypto::sha1(*newNonce1);
            auto newNonceHash1 = newNonceHash1Full->subData((int)newNonceHash1Full->size - 16, newNonceHash1Full->size);

            std::shared_ptr<StreamSlice> newNonce2;
            {
                OutputStream newNonce2s;
                newNonce2s.writeData(*newNonce);
                uint8_t tmp2 = 2;
                newNonce2s.writeUInt8(1);
                newNonce2s.writeData(*authKeyAuxHash);

                newNonce2 = newNonce2s.currentBytes();
            }

            auto newNonceHash2Full = Crypto::sha1(*newNonce2);
            auto newNonceHash2 = newNonceHash2Full->subData((int)newNonceHash2Full->size - 16, newNonceHash2Full->size);

            std::shared_ptr<StreamSlice> newNonce3;
            {
                OutputStream newNonce3s;
                newNonce3s.writeData(*newNonce);
                uint8_t tmp3 = 3;
                newNonce3s.writeUInt8(1);
                newNonce3s.writeData(*authKeyAuxHash);

                newNonce3 = newNonce3s.currentBytes();
            }

            auto newNonceHash3Full = Crypto::sha1(*newNonce3);
            auto newNonceHash3 = newNonceHash3Full->subData((int)newNonceHash3Full->size - 16, newNonceHash3Full->size);

            if (auto setClientDhParamsResponseOkMessage = std::dynamic_pointer_cast<ClientDhParamsResponseOkMessage>(setClientDhParamsResponseMessage))
            {
                if (setClientDhParamsResponseOkMessage->nextNonceHash1 == newNonceHash1)
                {
                    stage = DatacenterAuthStage::done;
                    currentStateMessageId = 0;
                    currentStateMessageSeqNo = 0;
                    currentStateTransactionId = 0;

                    if (auto strongDelegate = delegate.lock())
                        strongDelegate->authMessageServiceCompletedWithAuthKey(*this, this->authKey, (int64_t)message->timestamp * 4294967296);
                } else {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> SetClientDhParamsResponseOkMessage nextNonce1 mismatch");
                    reset(proto);
                    return;
                }
            }
            else if (auto setClientDhParamsResponseRetryMessage = std::dynamic_pointer_cast<ClientDhParamsResponseRetryMessage>(setClientDhParamsResponseMessage))
            {
                if (setClientDhParamsResponseRetryMessage->nextNonceHash2 == newNonceHash2)
                {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> retry DH");
                    reset(proto);
                    return;
                }
                else {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> invalid DH answer nonce hash 2");
                    reset(proto);
                    return;
                }
            }
            else if (auto setClientDhParamsResponseFailMessage = std::dynamic_pointer_cast<ClientDhParamsResponseFailMessage>(setClientDhParamsResponseMessage))
            {
                if (setClientDhParamsResponseFailMessage->nextNonceHash3 == newNonceHash3)
                {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> fail DH");
                    reset(proto);
                    return;
                }
                else {
                    LOGE("[DatacenterAuthMessageService protoDidReceiveMessage] -> invalid DH params response");
                    reset(proto);
                    return;
                }
            }
        }
    }
}

void gpproto::DatacenterAuthMessageService::protoTransactionsMayHaveFailed(const std::shared_ptr<gpproto::Proto> &proto, std::vector<int> transactionIds) {

}

void gpproto::DatacenterAuthMessageService::protoMessageDeliveryFailed(const std::shared_ptr<gpproto::Proto> &proto,
                                                                       int64_t messageId) {

}

void gpproto::DatacenterAuthMessageService::protoMessagesDeliveryConfirmed(const std::shared_ptr<gpproto::Proto> &proto,
                                                                           std::vector<int64_t> messages) {
}

void gpproto::DatacenterAuthMessageService::protoErrorReceived(const std::shared_ptr<gpproto::Proto> &proto,
                                                               int32_t errorCode) {
}

void gpproto::DatacenterAuthMessageService::protoWillAddService(const std::shared_ptr<gpproto::Proto> &proto) {
    LOGV("DatacenterAuthMessageService will addService");
}

void gpproto::DatacenterAuthMessageService::protoDidAddService(const std::shared_ptr<gpproto::Proto> &proto) {
    LOGV("DatacenterAuthMessageService did addService");
    proto->requestTransportTransactions();
}

void gpproto::DatacenterAuthMessageService::protoWillRemoveService(const std::shared_ptr<gpproto::Proto> &proto) {

}

void gpproto::DatacenterAuthMessageService::protoDidRemoveService(const std::shared_ptr<gpproto::Proto> &proto) {

}

void gpproto::DatacenterAuthMessageService::protoAllTransactionsMayHaveFailed(const std::shared_ptr<gpproto::Proto> &proto) {

}

void gpproto::DatacenterAuthMessageService::protoDidChangeSession(const std::shared_ptr<gpproto::Proto> &proto) {
    reset(proto);
}

void gpproto::DatacenterAuthMessageService::protoServerDidChangeSession(const std::shared_ptr<gpproto::Proto> &proto) {
    reset(proto);
}

void
gpproto::DatacenterAuthMessageService::protoNetworkAvailabilityChanged(const std::shared_ptr<gpproto::Proto> &proto,
                                                                       bool isNetworkAvailable) {
}

void gpproto::DatacenterAuthMessageService::protoConnectionStateChanged(const std::shared_ptr<gpproto::Proto> &proto,
                                                                        bool isConnected) {
    if (isConnected)
        proto->requestTransportTransactions();
}

void gpproto::DatacenterAuthMessageService::protoAuthTokenUpdated(const std::shared_ptr<gpproto::Proto> &proto) {

}

void DatacenterAuthMessageService::reset(std::shared_ptr<gpproto::Proto> proto) {
    currentStateMessageId = 0;
    currentStateMessageSeqNo = 0;
    currentStateTransactionId = 0;

    nonce = nullptr;
    serverNonce = nullptr;
    newNonce = nullptr;

    dhP = nullptr;
    dhQ = nullptr;
    publicKeyFingerprint = 0;
    encryptedClientData = nullptr;

    authKey = nullptr;
    encryptedClientData = nullptr;

    publicKeys = defaultServerPublicKeys();

    proto->requestTransportTransactions();
}
