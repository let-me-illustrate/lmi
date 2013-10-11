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
#include "ihs_irc7702.hpp"

#include "assert_lmi.hpp"
#include "materially_equal.hpp"
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
    static Irc7702& instantiate_old(int issue_age);
    static void compare_premiums(int issue_age, double target);
    static void mete_premiums();
    static void mete_instantiate_old();
    static void mete_premiums_old();
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

/// The obsolescent GPT class more or less requires this ugliness.

Irc7702* ugliness = 0;

/// Instantiate obsolescent GPT class.

Irc7702& gpt_test::instantiate_old(int issue_age)
{
#if defined LMI_COMO_WITH_MINGW
    throw "Code that uses this obsolescent class segfaults with como.";
#else // !defined LMI_COMO_WITH_MINGW
    int const length = q_m.size();
    std::vector<double> const zero(length, 0.0);
    delete ugliness;
    ugliness =
        (new Irc7702
            (mce_gpt                         // a_Test7702
            ,issue_age                       // a_IssueAge
            ,issue_age + length              // a_EndtAge
            ,q_m                             // a_Qc
            ,glp_ic                          // a_GLPic
            ,gsp_ic                          // a_GSPic
            ,glp_ic                          // a_Ig [ignored here, in effect]
            ,zero                            // a_IntDed
            ,0.0                             // a_PresentBftAmt
            ,0.0                             // a_PresentSpecAmt
            ,0.0                             // a_LeastBftAmtEver
            ,mce_option1_for_7702            // a_PresentDBOpt
            ,policy_fee_annual               // a_AnnChgPol
            ,policy_fee_monthly              // a_MlyChgPol
            ,specamt_load_monthly            // a_MlyChgSpecAmt
            ,1000000000.0                    // a_SpecAmtLoadLimit [in effect, no limit]
            ,qab_adb_rate                    // a_MlyChgADD
            ,1000000000.0                    // a_ADDLimit [in effect, no limit]
            ,prem_load_target                // a_LoadTgt
            ,prem_load_excess                // a_LoadExc
            // Plausible low default target (overridden by compare_premiums()):
            ,1000.0                          // a_TargetPremium
            ,round_to<double>(2, r_upward)   // a_round_min_premium
            ,round_to<double>(2, r_downward) // a_round_max_premium
            ,round_to<double>(0, r_upward)   // a_round_min_specamt
            ,round_to<double>(0, r_downward) // a_round_max_specamt
            ,0                               // a_InforceYear
            // Kludge to prevent Initialize7702() from calculating premiums:
            ,1                               // a_InforceMonth
            ,0.0                             // a_InforceGLP
            ,0.0                             // a_InforceCumGLP
            ,0.0                             // a_InforceGSP
            ,0.0                             // a_InforceCumPremsPaid
            )
        );
    return *ugliness;
#endif // !defined LMI_COMO_WITH_MINGW
}

/// Compare {GSP, GLP opt 1, GLP opt 2} for old and new GPT classes.
///
/// For the standard table provided by sample_q(), test all issue ages
/// and durations, with both a high and a low target because target
/// affects the algorithm significantly. Counting each premium triplet
/// as a single test, the number of tests should be (and is) the 100th
/// triangular number times two (targets): twice 101 choose two, which
/// is 2*(100*101)/2 = 10100.

