//
// Created by Jaloliddin Erkiniy on 9/3/18.
//

#ifndef GPPROTO_PROTOINTERNALMESSAGE_H
#define GPPROTO_PROTOINTERNALMESSAGE_H

#include "gp/utils/StreamSlice.h"
#include <vector>

namespace gpproto
{
    class ProtoInternalMessage {
    public:
        ProtoInternalMessage() = default;
        ProtoInternalMessage(const ProtoInternalMessage&) = delete;
        virtual ~ProtoInternalMessage() = default;
    };

    class ResPqMessage : public ProtoInternalMessage {
    public:
        ResPqMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, uint64_t serverPublicKeyFingerprints)
                : nonce(std::move(nonce)),
                  serverNonce(std::move(serverNonce)),
                  serverPublicKeyFingerprints(serverPublicKeyFingerprints) {};

        const std::shared_ptr<StreamSlice> nonce;
        const std::shared_ptr<StreamSlice> serverNonce;
        const uint64_t serverPublicKeyFingerprints;
    };

    class ServerDhParamsMessage : public ProtoInternalMessage {
    public:
        ServerDhParamsMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce)
                : nonce(std::move(nonce)),
                  serverNonce(std::move(serverNonce)) {};

        const std::shared_ptr<StreamSlice> nonce;
        const std::shared_ptr<StreamSlice> serverNonce;
    };

    class ServerDhParamsFailMessage : public ServerDhParamsMessage {
    public:
        ServerDhParamsFailMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, std::shared_ptr<StreamSlice>& nextNonceHash)
                : ServerDhParamsMessage(nonce, serverNonce),
                  nextNonceHash(std::move(nextNonceHash)) {};

        const std::shared_ptr<StreamSlice> nextNonceHash;
    };

    class ServerDhParamsOkMessage : public ServerDhParamsMessage {
    public:
        ServerDhParamsOkMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, std::shared_ptr<StreamSlice>& encryptedResponse)
                : ServerDhParamsMessage(nonce, serverNonce),
                  encryptedResponse(std::move(encryptedResponse)) {};

        const std::shared_ptr<StreamSlice> encryptedResponse;
    };

    class ServerDhInnerDataMessage : public ProtoInternalMessage {
    public:
        ServerDhInnerDataMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, int32_t g, std::shared_ptr<StreamSlice>& dhPrime, std::shared_ptr<StreamSlice>& ga, int32_t serverTime)
                : nonce(std::move(nonce)),
                  serverNonce(std::move(serverNonce)),
                  g(g),
                  dhPrime(std::move(dhPrime)),
                  ga(std::move(ga)),
                  serverTime(serverTime) {};

        const std::shared_ptr<StreamSlice> nonce;
        const std::shared_ptr<StreamSlice> serverNonce;
        const int32_t g;
        const std::shared_ptr<StreamSlice> dhPrime;
        const std::shared_ptr<StreamSlice> ga;
        const int32_t serverTime;
    };

    class ClientDhParamsResponseMessage : public ProtoInternalMessage {
    public:
        ClientDhParamsResponseMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce)
                : nonce(std::move(nonce)),
                  serverNonce(std::move(serverNonce)) {};

        const std::shared_ptr<StreamSlice> nonce;
        const std::shared_ptr<StreamSlice> serverNonce;
    };

    class ClientDhParamsResponseOkMessage : public ClientDhParamsResponseMessage {
    public:
        ClientDhParamsResponseOkMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, std::shared_ptr<StreamSlice>& nextNonceHash1)
                : ClientDhParamsResponseMessage(nonce, serverNonce),
                  nextNonceHash1(std::move(nextNonceHash1)) {};

        const std::shared_ptr<StreamSlice> nextNonceHash1;
    };

    class ClientDhParamsResponseRetryMessage : public ClientDhParamsResponseMessage {
    public:
        ClientDhParamsResponseRetryMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, std::shared_ptr<StreamSlice>& nextNonceHash2)
                : ClientDhParamsResponseMessage(nonce, serverNonce),
                  nextNonceHash2(std::move(nextNonceHash2)) {};

        const std::shared_ptr<StreamSlice> nextNonceHash2;
    };

    class ClientDhParamsResponseFailMessage : public ClientDhParamsResponseMessage {
    public:
        ClientDhParamsResponseFailMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, std::shared_ptr<StreamSlice>& nextNonceHash3)
                : ClientDhParamsResponseMessage(nonce, serverNonce),
                  nextNonceHash3(std::move(nextNonceHash3)) {};

        const std::shared_ptr<StreamSlice> nextNonceHash3;
    };

    class PingMessage : public ProtoInternalMessage {
    public:
        explicit PingMessage(int64_t pingId)
                : pingId(pingId) {};

        const int64_t pingId;
    };

    class RpcResultMessage : public ProtoInternalMessage {
    public:
        RpcResultMessage(int64_t requestMessageId, std::shared_ptr<StreamSlice>& responseData)
                : requestMessageId(requestMessageId),
                  responseData(std::move(responseData)) {};

        const int64_t requestMessageId;
        const std::shared_ptr<StreamSlice> responseData;
    };

    class RpcError : public ProtoInternalMessage {
    public:
        RpcError(int32_t code, std::string& description)
                : code(code),
                  description(std::move(description)) {};

        const int32_t code;
        const std::string description;
    };

    class PongMessage : public ProtoInternalMessage {
    public:
        PongMessage(int64_t messageId, int64_t pingId)
                : messageId(messageId),
                  pingId(pingId) {};

        const int64_t messageId;
        const int64_t pingId;
    };

    class MsgsAckMessage : public ProtoInternalMessage {
    public:
        explicit MsgsAckMessage(std::vector<int64_t>& messageIds)
                : messageIds(std::move(messageIds)) {};

        const std::vector<int64_t> messageIds;
    };

    class Message : public ProtoInternalMessage {
    public:
        Message(int64_t messageId, int32_t seqNo, std::shared_ptr<StreamSlice>& body)
                : messageId(messageId),
                  seqNo(seqNo),
                  body(std::move(body)) {};

        const int64_t messageId;
        const int32_t seqNo;
        const std::shared_ptr<StreamSlice> body;
    };

    class Container : public ProtoInternalMessage {
    public:
        explicit Container(std::vector<std::shared_ptr<Message>>& messages)
                : messages(std::move(messages)) {};

        const std::vector<std::shared_ptr<Message>> messages;
    };

    class FutureSalt : public ProtoInternalMessage {
    public:
        FutureSalt(int32_t validSince, int32_t validUntil, int64_t salt)
                : validSince(validSince),
                  validUntil(validUntil),
                  salt(salt) {};

        const int32_t validSince;
        const int32_t validUntil;
        const int64_t salt;
    };

    class FutureSaltsMessage : public ProtoInternalMessage {
    public:
        FutureSaltsMessage(int64_t requestMessageId, int32_t now, std::vector<std::shared_ptr<FutureSalt>>& salts)
                : requestMessageId(requestMessageId),
                  now(now),
                  salts(std::move(salts)) {};

        const int64_t requestMessageId;
        const int32_t now;
        const std::vector<std::shared_ptr<FutureSalt>> salts;
    };

    class BadMsgNotificationMessage : public ProtoInternalMessage {
    public:
        BadMsgNotificationMessage(int64_t badMessageId, int32_t badSeqNo, int32_t errorCode)
                : badMessageId(badMessageId),
                  badSeqNo(badSeqNo),
                  errorCode(errorCode) {};

        const int64_t badMessageId;
        const int32_t badSeqNo;
        const int32_t errorCode;
    };

    class BadServerSaltNotificationMessage : public ProtoInternalMessage {
    public:
        BadServerSaltNotificationMessage(int64_t badMessageId, int32_t badSeqNo, int32_t errorCode, int64_t validServerSalt)
                : badMessageId(badMessageId),
                  badSeqNo(badSeqNo),
                  errorCode(errorCode),
                  validServerSalt(validServerSalt) {};

        const int64_t badMessageId;
        const int32_t badSeqNo;
        const int32_t errorCode;
        const int64_t validServerSalt;
    };

};
#endif //GPPROTO_PROTOINTERNALMESSAGE_H
