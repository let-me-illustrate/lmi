// Quiet NaN on IEC559-conforming implementations; otherwise, an
// implausible value, optionally specified.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: quiet_nan.hpp,v 1.9 2008-12-27 02:56:53 chicares Exp $

#ifndef quiet_nan_hpp
#define quiet_nan_hpp

#include "config.hpp"

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits/arithmetic_traits.hpp>
#else  // Defined __BORLANDC__ .
#   define BOOST_STATIC_ASSERT(deliberately_ignored) class IgNoRe
#endif // Defined __BORLANDC__ .

#include <limits>

// For non-conforming implementations, an 'implausible' value may be
// specified if desired, although there's no guarantee that it won't
// arise in practice. If none is specified, then we choose one with
// FLT_DIG decimal digits and an exponent a bit under FLT_MAX_10_EXP,
// using the minimum values of those macros in C99 5.2.4.2.2/8.

// TODO ?? Consider:
// http://lists.boost.org/MailArchives/boost/msg12131.php

template<typename T>
T implausible_value(T const& t = -9.99999e35)
{
    BOOST_STATIC_ASSERT(::boost::is_float<T>::value);

#if defined __BORLANDC__
    // Without this 'workaround', bc++5.5.1 gives a BSOD on msw xp.
    return t;
#else // __BORLANDC__

    if(std::numeric_limits<T>::has_quiet_NaN)
        {
        return std::numeric_limits<T>::quiet_NaN();
        }
    else
        {
        return t;
        }
#endif // Not newer borland compiler.
}

#endif // quiet_nan_hpp

