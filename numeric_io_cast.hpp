// Decimal conversion between std::string and arithmetic types.
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

// $Id: numeric_io_cast.hpp,v 1.1 2005-02-03 16:03:37 chicares Exp $

// Converts between arithmetic types and their std::string decimal
// representations, in these cases only:
//    to an arithmetic type from std::string or from char const*
//    to std::string from an arithmetic type
// and, trivially,
//    to std::string from std::string or from char const*
// Conversion between std::strings and other types can be supported by
// extension.
//
// Octal-literals are transformed to decimal-literals before conversion
// to a numeric type. The declared design goal is to implement decimal
// conversion, so "077" means seventy-seven, not sixty-three.
//
// Exceptions:
//
//  - Types std::wstring and wchar_t are unsupported for now because I
//    don't happen to use them.
//
//  - Type long double is unsupported for now because of problems with
//    the standard library implementations I use.
//
// Template function numeric_io_cast() is superficially similar to,
// but essentially different from, the well-known method
//   double d;
//   std::stringstream ss;
//   ss << 2.718281828459045, ss >> d;
// That method does whatever stream inserters and extractors do. For
// floating-point types, it loses precision, for example truncating
// the decimal value above to 2.71828 . For
//   std::string s;
//   std::stringstream ss;
//   ss << "Truncated after first space.", ss >> s;
// it returns "Truncated". And, as this discussion:
//   http://www.gotw.ca/publications/mill19.htm
// observes, it is generally more than an order of magnitude slower,
// though that's probably an artifact of the iostreams library.
// With g++-3.2.3, it's two decimal orders of magnitude slower.
//
// numeric_io_cast(), on the other hand, does whatever the strtoX()
// and Xprintf() families do (except for octal). It is specially
// tailored for converting between std::string and arithmetic types,
// and generally supports no other conversions. It preserves all
// possible floating-point decimal precision. And it is faster than
// the std::stringstream technique for all compilers tested in 2004.
//
// The behavior of numeric_io_cast() with character types may seem
// surprising at first, because it treats them as decimal numbers.
// Therefore, casting from std::string("1") to char returns '\1', and
// casting that result back to std::string returns the original value,
// but casting from std::string("A") to char is an error.

#ifndef numeric_io_cast_hpp
#define numeric_io_cast_hpp

#include "config.hpp"

#include "numeric_io_traits.hpp"

#include <sstream>
#include <stdexcept>
#include <stdio.h> // snprintf()
#include <string>
#include <typeinfo>

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

// COMPILER !! Discuss a better workaround with como.
//#if defined __COMO__ && defined __MINGW32_MAJOR_VERSION && __MINGW32_MAJOR_VERSION < 3
//#if defined __COMO__ && defined __MINGW32_VERSION
#if defined __COMO__
#   define snprintf _snprintf
#endif // Old gcc compiler.

inline std::string trim_superfluous_leading_zeros(std::string const& s)
{
    if(s.empty())
        {
        return s;
        }
    std::string::size_type p = s.find_first_not_of('0');
    if(std::string::npos == p)
        {
        return s.substr(s.size() - 1);
        }
    else
        {
        return s.substr(p);
        }
}


// A compile-time failure iff this template is ever instantiated is
// desired, but the straightforward
//   BOOST_STATIC_ASSERT(0);
// can fail even if it's never instantiated; instead, it is asserted
// that both template parameters are void, which 14.1/7 forbids.
//
// In case that compile-time assertion doesn't fail, e.g. because the
// borland compiler doesn't support the idiom, a run-time exception is
// thrown from To().
//
template<typename To, typename From>
struct numeric_converter
{
#ifndef __BORLANDC__
    BOOST_STATIC_ASSERT(boost::is_void<To>::value);
    BOOST_STATIC_ASSERT(boost::is_void<From>::value);
#endif // ! defined __BORLANDC__

    To operator()(From const& from) const
        {
        std::ostringstream err;
        err
            << "Cannot convert '"
            << from
            << "' from type "
            << typeid(From).name()
            << " to type "
            << typeid(To).name()
            << "'."
            ;
        throw std::runtime_error(err.str());
        }
};

// Conversion from string to arithmetic type uses all available
// precision for floating point.
//
template<typename To>
struct numeric_converter<To, std::string>
{
#ifndef __BORLANDC__
    BOOST_STATIC_ASSERT(boost::is_arithmetic<To>::value);
#endif // ! defined __BORLANDC__

