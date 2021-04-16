// Include boost/regex.hpp, guarding it with a pragma.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef boost_regex_hpp
#define boost_regex_hpp

#include "config.hpp"

#if defined LMI_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wchar-subscripts"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   pragma clang diagnostic ignored "-Wdeprecated-copy"
#   pragma clang diagnostic ignored "-Wkeyword-macro"
#   pragma clang diagnostic ignored "-Wparentheses-equality"
#   pragma clang diagnostic ignored "-Wregister"
#elif defined LMI_GCC
#   pragma GCC diagnostic push
#   if 7 <= __GNUC__
#       pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#       pragma GCC diagnostic ignored "-Wregister"
#   endif // 7 <= __GNUC__
#   if 9 <= __GNUC__
#       pragma GCC diagnostic ignored "-Wdeprecated-copy"
#   endif // 9 <= __GNUC__
#   pragma GCC diagnostic ignored "-Wshadow"
#   pragma GCC diagnostic ignored "-Wswitch-enum"
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#endif // defined LMI_GCC
#include <boost/regex.hpp>
#if defined LMI_CLANG
#   pragma clang diagnostic pop
#elif defined LMI_GCC
#   pragma GCC diagnostic pop
#endif // defined LMI_GCC

#endif // boost_regex_hpp
