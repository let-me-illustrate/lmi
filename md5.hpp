/* md5.h - Declaration of functions and data types used for MD5 sum
 * computing library functions.
 * Copyright 1995, 1996, 2000 Free Software Foundation, Inc.
 * NOTE: The canonical source of this file is maintained with the GNU C
 * Library. Bugs can be reported to bug-glibc@prep.ai.mit.edu.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* 2003-02 GWC got this code from
 *   http://www.nondot.org/gcc/md5_8h-source.html
 * and modified it as indicated below by initials 'GWC'. Modifications are
 *   Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares
 * and licensed under the same terms as the original, as given above.
 * This is a derived work. Any defect in it should not reflect on
 * Ulrich Drepper's reputation.
 *
 * http://savannah.nongnu.org/projects/lmi
 * email: <gchicares@sbcglobal.net>
 * snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
 *
 * $Id$
 *
 * GWC: File renamed md5.h --> md5.hpp so that grep whatever *.?pp
 * can be used across the lmi project.
 *
 * GWC: Transplanting this code from a system library to application
 * code makes original include guard "_MD5_H" problematic under C++98
 * 17.4.3.1.2, so change it.
 *
 * For subsequent GWC modifications, see 'ChangeLog'.
 */

#ifndef md5_hpp
#define md5_hpp

// GWC: lmi compatibility header.
#include "config.hpp"

#include <cstddef> // std::size_t
#include <cstdio>  // GWC added this required header.

/* GWC: Make this header usable with C++. */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* GWC: Unconditionally assume we have the standard C headers.
 * #if defined HAVE_LIMITS_H || _LIBC
 * # include <limits.h>
 * #endif
 */
#include <limits.h>

/* The following contortions are an attempt to use the C preprocessor
 * to determine an unsigned integral type that is 32 bits wide. An
 * alternative approach is to use autoconf's AC_CHECK_SIZEOF macro, but
 * doing that would require that the configure script compile and *run*
 * the resulting executable. Locally running cross-compiled executables
 * is usually not possible.
 */

#ifdef _LIBC
# include <sys/types.h>
typedef u_int32_t md5_uint32;
#else
#  define INT_MAX_32_BITS 2147483647

/* If UINT_MAX isn't defined, assume it's a 32-bit type.
 * This should be valid for all systems GNU cares about because
 * that doesn't include 16-bit systems, and only modern systems
 * (that certainly have <limits.h>) have 64+-bit integral types.
 */

# ifndef INT_MAX
#  define INT_MAX INT_MAX_32_BITS
# endif

# if INT_MAX == INT_MAX_32_BITS
   typedef unsigned int md5_uint32;
# else
#  if SHRT_MAX == INT_MAX_32_BITS
    typedef unsigned short int md5_uint32;
#  else
#   if LONG_MAX == INT_MAX_32_BITS
     typedef unsigned long int md5_uint32;
#   else
     /* The following line is intended to evoke an error.
      * Using #error is not portable enough.
      */
     "Cannot determine unsigned 32-bit data type."
#   endif
#  endif
# endif
#endif

/* GWC: Here, __STDC__ is tested, but what should be done if that test
 * fails? The gnu project assumes it may have a pre-1989 C compiler. I
 * prefer to assume I have a conforming compiler in a mode of relaxed
 * conformance that nonetheless permits prototypes--which are required
 * by C++ anyway.
 *
 * [space follows leading underscores in reserved name]
 * At any rate, "_ _ P" is a reserved name, and moving it from glibc
 * to lmi makes that a problem not just in theory but also in fact
 * because it conflicts with GNU/Linux system headers, so it's renamed
 * here (and explicitly undefined at the end of the header). Even in
 * this commented-out code, "_ _ P" has been rewritten as "MD5_P" to
 * avoid triggering lmi's tests for reserved identifiers.
 *
 * #undef MD5_P
 * #if defined (__STDC__) && __STDC__
 * #define MD5_P(x) x
 * #else
 * #define MD5_P(x) ()
 * #endif
 */
#define LMI_P(x) x

/* Structure to save state of computation between the single steps. */
struct md5_ctx
{
  md5_uint32 A;
  md5_uint32 B;
  md5_uint32 C;
  md5_uint32 D;

  md5_uint32 total[2];
  md5_uint32 buflen;
  char buffer[128];
};

/*
 * The following three functions are build up the low level used in
 * the functions `md5_stream' and `md5_buffer'.
 */

/* Initialize structure containing state of computation.
 * (RFC 1321, 3.3: Step 3)
 */
extern void md5_init_ctx LMI_P ((struct md5_ctx *ctx));

/* Starting with the result of former calls of this function (or the
 * initialization function update the context for the next LEN bytes
 * starting at BUFFER.
 * TRICKY !! It is necessary that LEN is a multiple of 64.
 */
extern void md5_process_block LMI_P ((const void *buffer, std::size_t len,
                    struct md5_ctx *ctx));

/* Starting with the result of former calls of this function (or the
 * initialization function update the context for the next LEN bytes
 * starting at BUFFER.
 * It is NOT required that LEN is a multiple of 64.
 */
extern void md5_process_bytes LMI_P ((const void *buffer, std::size_t len,
                    struct md5_ctx *ctx));

/* Process the remaining bytes in the buffer and put result from CTX
 * in first 16 bytes following RESBUF. The result is always in little
 * endian byte order, so that a byte-wise output yields to the wanted
 * ASCII representation of the message digest.
 *
 * IMPORTANT: On some systems it is required that RESBUF is correctly
 * aligned for a 32 bits value.
 */
extern void *md5_finish_ctx LMI_P ((struct md5_ctx *ctx, void *resbuf));

/* Put result from CTX in first 16 bytes following RESBUF. The result is
 * always in little endian byte order, so that a byte-wise output yields
 * to the wanted ASCII representation of the message digest.
 *
 * IMPORTANT: On some systems it is required that RESBUF is correctly
 * aligned for a 32 bits value.
 */
extern void *md5_read_ctx LMI_P ((const struct md5_ctx *ctx, void *resbuf));

/* Compute MD5 message digest for bytes read from STREAM. The
 * resulting message digest number will be written into the 16 bytes
 * beginning at RESBLOCK.
 */
extern int md5_stream LMI_P ((std::FILE *stream, void *resblock));

/* Compute MD5 message digest for LEN bytes beginning at BUFFER. The
 * result is always in little endian byte order, so that a byte-wise
 * output yields to the wanted ASCII representation of the message
 * digest.
 */
extern void *md5_buffer LMI_P ((const char *buffer, std::size_t len, void *resblock));
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

/* GWC: Explicitly undefine prototype macro. */
#undef LMI_P

#endif // md5_hpp

