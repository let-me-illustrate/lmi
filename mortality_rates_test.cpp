// Mortality rates--unit test.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "mortality_rates.hpp"

#include "assert_lmi.hpp"
#include "materially_equal.hpp"
#include "math_functions.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"

namespace
{
/// Arbitrary q's spanning a wide range, for testing.

std::vector<double> annual_rates()
{
    static int const n = 8;
    static double const q[n] =
        {0.0
        ,0.000001
        ,0.001
        ,0.01
        ,0.1
        ,0.5
        ,0.999755859375 // 1 - 2^12
        ,1.0
        };
    return std::vector<double>(q, q + n);
}

/// Monthly rates corresponding to annual_rates().
///
/// Only fifteen digits are given: see floating_point_decimals()
/// elsewhere for a discussion.
///
/// For q = 0.000001, a monthly equivalent calculated naively as
///   qm = 1 - (1-q)^(1/12)
///   qm = qm / (1-qm)
/// diverges even in the tenth significant digit. Values given here
/// use std::expm1() and std::log1p() for better accuracy.

std::vector<double> monthly_rates()
{
    static int const n = 8;
    static double const q[n] =
        {0.0
        ,0.0000000833333784722536
        ,0.0000833785035928555
        ,0.000837878812291897
        ,0.00881870060450726
        ,0.0594630943592953
        ,1.0
        ,1.0
        };
    return std::vector<double>(q, q + n);
}
} // Unnamed namespace.

void MortalityRates::fetch_parameters(BasicValues const&) {throw "Error";}

MortalityRates::MortalityRates()
    :Length_               {0}
    ,AllowAdb_             {false}
    ,AllowChild_           {false}
    ,AllowFlatExtras_      {false}
    ,AllowSpouse_          {false}
    ,AllowSubstdTable_     {false}
    ,AllowTerm_            {false}
    ,AllowWp_              {false}
    ,CCoiIsAnnual_         {false}
    ,GCoiIsAnnual_         {false}
    ,IsTgtPremTabular_     {false}
    ,MaxMonthlyCoiRate_    {1.0}
    ,CountryCoiMultiplier_ {1.0}
    ,IsPolicyRated_        {false}
    ,SubstandardTable_     {mce_table_none}
    ,round_coi_rate_       (0, r_not_at_all)
{
}

class mortality_rates_test
{
  public:
    static void test()
        {
        LMI_ASSERT(annual_rates().size() == monthly_rates().size());
        test_4095_4096ths();
        test_annual_to_monthly_conversion();
        test_guaranteed_rates( 1.0, 1.0, round_to<double>(0, r_not_at_all));
        test_guaranteed_rates( 0.9, 1.0, round_to<double>(0, r_not_at_all));
        test_guaranteed_rates( 1.1, 1.0, round_to<double>(0, r_not_at_all));
        test_guaranteed_rates( 1.0, 0.9, round_to<double>(0, r_not_at_all));
        test_guaranteed_rates(10.0, 0.9, round_to<double>(0, r_not_at_all));
        }

  private:
    static void test_4095_4096ths();
    static void test_annual_to_monthly_conversion();
    static void test_guaranteed_rates
        (double           mult
        ,double           max
        ,round_to<double> rounder
        );
};

/// Test a calculation that ought to be exact.
///
/// 0.999755859375 should be exactly representable as long as the
/// hardware accommodates at least a twelve-bit mantissa.
///
/// If
///   q = 4095/4096
/// then
///   0.5 = 1 - (1-q)^(1/12)
/// and
///   1 = 0.5 / (1-0.5)
/// which is a boundary for coi_rate_from_q().
///
/// In this case, a test for absolute floating-point equality ought to
/// be appropriate.

void mortality_rates_test::test_4095_4096ths()
{
    static double const q = static_cast<double>(1.0L - 1.0L / 4096.0L);
    LMI_TEST_EQUAL(0.999755859375, q);
    LMI_TEST_EQUAL(1.0, coi_rate_from_q<double>()(q, 1.0));
}

void mortality_rates_test::test_annual_to_monthly_conversion()
{
    for(int j = 0; j < lmi::ssize(annual_rates()); ++j)
        {
        LMI_TEST
            (materially_equal
                (                          monthly_rates()[j]
                ,coi_rate_from_q<double>()(annual_rates ()[j], 1.0)
                )
            );
        }
}

void mortality_rates_test::test_guaranteed_rates
    (double           mult
    ,double           max
    ,round_to<double> rounder
    )
{
    MortalityRates z;
    z.Length_            = lmi::ssize(annual_rates());

    z.GCoiMultiplier_    = std::vector<double>(z.Length_, mult);
    z.MaxMonthlyCoiRate_ = max;
    z.round_coi_rate_    = rounder;

    std::cout
        << "Testing with"
        << " mult = "      << mult
        << ", max = "      << max
        << ", decimals = " << rounder.decimals()
        << ", style = "    << rounder.style()
        << ".\n"
        << std::flush
        ;

    z.GCoiIsAnnual_   = true;
    z.MonthlyGuaranteedCoiRates_ = annual_rates();
    z.SetGuaranteedRates();
    std::vector<double> v0 = z.MonthlyGuaranteedCoiRates_;

    z.GCoiIsAnnual_   = false;
    z.MonthlyGuaranteedCoiRates_ = monthly_rates();
    z.SetGuaranteedRates();
    std::vector<double> v1 = z.MonthlyGuaranteedCoiRates_;

    for(int j = 0; j < z.Length_; ++j)
        {
        double x = z.GCoiMultiplier_[j] * monthly_rates()[j];
        x = std::min(x, z.MaxMonthlyCoiRate_);
        x = rounder(x);

        double y = z.GCoiMultiplier_[j] * annual_rates()[j];
        y = coi_rate_from_q<double>()(y, z.MaxMonthlyCoiRate_);
        y = rounder(y);

        LMI_TEST(materially_equal(y, v0[j]));
        LMI_TEST(materially_equal(x, v1[j]));
// This needn't necessarily hold:
//        LMI_TEST(materially_equal(x, y));
// To compare values:
//        std::cout << j << '\t' << x << '\t' << y << std::endl;
        }
}

int test_main(int, char*[])
{
    mortality_rates_test::test();
    return EXIT_SUCCESS;
}
