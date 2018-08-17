//
// Created by Jaloliddin Erkiniy on 8/17/18.
//

#include "BigNum.h"
#include <limits>
#include <openssl/bn.h>

using namespace gpproto;

class BigNumContext::Impl {
public:
    BN_CTX *big_num_context;
    Impl() : big_num_context(BN_CTX_new()) {}

    Impl(const Impl &other) = delete;

    Impl &operator=(const Impl &other) = delete;

    Impl(Impl &&other) = delete;

    Impl &operator=(Impl &&other) = delete;

    ~Impl() {
        BN_CTX_free(big_num_context);
    }
};

BigNumContext::BigNumContext() : impl_(std::make_unique<Impl>()){

}

BigNumContext::BigNumContext(BigNumContext &&other) = default;

BigNumContext &BigNumContext::operator=(BigNumContext &&other) = default;

BigNumContext::~BigNumContext() = default;

class BigNum::Impl {
public:
    BIGNUM *big_num;

    Impl() : Impl(BN_new()) {}

    explicit Impl(BIGNUM *big_num) : big_num(big_num) {}

    Impl(const Impl &other) = delete;

    Impl &operator=(const Impl &other) = delete;

    Impl(Impl &&other) = delete;

    Impl &operator=(Impl &&other) = delete;

    ~Impl() {
        BN_clear_free(big_num);
    }
};

BigNum::BigNum() : impl_(std::make_unique<Impl>()) {

}

BigNum::BigNum(const BigNum &other) : BigNum() {
    *this = other;
}

BigNum &BigNum::operator=(const BigNum &other) {
    BIGNUM *result = BN_copy(impl_->big_num, other.impl_->big_num);
    return *this;
}

BigNum::BigNum(BigNum &&other) = default;

BigNum &BigNum::operator=(BigNum &&other) = default;

BigNum::~BigNum() = default;

BigNum BigNum::from_binary(const StreamSlice &str) {
    return BigNum(std::make_unique<Impl>(BN_bin2bn(str.bytes, (int)(str.size), nullptr)));
}

BigNum BigNum::from_decimal(const std::string& str) {
    BigNum result;
    int err = BN_dec2bn(&result.impl_->big_num, str.c_str());
    return result;
}

BigNum BigNum::from_raw(void *openssl_big_num) {
    return BigNum(std::make_unique<Impl>(static_cast<BIGNUM *>(openssl_big_num)));
}

BigNum::BigNum(std::unique_ptr<Impl> &&impl) : impl_(std::move(impl)) {
}

void BigNum::ensure_const_time() {
    BN_set_flags(impl_->big_num, BN_FLG_CONSTTIME);
}

int BigNum::get_num_bits() const {
    return BN_num_bits(impl_->big_num);
}

int BigNum::get_num_bytes() const {
    return BN_num_bytes(impl_->big_num);
}

void BigNum::set_bit(int num) {
    int result = BN_set_bit(impl_->big_num, num);
}

void BigNum::clear_bit(int num) {
    int result = BN_clear_bit(impl_->big_num, num);
}

bool BigNum::is_bit_set(int num) const {
    return BN_is_bit_set(impl_->big_num, num) != 0;
}

bool BigNum::is_prime(BigNumContext &context) const {
    int result = BN_is_prime_ex(impl_->big_num, BN_prime_checks, context.impl_->big_num_context, nullptr);
    return result == 1;
}

void BigNum::operator+=(uint32 value) {
    int result = BN_add_word(impl_->big_num, value);
}

void BigNum::operator-=(uint32 value) {
    int result = BN_sub_word(impl_->big_num, value);
}

void BigNum::operator*=(uint32 value) {
    int result = BN_mul_word(impl_->big_num, value);
}

void BigNum::operator/=(uint32 value) {
    BN_ULONG result = BN_div_word(impl_->big_num, value);
}

uint32 BigNum::operator%(uint32 value) const {
    BN_ULONG result = BN_mod_word(impl_->big_num, value);
    return (uint32)(result);
}

void BigNum::set_value(uint32 new_value) {
    if (new_value == 0) {
        BN_zero(impl_->big_num);
    } else {
        int result = BN_set_word(impl_->big_num, new_value);
    }
}

BigNum BigNum::clone() const {
    BIGNUM *result = BN_dup(impl_->big_num);
    return BigNum(std::make_unique<Impl>(result));
}

std::string BigNum::to_binary(int exact_size) const {
    int num_size = get_num_bytes();
    if (exact_size == -1) {
        exact_size = num_size;
    } else {

    }
    std::string res(exact_size, '\0');
    BN_bn2bin(impl_->big_num, reinterpret_cast<unsigned char *>(&res[exact_size - num_size]));
    return res;
}

std::string BigNum::to_decimal() const {
    char *result = BN_bn2dec(impl_->big_num);
    std::string res(result);
    OPENSSL_free(result);
    return res;
}

void BigNum::random(BigNum &r, int bits, int top, int bottom) {
    int result = BN_rand(r.impl_->big_num, bits, top, bottom);
}

void BigNum::add(BigNum &r, const BigNum &a, const BigNum &b) {
    int result = BN_add(r.impl_->big_num, a.impl_->big_num, b.impl_->big_num);
}

void BigNum::sub(BigNum &r, const BigNum &a, const BigNum &b) {
    int result = BN_sub(r.impl_->big_num, a.impl_->big_num, b.impl_->big_num);
}

void BigNum::mul(BigNum &r, BigNum &a, BigNum &b, BigNumContext &context) {
    int result = BN_mul(r.impl_->big_num, a.impl_->big_num, b.impl_->big_num, context.impl_->big_num_context);
}

void BigNum::mod_mul(BigNum &r, BigNum &a, BigNum &b, const BigNum &m, BigNumContext &context) {
    int result = BN_mod_mul(r.impl_->big_num, a.impl_->big_num, b.impl_->big_num, m.impl_->big_num,
                            context.impl_->big_num_context);


}

void BigNum::div(BigNum *quotient, BigNum *remainder, const BigNum &dividend, const BigNum &divisor,
                 BigNumContext &context) {
    auto q = quotient == nullptr ? nullptr : quotient->impl_->big_num;
    auto r = remainder == nullptr ? nullptr : remainder->impl_->big_num;
    if (q == nullptr && r == nullptr) {
        return;
    }

    auto result = BN_div(q, r, dividend.impl_->big_num, divisor.impl_->big_num, context.impl_->big_num_context);
}

void BigNum::mod_exp(BigNum &r, const BigNum &a, const BigNum &p, const BigNum &m, BigNumContext &context) {
    int result = BN_mod_exp(r.impl_->big_num, a.impl_->big_num, p.impl_->big_num, m.impl_->big_num,
                            context.impl_->big_num_context);
}

void BigNum::gcd(BigNum &r, BigNum &a, BigNum &b, BigNumContext &context) {
    int result = BN_gcd(r.impl_->big_num, a.impl_->big_num, b.impl_->big_num, context.impl_->big_num_context);
}

int BigNum::compare(const BigNum &a, const BigNum &b) {
    return BN_cmp(a.impl_->big_num, b.impl_->big_num);
}