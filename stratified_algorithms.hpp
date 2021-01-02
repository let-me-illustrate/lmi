// Numerical algorithms for stratified rates.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef stratified_algorithms_hpp
#define stratified_algorithms_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "miscellany.hpp"               // minmax

#include <algorithm>                    // is_sorted(), upper_bound()
#include <vector>

/// Numerical algorithms for stratified rates.
///
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
///   0.01 infinity   <-- third  bracket: [5000, infinity)
///
/// Limits are constrained to be positive and nondecreasing. The first
/// bracket extends from zero (implicitly) to the first limit. The last
/// limit must be positive infinity.
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
// 'tiered' and 'banded' limits are expressed differently: as
// incremental and cumulative limits, respectively. This may confuse
// the careless reader, but presents limits to the product-editor user
// in the least astonishing way.

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
/// Use positive infinity as the last element of 'incremental_limits'
/// in order to apply the last element of 'rates' to any excess over
/// the penultimate element of 'incremental_limits'.

template<typename T>
struct tiered_product
{
    T operator()
        (T const&              new_incremental_amount
        ,T const&              prior_total_amount
        ,std::vector<T> const& incremental_limits
        ,std::vector<T> const& rates
        ) const;
};

/// Amount times tiered rates.
///
/// Throws on precondition violation.
///
/// Preconditions:
///
/// Both scalar 'amount' arguments are nonnegative.
///
/// 'incremental_limits' is nonempty.
///
/// 'rates' has the same size as 'incremental_limits'; its elements
/// are unconstrained.
///
/// Elements of 'incremental_limits' are nonnegative and not all zero.
///
/// Rationale: Users may wish to suppress a bracket experimentally by
/// making its range temporarily empty without actually deleting it.

template<typename T>
T tiered_product<T>::operator()
    (T const&              new_incremental_amount
    ,T const&              prior_total_amount
    ,std::vector<T> const& incremental_limits
    ,std::vector<T> const& rates
    ) const
{
    // Cache T(0) in case it's expensive to construct.
    T const zero = T(0);

    LMI_ASSERT(zero <= new_incremental_amount);
    LMI_ASSERT(zero <= prior_total_amount);
    LMI_ASSERT(!incremental_limits.empty());
    LMI_ASSERT(rates.size() == incremental_limits.size());

    minmax<T> extrema(incremental_limits);
    LMI_ASSERT(zero <= extrema.minimum());
    LMI_ASSERT(zero <  extrema.maximum());

    T result = zero;
    T remaining_amount = new_incremental_amount;
    T unused_prior_amount = prior_total_amount;
    for
        (typename std::vector<T>::size_type j = 0
        ;j < incremental_limits.size()
        ;++j
        )
        {
        T unfilled_band_increment = incremental_limits[j] - unused_prior_amount;
        unused_prior_amount -= incremental_limits[j];
        unused_prior_amount = std::max(zero, unused_prior_amount);
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
    // Cache T(0) in case it's expensive to construct.
    T const zero = T(0);

    T product = tiered_product<T>()(amount, zero, incremental_limits, rates);
    T result = rates.at(0);
    if(zero != amount)
        {
        result = product / amount;
        }
    return result;
}

/// Banded rate for a given amount.
///
/// Like banded_product, but returns rate rather than product.
///
/// Brackets are specified by cumulative (not incremental) limits.

template<typename T>
struct banded_rate
{
    T operator()
        (T const&              total_amount
        ,std::vector<T> const& cumulative_limits
        ,std::vector<T> const& rates
        ) const;
};

/// Banded rate for a given amount.
///
/// Throws on precondition violation.
///
/// Preconditions:
///
/// 'total_amount' is nonnegative.
///
/// 'cumulative_limits' is nonempty.
///
/// 'rates' has the same size as 'cumulative_limits'; its elements
/// are unconstrained.
///
/// Elements of 'cumulative_limits' are nonnegative and not all zero;
/// they are nondecreasing, though not necessarily increasing.
///
/// Rationale: Users may wish to suppress a bracket experimentally by
/// making its range temporarily empty without actually deleting it.

template<typename T>
T banded_rate<T>::operator()
    (T const&              total_amount
    ,std::vector<T> const& cumulative_limits
    ,std::vector<T> const& rates
    ) const
{
    // Cache T(0) in case it's expensive to construct.
    T const zero = T(0);

    LMI_ASSERT(zero <= total_amount);
    LMI_ASSERT(!cumulative_limits.empty());
    LMI_ASSERT(rates.size() == cumulative_limits.size());

    minmax<T> extrema(cumulative_limits);
    LMI_ASSERT(zero <= extrema.minimum());
    LMI_ASSERT(zero <  extrema.maximum());

    std::vector<T> const& z(cumulative_limits);
    LMI_ASSERT(std::is_sorted(z.begin(), z.end()));

    // Ignore the last limit. It's asserted elsewhere to be infinity.
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
/// The implementation sedulously avoids catastrophic cancellation.
/// Where it presently does this:
///   if(a == r)     {        a = r = zero;}
///   else if(a < r) {a -= r;     r = zero;}
///   else           {r -= a; a     = zero;}
/// an earlier version did this instead:
///   T z(std::max(a, r));
///   a -= z;
///   r -= z;
/// which often caused neither a nor r to equal zero exactly.
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
    T const zero = T(0); // Cache T(0) in case it's expensive to construct.
    T r(delta);          // Return value.
    if(zero == r)
        {
        return r;
        }

    // To the extent that a negative delta (a positive increment) is
    // available, use it to bring any negative account to zero, in
    // account-preference order.

    if(a < zero && r < zero)
        {
        if(a == r)     {        a = r = zero;}
        else if(a < r) {a -= r;     r = zero;} // |r| < |a|
        else           {r -= a; a     = zero;}
        }
    if(b < zero && r < zero)
        {
        if(b == r)     {        b = r = zero;}
        else if(b < r) {b -= r;     r = zero;} // |r| < |b|
        else           {r -= b; b     = zero;}
        }

    // Apply any remaining negative delta (a positive increment) to
    // the preferred account.

    if(r < zero)  {a -= r; r = zero;}
    LMI_ASSERT(zero <= r);

    // To the extent that positive account balances are available,
    // reduce them by any remaining positive delta (decrement) in
    // account-preference order, but don't make any account balance
    // negative.

    if(zero < a && zero < r)
        {
        if(a == r)     {        a = r = zero;}
        else if(r < a) {a -= r;     r = zero;} // |r| < |a|
        else           {r -= a; a     = zero;}
        }
    if(zero < b && zero < r)
        {
        if(b == r)     {        b = r = zero;}
        else if(r < b) {b -= r;     r = zero;} // |r| < |b|
        else           {r -= b; b     = zero;}
        }
    LMI_ASSERT(zero <= r);

    // Return any remaining decrement that couldn't be applied because
    // both accounts have already been reduced to zero.

    return r;
}

#endif // stratified_algorithms_hpp
