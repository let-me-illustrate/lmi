// Configuration.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// Configuration header for compiler quirks. Include at the beginning of
// every .hpp file (and nowhere else).

#ifndef config_hpp
#define config_hpp

#if defined __cplusplus
// Namespace alii.
namespace boost {namespace filesystem {} }
namespace fs = boost::filesystem;
#endif // defined __cplusplus

// The msw platform-identifying macro that its vendor encourages
// people to use contains the word "win". I don't consider a non-free
// operating system a win, and won't advertise it as such by writing
// that macro repeatedly. Instead, I follow rms's guidance:
//   http://lists.gnu.org/archive/html/bug-gnulib/2003-06/msg00081.html
// "...you can test __WIN32__ once and define some other macro,
// which you can test subsequently."
//
// I define macro LMI_POSIX not to avoid saying "POSIX", but because
// someone might point out a better way to detect POSIX, and a macro
// defined in only one place can easily be modified. There seems to be
// no direct way to detect POSIX, though, so I use common macros that
// regrettably appear to name a non-free operating system.
//
// Reference: http://predef.sourceforge.net/preos.html

#if defined unix || defined __unix__ || defined __unix // Detected POSIX.
#   define LMI_POSIX
#elif defined __WIN32__ || defined _WIN32 || defined WIN32 // Detected msw.
#   define LMI_MSW
#else  // Unknown OS.
#   error Unknown operating system. Consider contributing support.
#endif // Unknown OS.

// Reference: http://predef.sourceforge.net/prearch.html

#if defined __x86_64 || defined __x86_64__ || defined __amd64 || defined __amd64__ || defined _M_X64
#   define LMI_X86
#   define LMI_X86_64
#elif defined _X86_ || defined _M_IX86 || defined i386 || defined __i386
    // Not amd64, so presumably x86-32.
#   define LMI_X86
#   define LMI_X86_32
#else  // Unknown hardware.
#   error Unknown hardware. Consider contributing support.
#endif // Unknown hardware.

// Test for x87.
//
// For gcc, test the __SSE_MATH__ macro. It might seem logical to test
// the __SSE__ macro instead; however, for x86_64, gcc defines __SSE__
// even if SSE is explicitly disabled with '-mfpmath=387'.
//
// The _M_IX86_FP test is conditional on _MSC_VER. It would be
// slightly better to use LMI_MSC in the conditional instead, but
// that doesn't justify moving this test block far away from where
// LMI_X86 is set. See:
//   https://lists.nongnu.org/archive/html/lmi/2017-01/msg00009.html

#if defined LMI_X86
#   if defined __GNUC__
#       if !defined __SSE_MATH__
#           define LMI_X87
#       endif // !defined __SSE_MATH__
#   elif defined _MSC_VER
#       if defined _M_IX86_FP && _M_IX86_FP == 0
#           define LMI_X87
#       endif // defined _M_IX86_FP && _M_IX86_FP == 0
#   else   // Unknown compiler.
#       error Unknown compiler--cannot detect SSE. Consider contributing support.
#   endif  // Unknown compiler.
#endif // defined LMI_X86

// This selects a correct snprintf() for MinGW-w64.

#if defined __GNUC__
#   if !defined _ISOC99_SOURCE
#       define _ISOC99_SOURCE
#   endif // !defined _ISOC99_SOURCE
#endif // defined __GNUC__

// 'platform_dependent.hpp' includes standard headers in an unusual
// way, and must be included before any standard headers are seen.
// Do that here to quarantine the weirdness.
//
#include "platform_dependent.hpp"

// It is impossible to compile lmi with g++ prior to version 3, though
// old versions of gcc would be adequate for C translation units.

#if defined __GNUC__ && __GNUC__ < 3 && defined __cplusplus
#   error Obsolete compiler not supported.
#endif // Ancient gcc compiler.

