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

// $Id: stratified_algorithms.hpp,v 1.2 2005-08-22 14:49:13 chicares Exp $

#ifndef stratified_algorithms_hpp
#define stratified_algorithms_hpp

#include "config.hpp"

#include "alert.hpp"

#include <boost/detail/algorithm.hpp> // boost::is_sorted()

#include <functional>
#include <vector>

// Miscellaneous numerical routines and functors.

/// Definitions: the functors in this library distinguish 'tiered' from
/// 'banded' rates and values. The vernacular lacks precise terms for
/// these two important and distinct concepts, so those and other terms
/// are given special definitions here.
///
/// 'bracket': A row in a schedule of rates such as this:
///
///        cumulative
///   rate   limit
///   0.05    1000    <-- first  bracket: [   0, 1000)
///   0.02    5000    <-- second bracket: [1000, 5000)
///   0.01 infinity   <-- third  bracket: [5000, infinity]
///
/// Limits are constrained to be positive and nondecreasing. The first
/// bracket extends from zero (implicitly) to the first limit. The last
/// limit is implicitly infinite.
///
/// It is convenient to characterize brackets by their upper limits.
/// Brackets may also be characterized in terms of incremental rather
/// than cumulative limits: {1000, 4000, infinity} are the incremental
/// limits for this schedule.
///
/// Consider two different ways of applying that schedule of rates to
/// amounts of 900, 1500, and 10000.
///
/// 'tiered': A succession of rates applies incrementally to a
/// progressive series of slices that partition the full amount.
///     900 -->  45 = 0.05 *  900
///    1500 -->  60 = 0.05 * 1000 + 0.02 * (1500 - 1000)
///   10000 --> 180 = 0.05 * 1000 + 0.02 * (5000 - 1000) + 0.01 * (10000 - 5000)
/// Here, the function is continuous, and also monotonically increasing
/// (x <= y ==> f(x) <= f(y)) if the rates are all nonnegative and the
/// limits increasing, as is typically the case.
///
/// 'banded': A single scalar rate applies to the full amount.
///     900 -->  45 = 0.05 *   900
///    1500 -->  30 = 0.02 *  1500
///   10000 --> 100 = 0.01 * 10000
/// Here, 1500 is above the first limit, but below the second limit,
/// so it's in the second bracket. The second rate is applied to the
/// entire amount of 1500. The function is not monotone except in the
/// degenerate case of uniformly equal rates.

namespace tiered_and_banded_rates{} // doxygen workaround.

// TODO ?? Shortcomings:
//
// Unit tests lacking for 'tiered_product' and 'tiered_rate'.
// 'TieredGrossToNet' and 'TieredNetToGross' lack unit tests and
// probably should be template functions.
//
// Inline comments point out several specific defects.
//
// Documentation could be improved.
//
// TieredGrossToNet() and TieredNetToGross() seem to be two-bracket
// specializations that should be replaced by something more generic.
//
// 'tiered_product' and 'tiered_rate' masquerade as binary functions,
// but they are not. The arguments could be grouped in std::pair
// instances if binary functions are really needed, but probably it
// would be better not to derive from std::binary_function at all.

/// Determine net amount after subtracting a tiered proportion.
/// Example use: convert gross premium to net premium.
/// Only two tiers are allowed--more would require vector arguments.

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

/// Amount times tiered rates.
///
/// Brackets are specified by incremental (not cumulative) limits.
///
/// TODO ?? This should be done implicitly:
/// Use std::numeric_limits<T>::max() as the last element of
/// 'incremental_limits' in order to apply the last element of 'rates'
/// to any excess over the penultimate element of 'incremental_limits'.

template<typename T>
struct tiered_product
    :public std::binary_function<T, T, T>
{
    T operator()
        (T const&              new_incremental_amount
        ,T const&              prior_total_amount
        ,std::vector<T> const& incremental_limits
        ,std::vector<T> const& rates
        ) const;
};

