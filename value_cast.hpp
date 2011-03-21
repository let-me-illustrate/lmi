// General conversion between types.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#ifndef value_cast_hpp
#define value_cast_hpp

#include "config.hpp"

#include "numeric_io_cast.hpp"
#include "stream_cast.hpp"

#if !defined __BORLANDC__

#include <boost/cast.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_pointer.hpp>

#include <sstream>
#include <stdexcept>

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

/// Function template value_cast() converts between types, choosing a
/// conversion method in the following order of decreasing preference:
///   numeric_value_cast  for number <--> number
///   direct conversion   for intraconvertible types not both numeric
///   numeric_io_cast     for number <--> string
///   stream_cast         for all other cases
///
/// Arithmetic types are handled more quickly and precisely by
/// numeric_io_cast than by stream_cast. In boost-1.31.0, function
/// template boost::lexical_cast has been tweaked to offer more
/// precision, but less than can be achieved; and, as this discussion:
///   http://www.gotw.ca/publications/mill19.htm
/// observes, it is generally about an order of magnitude slower,
/// though that's probably an artifact of the iostreams library.
/// With MinGW g++-3.4.2, it's about one-fifth as fast. Especially for
/// applications that perform a lot of numeric input and output, it is
/// better to delegate to routines designed for precision and
/// performance.
///
/// Because value_cast() automatically chooses the best algorithm, it
/// is appropriate for general use. It is also appropriate to use the
/// more-specialized numeric_io_cast() directly for conversions known
/// to be within the scope of its design. Direct use of stream_cast()
/// is to be avoided in general because of its poor performance, and
/// to be avoided especially for numerics because of its poor
/// accuracy.
///
/// Order of template and function parameters.
///
/// The order of template parameters is the same as for similar
/// templates in boost (parameters and arguments altered here to
/// emphasize similarity):
///   template<typename To, typename From> To value_cast      (From ); // lmi
///   template<typename To, typename From> To lexical_cast    (From ); // boost
///   template<typename To, typename From> To polymorphic_cast(From*); // boost
///   template<typename To, typename From> To numeric_cast    (From ); // boost
///
/// The alternative form
///   template<typename To, typename From> To value_cast   (From, To);
/// is handy when the type 'To' is easier to deduce than to specify.
/// The order of template parameters is reversed for function
/// parameters so that the 'To' function parameter can be elided.
/// The same order-reversal would be necessary if a default function
/// argument were used; that approach is not taken because actually
/// using the default would gratuitously require type 'From' to be
/// DefaultConstructible (although that requirement remains in effect
/// when conversion is delegated to stream_cast(), in whose design it
/// inheres). The last two boost templates mentioned above actually
/// use such a default function argument, in boost-1.31.0 named
/// BOOST_EXPLICIT_DEFAULT_TARGET, with the same order-reversal,
/// although their rationale is not to increase the expressive power,
/// but merely to work around a compiler defect.
///
/// All uses of class boost::is_convertible here are commented to avoid
/// confusion due to the surprising order of its template parameters:
/// compare
///   boost::is_convertible<From,To>(From z)
/// to the declarations above. Also see this comment:
///   http://lists.boost.org/Archives/boost/2006/01/99722.php
///   "BTW Its a real pain that the parameter order for is_convertible
///   isnt the other way round IMO. Its confusing and error prone
///   because casts go the other way."

template<typename To, typename From>
To value_cast(From const& from);

enum cast_method
    {e_both_numeric
    ,e_direct
    ,e_numeric_io
    ,e_stream
    };

template<typename T>
struct is_string
{
    // Here, is_convertible means 'T' is convertible to std::string.
    enum {value = boost::is_convertible<T,std::string>::value};
};

template<typename T>
void throw_if_null_pointer(T)
{
}

template<typename T>
void throw_if_null_pointer(T* t)
{
    if(0 == t)
        {
        throw std::runtime_error("Null pointer holds no value to convert.");
        }
}

/// Function template numeric_value_cast() wraps boost::numeric_cast
/// to make it DWISOTT according to the boost-1.31.0 documentation:
///   "An exception is thrown when a runtime value-preservation
///   check fails."
/// The problem is that
///   boost::numeric_cast<int>(2.71828);
/// returns the integer 2 without throwing, but 2.71828 and 2 are
/// different values. It seems unreasonable to call truncation a
/// value-preserving relation.

template<typename To, typename From>
To numeric_value_cast(From const& from)
{
    To result = boost::numeric_cast<To>(from);
    if(result == from)
        {
        return result;
        }
    else
        {
        std::ostringstream oss;
        oss
            << "Value not preserved converting "
            << numeric_io_cast<std::string>(from)
            << " to "
            << numeric_io_cast<std::string>(result)
            << " ."
            ;
        throw std::runtime_error(oss.str());
        }
}

/// Class template value_cast_choice is an appurtenance of function
/// template value_cast(); it selects the best conversion method.

template<typename To, typename From>
struct value_cast_choice
{
    enum
        {
        // Here, is_convertible means 'From' is convertible to 'To'.
        convertible = boost::is_convertible<From,To>::value
        };

    enum
        {
        both_numeric =
                boost::is_arithmetic<From>::value
            &&  boost::is_arithmetic<To  >::value
        };

    enum
        {
        one_numeric_one_string =
                boost::is_arithmetic<From>::value && is_string<To  >::value
            ||  boost::is_arithmetic<To  >::value && is_string<From>::value
        };

    enum
        {choice =
            convertible
                ?both_numeric
                    ?e_both_numeric
                    :e_direct
                :one_numeric_one_string
                    ?e_numeric_io
                    :e_stream
        };
};

template<typename To, typename From, int = value_cast_choice<To,From>::choice>
struct value_cast_chooser
{
};

template<typename To, typename From>
struct value_cast_chooser<To,From,e_both_numeric>
{
    static cast_method method()     {return e_both_numeric;}
    To operator()(From const& from) {return numeric_value_cast<To>(from);}
};

template<typename To, typename From>
struct value_cast_chooser<To,From,e_direct>
{
    static cast_method method()     {return e_direct;}
    To operator()(From const& from) {throw_if_null_pointer(from); return from;}
};

template<typename To, typename From>
struct value_cast_chooser<To,From,e_numeric_io>
{
    static cast_method method()     {return e_numeric_io;}
    To operator()(From const& from) {return numeric_io_cast<To>(from);}
};

template<typename To, typename From>
struct value_cast_chooser<To,From,e_stream>
{
    static cast_method method()     {return e_stream;}
    To operator()(From const& from) {return stream_cast<To>(from);}
};

template<typename To, typename From>
To value_cast(From const& from)
{
    BOOST_STATIC_ASSERT(!boost::is_pointer<To>::value);
    return value_cast_chooser<To,From>()(from);
}

template<typename To, typename From>
To value_cast(From const& from, To)
{
    return value_cast<To,From>(from);
}

#else // defined __BORLANDC__

// COMPILER !! It's not worth working around borland defects: they are
// legion. Don't rely on these casts to DTRT with borland tools.

template<typename To, typename From>
To value_cast(From const& from, To = To())
{
    return stream_cast<To>(from);
}

template<typename To, typename From>
To numeric_value_cast(From const& from)
{
    return static_cast<To>(from);
}

#endif // defined __BORLANDC__

#endif // value_cast_hpp

