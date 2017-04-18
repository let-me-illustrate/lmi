// Numeric stinted cast, across whose bourn no value is returned.
//
// Copyright (C) 2017 Gregory W. Chicares.
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

#ifndef bourn_cast_hpp
#define bourn_cast_hpp

#include "config.hpp"

#include "rtti_lmi.hpp"                 // lmi::TypeInfo [demangling]

#include <cmath>                        // isinf(), isnan(), ldexp(), signbit()
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>                  // std::integral_constant

#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wsign-compare"
#   if 5 <= __GNUC__
#       pragma GCC diagnostic ignored "-Wbool-compare"
#   endif // 5 <= __GNUC__
#endif // defined __GNUC__

/// Floating to floating.
///
/// Handle special cases first:
///  - infinities are interconvertible: no exception wanted;
///  - C++11 [4.8/1] doesn't require static_cast to DTRT for NaNs;
/// then convert iff within range.
///
/// Alternatively, a case could be made for converting out-of-range
/// values to infinity, e.g.,
///   (float)(DBL_MAX) --> INFINITY
/// citing IEEE 754-2008 [5.4.2] "conversion ... to a narrower format
/// ... shall be rounded as specified in Clause 4" and [4.3.1] "an
/// infinitely precise result [exceeding the normalized maximum] shall
/// round to [infinity]", and C99 [F.3] "conversions for floating
/// types provide the IEC 60559 conversions between floating-point
/// precisions"; however, C++11 [4.8.1] still says this is undefined
/// behavior, and such a conversion is unlikely to be intentional.
///
/// IEEE 754-2008 [6.2] would return a NaN argument unchanged wherever
/// possible. While a compiler could do that, this implementation in
/// general cannot: even in the case of conversion to the same type,
/// returning the argument would involve undefined behavior in the
/// case of a signaling NaN [C99 F.2.1].

template<typename To, typename From>
#if 201402L < __cplusplus
constexpr
#endif // 201402L < __cplusplus
inline To bourn_cast(From from, std::false_type, std::false_type)
{
    using   to_traits = std::numeric_limits<To  >;
    using from_traits = std::numeric_limits<From>;
    static_assert(!to_traits::is_integer && !from_traits::is_integer, "");

    if(std::isnan(from))
        return to_traits::quiet_NaN();
    if(std::isinf(from))
        return
            std::signbit(from)
            ? -to_traits::infinity()
            :  to_traits::infinity()
            ;
    if(from < to_traits::lowest())
        throw std::runtime_error("Cast would transgress lower limit.");
    if(to_traits::max() < from)
        throw std::runtime_error("Cast would transgress upper limit.");
    return static_cast<To>(from);
}

/// Integral to floating.

template<typename To, typename From>
#if 201402L < __cplusplus
constexpr
#endif // 201402L < __cplusplus
inline To bourn_cast(From from, std::false_type, std::true_type)
{
    using   to_traits = std::numeric_limits<To  >;
    using from_traits = std::numeric_limits<From>;
    static_assert(!to_traits::is_integer && from_traits::is_integer, "");

    // If this assertion fails, the comparisons below may be UB.
    static_assert(from_traits::digits < to_traits::max_exponent, "");

    if(from < to_traits::lowest())
        throw std::runtime_error("Cast would transgress lower limit.");
    if(to_traits::max() < from)
        throw std::runtime_error("Cast would transgress upper limit.");
    return static_cast<To>(from);
}

