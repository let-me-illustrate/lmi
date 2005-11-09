// Configuration.
//
// Copyright (C) 2001, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: config.hpp,v 1.12 2005-11-09 05:00:33 chicares Exp $

// Configuration header for compiler quirks. Include at the beginning of
// every .hpp file (and nowhere else).

#ifndef config_hpp
#define config_hpp

#ifdef __cplusplus
// Namespace alii.
namespace boost {namespace filesystem {} }
namespace fs = boost::filesystem;
#endif // Not C++.

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
// no direct way to detect POSIX, though, so I look for common macros
// that regrettably use four characters that could be taken as naming
// name a non-free operating system.

#if defined unix || defined __unix__ || defined __unix // Detected POSIX.
#   define LMI_POSIX
#elif defined __WIN32__ || defined _WIN32 || defined WIN32 // Detected msw.
#   define LMI_MSW
#else  // Unknown OS.
#   error "Unknown operating system. Consider contributing support."
#endif // Unknown OS.

#if defined _X86_ || defined _M_IX86 || defined i386 || defined __i386
#   define LMI_X86
#else  // Unknown hardware.
#   error "Unknown hardware. Consider contributing support."
#endif // Unknown hardware.

// This header #includes standard headers in an unusual way, and must
// be #included before any standard headers are seen.
//
#include "platform_dependent.hpp"

#if defined __GNUC__ && __GNUC__ < 3
#   error Obsolete compiler not supported.
#endif // Ancient gcc compiler.

#if defined __GNUC__
#   define LMI_GCC_VERSION \
        (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif // __GNUC__

#if defined __COMO__
//  09 Oct 2004 17:04:46 -0700 email from <comeau@comeaucomputing.com>
//  suggests this method to detect whether MinGW is the underlying C
//  compiler. Because it requires including some standard header
//  (including <_mingw.h> could be an error), this section has to
//  follow inclusion of "platform_dependent.hpp" above.
#   include <stdio.h>
#   if defined __MINGW32_VERSION
#       define LMI_COMO_WITH_MINGW
#   endif // __MINGW32_VERSION
#endif // __COMO__

#if defined __MINGW32__
//  Get definition of __MINGW32_VERSION.
#   include <_mingw.h>
#endif // __MINGW32__

#if defined __MINGW32_VERSION
#   define LMI_MINGW_VERSION \
        (__MINGW32_MAJOR_VERSION * 100 + __MINGW32_MINOR_VERSION)
#endif // __MINGW32_VERSION

#if defined __BORLANDC__ && __BORLANDC__ < 0x0550
#    error Obsolete compiler not supported.
#endif // Ancient borland compiler.

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

#   if defined __CYGWIN__ && defined __GNUC__ && 30203 <= LMI_GCC_VERSION
#       define OK_TO_INCLUDE_CONFIG_CYG323_HPP
#       include "config_cyg323.hpp"
#       undef  OK_TO_INCLUDE_CONFIG_CYG323_HPP
#   endif // Cygwin gcc 3.2.3+ .

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

