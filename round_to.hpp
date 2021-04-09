// Rounding.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef round_to_hpp
#define round_to_hpp

#include "config.hpp"

#include "currency.hpp"
#include "mc_enum_type_enums.hpp"       // enum rounding_style
#include "stl_extensions.hpp"           // nonstd::power()

#include <cmath>                        // fabs(), floor(), rint()
#include <limits>
#include <stdexcept>
#include <type_traits>                  // is_floating_point_v
#include <vector>

// Round a floating-point number to a given number of decimal places,
// following a given rounding style.

// Power-of-ten scaling factors are best represented in the maximum
// available precision, which is indicated by type 'max_prec_real'.
//
// Change this alias-declaration to use a nonstandard type or class
// with greater precision if desired.
//
// Alternatively, suppose your hardware offers an extended format,
// but you can't or don't take advantage of it--either your compiler
// uses the same representation for double and long double, or you
// set the hardware not to do calculations in extended precision.
// If the compiler nonetheless treats double and long double as
// distinct types, then it might generate extra machine code to
// convert between those types. You could prevent that by changing
// this alias-declaration to double.
using max_prec_real = long double;

inline rounding_style& default_rounding_style()
{
    static rounding_style default_style = r_to_nearest;
    return default_style;
}

namespace detail
{
// Auxiliary rounding functions: one for each supported rounding style.
// These functions avoid changing the hardware rounding mode as long
// as the library functions they call do not change it.

// Perform no rounding at all.
template<typename RealType>
RealType round_not(RealType r)
{
    return r;
}

// Round up.
template<typename RealType>
RealType round_up(RealType r)
{
    RealType i_part = std::rint(r);
    if(i_part < r)
        {
        // Suppose the value of 'i_part' is not exactly representable
        // in type 'RealType'. Then '++' doesn't increment it; it adds
        // unity, which doesn't change the value. That's OK though: in
        // that case, this code is unreachable.
        ++i_part;
        }
    return i_part;
}

// Round down.
template<typename RealType>
RealType round_down(RealType r)
{
    RealType i_part = std::rint(r);
    if(r < i_part)
        {
        --i_part;
        }
    return i_part;
}

// Truncate.
template<typename RealType>
RealType round_trunc(RealType r)
{
    RealType i_part = std::rint(r);
    RealType f_part = r - i_part;
    // Consider the integer part 'i_part' and the fractional part
    // 'f_part': the integer part is the final answer if
    //   both parts have the same sign (drop the fractional part), or
    //   the fractional part is zero (it doesn't matter), or
    //   the integer part is zero (so ignore the fractional part).
    // If integer and fractional parts have opposite signs
    //   (one positive and one negative) then add or subtract unity to
    //   get the next integer in the direction of zero.
    if(RealType(0) < i_part && f_part < RealType(0))
        {
        --i_part;
        }
    else if(RealType(0) < f_part && i_part < RealType(0))
        {
        ++i_part;
        }
    return i_part;
}

// Round to nearest using bankers method.
template<typename RealType>
RealType round_near(RealType r)
{
    RealType i_part = std::rint(r);
    RealType f_part = r - i_part;
    RealType abs_f_part = std::fabs(f_part);

    // If      |fractional part| <  .5, ignore it;
    // else if |fractional part| == .5, ignore it if integer part is even;
    // else add sgn(fractional part).
    if
        (
           (RealType(0.5) < abs_f_part)
        || (
              RealType(0.5) == abs_f_part
           && i_part
                  != RealType(2)
                  *  std::floor(RealType(0.5) * i_part)
           )
        )
        {
        if(f_part < RealType(0))
            {
            --i_part;
            }
        else if(RealType(0) < f_part)
            {
            ++i_part;
            }
        }
    return i_part;
}

template<typename RealType>
[[noreturn]]
RealType erroneous_rounding_function(RealType)
{
    throw std::logic_error("Erroneous rounding function.");
}
} // namespace detail

template<typename RealType>
class round_to
{
    static_assert(std::is_floating_point_v<RealType>);

