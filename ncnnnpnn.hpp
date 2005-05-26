// Format NNNN.NNN --> "N,NNN.NN".
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ncnnnpnn.hpp,v 1.3 2005-05-26 18:31:49 chicares Exp $

// Ideally we would use imbue an appropriate locale and use some facet
// like money_put, but most of our compilers's standard libraries don't
// support that, so we use this US-specific workaround.

#ifndef ncnnnpnn_hpp
#define ncnnnpnn_hpp

#include "config.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <sstream>

// TODO ?? Move to config_all.hpp .
#if (defined __BORLANDC__) && (0x0550 <= __BORLANDC__)
#   define BC_SINCE_5_5
#endif // Newer borland compiler version 5.5 or greater.

// TODO ?? Move to config_all.hpp .
#if (defined BC_SINCE_5_5) || (defined __GNUC__) && (3 <= __GNUC__)
#   define LMI_MODERN_STREAMS
#endif // Newer borland compiler version 5.5 or greater.

// Reference:
// http://groups.google.com/groups?selm=38C9B681.B8A036DF%40flash.net
inline std::string ncnnnpnn(long double value)
{
    if(value < 0.0L)
        {
        return '-' + ncnnnpnn(-value);
        }

    std::stringstream stream;
    stream << std::setiosflags(std::ios_base::fixed);
    stream << std::setprecision(2);
    std::string s_in;

// TODO ?? Do something like this in stream_cast.hpp too.
#ifdef LMI_MODERN_STREAMS
    if
        (  !(stream << value)
        || !(stream >> s_in)
        || !(stream >> std::ws).eof()
        )
#else // not LMI_MODERN_STREAMS
    stream << value;
    stream >> s_in;
    if(!(stream >> std::ws).eof())
#endif // LMI_MODERN_STREAMS
        {
        throw std::runtime_error("Stream error in ncnnnpnn.hpp .");
        }

    std::string::const_iterator sin_it(s_in.begin());
    std::string s_out;

    std::string::size_type const z(s_in.find('.'));
    if(std::string::npos == z)
        {
        // Infinities and NaNs have no decimal point, and need no commas.
        return s_in;
        }

    switch(z % 3)
        {
#if defined __BORLANDC__ || defined __COMO__
        // COMPILER !! This 'case' is unreachable; its sole purpose is
        // to shut up compiler diagnostics that otherwise complain
        // that the code that immediately follows is unreachable.
        case 3:
#endif // borland compiler
                do
                    {
                    if('.' == *sin_it) break;
                    s_out += ',';
        case 0:     s_out += *sin_it++;
        case 2:     s_out += *sin_it++;
        case 1:     s_out += *sin_it++;
                    } while(*sin_it);
        }

    s_out += s_in.at(    z); // decimal point
    s_out += s_in.at(1 + z); // tenths
    s_out += s_in.at(2 + z); // hundredths

    return s_out;
}

#endif // ncnnnpnn_hpp

