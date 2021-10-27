// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

template<typename T>
constexpr T infimum()
{
    using limits = std::numeric_limits<T>;
    static_assert(limits::is_bounded);
    if constexpr(limits::has_infinity)
        return -limits::infinity();
    else
        return limits::min();
}

template<typename T>
constexpr T supremum()
{
    using limits = std::numeric_limits<T>;
    static_assert(limits::is_bounded);
    if constexpr(limits::has_infinity)
        return limits::infinity();
    else
        return limits::max();
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
    static_assert(std::numeric_limits<T>::is_bounded);

  public:
    minmax() {}

    explicit minmax(std::vector<T> const& v)
        {
        if(v.empty())
            {
            return;
            }

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
    T minimum_ {supremum<T>()};
    T maximum_ {infimum <T>()};
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

/// Restore std::ios state on scope exit.
///
/// This is a blunt and simple tool that saves and restores many
/// things that are not often wanted, and may be unwanted, such as
/// tied streams and the exception mask.
///
/// If no use is found for this, it may be expunged someday.
///
/// Cannot be implemented in terms of std::ios_base, which lacks
/// copyfmt(). Could be templated if std::wios support is wanted.

class scoped_ios_fmt
{
  public:
    explicit scoped_ios_fmt(std::ios& ios)
        :ios_   {ios}
        ,saved_ (nullptr)
        {
        saved_.copyfmt(ios_);
        }

    ~scoped_ios_fmt()
        {
        ios_.copyfmt(saved_);
        }

  private:
    std::ios& ios_;
    std::ios  saved_;
};

/// Restore std::ios formatting state on scope exit.
///
/// Restores only formatting members, not the full stream state
/// (which includes tied streams and the exception mask, e.g.).
///
/// Cannot be implemented in terms of std::ios_base, which lacks
/// fill(). Could be templated if std::wios support is wanted.

class scoped_ios_format
{
  public:
    explicit scoped_ios_format(std::ios& ios)
        :ios_           {ios}
        ,old_fill_      {ios.fill()}
        ,old_flags_     {ios.flags()}
        ,old_precision_ {ios.precision()}
        ,old_width_     {ios.width()}
        {}

    ~scoped_ios_format()
        {
        ios_.fill     (old_fill_);
        ios_.flags    (old_flags_);
        ios_.precision(old_precision_);
        ios_.width    (old_width_);
        }

  private:
    std::ios&                 ios_;
    std::ios::char_type const old_fill_;
    std::ios::fmtflags  const old_flags_;
    std::streamsize     const old_precision_;
    std::streamsize     const old_width_;
};

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

/// Avoid compiler warning for unused variable or unused value.

template<typename T>
inline void stifle_unused_warning(T const&)
{}

#endif // miscellany_hpp