  public:
    /// The default ctor only makes the class DefaultConstructible;
    /// the object it creates throws on use.
    round_to() = default;
    round_to(int a_decimals, rounding_style);
    round_to(round_to const&) = default;
    round_to& operator=(round_to const&) = default;

    bool operator==(round_to const&) const;
    RealType operator()(RealType) const;
    std::vector<RealType> operator()(std::vector<RealType> const&) const;

    currency c(RealType) const;
    std::vector<currency> c(std::vector<RealType> const&) const;

    currency c(currency) const;
    std::vector<currency> c(std::vector<currency> const&) const;

    int decimals() const;
    rounding_style style() const;

  private:
    using rounding_fn_t = RealType (*)(RealType);
    rounding_fn_t select_rounding_function(rounding_style) const;

    int            decimals_          {0};
    rounding_style style_             {r_indeterminate};
    max_prec_real  scale_fwd_         {1.0};
    max_prec_real  scale_back_        {1.0};
    int            decimals_cents_    {0};
    max_prec_real  scale_fwd_cents_   {1.0};
    max_prec_real  scale_back_cents_  {1.0};
    rounding_fn_t  rounding_function_ {detail::erroneous_rounding_function};
};

// Division by an exact integer value should have slightly better
// accuracy in some cases. But profiling shows that multiplication by
// the reciprocal stored in scale_back_ makes a realistic application
// that performs a lot of rounding run about four percent faster with
// all compilers tested. TODO ?? The best design decision would be
// clearer if we quantified the effect on accuracy.

template<typename RealType>
round_to<RealType>::round_to(int a_decimals, rounding_style a_style)
    :decimals_          {a_decimals}
    ,style_             {a_style}
    ,decimals_cents_    {decimals_ - currency::cents_digits}
    ,rounding_function_ {select_rounding_function(style_)}
{
    constexpr max_prec_real one( 1.0);
    constexpr max_prec_real ten(10.0);

    if(0 <= decimals_)
        {
        scale_fwd_        = nonstd::power(ten, decimals_);
        scale_back_       = one / scale_fwd_;
        }
    else
        {
        scale_back_       = nonstd::power(ten, -decimals_);
        scale_fwd_        = one / scale_back_;
        }

    if(0 <= decimals_cents_)
        {
        scale_fwd_cents_  = nonstd::power(ten, decimals_cents_);
        scale_back_cents_ = one / scale_fwd_cents_;
        }
    else
        {
        scale_back_cents_ = nonstd::power(ten, -decimals_cents_);
        scale_fwd_cents_  = one / scale_back_cents_;
        }

    // This throws only if all use of the function object is invalid.
    // Even if it doesn't throw, there are numbers that it cannot round
    // without overflow, for instance
    //    std::numeric_limits<RealType>::max()
    // rounded to
    //    std::numeric_limits<RealType>::max_exponent10
    // decimals.
    if
        (decimals_ < std::numeric_limits<RealType>::min_exponent10
        ||           std::numeric_limits<RealType>::max_exponent10 < decimals_
        )
        {
        throw std::domain_error("Invalid number of decimals.");
        }
}

template<typename RealType>
bool round_to<RealType>::operator==(round_to const& z) const
{
    return decimals() == z.decimals() && style() == z.style();
}

// Profiling shows that inlining this member function makes a
// realistic application that performs a lot of rounding run about
// five percent faster with gcc.
template<typename RealType>
inline RealType round_to<RealType>::operator()(RealType r) const
{
    return static_cast<RealType>
        ( rounding_function_(static_cast<RealType>(r * scale_fwd_))
        * scale_back_
        );
}

template<typename RealType>
inline std::vector<RealType> round_to<RealType>::operator()
    (std::vector<RealType> const& v) const
{
    std::vector<RealType> z;
    z.reserve(v.size());
    for(auto const& i : v) {z.push_back(operator()(i));}
    return z;
}

