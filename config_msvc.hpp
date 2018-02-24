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

#if _MSC_VER < 1900
#   error At least MSVS 2017 is required.
#endif

#define LMI_COMPILER_USES_PCH

// MSVC standard library defines many POSIX symbols but with additional
// underscores to indicate that they are not ANSI C90
#define snprintf _snprintf
#define access _access

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
#pragma warning(disable: 4100 4127 4244 4267 4311 4312 4407 4511 4512 4702 4800)

#endif // config_msvc_hpp

