// Quiet NaN on IEC559-conforming implementations; otherwise, an
// implausible value, optionally specified.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: quiet_nan.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef quiet_nan_hpp
#define quiet_nan_hpp

#ifndef BC_BEFORE_5_5
// TODO ?? expunge #   include <boost/config.hpp>
#   include <boost/static_assert.hpp>
#   include <boost/type_traits/arithmetic_traits.hpp>
#endif // old borland compiler

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
#ifndef BC_BEFORE_5_5
    BOOST_STATIC_ASSERT(::boost::is_float<T>::value);
#endif // Old borland compiler.

#if defined __BORLANDC__ && !defined BC_BEFORE_5_5
    // Without this workaround, bc++5.5.1 gives a BSOD on msw xp.
    return t;
#else // Not newer borland compiler.
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