template<typename T>
T tiered_product<T>::operator()
    (T const&              new_incremental_amount
    ,T const&              prior_total_amount
    ,std::vector<T> const& incremental_limits
    ,std::vector<T> const& rates
    ) const
{
    LMI_ASSERT(incremental_limits.size() == rates.size());
    // We don't assert that 'incremental_limits' increase or that
    // 'rates' decrease.
    // TODO ?? Is this correct if they don't?
    // TODO ?? Must we assert that arguments are positive?

    // Cache T(0) in case it's expensive to construct.
    T const zero = T(0);

    T result = zero;
    T remaining_amount = new_incremental_amount;
    for
        (typename std::vector<T>::size_type j = 0
        ;j < incremental_limits.size()
        ;++j
        )
        {
        T unfilled_band_increment = incremental_limits[j] - prior_total_amount;
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
            result += rates[j] * remaining_amount;
            break;
            }
        else
            {
            result += rates[j] * unfilled_band_increment;
            remaining_amount -= unfilled_band_increment;
            }
        }

    return result;
}

/// Like tiered_product, but returns aggregate rate rather than product.

template<typename T>
struct tiered_rate
    :public std::binary_function<T, T, T>
{
    T operator()
        (T const& amount
        ,std::vector<T> const& incremental_limits
        ,std::vector<T> const& rates
        ) const;
};

template<typename T>
T tiered_rate<T>::operator()
    (T const& amount
    ,std::vector<T> const& incremental_limits
    ,std::vector<T> const& rates
    ) const
{
    T zero = T(0);
    T product = tiered_product<T>()(amount, zero, incremental_limits, rates);
    // TODO ?? Mustn't we assert that size() is nonzero?
    T result = rates[0];
    if(amount != T(0))
        {
        result = product / amount;
        }
    return result;
}

/// Like banded_product, but returns rate rather than product.

template<typename T>
struct banded_rate
    :public std::binary_function<T, T, T>
{
    T operator()
        (T const&              total_amount
        ,std::vector<T> const& incremental_limits
        ,std::vector<T> const& rates
        ) const;
};

template<typename T>
T banded_rate<T>::operator()
    (T const&              total_amount
    ,std::vector<T> const& cumulative_limits
    ,std::vector<T> const& rates
    ) const
{
    LMI_ASSERT
        (boost::is_sorted
            (cumulative_limits.begin()
            ,cumulative_limits.end()
            )
        );
    LMI_ASSERT(0 <= total_amount);

    // Don't assert that 'rates' decrease: it might seem weird if
    // they don't, but there's no reason to forbid it.

    // TODO ?? This is ghastly. As designed, the last limit must
    // exist, and it must equal std::numeric_limits<T>::max(); but
    // comparing that value to a stored copy for exact equality has
    // an indeterminate result. As a temporary workaround here, the
    // last value is ignored.
    std::vector<double>::const_iterator band = std::upper_bound
        (cumulative_limits.begin()
        ,cumulative_limits.end() - 1
        ,total_amount
        );
    return rates[band - cumulative_limits.begin()];
}

/// Amount times tiered rates.
///
/// Brackets are specified by cumulative (not incremental) limits.

template<typename T>
struct banded_product
    :public std::binary_function<T, T, T>
{
    T operator()
        (T const&              total_amount
        ,std::vector<T> const& cumulative_limits
        ,std::vector<T> const& rates
        ) const;
};

template<typename T>
T banded_product<T>::operator()
    (T const&              total_amount
    ,std::vector<T> const& cumulative_limits
    ,std::vector<T> const& rates
    ) const
{
    return
            total_amount
        *   banded_rate<double>()(total_amount, cumulative_limits, rates)
        ;
}

/// Progressively limit 'a' and 'b' such that their sum does not
/// exceed 'limit', taking any required reduction from 'a' first,
/// but not decreasing either 'a' or 'b' to less than zero.
///
/// Preconditions:
///   0 <= limit
/// Postconditions:
///   a <= its original value
///   b <= its original value
///   a+b <= limit

