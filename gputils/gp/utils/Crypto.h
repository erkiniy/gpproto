//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#ifndef GPPROTO_CRYPTO_H
#define GPPROTO_CRYPTO_H

#include "Common.h"
#include "StreamSlice.h"
#include <memory>

using namespace gpproto;
namespace gpproto::Crypto {
    uint64 pq_factorize(uint64 pq);
    void initCrypto();

    std::shared_ptr<StreamSlice> sha1(const StreamSlice& data);
    std::shared_ptr<StreamSlice> sha256(const StreamSlice& data);
}

#endif //GPPROTO_CRYPTO_H
