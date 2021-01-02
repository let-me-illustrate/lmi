// Test material equality of floating-point values.
//
// Copyright (C) 1998, 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
/// This is akin to the "essentially equal to" relation described in
/// Knuth, TAOCP, volume 2, section 4.2.2, equation (34), on page 219
/// of the second edition...but see:
///   https://lists.nongnu.org/archive/html/lmi/2018-02/msg00099.html
/// et seqq.
///
/// The default tolerance is 1.0E-13, a popular default value for the
/// APL language's 'comparison tolerance', and the only such value
/// permitted in the A+ language.
///
/// See the discussion here:
///   http://groups.google.com/groups?selm=41b09b41$2@newsgroups.borland.com
///
/// See also
///   http://groups.google.com/group/comp.lang.tcl/msg/3eee29ef533d73cd
/// which seems to differ from this implementation only in that it
/// always treats nonzero quantities of opposite sign as materially
/// different.

template<typename T>
inline bool materially_equal(T t, T u, long double tolerance = 1.0E-13L)
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

template<typename T>
inline T material_difference(T t, T u, long double tolerance = 1.0E-13L)
{
    return materially_equal(t, u, tolerance) ? 0.0 : t - u;
}

#endif // materially_equal_hpp
