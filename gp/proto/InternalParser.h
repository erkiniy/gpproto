//
// Created by Jaloliddin Erkiniy on 9/3/18.
//

#ifndef GPPROTO_INTERNALPARSER_H
#define GPPROTO_INTERNALPARSER_H

#include "gp/proto/ProtoInternalMessage.h"
#include "gp/utils/StreamSlice.h"
#include "gp/utils/Logging.h"
#include "gp/utils/InputStream.h"
#include "gp/utils/InputStreamException.h"
#include "gp/utils/Crypto.h"

namespace gpproto::InternalParser {

    static std::shared_ptr<StreamSlice> decompressGZip(const std::shared_ptr<StreamSlice>& data) {
        return Crypto::gzip_unzip(data);
    }

    static std::shared_ptr<StreamSlice> unwrapMessage(const std::shared_ptr<StreamSlice>& data)
    {
        if (data->size < 4)
            return data;

        uint32_t signature = 0;
        memcpy(&signature, data->rbegin(), 4);

        if (signature == 0x3072cfa1)
        {
            auto packetData = data->subData(4, data->size - 4);
            auto is = std::make_unique<InputStream>(packetData);
            try {
                auto packetBytes = is->readBytes();
                return decompressGZip(packetBytes);
            } catch (InputStreamException & e) {
                LOGV("[InternalParser] InputStream error %s", e.message.c_str());
                return nullptr;
            }
        }

        return data;
    }

    static std::shared_ptr<ProtoInternalMessage> parseMessage(const std::shared_ptr<StreamSlice>& data)
    {
        InputStream is = InputStream(data);

        size_t totalDataSize = data->size;
        uint32_t signature = is.readUInt32();

        switch (signature)
        {
            case 0x15d04c69: {
                auto nonce = is.readData(16);
                auto serverNonce = is.readData(16);
                auto pq = is.readBytes();
                auto serverPublicKeySignature = is.readUInt64();

                return std::make_shared<ResPqMessage>(nonce, serverNonce, pq, serverPublicKeySignature);
            }
            case 0xcb73e0c5: {
                auto nonce = is.readData(16);
                auto serverNonce = is.readData(16);
                auto nextNonceHash = is.readData(16);

                return std::make_shared<ServerDhParamsFailMessage>(nonce, serverNonce, nextNonceHash);
            }
            case 0xf026f3c8: {
                auto nonce = is.readData(16);
                auto serverNonce = is.readData(16);
                auto encryptedResponse = is.readBytes();

                return std::make_shared<ServerDhParamsOkMessage>(nonce, serverNonce, encryptedResponse);
            }
            case 0xefeab3aa: {
                auto nonce = is.readData(16);
                auto serverNonce = is.readData(16);
                auto g = is.readInt32();
                auto dhPrime = is.readBytes();
                auto ga = is.readBytes();
                auto serverTime = is.readInt32();

                return std::make_shared<ServerDhInnerDataMessage>(nonce, serverNonce, g, dhPrime, ga, serverTime);
            }
            case 0x73d45989: {
                auto nonce = is.readData(16);
                auto serverNonce = is.readData(16);
                auto nextNonceHash1 = is.readData(16);

                return std::make_shared<ClientDhParamsResponseOkMessage>(nonce, serverNonce, nextNonceHash1);
            }
            case 0x5b04490c: {
                auto nonce = is.readData(16);
                auto serverNonce = is.readData(16);
                auto nextNonceHash2 = is.readData(16);

                return std::make_shared<ClientDhParamsResponseRetryMessage>(nonce, serverNonce, nextNonceHash2);
            }
            case 0x7250ae2a: {
                auto nonce = is.readData(16);
                auto serverNonce = is.readData(16);
                auto nextNonceHash3 = is.readData(16);

                return std::make_shared<ClientDhParamsResponseFailMessage>(nonce, serverNonce, nextNonceHash3);
            }
            case 0x7abe77ec: {
                auto pingId = is.readInt64();

                return std::make_shared<PingMessage>(pingId);
            }
            case 0xf35c6d01: {
                auto requestMessageId = is.readInt64();
                auto responseData = is.readRemainingData();

                return std::make_shared<RpcResultMessage>(requestMessageId, responseData);
            }
            case 0x2144ca19: {
                auto errorCode = is.readInt32();
                auto errorDescription = is.readString();

                return std::make_shared<RpcError>(errorCode, errorDescription);
            }
            case 0x347773c5: {
                auto messageId = is.readInt64();
                auto pingId = is.readInt64();

                return std::make_shared<PongMessage>(messageId, pingId);
            }
            case 0x62d6b459: {
                auto vectorSignature = is.readInt32();

                if (vectorSignature != 0x1cb5c415)
                {
                    LOGE("[InternalParser parseMessage] -> vector signature is wrong for MsgsAck message");
                    return nullptr;
                }
                auto count = is.readInt32();

                if (count < 0 || count > 500)
                    return nullptr;

                std::vector<int64_t> messageIds;
                messageIds.reserve((size_t)count);

                for (int i = 0; i < count; ++i)
                    messageIds.push_back(is.readInt64());

                return std::make_shared<MsgsAckMessage>(messageIds);
            }
            case 0x73f1f8dc: {
                auto count = is.readInt32();

                if (count < 0)
                    return nullptr;

                std::vector<std::shared_ptr<Message>> messages;
                messages.reserve((size_t)count);

                for (int i = 0; i < count; ++i)
                {
                    auto messageId = is.readInt64();
                    auto seqNo = is.readInt32();
                    auto length = is.readInt32();

                    if (length < 0 || length > 16 * 1024 * 1024 || length > totalDataSize)
                    {
                        LOGE("[InternalParser parseMessage] -> invalid length of Container");
                        return nullptr;
                    }

                    auto messageData = is.readData((size_t)length);
                    messages.push_back(std::make_shared<Message>(messageId, seqNo, messageData));
                }
                return std::make_shared<Container>(messages);
            }
            case 0xae500895: {
                auto requestMessageId = is.readInt64();
                auto now = is.readInt32();
                auto count = is.readInt32();

                if (count < 0 || count > 300)
                    return nullptr;

                std::vector<std::shared_ptr<FutureSalt>> salts;
                salts.reserve((size_t)count);

                for (int i = 0; i < count; ++i)
                {
                    auto validSince = is.readInt32();
                    auto validUntil = is.readInt32();
                    auto salt = is.readInt64();

                    salts.push_back(std::make_shared<FutureSalt>(validSince, validUntil, salt));
                }

                return std::make_shared<FutureSaltsMessage>(requestMessageId, now, salts);
            }
            case 0xa7eff811: {
                auto badMessageId = is.readInt64();
                auto badSeqNo = is.readInt32();
                auto errorCode = is.readInt32();

                return std::make_shared<BadMsgNotificationMessage>(badMessageId, badSeqNo, errorCode);
            }
            case 0xedab447b: {
                auto badMessageId = is.readInt64();
                auto badSeqNo = is.readInt32();
                auto errorCode = is.readInt32();
                auto validServerSalt = is.readInt64();

                return std::make_shared<BadServerSaltNotificationMessage>(badMessageId, badSeqNo, errorCode, validServerSalt);
            }
            default: {
                LOGE("[InternalParser parseMessage] -> cannot parse incoming message with signature %du", signature);
                return nullptr;
            }
        }
    }
}
#endif //GPPROTO_INTERNALPARSER_H
