// Traits for conversion between arithmetic types and strings.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: numeric_io_traits.hpp,v 1.1 2005-02-03 16:03:37 chicares Exp $

#ifndef numeric_io_traits_hpp
#define numeric_io_traits_hpp

#include "config.hpp"

#include <cmath>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <stdio.h> // Nonstandard (in C++98) snprintf().
#include <string>

#include <boost/cast.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

// Number of exact decimal digits to the right of the decimal point.
//
// Returns the maximum number of fractional decimal digits, q, such
// that a particular decimal number with q fractional decimal digits
// can be rounded into a particular floating-point type and back again
// without change to any of its digits. The value zero is here defined
// to have no fractional digits.
//
// The result differs from std::numeric_limits<T>::digits10 in two
// ways. For numbers all of whose digits are fractional, it returns
// either std::numeric_limits<T>::digits10 or one plus that quantity.
// For other numbers, it counts fractional digits instead of all
// digits.
//
// Notes: Truncation by static_cast<int> is appropriate because the
// result is constrained to be nonnegative. If negative results were
// wanted, it would be necessary to round toward negative infinity.
//
// Because many compilers in 2004 still don't implement C++98 26.5/6
// correctly, C99 functions fabsl() and log10l() are used here. It is
// less likely that these are incorrect than that the C++ overloads
// are missing, which would cause std::fabs() and std::log10() to be
// invoked for type long double.
//
template<typename T>
inline int floating_point_decimals(T t)
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    // Avoid taking zero's logarithm.
    if(0 == t)
        {
        return 0;
        }
    long double z = std::numeric_limits<T>::epsilon() * fabsl(t);
    return std::max(0, static_cast<int>(-log10l(z)));
}

// Simplify a formatted floating-point number.
//
// Precondition: 's' is a floating-point number formatted as if by
// snprintf() with format "%#.*f" or "%#.*Lf".
//
// Returns: 's' without any insignificant characters (trailing zeros
// after the decimal point, and the decimal point itself if followed
// by no nonzero digits).
//
// Note: The '#' flag ensures the presence of a decimal point in the
// argument, which this algorithm uses as a sentinel.
//
inline std::string simplify_floating_point(std::string const& s)
{
    std::string::const_reverse_iterator ri = s.rbegin();
  loop:
    switch(*ri)
        {
        case '0': if(++ri != s.rend()) goto loop;
        case '.': ++ri;
        default : ;
        }
    return std::string(s.begin(), ri.base());
}

// Template class numeric_conversion_traits.

template<typename T>
struct numeric_conversion_traits
{
    static int digits(T);
    static char const* fmt();
    static std::string simplify(std::string const&);
    static T strtoT(char const*, char**);
};

// Converting an int to string this way:
//   static_cast<T>(std::strtol(nptr, endptr, 0));
// seems distasteful because of the narrowing conversion. But there is
// no strtoi() in the C standard: C99 7.20.1.2 says that atoi() is
// equivalent to
//   (int)strtol(nptr, endptr, 10)
// except for the treatment of errors. Therefore, template function
// boost::numeric_cast() is used to detect narrowing conversions and
// throw an exception whenever they occur.
//
// It would seem nicer to choose a string-to-number conversion just by
// writing a function name: "std::strtoul", "std::strtod", etc. Here,
// however, the full implementation of
//   static T strtoT(char const*, char**);
// is supplied, mainly because the standard functions don't have the
// same number of arguments. It is probably unimportant that this
// tedious approach avoids the slight overhead of calling the
// conversion function through a pointer and guarding against actual
// narrowing conversions.

// Return C99 7.19.6.1/8 default precision for integral types.
// Calling snprintf() with a precision of zero and a value of
// zero would return no characters.
//
struct Integral{};
template<> struct numeric_conversion_traits<Integral>
{
    static int digits(long int) {return 1;}
    static std::string simplify(std::string const& s) {return s;}
};

template<> struct numeric_conversion_traits<char>
    :public numeric_conversion_traits<Integral>
{
    typedef char T;
    static char const* fmt()
        {
        return (0 == std::numeric_limits<T>::min())
            ? "%.*u"
            : "%.*i"
            ;
        }
    static T strtoT(char const* nptr, char** endptr)
        {
        return (0 == std::numeric_limits<T>::min())
            ? boost::numeric_cast<T>(std::strtoul(nptr, endptr, 0))
            : boost::numeric_cast<T>(std::strtol (nptr, endptr, 0))
            ;
        }
};

// C99's "%.*hhi" might be used instead if it gets added to C++.
template<> struct numeric_conversion_traits<signed char>
    :public numeric_conversion_traits<Integral>
{
    typedef signed char T;
    static char const* fmt() {return "%.*i";}
    static T strtoT(char const* nptr, char** endptr)
        {return boost::numeric_cast<T>(std::strtol(nptr, endptr, 0));}
};

