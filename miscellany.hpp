// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005 Gregory W. Chicares.
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

// $Id: miscellany.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef miscellany_hpp
#define miscellany_hpp

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ctime>
#include <iterator>
#include <string>
#include <stdexcept>

// TODO ?? Remove this header along with the old implentation after testing:
#include "alert.hpp"
// Test whether every element in a range equals the specified constant.
template<typename InputIterator, typename T>
bool each_equal(InputIterator first, InputIterator last, T const& t)
{
// TODO ?? Remove this old implentation after testing:
    bool rc = true;
    for(InputIterator i = first; i != last; ++i)
        if(!(t == *i))
            rc = false;

    bool new_answer = std::distance(first, last) == std::count(first, last, t);
    LMI_ASSERT(rc == new_answer);

    return std::distance(first, last) == std::count(first, last, t);
}

// No colons: works as a posix filename.
std::string iso_8601_datestamp_terse();

// Colons separate HH:MM:SS .
std::string iso_8601_datestamp_verbose();

template <typename T, std::size_t n>
inline std::size_t lmi_array_size(T(&)[n])
{
    return n;
}

inline bool is_ok_for_cctype(int c)
{
    return EOF == c || 0 <= c && c <= UCHAR_MAX;
}

// Functions lmi_tolower() and lmi_toupper() are derived works adapted
// from usenet message <5gf9d3$ikp@netlab.cs.rpi.edu> in 2005 by GWC.
// Any defect should not reflect on Fergus Henderson's reputation.

inline unsigned char lmi_tolower(unsigned char c)
{
    return std::tolower(c);
}

inline unsigned char lmi_toupper(unsigned char c)
{
    return std::toupper(c);
}

#endif // miscellany_hpp

