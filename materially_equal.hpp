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

// $Id: materially_equal.hpp,v 1.2 2005-02-03 16:01:27 chicares Exp $

#ifndef materially_equal_hpp
#define materially_equal_hpp

#include "config.hpp"

#include <algorithm>
#include <cmath>

// See the discussion in this usenet article:
// <41b09b41$2@newsgroups.borland.com>

// Deem two floating-point values materially equal
//   if they're actually equal,
//   or if the absolute value of their difference is less than
//     a tolerance times the lower in absolute value.
// The default tolerance is 1.0E-13 . I believe this is the
// algorithm used by the APL programming language.

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

#endif // materially_equal_hpp