// C99's "%.*hhi" might be used instead if it gets added to C++.
template<> struct numeric_conversion_traits<unsigned char>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned char T;
    static char const* fmt() {return "%.*u";}
    static T strtoT(char const* nptr, char** endptr)
        {return boost::numeric_cast<T>(std::strtoul(nptr, endptr, 0));}
};

// SOMEDAY !! Support this type when an actual need arises.
// The implementation here is untested.
#if 0
template<> struct numeric_conversion_traits<wchar_t>
    :public numeric_conversion_traits<Integral>
{
    typedef wchar_t T;
    static char const* fmt() {return "%.*lc";}
    static T strtoT(char const* nptr, char** endptr)
        {return boost::numeric_cast<T>(std::strtol(nptr, endptr, 0));}
};
#endif // 0

template<> struct numeric_conversion_traits<bool>
    :public numeric_conversion_traits<Integral>
{
    typedef bool T;
    static char const* fmt() {return "%.*i";}
    static T strtoT(char const* nptr, char** endptr)
        {return boost::numeric_cast<T>(std::strtol(nptr, endptr, 0));}
};

template<> struct numeric_conversion_traits<int>
    :public numeric_conversion_traits<Integral>
{
    typedef int T;
    static char const* fmt() {return "%.*i";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtol(nptr, endptr, 0));}
};

template<> struct numeric_conversion_traits<short int>
    :public numeric_conversion_traits<Integral>
{
    typedef short int T;
    static char const* fmt() {return "%.*hi";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtol(nptr, endptr, 0));}
};

template<> struct numeric_conversion_traits<long int>
    :public numeric_conversion_traits<Integral>
{
    typedef long int T;
    static char const* fmt() {return "%.*li";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtol(nptr, endptr, 0);}
};

// SOMEDAY !! Consider supporting type long long int when C++ does.
#if 0
template<> struct numeric_conversion_traits<long long int>
    :public numeric_conversion_traits<Integral>
{
    typedef long long int T;
    static char const* fmt() {return "%.*lli";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtoll(nptr, endptr, 0);}
};
#endif // 0

template<> struct numeric_conversion_traits<unsigned int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned int T;
    static char const* fmt() {return "%.*u";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtoul(nptr, endptr, 0));}
};

template<> struct numeric_conversion_traits<unsigned short int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned short int T;
    static char const* fmt() {return "%.*hu";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtoul(nptr, endptr, 0));}
};

template<> struct numeric_conversion_traits<unsigned long int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned long int T;
    static char const* fmt() {return "%.*lu";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtoul(nptr, endptr, 0);}
};

// SOMEDAY !! Consider supporting type long long int when C++ does.
#if 0
template<> struct numeric_conversion_traits<unsigned long long int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned long long int T;
    static char const* fmt() {return "%.*llu";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtoull(nptr, endptr, 0);}
};
#endif // 0

struct Floating{};
template<> struct numeric_conversion_traits<Floating>
{
    static std::string simplify(std::string const& s)
        {return simplify_floating_point(s);}
};

template<> struct numeric_conversion_traits<float>
    :public numeric_conversion_traits<Floating>
{
    typedef float T;
    static int digits(T t) {return floating_point_decimals(t);}
    static char const* fmt() {return "%#.*f";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtod(nptr, endptr);}
};

template<> struct numeric_conversion_traits<double>
    :public numeric_conversion_traits<Floating>
{
    typedef double T;
    static int digits(T t) {return floating_point_decimals(t);}
    static char const* fmt() {return "%#.*f";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtod(nptr, endptr);}
};

// COMPILER !! MinGW gcc-3.2.3 doesn't support "%Lf" correctly because
// it uses the ms C runtime library. Perhaps MinGW's nonstandard
// ldtoa() could be used instead.
#if 0
template<> struct numeric_conversion_traits<long double>
    :public numeric_conversion_traits<Floating>
{
    typedef long double T;
    static int digits(T t) {return floating_point_decimals(t);}
    static char const* fmt() {return "%#.*Lf";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtold(nptr, endptr);}
};
#else
template<> struct numeric_conversion_traits<long double>
    :public numeric_conversion_traits<Floating>
{
    typedef long double T;
    static int digits(T)
        {throw std::domain_error("Type 'long double' not supported.");}
    static char const* fmt()
        {throw std::domain_error("Type 'long double' not supported.");}
    static T strtoT(char const*, char**)
        {throw std::domain_error("Type 'long double' not supported.");}
};
#endif // 0

#endif // numeric_io_traits_hpp

