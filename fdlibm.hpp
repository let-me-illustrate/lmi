// FDLIBM header.
//
// Copyright (C) 2022 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// A portion of fdlibm's main header, extracted on 2022-05-23.
// URL for the original:
//    https://www.netlib.org/cgi-bin/netlibfiles.txt?format=txt&filename=fdlibm%2Ffdlibm.h
// Adapted for lmi by GWC; any defects introduced should not reflect
// on the reputations of the original authors.

/* @(#)fdlibm.h 1.5 04/04/22 */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#ifndef fdlibm_hpp
#define fdlibm_hpp

#include "config.hpp"

#include <stdint.h>

// Apparently the clang maintainers believe that floating-point
// endianness is necessarily the same as integer endianness.
#if defined __clang__
#   if !defined __FLOAT_WORD_ORDER__ && defined __BYTE_ORDER__
#       define __FLOAT_WORD_ORDER__ __BYTE_ORDER__
#   endif // !defined __FLOAT_WORD_ORDER__ && defined __BYTE_ORDER__
#endif // defined __clang__

// And MSVC maintainers don't believe in having different endianness
// values at all, so the compiler never predefines these symbols.
#if defined LMI_MSC
    #define __ORDER_BIG_ENDIAN__ 4321
    #define __ORDER_LITTLE_ENDIAN__ 1234
    #define __FLOAT_WORD_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif // defined LMI_MSC

#if !defined __FLOAT_WORD_ORDER__ || \
    !defined __ORDER_BIG_ENDIAN__ || \
    !defined __ORDER_LITTLE_ENDIAN__
#   error Expected endianness macros not defined.
#endif // expected endianness macros not defined

#if   __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__ // okay
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__    // also okay
#else  // not okay
#   error Unknown endianness.
#endif // not okay

// Improved type-punning macros adapted from:
//   https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=sysdeps/generic/math_private.h
// That file retains the original Sun copyright and license notices,
// and does not add any for glibc; the commit message suggests that
// the original work was done for netbsd.
//
// Adapted for lmi by GWC; any defects introduced should not reflect
// on the reputations of the original authors.

/* The original fdlibm code used statements like:
        n0 = ((*(int*)&one)>>29)^1; * index of high word *
        ix0 = *(n0+(int*)&x);       * high word of x *
        ix1 = *((1-n0)+(int*)&x);   * low word of x *
   to dig two 32 bit words out of the 64 bit IEEE floating point
   value.  That is non-ANSI, and, moreover, the gcc instruction
   scheduler gets it wrong.  We instead use the following macros.
   Unlike the original code, we determine the endianness at compile
   time, not at run time; I don't see much benefit to selecting
   endianness at run time.  */

/* A union which permits us to convert between a double and two 32 bit
   ints.  */

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
typedef union
{
  double value;
  struct
  {
    uint32_t msw;
    uint32_t lsw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;
#endif // __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__

#if   __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
typedef union
{
  double value;
  struct
  {
    uint32_t lsw;
    uint32_t msw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;
#endif // __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__

// Get two 32 bit ints from a double.

#define EXTRACT_WORDS(ix0,ix1,d)                                \
do {                                                            \
  ieee_double_shape_type ew_u;                                  \
  ew_u.value = (d);                                             \
  (ix0) = ew_u.parts.msw;                                       \
  (ix1) = ew_u.parts.lsw;                                       \
} while (0)

// Get the more significant 32 bit int from a double.

#define GET_HIGH_WORD(i,d)                                      \
do {                                                            \
  ieee_double_shape_type gh_u;                                  \
  gh_u.value = (d);                                             \
  (i) = gh_u.parts.msw;                                         \
} while (0)

static inline uint32_t hi_uint(double d)
{
    uint32_t i;
    GET_HIGH_WORD(i,d);
    return i;
}

#if defined __cplusplus && defined LMI_GCC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wold-style-cast"
#endif // defined __cplusplus && defined LMI_GCC
static inline int32_t hi_int(double d)
{
    uint32_t i;
    GET_HIGH_WORD(i,d);
    return (int32_t)i;
}
#if defined __cplusplus && defined LMI_GCC
#   pragma GCC diagnostic pop
#endif // defined __cplusplus && defined LMI_GCC

// Get the less significant 32 bit int from a double.

#define GET_LOW_WORD(i,d)                                       \
do {                                                            \
  ieee_double_shape_type gl_u;                                  \
  gl_u.value = (d);                                             \
  (i) = gl_u.parts.lsw;                                         \
} while (0)

static inline uint32_t lo_uint(double d)
{
    uint32_t i;
    GET_LOW_WORD(i,d);
    return i;
}

// Get all in one, efficient on 64-bit machines.

#define EXTRACT_WORDS64(i,d)                                    \
do {                                                            \
  ieee_double_shape_type gh_u;                                  \
  gh_u.value = (d);                                             \
  (i) = gh_u.word;                                              \
} while (0)

// Set a double from two 32 bit ints.

#define INSERT_WORDS(d,ix0,ix1)                                 \
do {                                                            \
  ieee_double_shape_type iw_u;                                  \
  iw_u.parts.msw = (ix0);                                       \
  iw_u.parts.lsw = (ix1);                                       \
  (d) = iw_u.value;                                             \
} while (0)

// Get all in one, efficient on 64-bit machines.

#define INSERT_WORDS64(d,i)                                     \
do {                                                            \
  ieee_double_shape_type iw_u;                                  \
  iw_u.word = (i);                                              \
  (d) = iw_u.value;                                             \
} while (0)

// Set the more significant 32 bits of a double from an int.

#define SET_HIGH_WORD(d,v)                                      \
do {                                                            \
  ieee_double_shape_type sh_u;                                  \
  sh_u.value = (d);                                             \
  sh_u.parts.msw = (v);                                         \
  (d) = sh_u.value;                                             \
} while (0)

// Set the less significant 32 bits of a double from an int.

#define SET_LOW_WORD(d,v)                                       \
do {                                                            \
  ieee_double_shape_type sl_u;                                  \
  sl_u.value = (d);                                             \
  sl_u.parts.lsw = (v);                                         \
  (d) = sl_u.value;                                             \
} while (0)

#if defined __cplusplus
extern "C"
{
#endif // defined __cplusplus
double fdlibm_expm1(double);
double fdlibm_log1p(double);
#if defined __cplusplus
} // extern "C"
#endif // defined __cplusplus

#endif // fdlibm_hpp
