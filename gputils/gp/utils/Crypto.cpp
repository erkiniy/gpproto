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

#include <zlib.h>
using namespace gpproto;

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

    if (static_cast<int>(size) > 8 || (static_cast<int>(size) == 8) && (pq_str.bytes[0] & 128) != 0)
        return pq_factorize_big(pq_str, p_str, q_str);

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
    unsigned char output[SHA_DIGEST_LENGTH];
    SHA1(data.bytes, data.size, output);
    return std::make_shared<StreamSlice>(output, SHA_DIGEST_LENGTH);
}

std::shared_ptr<StreamSlice> Crypto::sha256(const StreamSlice& data) {
    unsigned char output[SHA256_DIGEST_LENGTH];
    SHA256(data.bytes, data.size, output);
    return std::make_shared<StreamSlice>(output, SHA256_DIGEST_LENGTH);
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

    unsigned char out[data.size + AES_BLOCK_SIZE];

    AES_cbc_encrypt(data.bytes, out, data.size, &key, aes_iv->raw, encrypt);

    return std::make_shared<StreamSlice>(out, data.size);
}

std::shared_ptr<StreamSlice> Crypto::aes_cbc_encrypt(const UInt256 &aes_key, UInt128 *aes_iv,
                                                     const StreamSlice& plain) {
    return aes_cbc_xcrypt(aes_key, aes_iv, plain, true);
}

std::shared_ptr<StreamSlice> Crypto::aes_cbc_decrypt(const UInt256 &aes_key, UInt128 *aes_iv,
                                                     const StreamSlice& cypher) {
    return aes_cbc_xcrypt(aes_key, aes_iv, cypher, false);
}

