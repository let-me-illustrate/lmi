// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#ifndef miscellany_hpp
#define miscellany_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <algorithm>
#include <cctype>
#include <climits>                      // UCHAR_MAX
#include <cstddef>                      // size_t
#include <cstdio>                       // EOF
#include <ios>
#include <iterator>                     // distance()
#include <string>
#include <utility>
#include <vector>

/// Test whether every element in a range equals the specified constant.

template<typename InputIterator, typename T>
bool each_equal(InputIterator first, InputIterator last, T const& t)
{
    for(InputIterator i = first; i != last; ++i)
        {
        if(t != *i) return false;
        }
    return true;
}

/// Test whether every element in a range equals the specified constant.

template<typename RangeExpression, typename T>
bool each_equal(RangeExpression const& range, T const& t)
{
    for(auto const& i : range)
        {
        if(t != i) return false;
        }
    return true;
}

/// Test whether two files are identical. Arguments are filenames.

bool files_are_identical(std::string const&, std::string const&);

/// Ascertain vector minimum and maximum efficiently.
///
/// Heterogeneous relational operators are necessarily free functions.
///
/// Implicitly-declared special member functions do the right thing.

template<typename T>
class minmax
{
  public:
    explicit minmax(std::vector<T> const& v)
        {
        auto const& extrema = std::minmax_element(v.begin(), v.end());
        minimum_ = *extrema.first ;
        maximum_ = *extrema.second;
        }

    T minimum() const {return minimum_;}
    T maximum() const {return maximum_;}

  private:
    T minimum_;
    T maximum_;
};

template<typename T> bool operator< (T t, minmax<T> m) {return t <  m.minimum();}
template<typename T> bool operator<=(T t, minmax<T> m) {return t <= m.minimum();}
template<typename T> bool operator< (minmax<T> m, T t) {return m.maximum() <  t;}
template<typename T> bool operator<=(minmax<T> m, T t) {return m.maximum() <= t;}

/// Return the number of newline characters in a string.

std::size_t LMI_SO count_newlines(std::string const&);

/// Split an internally-newline-delimited string into lines.

std::vector<std::string> LMI_SO split_into_lines(std::string const&);

/// Escape text for html, e.g., "a < b" --> "a &lt; b".

std::string htmlize(std::string const&);

/// Ascertain whether string begins with prefix.

bool LMI_SO begins_with(std::string const& s, std::string const& prefix);

/// Ascertain whether string ends with suffix.

bool LMI_SO ends_with(std::string const& s, std::string const& suffix);

/// Remove superfluous characters from beginning of string.

void LMI_SO ltrim(std::string& s, char const* superfluous);

/// Remove superfluous characters from end of string.

void LMI_SO rtrim(std::string& s, char const* superfluous);

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

/// Compute the number of pages needed to display the given number of non-blank
/// rows in groups of the specified size separated by blank rows.
///
/// Preconditions: 0 < total_rows && 0 < rows_per_group <= rows_per_page

int LMI_SO page_count
    (int total_rows
    ,int rows_per_page
    ,int rows_per_group
    );

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

