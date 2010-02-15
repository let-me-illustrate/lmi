// Quiet NaN on IEC559-conforming implementations; otherwise, an
// implausible value, optionally specified.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// $Id$

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

/// It is sometimes profitable to initialize a floating-point variable
/// to a recognizably implausible value. A quiet NaN is generally the
/// best such value.
///
/// For non-conforming implementations, an 'implausible' value may be
/// specified if desired, although there's no guarantee that it won't
/// arise in practice. If none is specified, then we choose one with
/// FLT_DIG decimal digits and an exponent a bit under FLT_MAX_10_EXP,
/// using the minimum values of those macros in C99 5.2.4.2.2/8. The
/// same behavior is used for borland tools, which claim to support
/// qNaNs but do not:
///   http://lists.boost.org/MailArchives/boost/msg12131.php
///   there's no borland option to set the floating-point hardware to
///   allow quiet NaNs to work without raising an exception.
/// Without this workaround, bc++5.5.1 would produce a BSOD on msw xp.

template<typename T>
T implausible_value(T const& t = -9.99999e35)
{
    BOOST_STATIC_ASSERT(::boost::is_float<T>::value);

#if defined __BORLANDC__
    return t;
#else  // !defined __BORLANDC__

    if(std::numeric_limits<T>::has_quiet_NaN)
        {
        return std::numeric_limits<T>::quiet_NaN();
        }
    else
        {
        return t;
        }
#endif // !defined __BORLANDC__
}

#endif // quiet_nan_hpp

