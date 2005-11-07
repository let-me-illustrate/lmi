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

// $Id: config.hpp,v 1.11 2005-11-07 02:48:20 chicares Exp $

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

// INELEGANT !! Either the foregoing belongs in 'config_all.hpp',
// or 'config_all.hpp' should not exist.

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
#endif // Compiler is gcc.

#if defined HAVE_CONFIG_H // Using autoconf.
#   include "config.h"
#else // Not using autoconf.

#   define OK_TO_INCLUDE_CONFIG_ALL_HPP
#   include "config_all.hpp"
#   undef  OK_TO_INCLUDE_CONFIG_ALL_HPP

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

#   if defined __BORLANDC__ && __BORLANDC__ < 0x0550
#       error Obsolete compiler not supported.
#   endif // Ancient borland compiler.

#   if defined __BORLANDC__ && 0x0550 <= __BORLANDC__
#       define OK_TO_INCLUDE_CONFIG_BC551_HPP
#       include "config_bc551.hpp"
#       undef  OK_TO_INCLUDE_CONFIG_BC551_HPP
#   endif // Old borland compiler.

#endif // Not using autoconf.

// We include standard library headers this way:
//   #include <filename>
// using their standard names with no '.h' . While GNU tools look for
// 'filename', borland tools append an '.h' and look for 'filename.h' as
// permitted by 2.8/1, and indeed borland uses '.h' names for its standard
// library headers. For each standard header we use, we provide a forwarding
// header 'filename' with no '.h' so that GNU CPP can be used to generate
// autodependencies for command-line builds using 'make'.
//
// Unfortunately, bc++ never sees these forwarding headers, so we can't
// use them to patch this compiler's problems. What options do we have?
//
//  - Patch the borland headers. This may render the compiler unusable
//    for programs that use other workarounds. And other programs might
//    make different patches for the same reason. This is solipsistic.
//
//  - Override all standard headers by putting 'filename.h' in a directory
//    of our own that precedes the borland include directory on the search
//    path. But then there's no way to forward to the borland headers without
//    knowing where the borland headers reside, which may differ from one
//    installation to another. That could work well enough for makefiles,
//    but it's nasty with the borland IDE. (Yet it does work for gcc.)
//
//  - Write '#include <header.>' to convince borland tools that there really
//    is no '.h' . But we want our mainstream code to be standard.
//
//  - Include problematic headers in this file, which is included everywhere,
//    and put patches here.
//
// We choose the last option as the least evil, and mitigate the evil by
// including the problematic headers in 'pch.hpp' unless that creates
// additional problems (e.g. precompiling STL headers with bc++5.02 can
// produce internal compiler errors). The special borland headers use
// this approach.

#endif // config_hpp