template<typename T>
void progressively_limit(T& a, T& b, T const& limit)
{
    // Cache T(0) in case it's expensive to construct.
    T const zero = T(0);
    LMI_ASSERT(zero <= limit);
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

/// Progressively alter a and b, attempting to reduce their sum by
/// delta, satisfying as many of the following desiderata as
/// possible in the order given:
///   0 <= a (increasing a if a < 0)
///   0 <= b (increasing b if b < 0)
///   a + b == original value of (a + b) - delta, altering a first
/// and return [original value of (a + b) - delta] - (a + b).
///
/// Motivation: Universal life insurance policies, in the general form
/// that interests us for illustrations, have a 'separate' account and
/// a 'general' account for variable and fixed funds respectively.
/// Periodic deductions must be apportioned between these two types of
/// accounts. Most often, deductions are taken in proportion to the
/// balance in each account, but this routine instead supports a
/// different approach that prefers to take all deductions, to the
/// extent possible, from the account designated by the first argument
/// (the 'preferred' account).
///
/// In the simplest and most common case, both funds are positive, as
/// is the deduction, and the deduction is taken from the preferred
/// account until it is depleted (becomes zero), any remainder being
/// taken from the other account until it is depleted, and any final
/// remainder becoming the return value. In this case, neither account
/// is made negative here. If the return value is not zero, then the
/// calling function might debit it from a distinct 'deficit' account,
/// or from one of the accounts given here as arguments; that operation
/// is not performed here because it would complicate this routine and
/// make it less flexible--for instance, the return value might be
/// debited downstream from either the preferred or the other account,
/// and this routine doesn't need to know which.
///
/// Accordingly, there is no restriction on the sign of either account-
/// balance argument. (Probably it's impossible for a separate account
/// to have a negative balance, but this routine doesn't know which
/// argument that might be.) And the sign of delta is unrestricted in
/// order to accommodate negative charges, which may conceivably arise:
/// for instance, a loan normally occasions a reduction, and it may be
/// convenient to treat a loan repayment as a negative reduction.
///
/// In this more general sign-unrestricted case, any negative balance
/// must first be increased to zero if possible. Consider:
///     0 separate account (argument a--the preferred account)
///   -10 general account  (argument b)
///   -30 delta (a negative delta is a positive increment)
/// The preference order suggests debiting -30 (crediting 30) to the
/// separate account, but the general account must first be increased
/// to zero, resulting in
///    20 separate account
///     0 general account
///     0 return value
/// In the situation
///   -20 separate account (argument a--the preferred account)
///   -10 general account  (argument b)
///   -25 delta (a negative delta is a positive increment)
/// the preferred account would first be brought to zero, then the
/// other account would be made as nonnegative as possible, with result
///     0 separate account
///    -5 general account
///     0 return value
/// [Note: The alternative of forcing both accounts to be nonnegative,
/// transforming input
///   -20 argument a
///   -10 argument b
///     0 delta
/// into output
///     0 argument a
///     0 argument b
///   -30 return value
/// was considered and rejected because it destroys information.
/// The predisposition to suppose that one of the arguments represents
/// a separate account that can never be negative was considered an
/// insufficient reason to write this routine less generically, even
/// though it would be a significant simplification. --end note]
///
/// Preconditions:
///   None. In particular, there is no restriction on the algebraic
///   sign of a, b, or delta.
/// Postconditions:
///   a + b - return-value == original value of (a + b) - delta
///   0 <= return value

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
        T za(std::min(std::max(zero, a), r));
        a -= za;
        r -= za;
        if(zero < r)
            {
            T zb(std::min(std::max(zero, b), r));
            b -= zb;
            r -= zb;
            }
        }

    // In a precise number system, we could now assert:
    //   LMI_ASSERT(zero <= r);
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
    //   LMI_ASSERT(materially_equal(original_sum, a + b - r));

    return r;
}

#endif // stratified_algorithms_hpp

