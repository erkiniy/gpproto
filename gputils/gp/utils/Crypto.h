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
    int pq_factorize_big(const StreamSlice& pq_str, std::string *p_str, std::string *q_str);
    int pq_factorize(const StreamSlice& pq_str, std::string *p_str, std::string *q_str);

    void initCrypto();

    std::shared_ptr<StreamSlice> sha1(const StreamSlice& data);
    std::shared_ptr<StreamSlice> sha256(const StreamSlice& data);

    std::shared_ptr<StreamSlice> aes_cbc_encrypt(const UInt256 &aes_key, UInt128 *aes_iv, const StreamSlice& plain);
    std::shared_ptr<StreamSlice> aes_cbc_decrypt(const UInt256 &aes_key, UInt128 *aes_iv, const StreamSlice& cypher);


#endif //GPPROTO_CRYPTO_H
