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

#include "mc_enum_type_enums.hpp"       // enum rounding_style
#include "stl_extensions.hpp"           // nonstd::power()

#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
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

namespace detail
{
#if 1
/// Raise 'r' to the integer power 'n'.
///
/// Motivation: To raise an integer-valued real to a positive integer
/// power without any roundoff error as long as the result is exactly
/// representable. See:
///   https://lists.nongnu.org/archive/html/lmi/2016-12/msg00049.html
///
/// For negative 'n', the most accurate result possible is obtained by
/// calculating power(r, -n), and returning its reciprocal calculated
/// with the maximum available precision.
///
/// Because this template function is called only by the round_to
/// constructor, efficiency here is not crucial in the contemplated
/// typical case where a round_to object is created once and used to
/// round many numbers, whereas it is crucial to avoid roundoff error.
/// However, that does not justify gratuitous inefficiency, and the
/// use of power() here means that the number of multiplications is
/// O(log n), so this should be as fast as a library function that
/// has been optimized for accuracy.
///
/// Fails to check for overflow or undeflow, but the round_to ctor
/// does compare 'n' to the minimum and maximum decimal exponents,
/// which suffices there because its 'r' is always ten.

template<typename RealType>
RealType perform_pow(RealType r, int n)
{
    if(0 == n)
        {
        return RealType(1.0);
        }
    if(n < 0)
        {
        return max_prec_real(1.0) / nonstd::power(r, -n);
        }
    else
        {
        return nonstd::power(r, n);
        }
}

#else  // 0

/// Raise an integer-valued real to an integer power.
///
/// Motivation: calculate accurate powers of ten. See:
///   https://lists.nongnu.org/archive/html/lmi/2016-12/msg00049.html
/// Library authors often optimize pow() for integral exponents,
/// using multiplication rather than a transcendental calculation.
/// When 'r' is exactly representable, positive integral powers
/// returned by a high-quality std::pow() are likely to be exact if
/// they are exactly representable, or otherwise as accurate as they
/// can be; but for negative integral powers this integral-exponent
/// "optimization" may very well reduce accuracy, e.g., if 10^-3 is
/// calculated as (0.1 * 0.1 * 0.1). Because the positive-exponent
/// case is likely to be treated ideally by the library author, when
/// 'n' is negative this function calls std::pow() to calculate the
/// positive power and returns the reciprocal: 1 / (10 * 10 * 10)
/// in the preceding example.

template<typename RealType>
RealType perform_pow(RealType r, int n)
{
    if(0 == n)
        {
        return RealType(1.0);
        }
    else if(n < 0)
        {
        return RealType(1.0) / std::pow(r, -n);
        }
    else
        {
        return std::pow(r, n);
        }
}

#endif // 0
} // namespace detail

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
    round_to(int decimals, rounding_style style);
    round_to(round_to const&) = default;
    round_to& operator=(round_to const&) = default;

    bool operator==(round_to const&) const;
    RealType operator()(RealType r) const;
    std::vector<RealType> operator()(std::vector<RealType> r) const;

    int decimals() const;
    rounding_style style() const;

  private:
    using rounding_fn_t = RealType (*)(RealType);
    rounding_fn_t select_rounding_function(rounding_style) const;

    int decimals_                    {0};
    rounding_style style_            {r_indeterminate};
    max_prec_real scale_fwd_         {1.0};
    max_prec_real scale_back_        {1.0};
    rounding_fn_t rounding_function_ {detail::erroneous_rounding_function};
};

// Naran used const data members, reasoning that a highly optimizing
// compiler could then calculate std::pow(10.0, n) at compile time.
// Not all compilers do this. None available to the author do.
//
// Is this a design flaw? Const data members require initialization in
// the initializer-list, so this test detects a domain error only after
// it has produced the side effect of setting 'errno'. Thus, the strong
// guarantee is lost, and only the basic guarantee is provided.
//
// The guarantee could be strengthened by not throwing at all. That
// would be consistent with other math functions. But it's a shame to
// write new code that forces the user to check 'errno'.
//
// TODO ?? The data members were made non-const after profiling showed
// no penalty on four available compilers (not including Naran's).
// The code should now be reworked to provide the strong guarantee.

// Division by an exact integer value should have slightly better
// accuracy in some cases. But profiling shows that multiplication by
// the reciprocal stored in scale_back_ makes a realistic application
// that performs a lot of rounding run about four percent faster with
// all compilers tested. TODO ?? The best design decision would be
// clearer if we quantified the effect on accuracy.

template<typename RealType>
round_to<RealType>::round_to(int decimals, rounding_style a_style)
    :decimals_          {decimals}
    ,style_             {a_style}
    ,scale_fwd_         {detail::perform_pow(max_prec_real(10.0), decimals)}
    ,scale_back_        {max_prec_real(1.0) / scale_fwd_}
    ,rounding_function_ {select_rounding_function(a_style)}
{
/*
// TODO ?? This might improve accuracy slightly, but would prevent
// the data members from being const.
    if(0 <= decimals)
        {
        scale_fwd_  = detail::perform_pow(max_prec_real(10.0), decimals);
        scale_back_ = max_prec_real(1.0) / scale_fwd_;
        }
    else
        {
        scale_back_ = detail::perform_pow(max_prec_real(10.0), -decimals);
        scale_fwd_  = max_prec_real(1.0) / scale_back_;
        }
*/

    // This throws only if all use of the function object is invalid.
    // Even if it doesn't throw, there are numbers that it cannot round
    // without overflow, for instance
    //    std::numeric_limits<RealType>::max()
    // rounded to
    //    std::numeric_limits<RealType>::max_exponent10
    // decimals.
    if
        (  decimals < std::numeric_limits<RealType>::min_exponent10
        ||            std::numeric_limits<RealType>::max_exponent10 < decimals
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
        (rounding_function_(static_cast<RealType>(r * scale_fwd_)) * scale_back_
        );
}

template<typename RealType>
inline std::vector<RealType> round_to<RealType>::operator()(std::vector<RealType> r) const
{
    std::vector<RealType> z;
    z.reserve(r.size());
    for(auto const& i : r) {z.push_back(operator()(i));}
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