/// Floating to integral.
///
/// Integral max() must be one less than an integer power of two,
/// because C++11 [3.9.1/7] says "the representations of integral
/// types shall define values by use of a pure binary numeration
/// system", so the range of a signed eight-bit character (e.g.) is:
///   [-127, +127] sign and magnitude, or ones' complement; or
///   [-128, +127] two's complement;
/// and the maximum must be 2^digits - 1 in any case.
///
/// It is not always feasible to compare the argument's value directly
/// to this maximum in order to determine whether it is within range:
/// see test_m64_neighborhood() in the accompanying unit test for a
/// demonstration of the issues that arise in converting ULLONG_MAX to
/// IEEE 754 binary32. Therefore, the tractable throw-condition
///   maximum + 1 <= argument  // 'maximum + 1' == 2^digits exactly
/// is substituted for the intractable
///   maximum < argument       // 0xFF... may exceed float precision
/// To ensure that the addition 'maximum + 1' is not done in extended
/// precision (as actually observed with various versions of gcc), it
/// is performed through writes to volatile memory. To ensure that
/// the maximum can be incremented, a static assertion compares the
/// number of integral radix digits to the number of floating exponent
/// digits. This assertion would be expected to fail with a 128-bit
/// integral type and a 32-bit IEEE 754 float. It is written as a
/// static assertion rather than a throw-statement because 128-bit
/// long long integers are not generally available, so it is not
/// possible to test such logic today.
///
/// The result of ldexp() is guaranteed to be representable. If it
/// overflows, it returns HUGE_VAL[FL] according to C99 [7.12.1/4],
/// which is a positive infinity [F.9/2] for an implementation that
/// conforms to IEEE 754. It is okay if one or both of the limits
/// tested is an infinity: e.g., if the integral type has a maximum
/// too large for the floating type to represent finitely, then no
/// finite floating argument is too large to cast to the integral
/// type. Because radix is asserted upstream to be two for all types,
/// there is no need to use scalbn() in place of ldexp(); and as long
/// as the widest integer has less than (sizeof int) digits, there is
/// no need here for scalbln().

template<typename To, typename From>
#if 201402L < __cplusplus
constexpr
#endif // 201402L < __cplusplus
inline To bourn_cast(From from, std::true_type, std::false_type)
{
    using   to_traits = std::numeric_limits<To  >;
    using from_traits = std::numeric_limits<From>;
    static_assert(to_traits::is_integer && !from_traits::is_integer, "");

    static constexpr From limit = std::ldexp(From(1), to_traits::digits);

    static constexpr bool is_twos_complement(~To(0) == -To(1));

    if(to_traits::digits < from_traits::max_exponent)
        {
        static From const volatile raw_max = From(to_traits::max());
        static From const volatile adj_max = raw_max + From(1);
        if(is_twos_complement && limit != adj_max)
            throw std::runtime_error("Inconsistent limits.");
        }

    if(std::isnan(from))
        throw std::runtime_error("Cannot cast NaN to integral.");
    if(std::isinf(from))
        throw std::runtime_error("Cannot cast infinite to integral.");
    if(!to_traits::is_signed && from < 0)
        throw std::runtime_error("Cannot cast negative to unsigned.");
    if(from < -limit || from == -limit && !is_twos_complement)
        throw std::runtime_error("Cast would transgress lower limit.");
    if(limit <= from)
        throw std::runtime_error("Cast would transgress upper limit.");
    To const r = static_cast<To>(from);
    if(r != from)
        {
        lmi::TypeInfo from_type(typeid(From));
        lmi::TypeInfo   to_type(typeid(To  ));
        std::ostringstream oss;
        oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
        oss
            << "Cast from " << from << " [" << from_type << "]"
            << " to "       << r    << " [" << to_type   << "]"
            << " would not preserve value."
            ;
        throw std::runtime_error(oss.str());
        }
    return r;
}

/// Integral to integral.
///
/// Converts between integral types that may differ in size and
/// signedness, iff the value is between the maximum and minimum
/// values permitted for the target (To) type. Because of the
/// properties of integers, conversion between integral types
/// either preserves the notional value, or throws.
///
/// The underlying idea is discussed here:
///   https://groups.google.com/forum/#!original/comp.std.c++/WHu6gUiwXkU/ZyV_ejRrXFYJ
/// and here:
///   http://www.two-sdg.demon.co.uk/curbralan/code/numeric_cast/numeric_cast.hpp
/// and embodied in Kevlin Henney's original boost:numeric_cast,
/// distributed under the GPL-compatible Boost Software License.

template<typename To, typename From>
#if 201402L < __cplusplus
constexpr
#endif // 201402L < __cplusplus
inline To bourn_cast(From from, std::true_type, std::true_type)
{
    using   to_traits = std::numeric_limits<To  >;
    using from_traits = std::numeric_limits<From>;
    static_assert(to_traits::is_integer && from_traits::is_integer, "");

    if(! to_traits::is_signed && from < 0)
        throw std::runtime_error("Cannot cast negative to unsigned.");
    if(from_traits::is_signed && from < to_traits::lowest())
        throw std::runtime_error("Cast would transgress lower limit.");
    if(to_traits::max() < from)
        throw std::runtime_error("Cast would transgress upper limit.");
    return static_cast<To>(from);
}

