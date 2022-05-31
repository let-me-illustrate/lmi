// SGI extensions to STL.
//
// Copyright (C) 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// Copyright (C) 1994
// Hewlett-Packard Company
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  Hewlett-Packard Company makes no
// representations about the suitability of this software for any
// purpose.  It is provided "as is" without express or implied warranty.
//
// Copyright (C) 1996-1998
// Silicon Graphics Computer Systems, Inc.
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  Silicon Graphics makes no
// representations about the suitability of this software for any
// purpose.  It is provided "as is" without express or implied warranty.

// This implementation is a derived work based on code SGI released
// with the above disclaimers.
//
// Gregory W. Chicares modified it trivially in 2002 and 2005, and in
// later years as indicated in 'ChangeLog' or in `git log`. Any defect
// in it should not reflect on SGI's or HP's reputation.

#ifndef stl_extensions_hpp
#define stl_extensions_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "bin_exp.hpp"
#include "bourn_cast.hpp"
#include "miscellany.hpp"               // scoped_ios_format

#include <functional>                   // multiplies, plus
#include <iostream>
#include <stdexcept>                    // logic_error
#include <type_traits>                  // is_integral_v

namespace hidden
{
/// Identity element.

template <typename T> inline T identity_element(std::plus<T>)
{
    return T(0);
}

template <typename T> inline T identity_element(std::multiplies<T>)
{
    return T(1);
}

/// Returns x ** n, where 0 <= n, by right-to-left binary exponentiation.
///
/// See Knuth, TAOCP volume 2, section 4.6.3 (p. 442 in 2nd ed.).
///
/// Note that "multiplication" is required to be associative, but not
/// necessarily commutative.
///
/// GWC modification: throw on negative exponent--otherwise, the loop
/// may never terminate, because the bitwise operators don't work as
/// intended with negative values. Alternative not used: assert that
/// type Integer is unsigned, as the author evidently assumed--but
/// imposing that requirement now breaks too much existing lmi code.

template <typename T, typename Integer, typename MonoidOperation>
T power(T x, Integer n, MonoidOperation opr)
{
    static_assert(std::is_integral_v<Integer>);
    if(n < 0)
        {
        throw std::logic_error("power() called with negative exponent.");
        }
    if(n == 0)
        {
        return identity_element(opr);
        }
    else
        {
        while ((n & 1) == 0)
            {
            n >>= 1;
            x = opr(x, x);
            }
        T result = x;
        n >>= 1;
        while (n != 0)
            {
            x = opr(x, x);
            if((n & 1) != 0)
                result = opr(result, x);
            n >>= 1;
            }
        return result;
        }
}
} // namespace hidden

namespace nonstd
{
template <typename T, typename Integer>
inline T power(T x, Integer n)
{
    using F = std::conditional_t<std::is_floating_point_v<T>, T, double>;

    T const r = hidden::power(x, n, std::multiplies<T>());
    F const a = bourn_cast<F>(r);
    F const b = bin_exp(bourn_cast<F>(x), bourn_cast<int>(n));
    if(a != b)
        {
        scoped_ios_format meaningless_name(std::cout);
        std::cout.precision(32);
        std::cout
            << "power <> bin_exp:\n"
            << x << " x\n"
            << n << " n\n"
            << a << " a\n"
            << b << " b\n"
            << a - b << " a - b\n"
            << (a - b) / a << " (a - b) / a\n"
            << std::endl
            ;
        }
    LMI_ASSERT(a == b);
    return r;
}
} // namespace nonstd

#endif // stl_extensions_hpp
