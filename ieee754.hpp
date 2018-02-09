// IEEE 754 esoterica.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#ifndef ieee754_hpp
#define ieee754_hpp

#include "config.hpp"

#include <limits>
#include <type_traits>

/// Quiet NaN if available; else a slightly less implausible value.
///
/// It is sometimes profitable to initialize a floating-point variable
/// to a recognizably implausible value. A quiet NaN is generally the
/// best such value.
///
/// For non-conforming implementations, an 'implausible' value may be
/// specified if desired, although there's no guarantee that it won't
/// arise in practice. If none is specified, then we choose one with
/// FLT_DIG decimal digits and an exponent a bit under FLT_MAX_10_EXP,
/// using the minimum values of those macros in C99 5.2.4.2.2/8.

template<typename T>
T implausible_value(T const& t = -9.99999e35)
{
    static_assert(::std::is_floating_point<T>::value);

    if(std::numeric_limits<T>::has_quiet_NaN)
        {
        return std::numeric_limits<T>::quiet_NaN();
        }
    else
        {
        return t;
        }
}

/// Return positive infinity.
///
/// Preconditions: T is a floating-point type that has an infinity.
///
/// Throws if any precondition is violated.
///
/// Rationale: std::numeric_limits<T>::infinity() silently returns
/// zero if T has no infinity [18.2.1.5/1]; this function throws in
/// that case to prevent surprises.
///
/// Static variables are volatile if initializing them might trigger a
/// hardware exception.

template<typename T>
inline T infinity()
{
    static_assert(::std::is_floating_point<T>::value);
    static_assert(std::numeric_limits<T>::has_infinity);
    static T const volatile z = std::numeric_limits<T>::infinity();
    return z;
}

/// Ascertain whether argument is infinite.
///
/// Preconditions: T is a floating-point type. (It need not have an
/// infinity.)
///
/// Throws if any precondition is violated.
///
/// Eventually, the C++ standard library will provide std::isinf<T>(),
/// which might replace this implementation when compilers support it.
///
/// Static variables are volatile if initializing them might trigger a
/// hardware exception.
///
/// The present implementation compares the argument to positive and
/// negative infinity, if infinity is available: it doesn't seem too
/// outrageous to presume that infinity is negatable and that its
/// positive and negative representations are unique. Alternatively,
/// these conditions might be tested:
///               std::numeric_limits<T>::max() < argument
///   argument < -std::numeric_limits<T>::max()
/// but it doesn't seem any safer to assume that would work better on
/// a machine that doesn't conform to IEEE 754.

template<typename T>
inline bool is_infinite(T t)
{
    static_assert(::std::is_floating_point<T>::value);
    static T const volatile pos_inf =  std::numeric_limits<T>::infinity();
    static T const volatile neg_inf = -std::numeric_limits<T>::infinity();
    static bool const has_inf = std::numeric_limits<T>::has_infinity;
    return has_inf && (pos_inf == t || neg_inf == t);
}

/// Floating-point numbers that represent integers scaled by negative
/// powers of ten are inexact. For example, a premium rate of $2.40
/// per $1000 is notionally 0.0024, but to the hardware may look like:
///   0.0023999999999999998 [0x3ff69d495182a9930800]
/// Multiplying that number by a million dollars and rounding down to
/// cents yields 2399.99, where 2400.00 is wanted.
///
/// SOMEDAY !! The best way to handle this is to store integers. For
/// the time being, multiplying by 1 + LDBL_EPSILON in problematic
/// circumstances avoids this embarrassment while introducing an error
/// that shouldn't matter.

inline long double ldbl_eps_plus_one()
{
    static long double const z = 1.0L + std::numeric_limits<long double>::epsilon();
    return z;
}

#endif // ieee754_hpp

