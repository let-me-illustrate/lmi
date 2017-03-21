// Numeric stinted cast, across whose bourn no value is returned.
//
// Copyright (C) 2017 Gregory W. Chicares.
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

#ifndef bourn_cast_hpp
#define bourn_cast_hpp

#include "config.hpp"

#include <limits>
#include <stdexcept>

/// Numeric stinted cast, across whose bourn no value is returned.
///
/// Perform a static_cast between numeric types, but throw if the
/// value is out of range. For example:
///   bourn_cast<unsigned int>( 1); // Returns 1U.
///   bourn_cast<unsigned int>(-1); // Throws.
///
/// Motivation: To convert between integral types that may differ in
/// size and signedness, iff the value is between the maximum and
/// minimum values permitted for the target (From) type. Because of
/// the properties of integers, conversion between integral types
/// either preserves the notional value, or throws.
///
/// Both From and To must be types for which std::numeric_limits is
/// specialized. Use with floating-point types is neither forbidden
/// nor encouraged. Integral-to-floating conversion is highly unlikely
/// to exceed bounds, but may lose precision. Floating-to-integral
/// conversion is extremely unlikely to preserve value, so a rounding
/// facility is generally preferable. No special attention is given to
/// exotic values such as infinities, NaNs, or negative zero. For now,
/// bourn_cast<>() is intended as a simple replacement for the heavier
/// "improved" boost::numeric_cast<>(), but in the future floating-
/// point types may be forbidden.
///
/// This is a derived work based on Kevlin Henney's numeric_cast,
/// which is presented on his site without any copyright notice:
///   http://www.two-sdg.demon.co.uk/curbralan/code/numeric_cast/numeric_cast.hpp
/// and also as part of boost, with the following notice:
///   (C) Copyright Kevlin Henney and Dave Abrahams 1999.
///   Distributed under the Boost Software License, Version 1.0.
/// According to
///   http://www.gnu.org/philosophy/license-list.html
///   "This is a simple, permissive non-copyleft free software
///   license, compatible with the GNU GPL."
///
/// Rewritten by Gregory W. Chicares in 2017. Any defect here should
/// not reflect on Kevlin Henney's reputation.
///
/// Also see:
///   https://groups.google.com/forum/#!original/comp.std.c++/WHu6gUiwXkU/ZyV_ejRrXFYJ
/// which may be an independent redesign.

template<typename To, typename From>
#if 201402L < __cplusplus
constexpr
#endif // 201402L < __cplusplus
inline To bourn_cast(From from)
{
    using to_traits   = std::numeric_limits<To>;
    using from_traits = std::numeric_limits<From>;
    static_assert(  to_traits::is_specialized, "");
    static_assert(from_traits::is_specialized, "");

#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wsign-compare"
#   if 5 <= __GNUC__
#       pragma GCC diagnostic ignored "-Wbool-compare"
#   endif // 5 <= __GNUC__
#endif // defined __GNUC__
    if(! to_traits::is_signed && from < 0)
        throw std::runtime_error("Cast would convert negative to unsigned.");
    if(from_traits::is_signed && from < to_traits::lowest())
        throw std::runtime_error("Cast would transgress lower limit.");
    if(to_traits::max() < from)
        throw std::runtime_error("Cast would transgress upper limit.");
    return static_cast<To>(from);
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
}

#endif // bourn_cast_hpp

