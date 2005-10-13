// Test material equality of floating-point values.
//
// Copyright (C) 1998, 2001, 2002, 2005 Gregory W. Chicares.
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

// $Id: materially_equal.hpp,v 1.3 2005-10-13 01:05:26 chicares Exp $

#ifndef materially_equal_hpp
#define materially_equal_hpp

#include "config.hpp"

#include <algorithm>
#include <cmath>

/// Determine whether two floating-point values are materially equal.
/// Deem that to be the case if either:
///   they're actually equal; or
///   the absolute value of their difference is less than a specified
///     tolerance times the lesser in absolute value.
/// The default tolerance is 1.0E-13 . I believe this is the
/// algorithm used by the APL programming language.
///
/// See the discussion in this usenet article:
/// <41b09b41$2@newsgroups.borland.com>

inline bool materially_equal
    (long double t
    ,long double u
    ,long double tolerance = 1.0E-13L
    )
{
    return
            t == u
        ||  std::fabs(t - u) <= tolerance * std::min(std::fabs(t), std::fabs(u))
        ;
}

/// Difference between two values if materially unequal, else zero.
/// The motivation is avoidance of certain catastrophic cancellation
/// problems. For example, net claims might be defined as gross
/// claims minus account value released on death; however, the two
/// quantities subtracted might be notionally identical yet trivially
/// different in fact because of intentionally different rounding; but
/// any difference arising out of rounding is a nuisance at best, and
/// at worst might have the wrong algebraic sign, which most experts
/// would consider incorrect prima facie. Even worse, one experience-
/// rating formula uses net claims in the denominator of a fraction
/// that can assume an implausibly enormous value unless differences
/// that are mere rounding artifacts are not actively suppressed.

inline long double material_difference
    (long double t
    ,long double u
    ,long double tolerance = 1.0E-13L
    )
{
    return materially_equal(t, u, tolerance) ? 0.0 : t - u;
}

#endif // materially_equal_hpp

