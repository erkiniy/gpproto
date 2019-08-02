//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#ifndef GPPROTO_CRYPTO_H
#define GPPROTO_CRYPTO_H

#include <memory>
#include "gp/utils/StreamSlice.h"
#include "gp/utils/Common.h"

namespace gpproto::Crypto {

    uint64 pq_factorize(uint64 pq);

//    int pq_factorize_big(const StreamSlice &pq_str, std::string *p_str, std::string *q_str);

    int pq_factorize(const StreamSlice &pq_str, std::string *p_str, std::string *q_str);

    void initCrypto();

    std::shared_ptr<StreamSlice> sha1(const StreamSlice &data);

    std::shared_ptr<StreamSlice> sha256(const StreamSlice &data);

    std::shared_ptr<StreamSlice> sha256Subdata(const StreamSlice& data, size_t offset, size_t length);

    std::shared_ptr<StreamSlice> aes_cbc_encrypt(const UInt256 &aes_key, const UInt128 &aes_iv, const StreamSlice &plain);

    std::shared_ptr<StreamSlice> aes_cbc_decrypt(const UInt256 &aes_key, const UInt128 &aes_iv, const StreamSlice &cypher);

    std::shared_ptr<StreamSlice> rsa_encrypt_x509(const std::string &publicKey, const StreamSlice &plain);

    std::shared_ptr<StreamSlice> rsa_encrypt(const std::string &publicKey, const StreamSlice &plain);

    std::shared_ptr<StreamSlice> gzip_zip(const std::shared_ptr<StreamSlice>& data);

    std::shared_ptr<StreamSlice> gzip_unzip(const std::shared_ptr<StreamSlice>& data);

    std::shared_ptr<StreamSlice> mod_exp(const std::shared_ptr<StreamSlice>& base, const std::shared_ptr<StreamSlice>& exp, const std::shared_ptr<StreamSlice>& modulus);

    bool isSafeG(int32_t g);

    std::shared_ptr<StreamSlice> stripPublicKeyHeader(const StreamSlice& d_key);

}


#endif //GPPROTO_CRYPTO_H
