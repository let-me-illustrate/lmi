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

#include <functional>                   // multiplies, plus
#include <stdexcept>                    // logic_error

namespace nonstd
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

/// Returns x ** n, where 0 <= n.
///
/// Note that "multiplication" is required to be associative, but not
/// necessarily commutative.
///
/// GWC modification: throw on negative exponent--otherwise, the loop
/// appears not to terminate.

template <typename T, typename Integer, typename MonoidOperation>
T power(T x, Integer n, MonoidOperation opr)
{
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

template <typename T, typename Integer>
inline T power(T x, Integer n)
{
    return power(x, n, std::multiplies<T>());
}
} // namespace nonstd

#endif // stl_extensions_hpp
