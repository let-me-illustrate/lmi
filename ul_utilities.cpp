// UL utilities.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ul_utilities.hpp"

#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "calendar_date.hpp"
#include "materially_equal.hpp"
#include "round_to.hpp"

#include <algorithm>                    // generate(), min()
#include <cfenv>                        // fesetround()
#include <cmath>                        // nearbyint(), pow()
#include <cstdint>                      // uint64_t
#include <numeric>                      // inner_product()
#include <vector>

/// Possibly off-anniversary premium to be shown on list bill.
///
/// Motivation: Group UL certificates may be issued on any date, but
/// plan sponsors want to pay for all certificates on the same date.
///
/// Calculate a certificate's discounted premium for a billing cycle,
/// during which a certificate anniversary might occur and insurance
/// age might therefore change. The billing interval is any of the
/// customary life-insurance payment modes. For the maximum billing
/// period of one year, the demographic notation used on page 129 of
/// Batten, _Mortality Table Construction_, 1978 (ISBN 0-13-601302-3)
/// would be:
///   - (delta)premium(x)   [here, prem_ante] for monthly charges at
///     age x from the bill date to the anniversary age change;
///   - (alpha)premium(x+1) [here, prem_post] for monthly charges at
///     age x+1 from the anniversary age change to the end of the
///     billing year (zero if the certificate matures at age x+1).
/// Construct a monthly vector of delta or alpha premiums as the
/// case may be, then discount them, at an interest rate given as
/// v upper twelve, for the period defined by the given mode.
///
/// Return the discounted value, which is to be rounded by the caller.
///
/// Implementation notes:
///  - p0 is a vector of premiums, one for each month in the billing
///    cycle: zero or more delta values, followed by zero or more
///    alpha values.
///  - v is a vector of discount factors v12^0, v12^1, v12^2... .

double list_bill_premium
    (double               prem_ante
    ,double               prem_post
    ,mcenum_mode          mode
    ,calendar_date const& cert_date
    ,calendar_date const& bill_date
    ,double               v12
    )
{
    // Bill nothing for a not-yet-issued certificate. Alternatively,
    // the condition tested here might be asserted.
    if(bill_date < cert_date) return 0.0;
    // Number of alpha months in the twelvemonth starting on bill date.
    int const inforce_months_mod_12 = years_and_months_since
        (cert_date, bill_date, false).second
        ;
    // Number of delta months in the twelvemonth starting on bill date.
    int const months_ante = 12 - inforce_months_mod_12;
    // Transform frequency to wavelength:
    //   mode {1, 2, 4, 12} --> m {12, 6, 3, 1}
    int const m = 12 / static_cast<int>(mode);
    std::vector<double> p0(std::min(m, months_ante), prem_ante);
    std::vector<double> p1(m - p0.size(), prem_post);
    p0.insert(p0.end(), p1.begin(), p1.end());
    int i = 0;
    std::vector<double> v(p0.size());
    std::generate(v.begin(), v.end(), [&i, v12] {return std::pow(v12, i++);});
    return std::inner_product(p0.begin(), p0.end(), v.begin(), 0.0);
}

currency rate_times_currency
    (double                  rate
    ,currency                amount
    ,round_to<double> const& rounder
    )
{
    using uint64 = std::uint64_t;

    // Precondition (asserted below): the premium-rate argument is
    // precise to at most eight decimals, any further digits being
    // representation error. In practice, eight is almost universally
    // adequate, but this is a parameter that may be adjusted if more
    // decimals are required. The accompanying unit test gives some
    // illustrative examples of this precondition's effect.
    constexpr int radix {100'000'000};
    // Premium rate and specified amount are nonnegative by their nature.
    LMI_ASSERT(0.0 <= rate);
    LMI_ASSERT(C0  <= amount);
    // Do not save and restore prior rounding direction, because lmi
    // generally expects rounding to nearest everywhere.
    std::fesetround(FE_TONEAREST);
    // Make 'rate' a shifted integer.
    // Shift the decimal point eight places, discarding anything further.
    // Store the result as a wide integer, to be used in integer math.
    // Use bourn_cast<>() for conversions here and elsewhere: it
    // implicitly asserts that values are preserved.
    uint64 irate = bourn_cast<uint64>(std::nearbyint(rate * radix));
    // If the rate really has more than eight significant (non-erroneous)
    // digits, then throw. Alternatively, one might simply
    //   return rounder.c(amount * rate);
    // in that case: i.e., treat all digits as significant, assume
    // there is no representation error to be removed, and perform
    // the multiplication in double precision (perhaps with less
    // accuracy than might otherwise be obtained).
    LMI_ASSERT(materially_equal(bourn_cast<double>(irate), rate * radix));
    // Multiply integer rate by integral-cents amount.
    // Use a large integer type to avoid overflow.
    uint64 iprod = irate * bourn_cast<uint64>(amount.cents());
    // Result is an integer--safe to represent as double now.
    // Function from_cents() has its own value-preservation test.
    currency cprod = from_cents(bourn_cast<double>(iprod));
    // Unshift the result, and round it in the specified direction.
    // Dividing two integers generally yields a nonzero remainder,
    // in which case do the division in floating point and round its
    // result. However, if the remainder of integer division is zero,
    // then the result is exact, in which case the corresponding
    // rounded floating-point division may give the wrong answer.
    uint64 quotient  = iprod / radix;
    uint64 remainder = iprod % radix;
    return
        ((0 == remainder)
        ? from_cents(bourn_cast<double>(quotient))
        : rounder.c(cprod / radix)
        );
}

currency max_modal_premium
    (double                  rate
    ,currency                specamt
    ,mcenum_mode             mode
    ,round_to<double> const& rounder
    )
{
    using uint64 = std::uint64_t;

    currency const annual_premium = rate_times_currency(rate, specamt, rounder);
    // Calculate modal premium from annual as a separate step,
    // using integer division to discard any fractional part.
    // In a sense, this is double rounding, which is often a
    // mistake, but here it's correct: the invariant
    //   mode * max_modal_premium <= max_annual premium
    // is explicitly desired. For example, if the maximum annual
    // premium is 12.30, then the monthly maximum is 1.02,
    // which is the highest level premium that can be paid twelve
    // times without exceeding the annual maximum: 12.24 <= 12.30 .
    uint64 annual_int = static_cast<uint64>(annual_premium.cents());
    return from_cents(bourn_cast<double>(annual_int / mode));
}
