// Miscellaneous mathematical routines.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: mathmisc.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef mathmisc_hpp
#define mathmisc_hpp

#include "config.hpp"

#include <cassert>
#include <functional>
#include <vector>

// Miscellaneous numerical routines and functors.

// TODO ?? Shortcomings:
//
// Unit tests lacking for 'tiered_product' and 'tiered_rate'.
// 'TieredGrossToNet' and 'TieredNetToGross' lack unit tests and
// probably should be template functions.
//
// Inline comments point out several specific defects.
//
// 'tiered_product' and 'tiered_rate' masquerade as binary functions,
// but they are not. Perhaps the arguments should be grouped in
// std::pair instances.

// Determine net amount after subtracting a tiered proportion.
// Example use: convert gross premium to net premium.
// Only two tiers are allowed--more would require vector arguments.

double TieredGrossToNet
    (double GrossAmount
    ,double Breakpoint
    ,double LoadUpToBreakpoint
    ,double LoadBeyondBreakpoint
    );

// Inverse of TieredGrossToNet()
double TieredNetToGross
    (double NetAmount
    ,double Breakpoint
    ,double LoadUpToBreakpoint
    ,double LoadBeyondBreakpoint
    );

// Multiply amount by banded multipliers. For example:
//   10% of the first 1000, plus 5% of the next 4000, plus....
// Bands are incremental, not aggregate, amounts: thus, in the example
// given, the next band would start after an aggregate  5000.
// Set last element of 'bands' to std::numeric_limits<T>::max() to
// apply the last element of 'multipliers' to any excess over the
// penultimate element of 'bands'.
template<typename T>
struct tiered_product
    :public std::binary_function<T, T, T>
{
    T operator()
        (T const&              new_incremental_amount
        ,T const&              prior_total_amount
        ,std::vector<T> const& bands
        ,std::vector<T> const& multipliers
        ) const;
};

template<typename T>
T tiered_product<T>::operator()
    (T const&              new_incremental_amount
    ,T const&              prior_total_amount
    ,std::vector<T> const& bands
    ,std::vector<T> const& multipliers
    ) const
{
    assert(bands.size() == multipliers.size());
    // We don't assert that bands increase or multipliers decrease.
    // TODO ?? Is this correct if they don't?
    // TODO ?? Must we assert that arguments are positive?

    // Cache T(0) in case it's expensive to construct.
    T const zero = T(0);

    T result = zero;
    T remaining_amount = new_incremental_amount;
    for(typename std::vector<T>::size_type j = 0; j < bands.size(); j++)
        {
        T unfilled_band_increment = bands[j] - prior_total_amount;
        if(unfilled_band_increment <= zero)
            {
            continue;
            }
        if(remaining_amount <= zero)
            {
            break;
            }
        if(remaining_amount <= unfilled_band_increment)
            {
            result += multipliers[j] * remaining_amount;
            break;
            }
        else
            {
            result += multipliers[j] * unfilled_band_increment;
            remaining_amount -= unfilled_band_increment;
            }
        }

    return result;
}

// Like tiered_product, but returns aggregate rate rather than product.
template<typename T>
struct tiered_rate
    :public std::binary_function<T, T, T>
{
    T operator()
        (T const& amount
        ,std::vector<T> const& bands
        ,std::vector<T> const& multipliers
        ) const;
};

template<typename T>
T tiered_rate<T>::operator()
    (T const& amount
    ,std::vector<T> const& bands
    ,std::vector<T> const& multipliers
    ) const
{
    T zero = T(0);
    T product = tiered_product<T>()(amount, zero, bands, multipliers);
    // TODO ?? Mustn't we assert that size() is nonzero?
    T result = multipliers[0];
    if(amount != T(0))
        {
        result = product / amount;
        }
    return result;
}

// Progressively limit 'a' and 'b' such that their sum does not
// exceed 'limit', taking any required reduction from 'a' first,
// but not decreasing either 'a' or 'b' to less than zero.
//
// Preconditions:
//   0 <= limit
// Postconditions:
//   a <= its original value
//   b <= its original value
//   a+b <= limit
//
template<typename T>
void progressively_limit(T& a, T& b, T const& limit)
{
    // Cache T(0) in case it's expensive to construct.
    T const zero = T(0);
    assert(zero <= limit);
    if(a <= zero && b <= zero)
        {
        return;
        }
    else if(a <= zero)
        {
        b = std::min(b, limit - a);
        return;
        }
    else if(b <= zero)
        {
        a = std::min(a, limit - b);
        return;
        }
    else
        {
        b = std::min(b, limit);
        a = std::min(a, limit - b);
        return;
        }
}

