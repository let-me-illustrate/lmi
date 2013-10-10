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

#include "assert_lmi.hpp"
#include "math_functors.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <vector>

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
    LMI_ASSERT(0 <= age && age < n);
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

// These could be made static members of class gpt_test, but this way
// is more convenient; see also:
//   https://www.securecoding.cert.org/confluence/display/cplusplus/MSC22-CPP.+Do+not+define+static+private+members

std::vector<double> q_m                  ;
std::vector<double> glp_ic               ;
std::vector<double> glp_ig               ;
std::vector<double> gsp_ic               ;
std::vector<double> gsp_ig               ;
std::vector<double> prem_load_target     ;
std::vector<double> prem_load_excess     ;
std::vector<double> policy_fee_monthly   ;
std::vector<double> policy_fee_annual    ;
std::vector<double> specamt_load_monthly ;
std::vector<double> qab_gio_rate         ;
std::vector<double> qab_adb_rate         ;
std::vector<double> qab_term_rate        ;
std::vector<double> qab_spouse_rate      ;
std::vector<double> qab_child_rate       ;
std::vector<double> qab_waiver_rate      ;
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

    static void initialize(int issue_age);
    static gpt_vector_parms v_parms();
    static gpt_cf_triad instantiate_cf();
};

/// Initialize GPT vector parameters.
///
/// Loads and charges are initialized with decimal-power multiples of
/// distinct primes, to make it easier to track down any discrepancy.
///
/// In order to expose problems that uniform values would mask, each
/// vector's first element is altered if necessary.

void gpt_test::initialize(int issue_age)
{
    static double const i_m_4 = i_upper_12_over_12_from_i<double>()(0.04);
    static double const i_m_6 = i_upper_12_over_12_from_i<double>()(0.06);
    q_m = sample_q(issue_age);
    int const length = q_m.size();
    glp_ic               .assign(length,   i_m_4);
    glp_ig               .assign(length,   i_m_4);
    gsp_ic               .assign(length,   i_m_6);
    gsp_ig               .assign(length,   i_m_6);
    prem_load_target     .assign(length,  0.03  );
    prem_load_excess     .assign(length,  0.02  );
    policy_fee_monthly   .assign(length,  5.0   );
    policy_fee_annual    .assign(length, 37.0   );
    specamt_load_monthly .assign(length,  0.0007);
    qab_gio_rate         .assign(length,  0.0011);
    qab_adb_rate         .assign(length,  0.0013);
    qab_term_rate        .assign(length,  0.0017);
    qab_spouse_rate      .assign(length,  0.0019);
    qab_child_rate       .assign(length,  0.0023);
    qab_waiver_rate      .assign(length,  0.0029);

    LMI_ASSERT(0 < length);
    glp_ic               [0] *= 1.01;
    glp_ig               [0] *= 1.01;
    gsp_ic               [0] *= 1.01;
    gsp_ig               [0] *= 1.01;
    prem_load_target     [0] *= 1.01;
    prem_load_excess     [0] *= 1.01;
    policy_fee_monthly   [0] *= 1.01;
    policy_fee_annual    [0] *= 1.01;
    specamt_load_monthly [0] *= 1.01;
    qab_gio_rate         [0] *= 1.01;
    qab_adb_rate         [0] *= 1.01;
    qab_term_rate        [0] *= 1.01;
    qab_spouse_rate      [0] *= 1.01;
    qab_child_rate       [0] *= 1.01;
    qab_waiver_rate      [0] *= 1.01;
}

/// Instantiate vector parameters from globals set by initialize().

gpt_vector_parms gpt_test::v_parms()
{
    gpt_vector_parms z =
        {prem_load_target
        ,prem_load_excess
        ,policy_fee_monthly
        ,policy_fee_annual
        ,specamt_load_monthly
        ,qab_gio_rate
        ,qab_adb_rate
        ,qab_term_rate
        ,qab_spouse_rate
        ,qab_child_rate
        ,qab_waiver_rate
        };
    return z;
}

/// Instantiate GPT commutation functions.

gpt_cf_triad gpt_test::instantiate_cf()
{
    return gpt_cf_triad(q_m, glp_ic, glp_ig, gsp_ic, gsp_ig, v_parms());
}

void gpt_test::test_premium_calculations()
{
}

void gpt_test::assay_speed()
{
    initialize(0);
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

