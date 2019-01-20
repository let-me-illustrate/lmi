// Include boost/regex.hpp, guarding it with a pragma.
//
// Copyright (C) 2017, 2018, 2019 Gregory W. Chicares.
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

#ifndef boost_regex_hpp
#define boost_regex_hpp

#include "config.hpp"

#if defined __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined __GNUC__
#   pragma GCC diagnostic push
#   if 7 <= __GNUC__
#       pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#       pragma GCC diagnostic ignored "-Wregister"
#       pragma GCC diagnostic ignored "-Wuseless-cast"
#   endif // 7 <= __GNUC__
#   pragma GCC diagnostic ignored "-Wshadow"
#endif // defined __GNUC__
#if defined _MSC_VER
#   pragma warning(push, 1)
#endif
#include <boost/regex.hpp>
#if defined __clang__
#   pragma clang diagnostic pop
#elif defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
#if defined _MSC_VER
#   pragma warning(pop)
#endif

#endif // boost_regex_hpp

