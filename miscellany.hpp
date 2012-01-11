// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#ifndef miscellany_hpp
#define miscellany_hpp

#include "config.hpp"

#include <boost/algorithm/minmax_element.hpp>

#include <algorithm>
#include <cctype>
#include <climits>  // UCHAR_MAX
#include <cstddef>  // std::size_t
#include <ios>
#include <iterator> // std::distance()
#include <string>
#include <utility>
#include <vector>

/// Test whether every element in a range equals the specified constant.

template<typename InputIterator, typename T>
bool each_equal(InputIterator first, InputIterator last, T const& t)
{
    return std::distance(first, last) == std::count(first, last, t);
}

/// Test whether two files are identical. Arguments are filenames.

bool files_are_identical(std::string const&, std::string const&);

/// Ascertain vector minimum and maximum efficiently.
///
/// Implicitly-declared special member functions do the right thing.

template<typename T>
class minmax
{
    typedef typename std::vector<T>::const_iterator extremum_t;
    typedef std::pair<extremum_t,extremum_t> extrema_t;

  public:
    explicit minmax(std::vector<T> const& v)
        {
        extrema_t extrema = boost::minmax_element(v.begin(), v.end());
        minimum_ = *extrema.first ;
        maximum_ = *extrema.second;
        }

    T minimum() {return minimum_;}
    T maximum() {return maximum_;}

  private:
    T minimum_;
    T maximum_;
};

std::string htmlize(std::string const&);

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

/// 27.4.4.1/3

inline std::ios::fmtflags set_default_format_flags(std::ios_base& stream)
{
    return stream.setf(std::ios::skipws | std::ios::dec);
}

/// Omitting colons yields a valid posix path.

std::string iso_8601_datestamp_terse();

/// Colons separate HH:MM:SS, so result is not a valid posix path.

std::string iso_8601_datestamp_verbose();

template<typename T, std::size_t n>
inline std::size_t lmi_array_size(T(&)[n])
{
    return n;
}

inline bool is_ok_for_cctype(int c)
{
    return (EOF == c) || (0 <= c && c <= UCHAR_MAX);
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

/// DWISOTT
///
/// Perhaps this function template's only legitimate use is within a
/// conditional-inclusion [16.1] block.

template<typename T>
inline void stifle_warning_for_unused_variable(T const&)
{}

/// DWISOTT
///
/// Casting to void is always permitted by 5.2.9/4 via 5.4/5 (cf. C99
/// 6.3.2.2).
///
/// Taking the argument's address prevents this gcc warning:
///   "object of type [X] will not be accessed in void context"
/// for volatile types.

template<typename T>
inline void stifle_warning_for_unused_value(T const& t)
{
    (void)&t;
}

#endif // miscellany_hpp

