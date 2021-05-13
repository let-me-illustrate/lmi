// Internal Revenue Code section 7702 guideline premium--unit test.
//
// Copyright (C) 2021 Gregory W. Chicares.
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

#include "gpt7702.hpp"

#include "assert_lmi.hpp"
#include "cso_table.hpp"
#include "et_vector.hpp"
#include "math_functions.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"

namespace
{
/// Convert annual mortality rates to monthly.
///
/// 7702 !! Duplicated in 'gpt_cf_triad_test.cpp': factor out.

std::vector<double> a_to_m(std::vector<double> const& q_a)
{
    std::vector<double> q_m(lmi::ssize(q_a));
    assign(q_m, apply_binary(coi_rate_from_q<double>(), q_a, 1.0 / 11.0));
    return q_m;
}

/// 1980 CSO Ultimate ANB Male Unismoke

std::vector<double> sample_q(int age)
{
    static std::vector<double> const q_a = cso_table
        (mce_1980cso
        ,oe_orthodox
        ,oe_age_nearest_birthday_ties_younger
        ,mce_male
        ,mce_unismoke
        );
    LMI_ASSERT(0 <= age && age < lmi::ssize(q_a));
    static std::vector<double> const q_m(a_to_m(q_a));
    return std::vector<double>(q_m.begin() + age, q_m.end());
}
} // Unnamed namespace.

class gpt_test
{
  public:
    static void test()
        {
        test_guideline_negative();
        }

  private:
    static void test_guideline_negative();
};

/// Validate a guideline-negative example.
///
/// Example similar to SOA textbook, page 101, Table V-4.
///                                                                     cum
///                                cum                     rejected    prems
///                      GLP       GLP        GSP forceout    pmt       paid
/// ------------------------------------------------------------------------
/// initialization   2035.42  50885.50   23883.74                   50000.00
/// GPT adjustment  -1804.87     ...     -5067.35
/// march of time             49080.63
/// decr prems paid                                                    ...
/// forceout                                        919.37          49080.63
///
/// See 'commutation_functions_test.cpp' for validated premium calculations.
///
/// GLP adjustment: // OL (textbook) vs. UL commfns: A + B - C
/// OL: 2035.42 + 3903.42 - 7743.71 = -1804.87
/// UL: 2074.40 + 3980.10 - 7900.49 = -1845.99 (each rounded)
/// UL: 2074.402884 + 3980.10414 - 7900.495224 = -1845.9882
///
/// GSP similarly:
/// OL: 23883.74 + 29453.12 - 58404.21 = -5067.35
/// UL: 24486.32 + 30225.88 - 59979.47 = -5267.27 (each rounded)
/// UL: 24486.3207 + 30225.8816 - 59979.4650 = -5267.2627

void gpt_test::test_guideline_negative()
{
    std::cout << "watch this space" << std::endl;
    LMI_ASSERT(100 == lmi::ssize(sample_q(0)));
}

int test_main(int, char*[])
{
    gpt_test::test();
    return EXIT_SUCCESS;
}
