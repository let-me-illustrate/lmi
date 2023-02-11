// Traits for conversion between arithmetic types and strings.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef numeric_io_traits_hpp
#define numeric_io_traits_hpp

#include "config.hpp"

#include "bourn_cast.hpp"
#include "global_settings.hpp"
#include "ieee754.hpp"                  // is_infinite<>()
#include "miscellany.hpp"               // rtrim()

#include <algorithm>                    // max()
#include <cmath>                        // fabs(), log10()
#include <cstdlib>                      // strto*()
#include <cstring>                      // strcmp(), strlen()
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

/// Number of exact decimal digits to the right of the decimal point.
///
/// Returns the maximum number of fractional decimal digits, q, such
/// that a particular decimal number with q fractional decimal digits
/// can be rounded into a particular floating-point type and back again
/// without change to any of its digits. The value zero is here defined
/// to have no fractional digits.
///
/// The result differs from std::numeric_limits<T>::digits10 in two
/// ways. For numbers all of whose digits are fractional, it returns
/// either std::numeric_limits<T>::digits10 or one plus that quantity.
/// For other numbers, it counts fractional digits instead of all
/// digits.
///
/// Notes: Truncation by static_cast<int> is appropriate because the
/// result is constrained to be nonnegative. If negative results were
/// wanted, it would be necessary to round toward negative infinity.

template<typename T>
inline int floating_point_decimals(T t)
{
    static_assert(std::is_floating_point_v<T>);
#if defined LMI_MSVCRT
    // COMPILER !! This C runtime not only writes infinity as "1.#INF"
    // instead of "inf" but also "respects" the precision specifier
    // when doing so, truncating it to "1." if this function were to
    // return zero.
    if(is_infinite(t))
        {
        return 4;
        }
#endif // defined LMI_MSVCRT
    // Avoid taking the logarithm of zero or infinity.
    if(0 == t || is_infinite(t))
        {
        return 0;
        }
    long double z = std::numeric_limits<T>::epsilon() * std::fabs(t);
    int const fuzz = global_settings::instance().regression_testing() ? -2 : 0;
    return std::max(0, fuzz + static_cast<int>(-std::log10(z)));
}

/// Simplify a formatted floating-point number.
///
/// Precondition: the argument is a floating-point number formatted as
/// if by std::snprintf() with format "%#.*f" or "%#.*Lf". Therefore,
/// it contains a decimal point (because of '#') preceded by at least
/// one digit (because of 'f'). These preconditions are not tested:
/// testing them would be costly, and this function is intended only
/// for internal use in a context that ensures their satisfaction.
///
/// Returns: 's' without any insignificant characters (trailing zeros
/// after the decimal point, and the decimal point itself if followed
/// by no nonzero digits).

inline std::string simplify_floating_point(std::string const& s)
{
    std::string z(s);
    rtrim(z, "0");
    rtrim(z, ".");
    return z;
}

/// Traits for conversion between arithmetic types and strings.
///
/// Converting an int to string this way:
///   static_cast<T>(std::strtol(nptr, endptr, 10));
/// seems distasteful because of the narrowing conversion. But there is
/// no strtoi() in the C standard: C99 7.20.1.2 says that atoi() is
/// equivalent to
///   (int)strtol(nptr, endptr, 10)
/// except for the treatment of errors. Therefore, function template
/// bourn_cast() is used to detect narrowing conversions and throw an
/// exception whenever they occur.
///
/// It would seem nicer to choose a string-to-number conversion just by
/// writing a function name: "std::strtoul", "std::strtod", etc. Here,
/// however, the full implementation of
///   static T strtoT(char const*, char**);
/// is supplied, mainly because the standard functions don't have the
/// same number of arguments. It is probably unimportant that this
/// tedious approach avoids the slight overhead of calling the
/// conversion function through a pointer and guarding against actual
/// narrowing conversions.

template<typename T>
struct numeric_conversion_traits
{
    static int digits(T);
    static char const* fmt();
    static std::string simplify(std::string const&);
    static T strtoT(char const*, char**);
};

/// Return C99 7.19.6.1/8 default precision for integral types.
/// Calling std::snprintf() with a precision of zero and a value of
/// zero would return no characters.

struct Integral{};
template<> struct numeric_conversion_traits<Integral>
{
    template<typename T> static int digits(T) {return 1;}
    static std::string simplify(std::string const& s) {return s;}
};

template<> struct numeric_conversion_traits<char>
    :public numeric_conversion_traits<Integral>
{
    typedef char T;
    static char const* fmt()
        {
        return std::numeric_limits<T>::is_signed
            ? "%.*i"
            : "%.*u"
            ;
        }
    static T strtoT(char const* nptr, char** endptr)
        {
        return std::numeric_limits<T>::is_signed
            ? bourn_cast<T>(std::strtol (nptr, endptr, 10))
            : bourn_cast<T>(std::strtoul(nptr, endptr, 10))
            ;
        }
};

/// C99's "%.*hhi" might be used instead if it gets added to C++.

template<> struct numeric_conversion_traits<signed char>
    :public numeric_conversion_traits<Integral>
{
    typedef signed char T;
    static char const* fmt() {return "%.*i";}
    static T strtoT(char const* nptr, char** endptr)
        {return bourn_cast<T>(std::strtol(nptr, endptr, 10));}
};

/// C99's "%.*hhi" might be used instead if it gets added to C++.

template<> struct numeric_conversion_traits<unsigned char>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned char T;
    static char const* fmt() {return "%.*u";}
    static T strtoT(char const* nptr, char** endptr)
        {return bourn_cast<T>(std::strtoul(nptr, endptr, 10));}
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
        {return bourn_cast<T>(std::strtol(nptr, endptr, 10));}
};
#endif // 0