    typedef std::string From;
    To operator()(From const& s) const
        {
        // Transform octal-literals to decimal-literals, to avoid
        // interpretation as octal.
        std::string trimmed = trim_superfluous_leading_zeros(s);
        char const* nptr = trimmed.c_str();
        // Pointer to which strtoT()'s 'endptr' argument refers.
        char* rendptr;
        To value = numeric_conversion_traits<To>::strtoT(nptr, &rendptr);
        if(rendptr == nptr)
            {
            std::ostringstream err;
            err
                << "Attempt to convert string '"
                << s
                << "' from type "
                << typeid(From).name()
                << " to type "
                << typeid(To).name()
                << " found nothing valid to convert."
                ;
            throw std::invalid_argument(err.str());
            }
        else if('\0' != *rendptr)
            {
            std::ostringstream err;
            err
                << "Attempt to convert string '"
                << s
                << "' from type "
                << typeid(From).name()
                << " to type "
                << typeid(To).name()
                << " failed on terminal substring '"
                << rendptr
                << "'."
                ;
            throw std::invalid_argument(err.str());
            }
        else
            {
            return value;
            }
        }
};

template<typename To>
struct numeric_converter<To, char const*>
{
    typedef char const* From;
    To operator()(From const& s) const
        {
        numeric_converter<To,std::string> z;
        return z(s);
        }
};

#ifdef __BORLANDC__
// COMPILER !! The borland compiler fails to ignore top-level
// cv-qualifiers as 14.8.2/1 requires. Reference:
//   http://groups.google.com/groups?as_umsgid=4164c8f0@newsgroups.borland.com
template<typename To>
struct numeric_converter<To, std::string const>
{
    typedef std::string From;
    To operator()(From const& s) const
        {
        numeric_converter<To,std::string> z;
        return z(s);
        }
};

// COMPILER !! The borland compiler needs this to convert from a
// string literal. It seems to type such constants as non-const char*,
// ignoring 4.2/2 .
template<typename To>
struct numeric_converter<To, char*>
{
    typedef char const* From;
    To operator()(From const& s) const
        {
        numeric_converter<To,std::string> z;
        return z(s);
        }
};
#endif // defined __BORLANDC__

// Conversion from arithmetic type to string uses maximum accurate
// precision for floating point.
//
template<typename From>
struct numeric_converter<std::string, From>
{
#ifndef __BORLANDC__
    BOOST_STATIC_ASSERT(boost::is_arithmetic<From>::value);
#endif // ! defined __BORLANDC__

    typedef std::string To;
    To operator()(From const& from) const
        {
        int const buffer_length = 10000;
        // Add one to buffer length, and append a null at the end of
        // the buffer, to work around a known problem in the ms C rtl.
        // Reference:
        //   http://www.gotw.ca/publications/mill19.htm
        // The borland rtl has a similar problem.
        char buffer[1 + buffer_length];
        buffer[buffer_length] = '\0';
        int actual_length = snprintf
            (buffer
            ,buffer_length
            ,numeric_conversion_traits<From>::fmt()
            ,numeric_conversion_traits<From>::digits(from)
            ,from
            );
        if(actual_length < 0)
            {
            std::ostringstream err;
            err
                << "Attempt to convert '"
                << from
                << "' to string failed: snprintf returned "
                << actual_length
                << ", indicating an encoding error with format string '"
                << numeric_conversion_traits<From>::fmt()
                << "'."
                ;
            throw std::runtime_error(err.str());
            }
        else if(buffer_length <= actual_length)
            {
            std::ostringstream err;
            err
                << "Attempt to convert '"
                << from
                << "' to string failed: snprintf returned "
                << actual_length
                << ", but buffer length is only "
                << buffer_length
                << "."
                ;
            throw std::runtime_error(err.str());
            }
        else
            {
            return numeric_conversion_traits<From>::simplify(To(buffer));
            }
        }
};

template<>
struct numeric_converter<std::string, std::string>
{
    typedef std::string From;
    typedef std::string To;
    To operator()(From const& from) const
        {
        return from;
        }
};

#ifdef __BORLANDC__
// COMPILER !! The borland compiler needs this to convert from a
// string literal. It seems to type such constants as non-const char*,
// ignoring 4.2/2 .
template<>
struct numeric_converter<std::string, char*>
{
    typedef char* From;
    typedef std::string To;
    To operator()(From const& from) const
        {
        return from;
        }
};
#endif // __BORLANDC__

template<>
struct numeric_converter<std::string, char const*>
{
    typedef char const* From;
    typedef std::string To;
    To operator()(From const& from) const
        {
        return from;
        }
};

template<typename To, typename From>
To numeric_io_cast(From from, To = To())
{
    numeric_converter<To,From> converter;
    return converter.operator()(from);
}

#if defined __COMO__ && defined __MINGW32_MAJOR_VERSION && __MINGW32_MAJOR_VERSION < 3
#   undef snprintf
#endif // Old gcc compiler.

#endif // numeric_io_cast_hpp

