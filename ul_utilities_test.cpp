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

#include "materially_equal.hpp"
#include "round_to.hpp"
#include "test_tools.hpp"

void test_max_modal_premium()
{
    round_to<double> const round_down(2, r_downward);
    round_to<double> const round_near(2, r_to_nearest);
    round_to<double> const round_not (2, r_not_at_all);
    round_to<double> const round_up  (2, r_upward);

    double   const rate    {0.0123456700000001};
    currency const specamt {9'876'543'21_cents};

    // This affects diagnostics shown when LMI_TEST_EQUAL() fails.
    std::cout.precision(21);

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