template<> struct numeric_conversion_traits<bool>
    :public numeric_conversion_traits<Integral>
{
    typedef bool T;
    static char const* fmt() {return "%.*i";}
    static T strtoT(char const* nptr, char** endptr)
        {return bourn_cast<T>(std::strtol(nptr, endptr, 10));}
};

template<> struct numeric_conversion_traits<int>
    :public numeric_conversion_traits<Integral>
{
    typedef int T;
    static char const* fmt() {return "%.*i";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtol(nptr, endptr, 10));}
};

template<> struct numeric_conversion_traits<short int>
    :public numeric_conversion_traits<Integral>
{
    typedef short int T;
    static char const* fmt() {return "%.*hi";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtol(nptr, endptr, 10));}
};

template<> struct numeric_conversion_traits<long int>
    :public numeric_conversion_traits<Integral>
{
    typedef long int T;
    static char const* fmt() {return "%.*li";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtol(nptr, endptr, 10);}
};

template<> struct numeric_conversion_traits<long long int>
    :public numeric_conversion_traits<Integral>
{
    typedef long long int T;
    static char const* fmt() {return "%.*lli";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtoll(nptr, endptr, 10);}
};

template<> struct numeric_conversion_traits<unsigned int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned int T;
    static char const* fmt() {return "%.*u";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtoul(nptr, endptr, 10));}
};

template<> struct numeric_conversion_traits<unsigned short int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned short int T;
    static char const* fmt() {return "%.*hu";}
    static T strtoT(char const* nptr, char** endptr)
        {return static_cast<T>(std::strtoul(nptr, endptr, 10));}
};

template<> struct numeric_conversion_traits<unsigned long int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned long int T;
    static char const* fmt() {return "%.*lu";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtoul(nptr, endptr, 10);}
};

template<> struct numeric_conversion_traits<unsigned long long int>
    :public numeric_conversion_traits<Integral>
{
    typedef unsigned long long int T;
    static char const* fmt() {return "%.*llu";}
    static T strtoT(char const* nptr, char** endptr)
        {return std::strtoull(nptr, endptr, 10);}
};

struct Floating{};
template<> struct numeric_conversion_traits<Floating>
{
    static std::string simplify(std::string const& s)
        {return simplify_floating_point(s);}
};

#if defined LMI_MSVCRT
/// COMPILER !! This C runtime's strtod() doesn't understand C99's
/// "inf[inity]". Work around that, but don't worry about NaNs.

inline double strtoFDL_msvc(char const* nptr, char** endptr)
{
    if(!nptr || !endptr)
        {
        throw std::runtime_error("Numeric conversion: precondition failure.");
        }
    char* rendptr; // Pointer to which second std::strtod() argument refers.
    double z = std::strtod(nptr, &rendptr);
    if('\0' == *rendptr)
        {
        *endptr = rendptr;
        return z;
        }
    else
        {
        bool negative = '-' == *nptr;
        if(negative) {++nptr;}
        if
            (  0 == std::strcmp(nptr, "inf")
            || 0 == std::strcmp(nptr, "INF")
            || 0 == std::strcmp(nptr, "infinity")
            || 0 == std::strcmp(nptr, "INFINITY")
            )
            {
            *endptr = const_cast<char*>(nptr) + std::strlen(nptr);
            return negative
                ? -std::numeric_limits<double>::infinity()
                :  std::numeric_limits<double>::infinity()
                ;
            }
        else
            {
            throw std::invalid_argument("Numeric conversion failed.");
            }
        }
}
#endif // defined LMI_MSVCRT

template<> struct numeric_conversion_traits<float>
    :public numeric_conversion_traits<Floating>
{
    typedef float T;
    static int digits(T t) {return floating_point_decimals(t);}
    static char const* fmt() {return "%#.*f";}
    static T strtoT(char const* nptr, char** endptr)
#if defined LMI_MSVCRT
        {return strtoFDL_msvc(nptr, endptr);}
#else  // !defined LMI_MSVCRT
        {return std::strtof(nptr, endptr);}
#endif // !defined LMI_MSVCRT
};

template<> struct numeric_conversion_traits<double>
    :public numeric_conversion_traits<Floating>
{
    typedef double T;
    static int digits(T t) {return floating_point_decimals(t);}
    static char const* fmt() {return "%#.*f";}
    static T strtoT(char const* nptr, char** endptr)
#if defined LMI_MSVCRT
        {return strtoFDL_msvc(nptr, endptr);}
#else  // !defined LMI_MSVCRT
        {return std::strtod(nptr, endptr);}
#endif // !defined LMI_MSVCRT
};

template<> struct numeric_conversion_traits<long double>
    :public numeric_conversion_traits<Floating>
{
    typedef long double T;
    static int digits(T t) {return floating_point_decimals(t);}
#if defined LMI_MSVCRT
// COMPILER !! This C runtime doesn't support "%Lf" correctly.
    static char const* fmt()
        {throw std::domain_error("Type 'long double' not supported.");}
#else  // !defined LMI_MSVCRT
    static char const* fmt() {return "%#.*Lf";}
#endif // !defined LMI_MSVCRT
    static T strtoT(char const* nptr, char** endptr)
#if defined LMI_MSVCRT
        {return strtoFDL_msvc(nptr, endptr);}
#else  // !defined LMI_MSVCRT
        {return std::strtold(nptr, endptr);}
#endif // !defined LMI_MSVCRT
};

#endif // numeric_io_traits_hpp
