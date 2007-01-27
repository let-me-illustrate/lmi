// Configuration for MinGW gcc-3.2.3 .
//
// Copyright (C) 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: config_ming323.hpp,v 1.7 2007-01-27 00:00:51 wboutin Exp $

// Configuration header for compiler quirks--mingw gcc-3.2.3 .

#ifndef config_ming323_hpp
#define config_ming323_hpp

#ifndef OK_TO_INCLUDE_CONFIG_MING323_HPP
#   error This file is not intended for separate inclusion.
#endif // OK_TO_INCLUDE_CONFIG_MING323_HPP

#if 30203 <= LMI_GCC_VERSION
#   // Copacetic.
#else  // Not MinGW gcc-3.2.3+ .
#   error Use this file only for MinGW gcc version 3.2.3 or higher.
#endif // Not MinGW gcc-3.2.3+ .

// Assume C++98 conformance by default. Define macros to indicate
// departures from C++98, whether positive (such as providing C99
// functions) or negative.

// Version numbers are in 'include/_mingw.h' here:
//   http://cygwin.com/cgi-bin/cvsweb.cgi/src/winsup/mingw/?cvsroot=src

#if 308 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_EXPM1
#endif // 308 <= LMI_MINGW_VERSION

#if 202 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_LOG1P
#endif // 202 <= LMI_MINGW_VERSION

#if 200 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_RINT
#endif // 200 <= LMI_MINGW_VERSION

#if 204 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_STRTOLD
#endif // 204 <= LMI_MINGW_VERSION

#endif // config_ming323_hpp

