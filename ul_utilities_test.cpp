// UL utilities--unit test.
//
// Copyright (C) 2022 Gregory W. Chicares.
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

#include "bourn_cast.hpp"
#include "materially_equal.hpp"
#include "round_to.hpp"
#include "stl_extensions.hpp"           // nonstd::power()
#include "test_tools.hpp"

#include <cfenv>                        // fesetround()
#include <cfloat>                       // DECIMAL_DIG
#include <cmath>                        // floor(), nearbyint()
#include <cstdint>                      // uint64_t
#include <string>

void test_max_modal_premium()
{
    // This affects diagnostics shown when LMI_TEST_EQUAL() fails.
    std::cout.precision(DECIMAL_DIG);

    // These are generally useful for testing.
    round_to<double> const round_down(2, r_downward);
    round_to<double> const round_near(2, r_to_nearest);
    round_to<double> const round_not (2, r_not_at_all);
    round_to<double> const round_up  (2, r_upward);

    // Demonstrate the reasonableness of using materially_equal(),
    // with its default tolerance, in rate_times_currency().

    auto test_excess_precision = [](double rate)
        {
        using uint64 = std::uint64_t;
        constexpr int radix {100'000'000};
        std::fesetround(FE_TONEAREST);
        uint64 irate = bourn_cast<uint64>(std::nearbyint(rate * radix));
        return !materially_equal(bourn_cast<double>(irate), rate * radix);
        };
    LMI_TEST(!test_excess_precision(0.0                    ));
    LMI_TEST(!test_excess_precision(0.00000001             ));
    LMI_TEST(!test_excess_precision(0.000000010000000000001));
    LMI_TEST( test_excess_precision(0.00000001000000000001 ));
    LMI_TEST( test_excess_precision(0.0000000100000000001  ));
    LMI_TEST( test_excess_precision(0.000000010000000001   ));
    LMI_TEST( test_excess_precision(0.00000001000000001    ));
    LMI_TEST( test_excess_precision(0.0000000100000001     ));
    LMI_TEST( test_excess_precision(0.000000010000001      ));
    LMI_TEST(!test_excess_precision(0.999999990000000      ));
    LMI_TEST(!test_excess_precision(0.999999990000001      ));
    LMI_TEST(!test_excess_precision(0.99999999000001       ));
    LMI_TEST(!test_excess_precision(0.9999999900001        ));
    LMI_TEST( test_excess_precision(0.999999990001         ));
    LMI_TEST( test_excess_precision(0.99999999001          ));
    LMI_TEST( test_excess_precision(0.99999999999          ));
    LMI_TEST( test_excess_precision(0.999999999999         ));
    LMI_TEST(!test_excess_precision(0.9999999999999        ));
    LMI_TEST(!test_excess_precision(0.99999999999999       ));
    LMI_TEST(!test_excess_precision(0.999999999999999      ));

    // Decimal values of certain constants.

    LMI_TEST_EQUAL(18446744073709551615ULL      , UINT64_MAX);
    LMI_TEST_EQUAL(18446744073709551615ULL      , nonstd::power(2ULL, 64) - 1);
    LMI_TEST_EQUAL(184467440737ULL              , UINT64_MAX / 100000000);
    // Same, with dollars-and-cents separators:
    LMI_TEST_EQUAL(184'467'440'737'095'516'15ULL, UINT64_MAX);
    LMI_TEST_EQUAL(1'844'674'407'37ULL          , UINT64_MAX / 100000000);
    // To calculate cents_limit, use integer division, which truncates.
    // Mixed-mode arithmetic with explicit downward rounding gives the
    // same outcome, but the calculation requires extended precision:
//  LMI_TEST_EQUAL(184467440737ULL   , std::floor(UINT64_MAX / 1.0e8)); // error
    LMI_TEST_EQUAL(184467440737ULL   , std::floor(UINT64_MAX / 1.0e8L));

    // Surprising but correct values seen in original development.

    // Unsigned integers don't overflow; their arithmetic is modular.
    // The only error is expecting them to behave otherwise. To verify
    // these values, use 'bc':
    //   $bc
    //   (999999999999 * 100000000) % 18446744073709551616
    //   7766279631352241920
    //   (999999999999999 * 100000000) % 18446744073709551616
    //   200376420420689664
    //   quit
    LMI_TEST_EQUAL(7766279631352241920ULL, 100000000 *     9'999'999'999'99ULL);
    LMI_TEST_EQUAL( 200376420420689664ULL, 100000000 * 9'999'999'999'999'99ULL);

    // Test limits and precondition violations.

    constexpr double   neg_rate  {-0.00000001};
    constexpr double   zero_rate { 0.00000000};
    constexpr double   low_rate  { 0.00000001};
    constexpr double   high_rate { 0.99999999};
    constexpr double   unit_rate { 1.00000000};
    constexpr double   ott_rate  { 1.00000001}; // ott: "over the top"

    constexpr currency neg_amt   {-1_cents};
    constexpr currency zero_amt  { 0_cents};
    constexpr currency low_amt   { 1_cents};
    constexpr currency mid_amt   { 100'000'00_cents};
    constexpr currency high_amt  { 999'999'999'99_cents};
    //                           1'844'674'407'37 cents_limit
    constexpr currency epic_amt {9'999'999'999'99_cents};

    // Throw if either multiplicand is negative.
    LMI_TEST_THROW
        (rate_times_currency(neg_rate , zero_amt, round_near)
        ,std::runtime_error
        ,"Assertion '0.0 <= rate' failed."
        );
    LMI_TEST_THROW
        (rate_times_currency(zero_rate, neg_amt , round_near)
        ,std::runtime_error
        ,"Assertion 'C0 <= amount' failed."
        );
    LMI_TEST_THROW
        (rate_times_currency(neg_rate , neg_amt , round_near)
        ,std::runtime_error
        ,"Assertion '0.0 <= rate' failed."
        );

    // Throw if rate is too high.
    LMI_TEST_THROW
        (rate_times_currency(ott_rate , zero_amt, round_near)
        ,std::runtime_error
        ,"Assertion 'rate <= 1.0' failed."
        );

    currency const a00 = rate_times_currency(zero_rate, zero_amt, round_near);
    LMI_TEST_EQUAL(a00, 0_cents);
    currency const a01 = rate_times_currency(zero_rate, low_amt , round_near);
    LMI_TEST_EQUAL(a01, 0_cents);
    currency const a02 = rate_times_currency(zero_rate, mid_amt , round_near);
    LMI_TEST_EQUAL(a02, 0_cents);
    currency const a03 = rate_times_currency(zero_rate, high_amt, round_near);
    LMI_TEST_EQUAL(a03, 0_cents);
    currency const a04 = rate_times_currency(zero_rate, epic_amt, round_near);
    LMI_TEST_EQUAL(a04, 0_cents);

    currency const a10 = rate_times_currency(low_rate , zero_amt, round_near);
    LMI_TEST_EQUAL(a10, 0_cents);
    currency const a11 = rate_times_currency(low_rate , low_amt , round_near);
    LMI_TEST_EQUAL(a11, 0_cents);
    currency const a12 = rate_times_currency(low_rate , mid_amt , round_near);
    LMI_TEST_EQUAL(a12, 0_cents);
    currency const a13 = rate_times_currency(low_rate , high_amt, round_near);
    LMI_TEST_EQUAL(a13, 10'00_cents);
    currency const a14 = rate_times_currency(low_rate , epic_amt, round_near);
    LMI_TEST_EQUAL(a14, 100'00_cents);

    currency const a20 = rate_times_currency(high_rate, zero_amt, round_near);
    LMI_TEST_EQUAL(a20, 0_cents);
    currency const a21 = rate_times_currency(high_rate, low_amt , round_near);
    LMI_TEST_EQUAL(a21, 1_cents);
    currency const a22 = rate_times_currency(high_rate, mid_amt , round_near);
    LMI_TEST_EQUAL(a22, 100'000'00_cents);
    currency const a23 = rate_times_currency(high_rate, high_amt, round_near);
    LMI_TEST_EQUAL(a23, 999'999'989'99_cents);
    currency const a24 = rate_times_currency(high_rate, epic_amt, round_near);
    LMI_TEST_EQUAL(a24, 9'999'999'899'99_cents);

    currency const a30 = rate_times_currency(unit_rate, zero_amt, round_near);
    LMI_TEST_EQUAL(a30, 0_cents);
    currency const a31 = rate_times_currency(unit_rate, low_amt , round_near);
    LMI_TEST_EQUAL(a31, 1_cents);
    currency const a32 = rate_times_currency(unit_rate, mid_amt , round_near);
    LMI_TEST_EQUAL(a32, 100'000'00_cents);
    currency const a33 = rate_times_currency(unit_rate, high_amt, round_near);
    LMI_TEST_EQUAL(a33, 999'999'999'99_cents);
    currency const a34 = rate_times_currency(unit_rate, epic_amt, round_near);
    LMI_TEST_EQUAL(a34, 9'999'999'999'99_cents);

    // Elucidate an example from an actual regression test:
    //   0.00000250 specified-amount load
    //      $250000 specified amount
    // In binary64 arithmetic, this is
    //   0.00000250 * 250000 = 0.625 (approximately!)
    // which is to be rounded to the nearest or even cent; 62 and 63 cents are
    // equally near, but 62 is even, so the answer should be 62.
    //
    // However, for the i686+x87 architecture still used in production for now,
//  LMI_TEST_EQUAL(0.62, round_near(0.00000250 * 250'000'00_cents)); // failed
    // the answer was sixty-three cents, evidently because of representation
    // error in "0.00000250", which is eradicated by shifting the decimal point
    // rightward to produce a rational number with a power-of-ten denominator.
    // In the worst regression between i686 and x86_64, this difference of one
    // cent in a monthly deduction grew to $79.19 at compound interest over 75
    // years.
    currency m00 = max_modal_premium
        (0.00000250
        ,250'000'00_cents
        ,mce_annual
        ,round_near
        );
    LMI_TEST_EQUAL(62_cents, m00);

    // Example of fallback to floating point, from an actual regression test:
    //     184467440737 = ⌊UINT64_MAX / 1.0e8⌋
    //   $ 1844674407.37 cents_limit, as dollars and cents
    //   $24534504428.00 amount (exceeds cents_limit)
    //    0.055394150000000003109 rate
    //     1359068018.46029639244 amount * rate: floating-point calculation
    //     1359068018.46029620    amount * rate: exact fixed-point answer
    currency n00 = rate_times_currency
        (0.05539415
        ,24534504428'00_cents
        ,round_near
        );
    LMI_TEST_EQUAL(1'359'068'018'46_cents, n00);
    // The 1359068018.46 rounded-near answer that would ideally be desired
    // is representable as an exact number of cents, and the product
    //   5539415 * 24534504428 = 135906801846029620
    // actually can be calculated exactly in 64-bit integer arithmetic:
    LMI_TEST_EQUAL  (135906801846029620ULL, 5539415ULL * 24534504428ULL);
    //             18446744073709551615ULL = UINT64_MAX
    LMI_TEST        (135906801846029620ULL < UINT64_MAX);
    // and can even be divided by 1000000 to get (truncated) cents:
    LMI_TEST_EQUAL  (135906801846ULL, 5539415ULL * 24534504428ULL / 1000000);
    // but that exceeds the precision of binary64 arithmetic:
    //                 9007199254740991 = (1ULL << 53) - 1
    // (so 135906801846029616 is the (inaccurate) answer that a spreadsheet
    // would most likely offer. This naive test:
//  LMI_TEST_UNEQUAL(135906801846029620.0, 5539415.0 * 24534504428.0);
    // may succeed or fail, but this one should be more accurate:
    std::string product_str = std::to_string(5539415.0 * 24534504428.0);
    LMI_TEST_UNEQUAL("135906801846029620", product_str);
    // ).
    //
    // Given that an exact integer calculation can be performed in
    // this case, why is that not done? The reason is that the choice
    // is made using a fast but slightly coarse test that ignores the
    // value of 'rate' and the actual precision of 'amount', regarding
    // the problem:
    //   $24534504428.00 amount
    //        0.05539415 rate
    // as though it were:
    //   $24534504428.99 amount (exceeds cents_limit)
    //        0.99999999 rate
    // which cannot be performed in 64-bit integer arithmetic, as may
    // be grasped intuitively by concatenating the significant digits
    // and comparing to UINT64_MAX:
    //    245345044289999999999 concatenation
    //    \--amount---/\-rate-/
    //     18446744073709551615 UINT64_MAX

    // Test a rate and a specamt that use maximal precision,
    // because so many real-world examples are along the lines of
    // 0.01 * $100,000 .

    constexpr double   rate    {0.0123456700000001};
    constexpr currency specamt {9'876'543'21_cents};

    LMI_TEST(materially_equal(12193254.3211401, rate * specamt.cents()));

    currency p01 = max_modal_premium(rate, specamt, mce_annual , round_down);
    LMI_TEST_EQUAL(121'932'54_cents, p01);
    currency p02 = max_modal_premium(rate, specamt, mce_annual , round_near);
    LMI_TEST_EQUAL(121'932'54_cents, p02);
    currency p03 = max_modal_premium(rate, specamt, mce_annual , round_up  );
    LMI_TEST_EQUAL(121'932'55_cents, p03);

    LMI_TEST(materially_equal(10161.045267617, rate * specamt.cents() / 1200));
    // Annual premium 'p01' is already rounded down to cents.
    // Monthly premium is derived from annual.
    LMI_TEST(materially_equal(10161.0450, p01 / 12));

    currency p04 = max_modal_premium(rate, specamt, mce_monthly, round_down);
    LMI_TEST_EQUAL( 10'161'04_cents, p04);
    currency p05 = max_modal_premium(rate, specamt, mce_monthly, round_near);
    LMI_TEST_EQUAL( 10'161'04_cents, p05);
    // Rounding direction pertains to annual, not monthly.
    // Monthly is always rounded down, to preserve the
    //   12 * monthly <= annual
    // invariant. Therefore, instead of
    //   X/12, rounded up,
    // this is
    //   (X, rounded down) / 12, discarding the remainder.
    currency p06 = max_modal_premium(rate, specamt, mce_monthly, round_up  );
    LMI_TEST_EQUAL( 10'161'04_cents, p06);

    // Real-world examples from system test.

    currency q00 = max_modal_premium
        (0.0195527999999999986536, 1'000'000'00_cents, mce_annual , round_down);
    LMI_TEST_EQUAL(19'552'80_cents, q00);

    currency q01 = max_modal_premium
        (0.0195527999999999986536, 2'000'000'00_cents, mce_annual , round_down);
    LMI_TEST_EQUAL(39'105'60_cents, q01);

    currency q02 = max_modal_premium
        (0.0193523999999999987698, 1'000'000'00_cents, mce_monthly, round_down);
    LMI_TEST_EQUAL( 1'612'70_cents, q02);

    currency q03 = max_modal_premium
        (0.0128891999999999999627,   500'000'00_cents, mce_monthly, round_down);
    LMI_TEST_EQUAL(   537'05_cents, q03);

    currency q04 = max_modal_premium
        (0.0128891999999999999627, 1'000'000'00_cents, mce_monthly, round_down);
    LMI_TEST_EQUAL( 1'074'10_cents, q04);

    currency q05 = max_modal_premium
        (0.0105983999999999991409,    50'000'00_cents, mce_annual , round_down);
    LMI_TEST_EQUAL(   529'92_cents, q05);

    currency q06 = max_modal_premium
        (0.0169656000000000008188,   250'000'00_cents, mce_annual , round_down);
    LMI_TEST_EQUAL( 4'241'40_cents, q06);

    currency q07 = max_modal_premium
        (0.0169656000000000008188,   250'000'00_cents, mce_monthly, round_down);
    LMI_TEST_EQUAL(   353'45_cents, q07);

    currency q08 = max_modal_premium
        (0.0169656000000000008188, 1'000'000'00_cents, mce_annual , round_down);
    LMI_TEST_EQUAL(16'965'60_cents, q08);

    currency q09 = max_modal_premium
        (0.0382740000000000024638, 2'100'000'00_cents, mce_annual , round_down);
    LMI_TEST_EQUAL(80'375'40_cents, q09);
}

int test_main(int, char*[])
{
    test_max_modal_premium();

    return EXIT_SUCCESS;
}
