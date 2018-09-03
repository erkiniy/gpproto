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
    };

    class ResPqMessage : public ProtoInternalMessage {
    public:
        ResPqMessage(std::shared_ptr<StreamSlice>& nonce, std::shared_ptr<StreamSlice>& serverNonce, std::vector<int64_t>& serverPublicKeyFingerprints)
                : nonce(std::move(nonce)),
                  serverNonce(std::move(serverNonce)),
                  serverPublicKeyFingerprints(std::move(serverPublicKeyFingerprints)) {};

        const std::shared_ptr<StreamSlice> nonce;
        const std::shared_ptr<StreamSlice> serverNonce;
        const std::vector<int64_t> serverPublicKeyFingerprints;
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
};
#endif //GPPROTO_PROTOINTERNALMESSAGE_H
