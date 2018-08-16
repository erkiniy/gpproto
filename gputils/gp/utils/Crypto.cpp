//
// Created by Jaloliddin Erkiniy on 8/16/18.
//

#include "Crypto.h"

#include <openssl/aes.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include <zlib.h>

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

std::shared_ptr<StreamSlice> sha1(const StreamSlice& data) {
    
}
std::shared_ptr<StreamSlice> sha256(const StreamSlice& data) {

}

