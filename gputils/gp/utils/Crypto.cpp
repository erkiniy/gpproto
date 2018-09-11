//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#include "Crypto.h"
#include "Random.h"

#include <openssl/aes.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <zlib.h>

#include "gp/utils/OutputStream.h"
#include "gp/utils/BigNum.h"

using namespace gpproto;

static int get_evp_pkey_type(EVP_PKEY *pkey) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    return EVP_PKEY_type(pkey->type);
#else
    return EVP_PKEY_base_id(pkey);
#endif
}

template <class FromT>
static std::string as_big_endian_string(const FromT &from) {
    size_t size = sizeof(from);
    std::string res(size, '\0');

    auto ptr = reinterpret_cast<const unsigned char *>(&from);
    std::memcpy(&res[0], ptr, size);

    size_t i = size;
    while (i && res[i - 1] == 0) {
        i--;
    }

    res.resize(i);
    std::reverse(res.begin(), res.end());
    return res;
}

static uint64 gcd(uint64 a, uint64 b) {
    if (a == 0) {
        return b;
    }
    if (b == 0) {
        return a;
    }

    int shift = 0;
    while ((a & 1) == 0 && (b & 1) == 0) {
        a >>= 1;
        b >>= 1;
        shift++;
    }

    while (true) {
        while ((a & 1) == 0) {
            a >>= 1;
        }
        while ((b & 1) == 0) {
            b >>= 1;
        }
        if (a > b) {
            a -= b;
        } else if (b > a) {
            b -= a;
        } else {
            return a << shift;
        }
    }
}

static int min(int x, int y) {
    return x > y ? y : x;
}

static int max(int x, int y) {
    return x > y ? x : y;
}

uint64 Crypto::pq_factorize(uint64 pq) {
    if (pq < 2 || pq > (static_cast<uint64>(1) << 63)) {
        return 1;
    }
    uint64 g = 0;
    for (int i = 0, it = 0; i < 3 || it < 1000; i++) {
        uint64 q = Random::fast(17, 32) % (pq - 1);
        uint64 x = Random::fastUInt64() % (pq - 1) + 1;
        uint64 y = x;
        int lim = 1 << (min(5, i) + 18);
        for (int j = 1; j < lim; j++) {
            it++;
            uint64 a = x;
            uint64 b = x;
            uint64 c = q;

            // c += a * b
            while (b) {
                if (b & 1) {
                    c += a;
                    if (c >= pq) {
                        c -= pq;
                    }
                }
                a += a;
                if (a >= pq) {
                    a -= pq;
                }
                b >>= 1;
            }

            x = c;
            uint64 z = x < y ? pq + x - y : x - y;
            g = gcd(z, pq);
            if (g != 1) {
                break;
            }

            if (!(j & (j - 1))) {
                y = x;
            }
        }
        if (g > 1 && g < pq) {
            break;
        }
    }
    if (g != 0) {
        uint64 other = pq / g;
        if (other < g) {
            g = other;
        }
    }
    return g;
}

int Crypto::pq_factorize(const StreamSlice& pq_str, std::string *p_str, std::string *q_str) {
    size_t size = pq_str.size;

//    if (static_cast<int>(size) > 8 || ((static_cast<int>(size) == 8) && (pq_str.bytes[0] & 128) != 0))
//        return pq_factorize_big(pq_str, p_str, q_str);

    auto ptr = pq_str.bytes;
    uint64 pq = 0;

    for (int i = 0; i < static_cast<int>(size); i++) {
        pq = (pq << 8) | ptr[i];
    }

    uint64 p = pq_factorize(pq);
    if (p == 0 || pq % p != 0)
        return -1;

    *p_str = as_big_endian_string(p);
    *q_str = as_big_endian_string(pq / p);

    return 0;
}


void gpproto::Crypto::initCrypto() {
    static bool inited = false;

    if (inited)
        return;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    OPENSSL_init_crypto(0, nullptr);
#else
    OpenSSL_add_all_algorithms();
#endif
    inited = true;
}

std::shared_ptr<StreamSlice> Crypto::sha1(const StreamSlice& data) {
    auto output = std::make_shared<StreamSlice>(SHA256_DIGEST_LENGTH);
    SHA1(data.bytes, data.size, output->begin());
    return output;
}

std::shared_ptr<StreamSlice> Crypto::sha256(const StreamSlice& data) {
    return sha256Subdata(data, 0, data.size);
}

std::shared_ptr<StreamSlice> Crypto::sha256Subdata(const StreamSlice &data, size_t offset, size_t length) {
    auto output = std::make_shared<StreamSlice>(SHA256_DIGEST_LENGTH);
    SHA256(data.bytes + offset, length, output->begin());
    return output;
}

std::shared_ptr<StreamSlice> aes_cbc_xcrypt(const UInt256 &aes_key, UInt128 *aes_iv,
                                            const StreamSlice &data, bool encrypt) {
    AES_KEY key;
    int err;
    if (encrypt)
        err = AES_set_encrypt_key(aes_key.raw, 256, &key);
    else
        err = AES_set_decrypt_key(aes_key.raw, 256, &key);

    if (err)
        return nullptr;

    auto out = std::make_shared<StreamSlice>(data.size);

    AES_cbc_encrypt(data.bytes, out->begin(), data.size, &key, aes_iv->raw, encrypt);

    return out;
}

