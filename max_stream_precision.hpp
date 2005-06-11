// Precision for lossless floating-point I/O.
//
// Copyright (C) 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: max_stream_precision.hpp,v 1.2 2005-06-11 15:04:25 chicares Exp $

#ifndef max_stream_precision_hpp
#define max_stream_precision_hpp

#include "config.hpp"

#include <cmath>
#include <limits>

/// Return the stream precision necessary to map any base-ten
/// scientific-notation representation back to binary without
/// loss of accuracy.
///
/// Return a value of type int, rather than type std::streamsize .
/// Rationale: this value is intended to be used with function
///   std::ios_base::precision(std::streamsize)
/// and manipulator
///   std::setprecision(int)
/// and its value can't plausibly exceed the range of an int, so it is
/// preferable to avoid a narrowing conversion in the first use case.
///
/// Reference: c99 5.2.4.2.2/8 (DECIMAL_DIG)
/// ...but that's used here because it's not in c++98 .
///
/// INELEGANT !! Would it be better to make the type a template
/// argument, and pass the stream as an argument so that
/// we can make sure 'scientific' is set? How about making
/// this a manipulator?

inline int max_stream_precision()
{
    static const int nbits = std::numeric_limits<long double>::digits;
    static const double precision =
            1.0
        +   std::ceil(std::log10(std::pow(2.0, nbits)))
        ;
    return static_cast<int>(precision);
}

#endif // max_stream_precision_hpp