#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__

/// Numeric stinted cast, across whose bourn no value is returned.
///
/// Perform a static_cast between numeric types, but throw if its
/// behavior is undefined or if the value is out of range--e.g.:
///   bourn_cast<unsigned int>( 1);        // Returns 1U.
///   bourn_cast<unsigned int>(-1);        // Throws: out of range.
///   bourn_cast<unsigned int>(-1.0);      // Throws: UB.
///   bourn_cast<bool>(2);                 // Throws: out of range.
///   bourn_cast<float>((double)INFINITY); // Returns infinity.
///   bourn_cast<int>  ((double)INFINITY); // Throws.
///   bourn_cast<float>(DBL_MAX);          // Throws: UB.
///   bourn_cast<unsigned int>(3.0);       // Returns 3U.
///   bourn_cast<unsigned int>(3.14);      // Throws: 3.14 != 3U.
/// Thus, value is preserved exactly, except for the roundoff expected
/// when converting a floating-point value to a different floating-
/// point type that can represent it only with a loss of precision.
///
/// Both From and To must be types for which std::numeric_limits is
/// specialized. Integral-to-floating conversion is highly unlikely
/// to exceed bounds, but may lose precision. Floating-to-integral
/// conversion is extremely unlikely to preserve value, in which case
/// an exception is thrown; but bourn_cast is appropriate for casting
/// an already-rounded integer-valued floating value to another type.
///
/// bourn_cast<>() is intended as a simple and correct replacement for
/// boost::numeric_cast<>(), which does the wrong thing in some cases:
///   http://lists.nongnu.org/archive/html/lmi/2017-03/msg00127.html
///   http://lists.nongnu.org/archive/html/lmi/2017-03/msg00128.html
/// It behaves the same way as boost::numeric_cast<>() except that,
/// instead of quietly truncating, it throws on floating-to-integral
/// conversions that would not preserve value.
///
/// The radix of all numeric types is asserted to be two because this
/// implementation has not been tested with any other radix, and some
/// parts depend on binary representations. It might seem that C++11
/// [3.9.1/7] guarantees this for integral types: "the representations
/// of integral types shall define values by use of a pure binary
/// numeration system". However, [18.3.2.4/22] says otherwise: "for
/// integer types, [radix] specifies the base of the representation",
/// and footnote 202 says that this "distinguishes types with bases
/// other than 2 (e.g. BCD)".
///
/// Facilities provided by <limits> are used to the exclusion of
/// <type_traits> functions such as
///   is_arithmetic()
///   is_floating_point()
///   is_integral()
///   is_signed()
///   is_unsigned()
/// so that UDTs with std::numeric_limits specializations can work
/// as expected.
///
/// It would be simple to write a correct implementation if there were
/// a type capable of representing any value of any arithmetic type.
/// This criterion is satisfied by x86_86's 80-bit long double type as
/// long as there is no wider floating type and no integral type has
/// more than 64 bits. However, because no such type generally exists,
/// floating and integral limits must be compared with great care in
/// order to avoid undefined behavior.

template<typename To, typename From>
#if 201402L < __cplusplus
constexpr
#endif // 201402L < __cplusplus
inline To bourn_cast(From from)
{
    using   to_traits = std::numeric_limits<To  >;
    using from_traits = std::numeric_limits<From>;

    static_assert(  to_traits::is_specialized, "");
    static_assert(from_traits::is_specialized, "");

    static_assert(2 ==   to_traits::radix, "");
    static_assert(2 == from_traits::radix, "");

    static constexpr bool   to_integer =   to_traits::is_integer;
    static constexpr bool from_integer = from_traits::is_integer;

    static_assert(  to_integer ||   to_traits::is_iec559, "");
    static_assert(from_integer || from_traits::is_iec559, "");

    return bourn_cast<To,From>
        (from
        ,std::integral_constant<bool,   to_integer>{}
        ,std::integral_constant<bool, from_integer>{}
        );
}

#endif // bourn_cast_hpp

