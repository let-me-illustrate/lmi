// Obsolete data conversion.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: value_cast_ihs.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

// boost.org has something similar, but it uses default precision (6) for
// numeric types, and doesn't work with bc++5.02 . Theirs is dated 2000-11;
// I wrote this one based on prior art from 2000-08, to wit: Dietmar Kuehl's
// posting to alt.comp.lang.learn.c-c++ dated 2000-08-06T22:32:29Z . Use
// Dietmar's or Kevlin's (boost.org) if you want more liberal licensing;
// this file is GPL.

#ifndef value_cast_ihs_hpp
#define value_cast_ihs_hpp

#include "config.hpp"

#include <algorithm> // std::max()
#include <cmath>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept> // std::runtime_error
#include <string>
#include <typeinfo>

// TODO ?? Ideally, given
//   T t;
// for any floating-point type T, we want
//   t == value_cast_ihs<T>(value_cast_ihs<std::string>(t));
// to be true; and given
//   std::string s(value_cast_ihs<std::string>(t));
// we want
//   s == value_cast_ihs<std::string>(value_cast_ihs<T>(s));
// to be true as well.
//
// This is easier said than done. It is not trivial to prevent a
// number like .123 from turning into a string like ".12299997".
//
// For the time being, we choose to recognize only the first
// (digits10 - 1) digits and hope that arithmetic extractors
// and insertors will round the problem away: that is, that
// ".12299997" with its last digit suppressed will become
// ".1230000".
//
// Then, we truncate insignificant zeros, and the decimal point too
// if it is not required, as follows. We format floating point
// numbers with std::ios_base::showpoint so that the decimal point
// is always included. Then, starting from the right, we remove
// all successive characters in {'0', '.'}.

namespace detail
{
    static std::string strip_fp(std::string s)
    {
    int n = s.size();
    for(int j = s.size() - 1; 0 < j; --j)
        {
        if('0' == s[j])
            {
            --n;
            }
        else if('.' == s[j])
            {
            --n;
            break;
            }
        else
            break;
        }
    s.resize(n);
    return s;
    }

    template<typename S, typename T> struct stripper
    {
        static S strip(S s, T const = T())
        {return s;}
    };

    LMI_SPECIALIZATION struct stripper<std::string, float      >
    {
        static std::string strip(std::string s, float const = 0.0)
        {return strip_fp(s);}
    };

    LMI_SPECIALIZATION struct stripper<std::string, double     >
    {
        static std::string strip(std::string s, double const = 0.0)
        {return strip_fp(s);}
    };

    LMI_SPECIALIZATION struct stripper<std::string, long double>
    {
        static std::string strip(std::string s, long double const = 0.0)
        {return strip_fp(s);}
    };

    template<typename T> int safe_log10_ceil(T)
    {
        return 0;
    }

    LMI_SPECIALIZATION inline int safe_log10_ceil(float       z)
    {
        return (0.0 == z)
            ? 0
            : static_cast<int>(std::ceil(std::log10(std::fabs(z))));
    }

    LMI_SPECIALIZATION inline int safe_log10_ceil(double      z)
    {
        return (0.0 == z)
            ? 0
            : static_cast<int>(std::ceil(std::log10(std::fabs(z))));
    }

    LMI_SPECIALIZATION inline int safe_log10_ceil(long double z)
    {
        return (0.0 == z)
            ? 0
            : static_cast<int>(std::ceil(std::log10(std::fabs(z))));
    }

} // namespace detail

class bad_value_cast : public std::runtime_error
{
  public:
    explicit bad_value_cast(std::string const& what_arg)
    :std::runtime_error("stream cast: " + what_arg)
    {}
};