std::shared_ptr<StreamSlice> Crypto::aes_cbc_encrypt(const UInt256 &aes_key, UInt128 *aes_iv,
                                                     const StreamSlice& plain) {
    initCrypto();
    return aes_cbc_xcrypt(aes_key, aes_iv, plain, true);
}

std::shared_ptr<StreamSlice> Crypto::aes_cbc_decrypt(const UInt256 &aes_key, UInt128 *aes_iv,
                                                     const StreamSlice& cypher) {
    initCrypto();
    return aes_cbc_xcrypt(aes_key, aes_iv, cypher, false);
}

std::shared_ptr<StreamSlice> Crypto::gzip_zip(const std::shared_ptr<StreamSlice> &data) {
#warning implement gzip_zip
    return nullptr;
}

std::shared_ptr<StreamSlice> Crypto::gzip_unzip(const std::shared_ptr<StreamSlice> &data) {
    const int kMemoryChunkSize = 1024;

    auto length = data->size;
    int windowBits = 15 + 32;
    int retCode;

    unsigned char output[kMemoryChunkSize];
    uInt gotBack;
    std::unique_ptr<OutputStream> os = std::make_unique<OutputStream>();
    z_stream stream;

    if (length == 0)
        return nullptr;

    bzero(&stream, sizeof(z_stream));
    stream.avail_in = static_cast<uInt>(length);
    stream.next_in = data->bytes;

    retCode = inflateInit2(&stream, windowBits);
    if (retCode != Z_OK)
    {
        LOGE("Inflate failed with error %d", retCode);
        return nullptr;
    }

    do {
        stream.avail_out = kMemoryChunkSize;
        stream.next_out = output;
        retCode = inflate(&stream, Z_NO_FLUSH);
        if ((retCode != Z_OK) && (retCode != Z_STREAM_END))
        {
            LOGE("Inflate failed with error %d", retCode);
            inflateEnd(&stream);
            return nullptr;
        }
        gotBack = kMemoryChunkSize - stream.avail_out;
        if (gotBack > 0) {
            StreamSlice s(gotBack);
            memcpy(s.begin(), output, gotBack);
            os->writeData(s);
        }

    } while (retCode == Z_OK);

    inflateEnd(&stream);

    if (retCode != Z_STREAM_END)
        return nullptr;

    return os->currentBytes();
}

std::shared_ptr<StreamSlice> Crypto::rsa_encrypt_pkcs1(const std::string &publicKey, const StreamSlice &plain) {
    initCrypto();

    auto cKey = publicKey.c_str();
    LOGV("Public key %s", cKey);

    auto *bio = BIO_new_mem_buf(const_cast<void *>(static_cast<const void *>(cKey)), static_cast<size_t>(strlen(cKey)));

    if (bio == nullptr) {
        LOGE("[Crypto rsa_encrypt] -> cannot create BIO");
        return nullptr;
    }

    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (pkey == nullptr)
    {
        LOGE("[Crypto rsa_encrypt] -> cannot create public key");
        return nullptr;
    }

    if (get_evp_pkey_type(pkey) != EVP_PKEY_RSA) {
        LOGE("[Crypto rsa_encrypt] -> wrong key type, expected RSA");
        return nullptr;
    }

#if OPENSSL_VERSION_NUMBER < 0x10000000L
    RSA *rsa = pkey->pkey.rsa;
    int outlen = RSA_size(rsa);
    auto result = std::make_shared<StreamSlice>(outlen);
    if (RSA_public_encrypt(narrow_cast<int>(data.size()), const_cast<unsigned char *>(data.ubegin()),
                         res.as_slice().ubegin(), rsa, RSA_PKCS1_OAEP_PADDING) != outlen) {
#else
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) {
        LOGE("[Crypto rsa_encrypt] -> cannot create EVP_PKEY_CTX");
        return nullptr;
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        LOGE("[Crypto rsa_encrypt] -> cannot init EVP_PKEY_CTX");
        return nullptr;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        LOGE("[Crypto rsa_encrypt] -> cannot set RSA_PKCS1_OAEP padding in EVP_PKEY_CTX");
        return nullptr;
    }

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, plain.rbegin(), plain.size) <= 0) {
        LOGE("[Crypto rsa_encrypt] -> cannot calculate encrypted length");
        return nullptr;
    }

    auto result = std::make_shared<StreamSlice>(outlen);

    if (EVP_PKEY_encrypt(ctx, result->begin(), &outlen, plain.rbegin(), plain.size) <= 0) {
        LOGE("[Crypto rsa_encrypt] -> cannot encrypt");
        return nullptr;
    }
#endif

    BIO_free(bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);

    return result;
}


bool Crypto::isSafeG(int32_t g) {
    return g >= 2 && g <= 7;
}

std::shared_ptr<StreamSlice> Crypto::mod_exp(const std::shared_ptr<StreamSlice> &base,
                                             const std::shared_ptr<StreamSlice> &exp,
                                             const std::shared_ptr<StreamSlice> &modulus) {
    BigNumContext ctx;
    auto bnBase = BigNum::from_binary(*base);
    auto bnExp = BigNum::from_binary(*exp);
    auto bnMod = BigNum::from_binary(*modulus);

    auto bnRest = BigNum();
    BigNum::mod_exp(bnRest, bnBase, bnExp, bnMod, ctx);

    auto result = bnRest.to_binary_slice();
    return result;
}
