// Precompiled header file for use with wx.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// In every '.cpp' file that is expected to benefit from precompiled
// headers, include exactly one of these PCH headers:
//   pchfile.hpp    // For TUs included in a non-wx-dependent library
//   pchfile_wx.hpp // For TUs included in a     wx-dependent library
// Include it before anything else except comments and whitespace.
// Never include any PCH header in any other file.

// Rationale for reverse include guards: see 'config*.hpp'.

#ifndef pchfile_wx_hpp
#define pchfile_wx_hpp

// Even if precompiled headers are not really being used, use this header to
// disable some warnings which are enabled for the rest of lmi code but have to
// be disabled for the code using wxWidgets as they occur in wxWidgets headers.
#if defined __GNUC__ && !defined __clang__
//#   pragma GCC diagnostic ignored "-Wcast-qual"
//#   pragma GCC diagnostic ignored "-Wdouble-promotion"
//#   pragma GCC diagnostic ignored "-Wextra-semi"
//#   pragma GCC diagnostic ignored "-Wformat-nonliteral"
//#   pragma GCC diagnostic ignored "-Wnoexcept"
//#   pragma GCC diagnostic ignored "-Wold-style-cast"
//#   pragma GCC diagnostic ignored "-Wsuggest-override"
//#   pragma GCC diagnostic ignored "-Wuseless-cast"
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif // defined __GNUC__ && !defined __clang__

#if defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH

#   define LMI_OKAY_TO_INCLUDE_PCHLIST_HPP
#   include "pchlist.hpp"
#   undef  LMI_OKAY_TO_INCLUDE_PCHLIST_HPP

#   define LMI_OKAY_TO_INCLUDE_PCHLIST_WX_HPP
#   include "pchlist_wx.hpp"
#   undef  LMI_OKAY_TO_INCLUDE_PCHLIST_WX_HPP

#endif // defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH

#endif // pchfile_wx_hpp
