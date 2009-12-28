// Configuration for Microsoft Visual C++ compiler.
//
// Copyright (C) 2001, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: config_bc551.hpp,v 1.6 2008/01/01 18:29:36 chicares Exp $

// Configuration header for compiler quirks -- Microsoft Visual C++

#ifndef config_msvc_hpp
#define config_msvc_hpp

#ifndef OK_TO_INCLUDE_CONFIG_MSVC_HPP
#   error This file is not intended for separate inclusion.
#endif // OK_TO_INCLUDE_CONFIG_MSVC_HPP

#define LMI_COMPILER_USES_PCH

// MSVC standard library defines many POSIX symbols but with additional
// underscores to indicate that they are not ANSI C90
#define snprintf _snprintf
#define access _access

// MSVC 7/8 CRT #define's some standard functions as macros even in <cstdio>
// while this is explicitly forbidden by the C++ 1998 standard in 17.4.1.2/6,
// in particular the footnote 159 explicitly states
//
//          This disallows the practice, allowed in C, of providing a
//          "masking macro" in addition to the function prototype.
//
// so we need to remove these masking macros to use the real function as
// otherwise expressions such as "std::ferror(f)" wouldn't compile
#if defined(__cplusplus) && _MSC_VER < 1600 // 1600 is VC9 a.k.a. MSVC2008
    #include <cstdio>

    #undef clearerr
    #undef feof
    #undef ferror
    #undef getc
    #undef getchar
    #undef putc
    #undef putchar
#endif

// MSVC does not define these constants from fcntl.h however, amazingly enough,
// uses the same values as Unix systems do with its _access() function (except
// that it doesn't support X_OK as files have no "executable" attribute under
// Windows, so don't define this one)
#define F_OK 0
#define W_OK 2
#define R_OK 4

// Disable several warnings which happen too many times (> 3000 in all) in LMI
// code currently. This is, of course, not the right thing to do and most if
// not all of these warnings should be reenabled after fixing the code.
#pragma warning(disable: 4100 4127 4244 4267 4311 4312 4511 4512 4702 4800)

#endif // config_msvc_hpp

