// General conversion between types.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: value_cast.hpp,v 1.6 2006-01-29 13:52:00 chicares Exp $

// Function template value_cast() converts between types, choosing a
// conversion method in the following order of decreasing preference:
//   direct conversion for directly convertible types
//   numeric_io_cast   for number <--> string
//   stream_cast       for all other cases
//
// Arithmetic types are handled more quickly and precisely by
// numeric_io_cast than by stream_cast. In boost-1.31.0, function
// template boost::lexical_cast has been tweaked to offer more
// precision, but less than can be achieved; and, as this discussion:
//   http://www.gotw.ca/publications/mill19.htm
// observes, it is generally more than an order of magnitude slower,
// though that's probably an artifact of the iostreams library.
// With g++-3.2.3, it's two decimal orders of magnitude slower.
//
// Especially for applications that perform a lot of numeric input and
// output, it is better to delegate to routines designed for precision
// and performance.
//
// Because value_cast() automatically does the right thing, it should
// generally be used instead of numeric_io_cast or stream_cast. But
// using one of those other casts directly when the types are known
// gives better results with compilers that can't handle value_cast(),
// such as borland.

// INELEGANT !! Test the runtime performance of value_cast() compared
// to the other casts it uses, to ensure that its overhead is minimal.

// INELEGANT !! Remove direct conversion from the numeric_io_cast and
// stream_cast implementations. After that change, revise the
// stream_cast documentation to explain that it won't convert an
// empty string to another string because the inserter will set
// failbit--which is an essential design feature--so this class
// really must be used instead. Describe here how to specialize
// template function value_cast for string-like types, as was done
// for template class datum_string.

// INELEGANT !! Exceptions thrown from numeric_io_cast and stream_cast
// ought perhaps to be derived from std::bad_cast.

// Note: boost::is_convertible<From,To> gives template arguments in
// the opposite of the order used for boost::lexical_cast<To,From>.
// The cast templates used here follow the latter order. All uses of
// boost::is_convertible here are commented to avoid ambiguity.

#ifndef value_cast_hpp
#define value_cast_hpp

#include "config.hpp"

#include "numeric_io_cast.hpp"
#include "stream_cast.hpp"

#ifndef __BORLANDC__

#include <boost/type_traits.hpp>

enum cast_method
    {e_direct
    ,e_numeric
    ,e_stream
    };

template<typename T>
struct is_string
{
    // Here, is_convertible means 'T' is convertible to std::string.
    enum {value = boost::is_convertible<T,std::string>::value};
};

template<typename To, typename From>
struct value_cast_choice
{
    enum
        {choice =
            // Here, is_convertible means 'From' is convertible to 'To'.
            boost::is_convertible<From,To>::value
            ?e_direct
            :   (
                    boost::is_arithmetic<From>::value && is_string<To  >::value
                ||  boost::is_arithmetic<To  >::value && is_string<From>::value
                )
                ?e_numeric
                :e_stream
        };
};

template<typename To, typename From, int = value_cast_choice<To,From>::choice>
struct value_cast_chooser
{};

template<typename To, typename From>
struct value_cast_chooser<To,From,e_direct>
{
    static cast_method method() {return e_direct;}
    To operator()(From from)    {return from;}
};

template<typename To, typename From>
struct value_cast_chooser<To,From,e_numeric>
{
    static cast_method method() {return e_numeric;}
    To operator()(From from)    {return numeric_io_cast<To>(from);}
};

template<typename To, typename From>
struct value_cast_chooser<To,From,e_stream>
{
    static cast_method method() {return e_stream;}
    To operator()(From from)    {return stream_cast<To>(from);}
};

template<typename To, typename From>
To value_cast(From from, To = To())
{
    return value_cast_chooser<To,From>()(from);
}

#else // __BORLANDC__

// COMPILER !! Precision of casts between string and arithmetic types
// is poor because borland can't handle the default implementation.
// It's not worth working around borland defects. Don't use value_cast
// with borland tools.

template<typename To, typename From>
To value_cast(From from, To = To())
{
    return stream_cast<To>(from);
}

#endif // __BORLANDC__

#endif // value_cast_hpp

