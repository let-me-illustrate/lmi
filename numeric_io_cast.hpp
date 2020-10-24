// Decimal conversion between std::string and arithmetic types.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef numeric_io_cast_hpp
#define numeric_io_cast_hpp

#include "config.hpp"

#include "numeric_io_traits.hpp"
#include "rtti_lmi.hpp"

#include <cstring>                      // strcmp()
#include <sstream>
#include <stdexcept>
#include <stdio.h>                      // snprintf()
#include <string>
#include <type_traits>
#include <typeinfo>

template<typename To, typename From>
struct numeric_converter;

/// Design notes for function template numeric_io_cast().
///
/// Converts between arithmetic types and their std::string decimal
/// representations, in these cases only:
///    to an arithmetic type from std::string or from char const*
///    to std::string from an arithmetic type
/// and, trivially,
///    to std::string from std::string or from char const*
/// Conversion between std::strings and other types can be supported
/// by extension.
///
/// Octal-literals are treated as decimal: leading zeros are ignored.
/// The declared design goal is to implement decimal conversion, so
/// "077" means seventy-seven, not sixty-three, while "099" is well
/// defined and means ninety-nine.
///
/// Exceptions:
///
///  - Types std::wstring and wchar_t are unsupported for now because
///    I don't happen to use them.
///
///  - Type long double is unsupported for now because of problems
///    with the standard library implementations I use.
///
/// Template function numeric_io_cast() is superficially similar to,
/// but essentially different from, the well-known method
///   double d;
///   std::stringstream ss;
///   ss << 2.718281828459045, ss >> d;
/// That method does whatever stream inserters and extractors do. For
/// floating-point types, it loses precision, for example truncating
/// the decimal value above to 2.71828 . For
///   std::string s;
///   std::stringstream ss;
///   ss << "Truncated after first space.", ss >> s;
/// it returns "Truncated". And, as this discussion:
///   http://www.gotw.ca/publications/mill19.htm
/// observes, it is generally much slower, probably because of memory-
/// allocation overhead and inefficient implementations of arithmetic
/// inserters and extractors. See the accompanying unit test for a way
/// to measure the speed difference.
///
/// numeric_io_cast(), on the other hand, does whatever the strtoX()
/// family does (with decimal base for integers). It is specially
/// tailored for converting between std::string and arithmetic types,
/// and generally supports no other conversions. It preserves all
/// possible floating-point decimal precision. And it is faster than
/// the std::stringstream technique for all compilers tested in 2004.
///
/// The behavior of numeric_io_cast() with builtin character types
/// (e.g., char, as opposed to char const*, which is a pointer type,
/// or std::string, which is not a builtin type) may seem surprising
/// at first: it treats them as decimal numbers. Thus, casting from
/// std::string("1") to char returns '\1', and casting that result
/// back to std::string returns the original value, while casting from
/// std::string("A") to char is an error. This follows C++98 3.9.1/7,
/// which defines char as an integer type, along with int, bool, etc.
/// Consistency is thus valued over the notion of a char as some sort
/// of degenerate string capable of holding single-digit decimal
/// integers as numerals.

template<typename To, typename From>
To numeric_io_cast(From from, To = To())
{
    numeric_converter<To,From> converter;
    return converter.operator()(from);
}

/// A compile-time failure iff this template is ever instantiated is
/// desired, but the straightforward
///   static_assert(0);
/// can fail even if it's never instantiated; instead, it is asserted
/// that both template parameters are void, which 14.1/7 forbids.
/// To make assurance doubly sure, a run-time exception is thrown in
/// any case.

template<typename To, typename From>
struct numeric_converter
{
    static_assert(std::is_void_v<To  >);
    static_assert(std::is_void_v<From>);

    To operator()(From const&) const
        {
        throw std::logic_error("Impossible numeric conversion.");
        }
};

// Conversion from string to arithmetic type uses all available
// precision for floating point.
//
template<typename To>
struct numeric_converter<To, std::string>
{
    static_assert(std::is_arithmetic_v<To>);

    typedef std::string From;
    To operator()(From const& from) const
        {
        char const* nptr = from.c_str();
        // Pointer to which strtoT()'s 'endptr' argument refers.
        char* rendptr;
        To value = numeric_conversion_traits<To>::strtoT(nptr, &rendptr);
        if(rendptr == nptr)
            {
            std::ostringstream err;
            err
                << "Attempt to convert string '"
                << from
                << "' from type "
                << lmi::TypeInfo(typeid(From))
                << " to type "
                << lmi::TypeInfo(typeid(To))
                << " found nothing valid to convert."
                ;
            throw std::invalid_argument(err.str());
            }
        else if('\0' != *rendptr)
            {
            std::ostringstream err;
            err
                << "Attempt to convert string '"
                << from
                << "' from type "
                << lmi::TypeInfo(typeid(From))
                << " to type "
                << lmi::TypeInfo(typeid(To))
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
    To operator()(From from) const
        {
        if(nullptr == from)
            {
            throw std::runtime_error
                ("Cannot convert (char const*)(0) to number."
                );
            }
        numeric_converter<To,std::string> z;
        return z(from);
        }
};

// Conversion from arithmetic type to string uses maximum accurate
// precision for floating point.
//
template<typename From>
struct numeric_converter<std::string, From>
{
    static_assert(std::is_arithmetic_v<From>);

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
#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdouble-promotion"
#   pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif // defined __GNUC__
        int actual_length = std::snprintf
            (buffer
            ,buffer_length
            ,numeric_conversion_traits<From>::fmt()
            ,numeric_conversion_traits<From>::digits(from)
            ,from
            );
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
        if(actual_length < 0)
            {
            std::ostringstream err;
            err
                << "Attempt to convert '"
                << from
                << "' to string failed: std::snprintf returned "
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
                << "' to string failed: std::snprintf returned "
                << actual_length
                << ", but buffer length is only "
                << buffer_length
                << "."
                ;
            throw std::runtime_error(err.str());
            }
        else
            {
#if defined LMI_MSVCRT
            // COMPILER !! This C runtime formats infinity as "1.#INF".
            // Instead, force C99 "inf".
            if(0 == std::strcmp(buffer, "1.#INF"))
                {
                return "inf";
                }
#endif // defined LMI_MSVCRT
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

template<>
struct numeric_converter<std::string, char const*>
{
    typedef char const* From;
    typedef std::string To;
    To operator()(From from) const
        {
        if(nullptr == from)
            {
            throw std::runtime_error
                ("Cannot convert (char const*)(0) to std::string."
                );
            }
        return from;
        }
};

#endif // numeric_io_cast_hpp
