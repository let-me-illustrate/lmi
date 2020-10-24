// General conversion between types.
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

#ifndef value_cast_hpp
#define value_cast_hpp

#include "config.hpp"

#include "bourn_cast.hpp"
#include "numeric_io_cast.hpp"
#include "stream_cast.hpp"

#include <stdexcept>
#include <string>
#include <type_traits>

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

/// Function template value_cast() converts between types, choosing a
/// conversion method in the following order of decreasing preference:
///   bourn_cast          for number <--> number
///   direct conversion   for interconvertible types not both numeric
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
/// All uses of class std::is_convertible here are commented to avoid
/// confusion due to the surprising order of its template parameters:
/// compare
///   std::is_convertible<From,To>(From z)
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
    enum {value = std::is_convertible_v<T,std::string>};
};

template<typename T>
inline constexpr bool is_string_v = is_string<T>::value;

template<typename T>
void throw_if_null_pointer(T)
{
}

template<typename T>
void throw_if_null_pointer(T* t)
{
    if(nullptr == t)
        {
        throw std::runtime_error("Null pointer holds no value to convert.");
        }
}

/// Class template value_cast_choice is an appurtenance of function
/// template value_cast(); it selects the best conversion method.
///
/// The choice among conversion methods depends in part on whether
/// an implicit conversion is available. Implicit conversions from
/// pointer or array to bool are disregarded as being infelicitous.
/// For example, given:
///   char const* untrue = "0";
/// these casts:
///   static_cast<bool>(untrue);  // converts pointer->bool
///   static_cast<bool>("0");     // converts array->pointer->bool
/// return 'true' because the conversions involve non-null pointers;
/// however, these casts:
///   value_cast<bool>(untrue);
///   value_cast<bool>("0");
/// preserve the value by returning 'false'.

template<typename To, typename From>
struct value_cast_choice
{
    enum
        {
        // Here, is_convertible means 'From' is convertible to 'To'.
        felicitously_convertible =
                std::is_convertible_v<From,To>
            &&!(std::is_array_v  <From> && std::is_same_v<bool,To>)
            &&!(std::is_pointer_v<From> && std::is_same_v<bool,To>)
        };

    enum
        {
        both_numeric =
                std::is_arithmetic_v<From>
            &&  std::is_arithmetic_v<To  >
        };

    enum
        {
        one_numeric_one_string =
                std::is_arithmetic_v<From> && is_string_v<To  >
            ||  std::is_arithmetic_v<To  > && is_string_v<From>
        };

    enum
        {choice =
            felicitously_convertible
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
    To operator()(From const& from) {return bourn_cast<To>(from);}
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
    static_assert(!std::is_pointer_v<To>);
    return value_cast_chooser<To,From>()(from);
}

template<typename To, typename From>
To value_cast(From const& from, To)
{
    return value_cast<To,From>(from);
}

#endif // value_cast_hpp
