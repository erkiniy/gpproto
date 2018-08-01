//
// Created by ProDigital on 8/1/18.
//

#ifndef GPPROTO_BYTEORDER_H
#define GPPROTO_BYTEORDER_H

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || defined(__BIG_ENDIAN__)

/*
 * Endiannes macro
 */

#   define GP_IS_LITTLE_ENDIAN  0
#   define GP_IS_BIG_ENDIAN     1

#elif defined(__BYTE_ORDER) && __BYTE_ORDER == _LITTLE_ENDIAN || defined(__LITTLE_ENDIAN__)

/*
 * Endiannes macro
 */

#   define GP_IS_LITTLE_ENDIAN  1
#   define GP_IS_BIG_ENDIAN     0

#elif defined(_i386_) || defined(i_386_) || defined(_X86_) || defined(x86) || \
    defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(__I86__)

/*
 * Generic i386 processor family, little-endian
 */
#   undef GP_M_I386
#   define GP_M_I386            1
#   define GP_IS_LITTLE_ENDIAN  1
#   define GP_IS_BIG_ENDIAN     0


#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || \
    defined(_M_X64) || defined(_M_AMD64)

/*
 * AMD 64bit processor family, little-endian
 */
#   undef GP_M_X86_64
#   define GP_M_X86_64            1
#   define GP_IS_LITTLE_ENDIAN    1
#   define GP_IS_BIG_ENDIAN       0


#elif defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(_M_I_A64)

/*
 * Intel IA64 processor family, little-endian
 */
#   undef GP_M_IA64
#   define GP_M_IA64            1
#   define GP_IS_LITTLE_ENDIAN  1
#   define GP_IS_BIG_ENDIAN     0

#elif defined(__alpha__) || defined(__alpha)

/*
 * DEC Alpha processor family, little-endian
 */
#   undef GP_M_ALPHA
#   define GP_M_ALPHA            1
#   define GP_IS_LITTLE_ENDIAN   1
#   define GP_IS_BIG_ENDIAN      0

#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(MIPS) || defined(_MIPS_)

/*
 * MIPS, bi-endian, so raise error if endiannes is not configured
 */
#   undef GP_M_MIPS
#   define GP_M_MIPS             1
#   if !GP_IS_LITTLE_ENDIAN && !GP_IS_BIG_ENDIAN
#      error Endiannes must be declared for this processor
#   endif

#elif defined(ARM) || defined(_ARM_) || defined(__arm__) || defined(_M_ARM)

/*
 * ARM, bi-endian, so raise error if endiannes is not configured
 */
#   undef GP_M_SPARC
#   define GP_M_SPARC            1
#   define GP_IS_LITTLE_ENDIAN   0
#   define GP_IS_BIG_ENDIAN      1

#else
#   error Undefined endianness
#endif

#endif //GPPROTO_BYTEORDER_H