namespace detail
{
    // Do the real work of conversion among types.
    template<typename To, typename From>
    To stream_cast(From source, To)
    {
        std::stringstream stream;
        To destination = To();

        // This affects only floating-point types [22.2.2.2.2/8].
        // We prefer fixed to scientific because it is more familiar
        // to less technically-inclined users.
        stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
        // Include decimal point (floating-point only) so that we can
        // use it as a sentry for stripping trailing zeros.
        stream.setf(std::ios_base::showpoint);

        // Since precision() affects only floating-point types [27.4.2.2/9],
        // it is safe to set it to a value that is appropriate only for such
        // types, even if no such type is involved.

        // For std::ios_base::fixed, precision() is the number of digits
        // after the decimal point. The maximum number of such digits that
        // are guaranteed to be significant is digits10 - std::log10(input).
        // We want one less digit than that: see note above.
#ifdef BC_BEFORE_5_5
#   pragma option -w-bei
#endif // BC_BEFORE_5_5
        std::streamsize prec = std::max
            (std::numeric_limits<From>::digits10
            ,std::numeric_limits<To  >::digits10
            );
#ifdef BC_BEFORE_5_5
#   pragma option -w.bei
#endif // BC_BEFORE_5_5

        prec -= safe_log10_ceil(source);

        prec--;
        // In no event do we want precision to be negative. It would seem
        // [22.2.2.2.2/11] that a negative precision would be ignored; but
        // we don't want to assume that standard library implementations
        // all correctly follow that interpretation. We need not worry
        // about unsigned arithmetic, since std::streamsize is a signed
        // integral type [27.4.1/2].
        if(prec < 0)
            {
            prec = 0;
            }
        stream.precision(prec);

        stream << source;
        if(!stream)
            {
            std::ostringstream error;
            error
                << "error after stream << source while trying to convert "
                << source
                << " to type "
                << typeid(To).name()
                ;
            throw bad_value_cast(error.str());
            }
        stream >> destination;
#ifndef BC_BEFORE_5_5
        // COMPILER !! Here bc++5.02 sets badbit as well as eofbit
        if(!stream)
            {
            std::ostringstream error;
            error
                << "error after stream >> destination while trying to convert "
                << "'" << source << "'"
                << " from type "
                << typeid(From).name()
                << " to type "
                << typeid(To).name()
                ;
            throw bad_value_cast(error.str());
            }
#endif // not old borland compiler
        stream >> std::ws;
#ifndef LMI_LACK_BASIC_IOSTREAMS
        if(!stream)
            {
            std::ostringstream error;
            error
                << "trailing whitespace remains after stream << source "
                << "while trying to convert "
                << source
                << " to type "
                << typeid(To).name()
                ;
            throw bad_value_cast(error.str());
            }
#endif // LMI_LACK_BASIC_IOSTREAMS
        if(!stream.eof())
            {
            std::ostringstream error;
            error
                << "not at eof after converting "
                << "'" << source << "'"
                << " from type "
                << typeid(From).name()
                << " to type "
                << typeid(To).name()
                ;
            throw bad_value_cast(error.str());
            }

        return destination;
    }
} // namespace detail

// We could write stream_cast like this and call it directly, but then
// specializing would be difficult in light of bc++5.2's need for a
// default argument (below). So instead we provide this interface, which
// better lends itself to specialization.
template<typename To, typename From>
#ifndef BC_BEFORE_5_5
    To value_cast_ihs(From source)
#else // old borland compiler
    To value_cast_ihs(From source, To = To())
#endif // old borland compiler
{
    return detail::stream_cast(source, To());
}


// Specializations.
// COMPILER !! gcc For some reason I don't understand, these are multiply
// defined with gcc-2.95.2-1 unless 'inline' is written after
// 'template<>'.

// To convert a char* to a string, just construct the string.
// We need to overload for both char* and char const*.
#ifndef BC_BEFORE_5_5
template<> inline std::string value_cast_ihs<std::string>(char* source)
#else // old borland compiler
std::string value_cast_ihs(char* source, std::string = std::string())
#endif // old borland compiler
{
    return source;
}

#ifndef BC_BEFORE_5_5
template<> inline std::string value_cast_ihs<std::string>(char const* source)
#else // old borland compiler
std::string value_cast_ihs(char const* source, std::string = std::string())
#endif // old borland compiler
{
    return source;
}

// To convert a string to a string, just return the argument.
// It would be nice to return it by const reference, but such
// a function signature wouldn't work for other types.
#ifndef BC_BEFORE_5_5
template<> inline std::string value_cast_ihs<std::string>(std::string source)
#else // old borland compiler
std::string value_cast_ihs(std::string source, std::string = std::string())
#endif // old borland compiler
{
    return source;
}

// When converting floating-point types to string, strip trailing
// insignificant digits. But see note above.

#ifndef BC_BEFORE_5_5
template<> inline std::string value_cast_ihs<std::string>(float source)
#else // old borland compiler
std::string value_cast_ihs(float source, std::string = std::string())
#endif // old borland compiler
{
    return detail::strip_fp(detail::stream_cast(source, std::string()));
}

#ifndef BC_BEFORE_5_5
template<> inline std::string value_cast_ihs<std::string>(double source)
#else // old borland compiler
std::string value_cast_ihs(double source, std::string = std::string())
#endif // old borland compiler
{
    return detail::strip_fp(detail::stream_cast(source, std::string()));
}

#ifndef BC_BEFORE_5_5
template<> inline std::string value_cast_ihs<std::string>(long double source)
#else // old borland compiler
std::string value_cast_ihs(long double source, std::string = std::string())
#endif // old borland compiler
{
    return detail::strip_fp(detail::stream_cast(source, std::string()));
}

// Normally we write
//   value_cast_ihs<desired_type>(value)
// but this template is helpful when we can't name the type desired but
// have a variable of that type. In such cases, this works as
//   value_cast_ihs<typeof(desired_type)>(value)
// would if the typeof() operator existed.
#ifndef BC_BEFORE_5_5
template<typename To, typename From>
To value_cast_ihs(From source, To)
{
    return value_cast_ihs<To>(source);
}
#endif // old borland compiler
#endif // value_cast_ihs_hpp