/// Round a double explicitly; return currency.
///
/// As long as the explicit rounding was to cents, or to a power of 10
/// times cents, the result is an exact integer. For example, to round
/// 1.234 to the nearest cent:
///   1.234 * 100.0 --> 123.400000000000005684342 // r * scale_fwd_ (=100.0)
///   123.400000000000005684342 --> 123.0 // rounding_function_()
///   123.0 --> 123.0 cents // * scale_back_cents_ (=1.0)
/// or to the nearest dollar:
///   1.234 * 1.0 --> 1.229999999999999982236 // r * scale_fwd_ (=1.0)
///   1.229999999999999982236 --> 1.0 // rounding_function_()
///   1.0 --> 100.0 cents // * scale_back_cents_ (=100.0)
/// It is rounding_function_(), not static_cast<>(), that transforms
/// the floating-point argument to an exact integer value.
///
/// The reason this function exists is to intercept that integer value
/// and multiply it by a nonnegative power of ten. If operator() were
/// used instead and its result multiplied by 100, it would no longer
/// be integral--in the first example above:
///   1.234 * 100.0 --> 123.400000000000005684342 // r * scale_fwd_ (=100.0)
///   123.400000000000005684342 --> 123.0 // rounding_function_()
///   123.0 --> 1.229999999999999982236 // * scale_back_ (=0.01)
///   1.229999999999999982236 * 100.0 --> nonintegral

template<typename RealType>
inline currency round_to<RealType>::c(RealType r) const
{
    RealType const z = static_cast<RealType>
        ( rounding_function_(static_cast<RealType>(r * scale_fwd_))
        * scale_back_cents_
        );
    // CURRENCY !! static_cast: possible range error
    return currency(static_cast<currency::data_type>(z), raw_cents {});
}

template<typename RealType>
inline std::vector<currency> round_to<RealType>::c
    (std::vector<RealType> const& v) const
{
    std::vector<currency> z;
    z.reserve(v.size());
    for(auto const& i : v) {z.push_back(c(i));}
    return z;
}

// CURRENCY !! need unit tests

/// Round currency to a potentially different precision.
///
/// In practice, lmi rounds almost all currency values to cents, and
/// rounding again to cents appropriately does nothing. But it rounds
/// some currency values to dollars (as configured in a '.rounding'
/// file that can be edited); rounding eleven cents to the nearest
/// dollar, e.g., must change the value.
///
/// This implementation does that as follows:
///   11 cents --> 0.11 (double)
///   0.11 --> 0 dollars (nearest)
/// Roundoff error in the first step doesn't matter. The critical
/// points for all rounding directions are some whole number plus
/// zero or one-half, which involve no roundoff error.

template<typename RealType>
inline currency round_to<RealType>::c(currency z) const
{
    return (decimals_ < currency::cents_digits) ? c(z.d()) : z;
}

template<typename RealType>
inline std::vector<currency> round_to<RealType>::c
    (std::vector<currency> const& v) const
{
    std::vector<currency> z;
    z.reserve(v.size());
    for(auto const& i : v) {z.push_back(c(i));}
    return z;
}

template<typename RealType>
int round_to<RealType>::decimals() const
{
    return decimals_;
}

template<typename RealType>
rounding_style round_to<RealType>::style() const
{
    return style_;
}

// Choose the auxiliary rounding function indicated by the argument.
template<typename RealType>
typename round_to<RealType>::rounding_fn_t
round_to<RealType>::select_rounding_function(rounding_style const a_style) const
{
    if
        (  a_style == default_rounding_style()
        && a_style != r_indeterminate
        )
        {
        return std::rint;
        }

    switch(a_style)
        {
        case r_toward_zero:
            {
            return detail::round_trunc;
            }
        case r_to_nearest:
            {
            return detail::round_near;
            }
        case r_upward:
            {
            return detail::round_up;
            }
        case r_downward:
            {
            return detail::round_down;
            }
        case r_current:
            {
            return std::rint;
            }
        case r_not_at_all:
            {
            return detail::round_not;
            }
        case r_indeterminate: // fall through--always invalid
        default:
            {
            throw std::domain_error("Invalid rounding style.");
            }
        }
}

#endif // round_to_hpp
