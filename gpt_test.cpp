// Internal Revenue Code section 7702 guideline premium--unit test.
//
// Copyright (C) 2013 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "gpt_commutation_functions.hpp"

#include "math_functors.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

namespace
{
/// Convert annual mortality rates to monthly.

std::vector<double> a_to_m(std::vector<double> const& q_a)
{
    std::vector<double> q_m(q_a.size());
    assign(q_m, apply_binary(coi_rate_from_q<double>(), q_a, 1.0 / 11.0));
    return q_m;
}

/// SOA database table 42: "1980 US CSO Male Age nearest".

std::vector<double> const& sample_q(int age)
{
    static int const n = 100;
    static double const q[n] =
        //     0        1        2        3        4        5        6        7        8        9
        {0.00418, 0.00107, 0.00099, 0.00098, 0.00095, 0.00090, 0.00086, 0.00080, 0.00076, 0.00074 // 00
        ,0.00073, 0.00077, 0.00085, 0.00099, 0.00115, 0.00133, 0.00151, 0.00167, 0.00178, 0.00186 // 10
        ,0.00190, 0.00191, 0.00189, 0.00186, 0.00182, 0.00177, 0.00173, 0.00171, 0.00170, 0.00171 // 20
        ,0.00173, 0.00178, 0.00183, 0.00191, 0.00200, 0.00211, 0.00224, 0.00240, 0.00258, 0.00279 // 30
        ,0.00302, 0.00329, 0.00356, 0.00387, 0.00419, 0.00455, 0.00492, 0.00532, 0.00574, 0.00621 // 40
        ,0.00671, 0.00730, 0.00796, 0.00871, 0.00956, 0.01047, 0.01146, 0.01249, 0.01359, 0.01477 // 50
        ,0.01608, 0.01754, 0.01919, 0.02106, 0.02314, 0.02542, 0.02785, 0.03044, 0.03319, 0.03617 // 60
        ,0.03951, 0.04330, 0.04765, 0.05264, 0.05819, 0.06419, 0.07053, 0.07712, 0.08390, 0.09105 // 70
        ,0.09884, 0.10748, 0.11725, 0.12826, 0.14025, 0.15295, 0.16609, 0.17955, 0.19327, 0.20729 // 80
        ,0.22177, 0.23698, 0.25345, 0.27211, 0.29590, 0.32996, 0.38455, 0.48020, 0.65798, 1.00000 // 90
        };
    static std::vector<double> const q_a(q + age, q + n);
    static std::vector<double> const q_m(a_to_m(q_a));
    return q_m;
}
} // Unnamed namespace.

/// Implicitly-declared special member functions do the right thing.

class gpt_test
{
  public:
    static void test()
        {
        test_premium_calculations();
        assay_speed();
        }

  private:
    static void test_premium_calculations();
    static void assay_speed();
};

void gpt_test::test_premium_calculations()
{
}

gpt_vector_parms v_parms()
{
    static std::vector<double> zero(sample_q(0).size(), 0.0);
    gpt_vector_parms z =
        {zero // prem_load_target
        ,zero // prem_load_excess
        ,zero // policy_fee_monthly
        ,zero // policy_fee_annual
        ,zero // specamt_load_monthly
        ,zero // qab_gio_rate
        ,zero // qab_adb_rate
        ,zero // qab_term_rate
        ,zero // qab_spouse_rate
        ,zero // qab_child_rate
        ,zero // qab_waiver_rate
        };
    return z;
}

/// Instantiate GPT commutation functions.

gpt_cf_triad instantiate_cf()
{
    static std::vector<double> zero(sample_q(0).size(), 0.0);
    static unsigned int length = sample_q(0).size();
    static std::vector<double> ic(length, i_upper_12_over_12_from_i<double>()(0.07));
    static std::vector<double> ig(length, i_upper_12_over_12_from_i<double>()(0.07));
    return gpt_cf_triad(sample_q(0), ic, ig, ic, ig, v_parms());
}

void gpt_test::assay_speed()
{
    std::cout
        << "\n  Speed tests..."
        << "\n  Init parms: " << TimeAnAliquot(v_parms             )
        << "\n  Triad     : " << TimeAnAliquot(instantiate_cf      )
        << std::endl
        ;
}

int test_main(int, char*[])
{
    gpt_test::test();
    return EXIT_SUCCESS;
}