#if defined __GNUC__
#   define LMI_GCC_VERSION \
        (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif // defined __GNUC__

#if defined __COMO__
#   define LMI_COMO_VERSION __COMO_VERSION__
#   define LMI_EDG_VERSION  __EDG_VERSION__
//  09 Oct 2004 17:04:46 -0700 email from <comeau@comeaucomputing.com>
//  suggests this method to detect whether MinGW is the underlying C
//  compiler. Because it requires including some standard header
//  (including <_mingw.h> could be an error), this section has to
//  follow inclusion of "platform_dependent.hpp" above. Comeau
//  suggested <stdio.h>, but <setjmp.h> is used here because it's the
//  standard header most unlikely to be used in a C++ program. Still,
//  it's a good idea to test not only __MINGW32_VERSION but also
//  __MINGW_H, in case <setjmp.h> was already included above through
//  "platform_dependent.hpp".
//
#   include <setjmp.h>
#   if defined __MINGW32_VERSION || defined __MINGW_H
#       define LMI_COMO_WITH_MINGW
#   endif // defined __MINGW32_VERSION || defined __MINGW_H
#endif // defined __COMO__

#if defined __MINGW32__
//  Get definition of __MINGW32_VERSION.
#   include <_mingw.h>
#endif // defined __MINGW32__

// lmi used the mingw.org toolchain through gcc-3.4.5, and switched
// to MinGW-w64's gcc-4.9.2 circa 2016-01-01. To distinguish one from
// the other, see:
//   http://comments.gmane.org/gmane.comp.gnu.mingw.w64.general/641
//   https://sourceforge.net/p/mingw-w64/mailman/message/28485906/
#if defined __MINGW64_VERSION_MAJOR
#   define LMI_MINGW_W64
#endif // defined __MINGW64_VERSION_MAJOR

// This is RTL version, not gcc version.
#if defined __MINGW32_VERSION || defined __MINGW64_VERSION_MAJOR
#   define LMI_MINGW_VERSION \
        (__MINGW32_MAJOR_VERSION * 100 + __MINGW32_MINOR_VERSION)
#endif // defined __MINGW32_VERSION || defined __MINGW64_VERSION_MAJOR

#if defined __BORLANDC__ && __BORLANDC__ < 0x0550
#    error Obsolete compiler not supported.
#endif // Ancient borland compiler.

// Give this toolchain its own lmi-specific macro. Rationale:
//   http://boost.cvs.sf.net/boost/boost/boost/config.hpp?annotate=1.1
//   Many other "compilers define _MSC_VER. Thus BOOST_MSVC. [...]
//   Must remain the last #elif since some other vendors (Metrowerks,
//   for example) also #define _MSC_VER"

#if defined _MSC_VER && !defined LMI_GCC_VERSION && !defined LMI_COMO_WITH_MINGW
#    define LMI_MSC
#endif // defined _MSC_VER && !defined LMI_GCC_VERSION && !defined LMI_COMO_WITH_MINGW

#if defined LMI_COMO_WITH_MINGW || defined LMI_MSC || defined __MINGW32__ && defined LMI_GCC_VERSION && LMI_GCC_VERSION < 30405
#   define LMI_MSVCRT
#endif // Compilers that use the msvc C runtime, without corrections such as libmingwex.

#if defined HAVE_CONFIG_H // Using autoconf.
#   include "config.h"
#else // Not using autoconf.

// Redundant include guards are passé: with modern tools, the benefit
// is not worth the ugliness. The guards here, however, are intended
// not to speed up compilation with antique tools that don't recognize
// normal include guards, but rather to insulate compiler-specific
// workarounds from each other. For instance, a change in the gcc
// configuration file cannot affect anything compiled with another
// toolset. See Vesa Karvonen's 2001-05-26 post to the boost mailing
// list.
//
// As a further precaution, these special headers have reverse include
// guards: if the appropriate guard is not defined, then including the
// header causes an error. We take care to undefine each reverse
// include guard immediately after using it.

#   if defined __MINGW32__ && defined __GNUC__ && 30203 <= LMI_GCC_VERSION
#       define OK_TO_INCLUDE_CONFIG_MING323_HPP
#       include "config_ming323.hpp"
#       undef  OK_TO_INCLUDE_CONFIG_MING323_HPP
#   endif // MinGW gcc 3.2.3+ .

#   if defined LMI_COMO_WITH_MINGW
#       define OK_TO_INCLUDE_CONFIG_COMO_WITH_MINGW_HPP
#       include "config_como_mingw.hpp"
#       undef  OK_TO_INCLUDE_CONFIG_COMO_WITH_MINGW_HPP
#   endif // Como with MinGW as the underlying C compiler.

#   if defined __BORLANDC__ && 0x0550 <= __BORLANDC__
#       define OK_TO_INCLUDE_CONFIG_BC551_HPP
#       include "config_bc551.hpp"
#       undef  OK_TO_INCLUDE_CONFIG_BC551_HPP
#   endif // Borland 5.5.1+ .

#endif // Not using autoconf.

#endif // config_hpp
