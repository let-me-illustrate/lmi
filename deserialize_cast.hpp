// Safe replacement for reinterpret_cast<POD_type>(char*).
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef deserialize_cast_hpp
#define deserialize_cast_hpp

#include "config.hpp"

#include <cstring>                      // memcpy()

/// Reinterpret char* contents as a value of the given type.
///
/// Motivation: to replace reinterpret_cast with a safe alternative
/// that is consistent with C++'s strict aliasing requirements, and
/// whose behavior is therefore always well defined; and which,
/// incidentally, doesn't elicit a type-punning diagnostic.
///
/// The memcpy() call is completely optimized away by all major
/// compilers (gcc, clang, msvc), so both
///   t = deserialize_cast<T>(z)
/// and
///   t = *reinterpret_cast<T*>(z)
/// generate equally efficient code.
///
/// See the thread culminating in this message:
///   https://lists.nongnu.org/archive/html/lmi/2015-09/msg00010.html

template<typename T>
inline T deserialize_cast(char const* z)
{
    T t;
    std::memcpy(&t, z, sizeof(T));
    return t;
}

#endif // deserialize_cast_hpp
