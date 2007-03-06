// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: miscellany.hpp,v 1.9 2007-03-06 18:13:37 wboutin Exp $

#ifndef miscellany_hpp
#define miscellany_hpp

#include "config.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ios>
#include <iterator>
#include <string>

// TODO ?? Unit tests are needed.

// TODO ?? Remove this header along with the old implentation after testing:
#include "alert.hpp"
// Test whether every element in a range equals the specified constant.
template<typename InputIterator, typename T>
bool each_equal(InputIterator first, InputIterator last, T const& t)
{
// TODO ?? Remove this old implementation after testing:
    bool rc = true;
    for(InputIterator i = first; i != last; ++i)
        {
        if(!(t == *i))
            {
            rc = false;
            }
        }

    bool new_answer = std::distance(first, last) == std::count(first, last, t);
    HOPEFULLY(rc == new_answer);

    return std::distance(first, last) == std::count(first, last, t);
}

/// Test whether two files are identical. Arguments are filenames.

bool files_are_identical(std::string const&, std::string const&);

inline std::ios_base::openmode ios_in_binary()
{
    return
          std::ios_base::in
        | std::ios_base::binary
        ;
}

inline std::ios_base::openmode ios_out_app_binary()
{
    return
          std::ios_base::out
        | std::ios_base::app
        | std::ios_base::binary
        ;
}

inline std::ios_base::openmode ios_out_trunc_binary()
{
    return
          std::ios_base::out
        | std::ios_base::trunc
        | std::ios_base::binary
        ;
}

// Omitting colons yields a valid posix path.
std::string iso_8601_datestamp_terse();

// Colons separate HH:MM:SS, so result is not a valid posix path.
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
    return static_cast<unsigned char>(std::tolower(c));
}

inline unsigned char lmi_toupper(unsigned char c)
{
    return static_cast<unsigned char>(std::toupper(c));
}

#endif // miscellany_hpp