// Progressively alter a and b, attempting to reduce their sum by
// delta, satisfying as many of the following desiderata as
// possible in the order given:
//   0 <= a (increasing a if a < 0)
//   0 <= b (increasing b if b < 0)
//   a + b == original value of (a + b) - delta, altering a first
// and return [original value of (a + b) - delta] - (a + b).
//
// Motivation: Universal life insurance policies, in the general form
// that interests us for illustrations, have a 'separate' account and
// a 'general' account for variable and fixed funds respectively.
// Periodic deductions must be apportioned between these two types of
// accounts. Most often, deductions are taken in proportion to the
// balance in each account, but this routine instead supports a
// different approach that prefers to take all deductions, to the
// extent possible, from the account designated by the first argument
// (the 'preferred' account).
//
// In the simplest and most common case, both funds are positive, as
// is the deduction, and the deduction is taken from the preferred
// account until it is depleted (becomes zero), any remainder being
// taken from the other account until it is depleted, and any final
// remainder becoming the return value. In this case, neither account
// is made negative here. If the return value is not zero, then the
// calling function might debit it from a distinct 'deficit' account,
// or from one of the accounts given here as arguments; that operation
// is not performed here because it would complicate this routine and
// make it less flexible--for instance, the return value might be
// debited downstream from either the preferred or the other account,
// and this routine doesn't need to know which.
//
// Accordingly, there is no restriction on the sign of either account-
// balance argument. (Probably it's impossible for a separate account
// to have a negative balance, but this routine doesn't know which
// argument that might be.) And the sign of delta is unrestricted in
// order to accommodate negative charges, which may conceivably arise:
// for instance, a loan normally occasions a reduction, and it may be
// convenient to treat a loan repayment as a negative reduction.
//
// In this more general sign-unrestricted case, any negative balance
// must first be increased to zero if possible. Consider:
//     0 separate account (argument a--the preferred account)
//   -10 general account  (argument b)
//   -30 delta (a negative delta is a positive increment)
// The preference order suggests debiting -30 (crediting 30) to the
// separate account, but the general account must first be increased
// to zero, resulting in
//    20 separate account
//     0 general account
//     0 return value
// In the situation
//   -20 separate account (argument a--the preferred account)
//   -10 general account  (argument b)
//   -25 delta (a negative delta is a positive increment)
// the preferred account would first be brought to zero, then the
// other account would be made as nonnegative as possible, with result
//     0 separate account
//    -5 general account
//     0 return value
// [Note: The alternative of forcing both accounts to be nonnegative,
// transforming input
//   -20 argument a
//   -10 argument b
//     0 delta
// into output
//     0 argument a
//     0 argument b
//   -30 return value
// was considered and rejected because it destroys information.
// The predisposition to suppose that one of the arguments represents
// a separate account that can never be negative was considered an
// insufficient reason to write this routine less generically, even
// though it would be a significant simplification. --end note]
//
// Preconditions:
//   None. In particular, there is no restriction on the algebraic
//   sign of a, b, or delta.
// Postconditions:
//   a + b - return-value == original value of (a + b) - delta
//   0 <= return value

template<typename T>
T progressively_reduce(T& a, T& b, T const& delta)
{
    // Cache T(0) in case it's expensive to construct.
    T const zero(0);

    // Return value.
    T r(delta);

    if(zero == r)
        {
        return r;
        }

    if(a < zero && r < zero)
        {
        T z(std::max(a, r));
        a -= z;
        r -= z;
        }
    if(b < zero && r < zero)
        {
        T z(std::max(b, r));
        b -= z;
        r -= z;
        }

    if(r < zero)
        {
        a -= r;
        r = zero;
        }
    else
        {
        T z(std::min(std::max(zero, a), r));
        a -= z;
        r -= z;
        if(zero < r)
            {
            T z(std::min(std::max(zero, b), r));
            b -= z;
            r -= z;
            }
        }

    // In a precise number system, we could now assert:
    //   assert(zero <= r);
    // But due to the imprecision of floating-point arithmetic, that
    // could easily fail. A value close to zero, but of random sign,
    // can arise from subtraction of two nearly-identical quantities.
    // There are many subtractions in this function, and it's not
    // necessarily obvious which one caused such an outcome, so it's
    // not trivial to set a reasonable tolerance. Instead, we just let
    // negatives arise as they will, and move them into the preferred
    // account, explicitly zeroing the return value.
    if(r < zero)
        {
        a -= r;
        r = zero;
        }
    // However, r can still have a tiny positive floating-point value
    // when it would be zero in a precise number system. There really
    // isn't anything we can do to prevent that.

    // Due to the imprecision of floating-point arithmetic, we could
    // probably force an assertion like this to fire with legitimate
    // input. It would hold, though, in a precise number system.
    //
    // At top: save original sum just for assertion.
    //   T const original_sum(a + b - r);
    // Just before exit:
    //   assert(materially_equal(original_sum, a + b - r));

    return r;
}

#endif // mathmisc_hpp

