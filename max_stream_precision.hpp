// Set ostream for lossless floating-point I/O.
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

// $Id: max_stream_precision.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef max_stream_precision_hpp
#define max_stream_precision_hpp

#include "config.hpp"

#include <cmath>
#include <ios>
#include <limits>

// Return the stream precision necessary to map any base-ten
// scientific-notation representation back to binary without
// loss of accuracy.
//
// Reference: c99 5.2.4.2.2/8 (DECIMAL_DIG)
// TODO ?? Why not just use that?
//
// TODO ?? Would it be better to make the type a template
// argument, and pass the stream as an argument so that
// we can make sure 'scientific' is set? How about making
// this a manipulator?
//
inline std::streamsize max_stream_precision()
{
    int nbits = std::numeric_limits<long double>::digits;
    double prec = 1 + std::ceil(std::log10(std::pow(2.0, nbits)));
    return static_cast<std::streamsize>(prec);
}

#endif // max_stream_precision_hpp

