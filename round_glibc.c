// Standard C round*() from glibc to fix MinGW-w64 defects.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// Analects of glibc's rounding implementation, gathered from relevant
// portions of the glibc sources on 2016-12-22. URLs for the originals
// are given in comments above each copied block. GWC is responsible
// for any defect of colligation, which should not reflect on the
// reputations of the original authors. Headers copied in relevant
// part rather than included, for compactness. Reformatted for
// concinnity with lmi. Function names de-uglified for direct use;
// aliasing removed.

// Motivation: roundl(0.499999999999999999973L) should return zero,
// but the MinGW-w64 implementation as of 2016-12 returns one instead
// (unless the rounding mode, which should be irrelevant, is toward
// infinity). See:
//   https://lists.nongnu.org/archive/html/lmi/2016-12/msg00042.html
//   https://sourceforge.net/p/mingw-w64/bugs/573/

// Normally 'config.hpp' would be used to detect MinGW-w64. However,
// that header is to be included only by header files, so instead
// include any other lmi header--'version.hpp' is merely the simplest.
#include "version.hpp"

#if defined __MINGW64_VERSION_MAJOR

#include <math.h>
#include <stdint.h>

// This prestandard typedef appears to have originated in BSD:
//   https://lists.freedesktop.org/archives/release-wranglers/2004-August/000925.html
typedef uint32_t u_int32_t;

// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/ieee754/dbl-64/s_round.c
/* Round double to integer away from zero. */
// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/ieee754/flt-32/s_roundf.c
/* Round float to integer away from zero. */
// https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=sysdeps/ieee754/ldbl-96/s_roundl.c
/* Round long double to integer away from zero.
   Copyright (C) 1997-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

// https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=sysdeps/generic/math_private.h
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* A union which permits us to convert between a double and two 32 bit
   ints.  */

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
typedef union
{
  double value;
  struct
  {
    u_int32_t msw;
    u_int32_t lsw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;
#endif // __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
typedef union
{
  double value;
  struct
  {
    u_int32_t lsw;
    u_int32_t msw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;
#endif // __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__

/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0,ix1,d)                                \
do {                                                            \
  ieee_double_shape_type ew_u;                                  \
  ew_u.value = (d);                                             \
  (ix0) = ew_u.parts.msw;                                       \
  (ix1) = ew_u.parts.lsw;                                       \
} while (0)

/* Set a double from two 32 bit ints.  */
#if !defined INSERT_WORDS
# define INSERT_WORDS(d,ix0,ix1)                                \
do {                                                            \
  ieee_double_shape_type iw_u;                                  \
  iw_u.parts.msw = (ix0);                                       \
  iw_u.parts.lsw = (ix1);                                       \
  (d) = iw_u.value;                                             \
} while (0)
#endif // !defined INSERT_WORDS

/* A union which permits us to convert between a float and a 32 bit
   int.  */

typedef union
{
  float value;
  u_int32_t word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */
#if !defined GET_FLOAT_WORD
# define GET_FLOAT_WORD(i,d)                                    \
do {                                                            \
  ieee_float_shape_type gf_u;                                   \
  gf_u.value = (d);                                             \
  (i) = gf_u.word;                                              \
} while (0)
#endif // !defined GET_FLOAT_WORD

/* Set a float from a 32 bit int.  */
#if !defined SET_FLOAT_WORD
# define SET_FLOAT_WORD(d,i)                                    \
do {                                                            \
  ieee_float_shape_type sf_u;                                   \
  sf_u.word = (i);                                              \
  (d) = sf_u.value;                                             \
} while (0)
#endif // !defined SET_FLOAT_WORD

// https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=sysdeps/ieee754/ldbl-96/math_ldbl.h

/* A union which permits us to convert between a long double and
   three 32 bit ints.  */

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
typedef union
{
  long double value;
  struct
  {
    int sign_exponent:16;
    unsigned int empty:16;
    u_int32_t msw;
    u_int32_t lsw;
  } parts;
} ieee_long_double_shape_type;
#endif // __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
typedef union
{
  long double value;
  struct
  {
    u_int32_t lsw;
    u_int32_t msw;
    int sign_exponent:16;
    unsigned int empty:16;
  } parts;
} ieee_long_double_shape_type;
#endif // __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__

/* Get three 32 bit ints from a double.  */

#define GET_LDOUBLE_WORDS(exp,ix0,ix1,d)                        \
do {                                                            \
  ieee_long_double_shape_type ew_u;                             \
  ew_u.value = (d);                                             \
  (exp) = ew_u.parts.sign_exponent;                             \
  (ix0) = ew_u.parts.msw;                                       \
  (ix1) = ew_u.parts.lsw;                                       \
} while (0)

/* Set a double from two 32 bit ints.  */

#define SET_LDOUBLE_WORDS(d,exp,ix0,ix1)                        \
do {                                                            \
  ieee_long_double_shape_type iw_u;                             \
  iw_u.parts.sign_exponent = (exp);                             \
  iw_u.parts.msw = (ix0);                                       \
  iw_u.parts.lsw = (ix1);                                       \
  (d) = iw_u.value;                                             \
} while (0)

// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/ieee754/dbl-64/s_round.c

double round(double x)
{
  int32_t i0, j0;
  u_int32_t i1;

  EXTRACT_WORDS (i0, i1, x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  if (j0 < 20)
    {
      if (j0 < 0)
        {
          i0 &= 0x80000000;
          if (j0 == -1)
            i0 |= 0x3ff00000;
          i1 = 0;
        }
      else
        {
          u_int32_t i = 0x000fffff >> j0;
          if (((i0 & i) | i1) == 0)
            /* X is integral.  */
            return x;

          i0 += 0x00080000 >> j0;
          i0 &= ~i;
          i1 = 0;
        }
    }
  else if (j0 > 51)
    {
      if (j0 == 0x400)
        /* Inf or NaN.  */
        return x + x;
      else
        return x;
    }
  else
    {
      u_int32_t i = 0xffffffff >> (j0 - 20);
      if ((i1 & i) == 0)
        /* X is integral.  */
        return x;

      u_int32_t j = i1 + (1 << (51 - j0));
      if (j < i1)
        i0 += 1;
      i1 = j;
      i1 &= ~i;
    }

  INSERT_WORDS (x, i0, i1);
  return x;
}

// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/ieee754/flt-32/s_roundf.c

float roundf(float x)
{
  int32_t i0, j0;

  GET_FLOAT_WORD (i0, x);
  j0 = ((i0 >> 23) & 0xff) - 0x7f;
  if (j0 < 23)
    {
      if (j0 < 0)
        {
          i0 &= 0x80000000;
          if (j0 == -1)
            i0 |= 0x3f800000;
        }
      else
        {
          u_int32_t i = 0x007fffff >> j0;
          if ((i0 & i) == 0)
            /* X is integral.  */
            return x;

          i0 += 0x00400000 >> j0;
          i0 &= ~i;
        }
    }
  else
    {
      if (j0 == 0x80)
        /* Inf or NaN.  */
        return x + x;
      else
        return x;
    }

  SET_FLOAT_WORD (x, i0);
  return x;
}

// https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=sysdeps/ieee754/ldbl-96/s_roundl.c

long double roundl(long double x)
{
  int32_t j0;
  u_int32_t se, i1, i0;

  GET_LDOUBLE_WORDS (se, i0, i1, x);
  j0 = (se & 0x7fff) - 0x3fff;
  if (j0 < 31)
    {
      if (j0 < 0)
        {
          se &= 0x8000;
          i0 = i1 = 0;
          if (j0 == -1)
            {
              se |= 0x3fff;
              i0 = 0x80000000;
            }
        }
      else
        {
          u_int32_t i = 0x7fffffff >> j0;
          if (((i0 & i) | i1) == 0)
            /* X is integral.  */
            return x;

          u_int32_t j = i0 + (0x40000000 >> j0);
          if (j < i0)
            se += 1;
          i0 = (j & ~i) | 0x80000000;
          i1 = 0;
        }
    }
  else if (j0 > 62)
    {
      if (j0 == 0x4000)
        /* Inf or NaN.  */
        return x + x;
      else
        return x;
    }
  else
    {
      u_int32_t i = 0xffffffff >> (j0 - 31);
      if ((i1 & i) == 0)
        /* X is integral.  */
        return x;

      u_int32_t j = i1 + (1 << (62 - j0));
      if (j < i1)
        {
          u_int32_t k = i0 + 1;
          if (k < i0)
            {
              se += 1;
              k |= 0x80000000;
            }
          i0 = k;
        }
      i1 = j;
      i1 &= ~i;
    }

  SET_LDOUBLE_WORDS (x, se, i0, i1);
  return x;
}

#endif // defined __MINGW64_VERSION_MAJOR
