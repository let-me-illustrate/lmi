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

#include "assert_lmi.hpp"
#include "ssize_lmi.hpp"                // sstrlen()

#include <cstdio>                       // snprintf()
#include <cstring>                      // strchr()
#include <string>

/// Format a double using thousands separators. Reference:
///   https://groups.google.com/groups?selm=38C9B681.B8A036DF%40flash.net

inline std::string duff_fmt(double value, int decimals)
{
    if(value < 0.0)
        {
        return '-' + duff_fmt(-value, decimals);
        }

    constexpr int buffer_size {1000};
    char in_buf [buffer_size];
    char out_buf[buffer_size];

    char* p = in_buf;
    char* q = out_buf;

    // Use '#' to force a decimal point unless infinite or NaN.
    int const length = std::snprintf(p, buffer_size, "%#.*f", decimals, value);
    LMI_ASSERT(0 < length && length < buffer_size);
    LMI_ASSERT(lmi::sstrlen(p) == length);

    char const*const r = std::strchr(p, '.');
    if(nullptr == r)
        {
        // Infinities and NaNs need no commas.
        return out_buf;
        }

    switch((r - p) % 3)
        {
                do
                    {
                    if('.' == *p) break;
                    *q++ = ',';  // fall through
        case 0:     *q++ = *p++; // fall through
        case 2:     *q++ = *p++; // fall through
        case 1:     *q++ = *p++;
                    }
                while(*p);
        }

    // The next character must be the decimal point.
    LMI_ASSERT('.' == *p);
    // If the next character after the decimal point is the
    // terminating null, then zero decimals must be wanted,
    // and the decimal point should be suppressed.
    if('\0' == *(1 + p))
        {
        LMI_ASSERT(0 == decimals);
        }
    else
        {
                do
                    {
                    *q++ = *p++;
                    }
                while(*p);
        }

    *q = '\0';
    return out_buf;
}

#endif // duff_fmt_hpp