void gpt_test::compare_premiums(int issue_age, double target)
{
    double const f3bft          = 120000.0;
    double const endt_bft       = 100000.0;
    double const chg_sa_amt     = 100000.0;
    double const qab_gio_amt    =      0.0;
    double const qab_adb_amt    = 100000.0;
    double const qab_term_amt   =      0.0;
    double const qab_spouse_amt =      0.0;
    double const qab_child_amt  =      0.0;
    double const qab_waiver_amt =      0.0;

    gpt_scalar_parms parms =
        {0 // duration: reset below
        ,target
        ,f3bft
        ,endt_bft
        ,chg_sa_amt
        ,qab_gio_amt
        ,qab_adb_amt
        ,qab_term_amt
        ,qab_spouse_amt
        ,qab_child_amt
        ,qab_waiver_amt
        };

    initialize(issue_age);

    gpt_cf_triad const z = instantiate_cf();

#if !defined LMI_COMO_WITH_MINGW
    // This test of the obsolescent class segfaults with como.
    Irc7702& z_old = instantiate_old(issue_age);
    // Set target (the other arguments don't matter here).
    z_old.Initialize7702(f3bft, endt_bft, mce_option1_for_7702, target);
#endif // !defined LMI_COMO_WITH_MINGW

    int const omega = sample_q(0).size();
    for(int duration = 0; duration < omega - issue_age; ++duration)
        {
        parms.duration = duration;
        double const r0 = z.calculate_premium(oe_gsp, mce_option1_for_7702, parms);
        double const r1 = z.calculate_premium(oe_glp, mce_option1_for_7702, parms);
        double const r2 = z.calculate_premium(oe_glp, mce_option2_for_7702, parms);
#if !defined LMI_COMO_WITH_MINGW
        double const r0_old = z_old.CalculateGSP(duration, f3bft, endt_bft, endt_bft                      );
        double const r1_old = z_old.CalculateGLP(duration, f3bft, endt_bft, endt_bft, mce_option1_for_7702);
        double const r2_old = z_old.CalculateGLP(duration, f3bft, endt_bft, endt_bft, mce_option2_for_7702);
        bool const all_materially_equal =
               materially_equal(r0, r0_old)
            && materially_equal(r1, r1_old)
            && materially_equal(r2, r2_old)
            ;
        BOOST_TEST(all_materially_equal);
        if(!all_materially_equal)
            {
            std::cout
                << "Discrepancy: issue_age = " << issue_age
                << " ; duration = " << duration
                << "\n  new: " << r0     << ' ' << r1     << ' ' << r2
                << "\n  old: " << r0_old << ' ' << r1_old << ' ' << r2_old
                << std::endl
                ;
            }
#endif // !defined LMI_COMO_WITH_MINGW
        }
}

void gpt_test::test_premium_calculations()
{
    int const omega = sample_q(0).size();
    for(int issue_age = 0; issue_age < omega; ++issue_age)
        {
        compare_premiums(issue_age,    1000.0);
        compare_premiums(issue_age, 1000000.0);
        }
}

void gpt_test::mete_premiums()
{
    static gpt_scalar_parms const parms =
        {     0   // duration
        ,  1000.0 // target
        ,120000.0 // f3bft
        ,100000.0 // endt_bft
        ,100000.0 // chg_sa_amt
        ,     0.0 // qab_gio_amt
        ,100000.0 // qab_adb_amt
        ,     0.0 // qab_term_amt
        ,     0.0 // qab_spouse_amt
        ,     0.0 // qab_child_amt
        ,     0.0 // qab_waiver_amt
        };
    static gpt_cf_triad const z = instantiate_cf();
    z.calculate_premium(oe_gsp, mce_option1_for_7702, parms);
    z.calculate_premium(oe_glp, mce_option1_for_7702, parms);
    z.calculate_premium(oe_glp, mce_option2_for_7702, parms);
}

/// Measure instantiation speed of old GPT class.
///
/// This simple pass-through function could have been written inline
/// with boost::bind, but it's preferable not to drag that in.

void gpt_test::mete_instantiate_old()
{
    instantiate_old(0);
}

void gpt_test::mete_premiums_old()
{
    static int    const duration =      0  ;
    static double const f3bft    = 120000.0;
    static double const endt_bft = 100000.0;
    static Irc7702 const& z = instantiate_old(duration);
    z.CalculateGSP(duration, f3bft, endt_bft, endt_bft                      );
    z.CalculateGLP(duration, f3bft, endt_bft, endt_bft, mce_option1_for_7702);
    z.CalculateGLP(duration, f3bft, endt_bft, endt_bft, mce_option2_for_7702);
}

void gpt_test::assay_speed()
{
    initialize(0);
    std::cout
        << "\n  Speed tests..."
        << "\n  Init parms: " << TimeAnAliquot(v_parms             )
        << "\n  Triad     : " << TimeAnAliquot(instantiate_cf      )
        << "\n  Prems     : " << TimeAnAliquot(mete_premiums       )
#if !defined LMI_COMO_WITH_MINGW
        << "\n  Triad old : " << TimeAnAliquot(mete_instantiate_old)
        << "\n  Prems old : " << TimeAnAliquot(mete_premiums_old   )
#endif // !defined LMI_COMO_WITH_MINGW
        << std::endl
        ;
}

int test_main(int, char*[])
{
    gpt_test::test();
    return EXIT_SUCCESS;
}

