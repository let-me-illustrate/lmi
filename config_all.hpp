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

// $Id: config_all.hpp,v 1.2 2005-04-05 12:36:37 chicares Exp $

// Configuration header for compiler quirks--generic configuration.
// Never include this file directly.

#ifndef config_all_hpp
#define config_all_hpp

#ifndef OK_TO_INCLUDE_CONFIG_ALL_HPP
#   error This file is not intended for separate inclusion.
#endif // OK_TO_INCLUDE_CONFIG_ALL_HPP

#if defined __BORLANDC__ && __BORLANDC__ < 0x0550 && !defined __COMO__
#   define BC_BEFORE_5_5
#endif // Old borland compiler.

#if defined __GNUC__ && __GNUC__ == 2 && __GNUC_MINOR__ == 95 && !defined __COMO__
#   define GCC_BEFORE_2_96
#endif // Old gcc compiler.

// TODO ?? Untested with como using gcc backend.
#if defined __GNUC__ || defined __COMO__
#   define LMI_PACKED_ATTRIBUTE __attribute__ ((packed))
#elif defined __BORLANDC__
#   define LMI_PACKED_ATTRIBUTE
#else // Neither gcc nor borland.
#   error Unknown compiler
#endif // Neither gcc nor borland.

#if defined __MINGW32__ && defined __GNUC__ && __GNUC__ == 3 && 4 <= __GNUC_MINOR__ && !defined __COMO__
#   define LMI_COMPILER_HAS_LOG1P
#endif // Recent MinGW.

#if defined BC_BEFORE_5_5 || defined GCC_BEFORE_2_96 && !defined __COMO__
#   define LMI_LACK_BASIC_IOSTREAMS
#endif // Old gcc or borland compiler, or unknown compiler.

#if defined BC_BEFORE_5_5 || defined GCC_BEFORE_2_96 && !defined __COMO__
#   define LMI_SSTREAM_LACKS_POSITIONING
#endif // Old gcc or borland compiler, or unknown compiler.

#if defined BC_BEFORE_5_5
// No std::iterator template.
#   define LMI_LACKS_STD_ITERATOR
#endif // Old borland compiler.

#if !defined BC_BEFORE_5_5
#   define LMI_SPECIALIZATION template<>
#else // Old borland compiler.
#   define LMI_SPECIALIZATION
#endif // Old borland compiler.

#endif // config_all_hpp

