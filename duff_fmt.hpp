// Format NNNN.NNN --> "N,NNN.NN".
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// Ideally we would use imbue an appropriate locale and use some facet
// like money_put, but most of our compilers's standard libraries don't
// support that, so we use this US-specific workaround.

#ifndef duff_fmt_hpp
#define duff_fmt_hpp

#include "config.hpp"

#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>

// Reference:
// http://groups.google.com/groups?selm=38C9B681.B8A036DF%40flash.net
inline std::string ncnnnpnn(long double value)
{
    if(value < 0.0L)
        {
        return '-' + ncnnnpnn(-value);
        }

    std::stringstream stream;
    stream << std::setiosflags(std::ios_base::fixed) << std::setprecision(2);
    std::string s_in;

    if
        (  !(stream << value)
        || !(stream >> s_in)
        || !(stream >> std::ws).eof()
        )
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
                do
                    {
                    if('.' == *sin_it) break;
                    s_out += ',';       // fall through
        case 0:     s_out += *sin_it++; // fall through
        case 2:     s_out += *sin_it++; // fall through
        case 1:     s_out += *sin_it++;
                    } while(*sin_it);
        }

    s_out += s_in.at(    z); // decimal point
    s_out += s_in.at(1 + z); // tenths
    s_out += s_in.at(2 + z); // hundredths

    return s_out;
}

#endif // duff_fmt_hpp
