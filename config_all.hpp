// Configuration.
//
// Copyright (C) 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: config_all.hpp,v 1.4 2005-05-26 22:01:15 chicares Exp $

// Configuration header for compiler quirks--generic configuration.
// Never include this file directly.

#ifndef config_all_hpp
#define config_all_hpp

#ifndef OK_TO_INCLUDE_CONFIG_ALL_HPP
#   error This file is not intended for separate inclusion.
#endif // OK_TO_INCLUDE_CONFIG_ALL_HPP

#if defined __BORLANDC__ && __BORLANDC__ < 0x0550
#   error Obsolete compiler not supported.
#endif // Old borland compiler.

#if defined __COMO__
//  09 Oct 2004 17:04:46 -0700 email from <comeau@comeaucomputing.com>
//  suggests this method to detect whether MinGW is the underlying C
//  compiler.
#   include <stdio.h>
#   if defined __MINGW32_VERSION
#       define LMI_COMO_WITH_MINGW
#   endif // __MINGW32_VERSION defined.
#endif // Como.

#if defined __GNUC__ && __GNUC__ < 3
#   error Obsolete compiler not supported.
#endif // Old gcc compiler.

// TODO ?? Untested with como using gcc backend.
#if defined __GNUC__ || defined LMI_COMO_WITH_MINGW
#   define LMI_PACKED_ATTRIBUTE __attribute__ ((packed))
#elif defined __BORLANDC__
#   define LMI_PACKED_ATTRIBUTE
#else // Neither gcc nor borland.
#   error Unknown compiler
#endif // Neither gcc, nor como with mingw, nor borland.

#if defined __MINGW32__ && defined __GNUC__ && __GNUC__ == 3 && 4 <= __GNUC_MINOR__
#   define LMI_COMPILER_HAS_EXPM1
#   define LMI_COMPILER_HAS_LOG1P
#endif // Recent MinGW.

#if defined __BORLANDC__ || defined __GNUC__
#   define LMI_MODERN_STREAMS
#endif

#endif // config_all_hpp

