// Configuration for MinGW gcc-3.2.3 .
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

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

#if 200 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_ISNAN
#endif // 200 <= LMI_MINGW_VERSION

#if 200 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_RINT
#endif // 200 <= LMI_MINGW_VERSION

#if 200 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_STRTOF
#endif // 200 <= LMI_MINGW_VERSION

#if 204 <= LMI_MINGW_VERSION
#   define LMI_COMPILER_PROVIDES_STRTOLD
#endif // 204 <= LMI_MINGW_VERSION

// Configure Boost library for this compiler in cases where it lacks
// features autodetection.

#if defined BOOST_MATH_EXPM1_INCLUDED || defined BOOST_MATH_LOG1P_INCLUDED
#   error config.hpp must be included before boost::math headers.
#endif

#if 308 <= LMI_MINGW_VERSION
#   define BOOST_HAS_EXP1M
#endif // 308 <= LMI_MINGW_VERSION

#if 202 <= LMI_MINGW_VERSION
#   define BOOST_HAS_LOG1P
#endif // 202 <= LMI_MINGW_VERSION

#ifndef BOOST_MATH_USE_C99
#   define BOOST_MATH_USE_C99
#endif

#endif // config_ming323_hpp

