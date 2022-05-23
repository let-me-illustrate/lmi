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

// Apparently the clang maintainers believe that floating-point
// endianness is necessarily the same as integer endianness.
#if defined __clang__
#   if !defined __FLOAT_WORD_ORDER__ && defined __BYTE_ORDER__
#       define __FLOAT_WORD_ORDER__ __BYTE_ORDER__
#   endif // !defined __FLOAT_WORD_ORDER__ && defined __BYTE_ORDER__
#endif // defined __clang__

#if !defined __FLOAT_WORD_ORDER__ || \
    !defined __ORDER_BIG_ENDIAN__ || \
    !defined __ORDER_LITTLE_ENDIAN__
#   error Expected endianness macros not defined.
#endif // expected endianness macros not defined

// https://www.netlib.org/fdlibm/readme
//
// NOT FIXED YET
//
//    3. Compiler failure on non-standard code
//         Statements like
//                     *(1+(int*)&t1) = 0;
//         are not standard C and cause some optimizing compilers (e.g. GCC)
//         to generate bad code under optimization.    These cases
//         are to be addressed in the next release.

#if   __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#   define FDLIBM_HI(x) *(1+(int*)&x)
#   define FDLIBM_LO(x) *(int*)&x
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#   define FDLIBM_HI(x) *(int*)&x
#   define FDLIBM_LO(x) *(1+(int*)&x)
#else  // unknown endianness
#   error Unknown endianness.
#endif // unknown endianness

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
