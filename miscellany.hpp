// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef miscellany_hpp
#define miscellany_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <algorithm>
#include <cctype>
#include <climits>                      // UCHAR_MAX
#include <cstdio>                       // EOF
#include <iomanip>
#include <ios>
#include <iterator>                     // distance()
#include <limits>                       // numeric_limits
#include <sstream>
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

bool files_are_identical(std::string const&, std::string const&);

template<typename T>
std::string floating_rep(T t)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    int realsize = static_cast<int>(sizeof(T));
#if defined __GNUC__ && defined LMI_X87
    // For gcc with x87, sizeof(long double) == 12, but only
    // ten bytes are significant--the other two are padding.
    if(12 == realsize)
        realsize = 10;
#endif // defined __GNUC__ && defined LMI_X87
    unsigned char const* u = reinterpret_cast<unsigned char const*>(&t);
    for(int j = realsize - 1; 0 <= j; --j)
        oss << std::setw(2) << static_cast<int>(u[j]);
    return oss.str();
}

/// Ascertain vector minimum and maximum efficiently.
///
/// Heterogeneous relational operators are necessarily free functions.
///
/// Implicitly-declared special member functions do the right thing.
///
/// SOMEDAY !! Make this usable with other containers than vector.

template<typename T>
class minmax
{
  public:
    minmax()
        :minimum_ {std::numeric_limits<T>::max()}
        ,maximum_ {std::numeric_limits<T>::min()}
        {
        }

    explicit minmax(std::vector<T> const& v)
        {
        auto const& extrema = std::minmax_element(v.begin(), v.end());
        minimum_ = *extrema.first ;
        maximum_ = *extrema.second;
        }

    void subsume(minmax<T> const& z)
        {
        minimum_ = std::min(minimum_, z.minimum_);
        maximum_ = std::max(maximum_, z.maximum_);
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

LMI_SO int scale_power(int max_power, double min_value, double max_value);

LMI_SO int count_newlines(std::string const&);

LMI_SO std::vector<std::string> split_into_lines(std::string const&);

std::string htmlize(std::string const&);

LMI_SO bool begins_with(std::string const& s, std::string const& prefix);

LMI_SO bool ends_with(std::string const& s, std::string const& suffix);

LMI_SO void ltrim(std::string& s, char const* superfluous);

LMI_SO void rtrim(std::string& s, char const* superfluous);

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

std::string iso_8601_datestamp_terse();

std::string iso_8601_datestamp_verbose();

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
