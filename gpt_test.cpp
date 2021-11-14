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

#include <cmath>                        // fabs()
#include <vector>

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
        test_sync();
        test_1035();
        test_guideline_negative();
        test_7702_f_6();
        }

  private:
    static void test_sync();
    static void test_1035();
    static void test_guideline_negative();
    static void test_7702_f_6();
};

void gpt_test::test_sync()
{
    int const issue_age = 35;
    int const length = 100 - issue_age;

    gpt_vector_parms v_parms =
        {.prem_load_target     = std::vector<double>(length, 0.00)
        ,.prem_load_excess     = std::vector<double>(length, 0.00)
        ,.policy_fee_monthly   = std::vector<double>(length, 0.00)
        ,.policy_fee_annual    = std::vector<double>(length, 0.00)
        ,.specamt_load_monthly = std::vector<double>(length, 0.00)
        ,.qab_gio_rate         = std::vector<double>(length, 0.00)
        ,.qab_adb_rate         = std::vector<double>(length, 0.00)
        ,.qab_term_rate        = std::vector<double>(length, 0.00)
        ,.qab_spouse_rate      = std::vector<double>(length, 0.00)
        ,.qab_child_rate       = std::vector<double>(length, 0.00)
        ,.qab_waiver_rate      = std::vector<double>(length, 0.00)
        };

    std::vector<double> i20(length, i_upper_12_over_12_from_i<double>()(0.020));
    std::vector<double> i40(length, i_upper_12_over_12_from_i<double>()(0.040));

    gpt7702 z(sample_q(issue_age), i20, i20, i40, i40, v_parms);

    gpt_scalar_parms s_parms_0 =
        {.duration       =      0
        ,.f3_bft         = 100000.0
        ,.endt_bft       = 100000.0
        ,.target_prem    =      0.0
        ,.chg_sa_base    = 100000.0
        ,.dbopt_7702     = mce_option1_for_7702
        };

    z.initialize_gpt
        (mce_gpt // defn_life_ins
        ,0.0     // fractional_duration
        ,0.0     // inforce_glp
        ,0.0     // inforce_cum_glp
        ,0.0     // inforce_gsp
        ,C0      // inforce_cum_f1A
        ,s_parms_0
        );
    // 7702 !! Should it be permissible to initialize twice?
    z.initialize_gpt
        (mce_gpt // defn_life_ins
        ,0.0     // fractional_duration
        ,0.0     // inforce_glp
        ,0.0     // inforce_cum_glp
        ,0.0     // inforce_gsp
        ,C0      // inforce_cum_f1A
        ,s_parms_0
        );
    z.update_gpt(s_parms_0, 0.0, C0);
    // 7702 !! Shouldn't it be forbidden to update twice in the same day?
    z.update_gpt(s_parms_0, 0.0, C0);
#if 0
    LMI_TEST_THROW
        (z.update_gpt(s_parms_0, 0.0, C0);
        ,std::runtime_error
        ,"Assertion 'cum_f1A_ <= guideline_limit()' failed."
        );
#endif // 0
}

void gpt_test::test_1035()
{
    int const issue_age = 35;
    int const length = 100 - issue_age;

    gpt_vector_parms v_parms =
        {.prem_load_target     = std::vector<double>(length, 0.00)
        ,.prem_load_excess     = std::vector<double>(length, 0.00)
        ,.policy_fee_monthly   = std::vector<double>(length, 0.00)
        ,.policy_fee_annual    = std::vector<double>(length, 0.00)
        ,.specamt_load_monthly = std::vector<double>(length, 0.00)
        ,.qab_gio_rate         = std::vector<double>(length, 0.00)
        ,.qab_adb_rate         = std::vector<double>(length, 0.00)
        ,.qab_term_rate        = std::vector<double>(length, 0.00)
        ,.qab_spouse_rate      = std::vector<double>(length, 0.00)
        ,.qab_child_rate       = std::vector<double>(length, 0.00)
        ,.qab_waiver_rate      = std::vector<double>(length, 0.00)
        };

    std::vector<double> i20(length, i_upper_12_over_12_from_i<double>()(0.020));
    std::vector<double> i40(length, i_upper_12_over_12_from_i<double>()(0.040));

    gpt7702 z(sample_q(issue_age), i20, i20, i40, i40, v_parms);

    gpt_scalar_parms s_parms_0 =
        {.duration       =      0
        ,.f3_bft         = 100000.0
        ,.endt_bft       = 100000.0
        ,.target_prem    =      0.0
        ,.chg_sa_base    = 100000.0
        ,.dbopt_7702     = mce_option1_for_7702
        };

    z.initialize_gpt
        (mce_gpt // defn_life_ins
        ,0.0     // fractional_duration
        ,0.0     // inforce_glp
        ,0.0     // inforce_cum_glp
        ,0.0     // inforce_gsp
        ,C0      // inforce_cum_f1A
        ,s_parms_0
        );
    LMI_TEST(std::fabs( 1799.8355 - z.glp_    ) < 0.01);
    LMI_TEST(std::fabs( 1799.8355 - z.cum_glp_) < 0.01);
    LMI_TEST(std::fabs(25136.3867 - z.gsp_    ) < 0.01);
    LMI_TEST(               C0   == z.forceout_amount_);
    LMI_TEST(               C0   == z.rejected_pmt_   );
    LMI_TEST(               C0   == z.cum_f1A_        );
    // A $30,000.00 exchange is too high.
    LMI_TEST_THROW
        (z.enqueue_exch_1035(30'000'00_cents);
        ,std::runtime_error
        ,"Assertion 'exch_amt <= guideline_limit()' failed."
        );
    z.enqueue_exch_1035(20'000'00_cents);
    z.update_gpt(s_parms_0, 0.0, C0);
    LMI_TEST(std::fabs( 1799.8355 - z.glp_    ) < 0.01);
    LMI_TEST(std::fabs( 1799.8355 - z.cum_glp_) < 0.01);
    LMI_TEST(std::fabs(25136.3867 - z.gsp_    ) < 0.01);
    LMI_TEST(               C0   == z.forceout_amount_);
    LMI_TEST(               C0   == z.rejected_pmt_   );
    LMI_TEST(    20'000'00_cents == z.cum_f1A_        );
}

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
    int const issue_age = 45;
    int const length = 100 - issue_age;

    gpt_vector_parms v_parms =
        {.prem_load_target     = std::vector<double>(length, 0.05)
        ,.prem_load_excess     = std::vector<double>(length, 0.05)
        ,.policy_fee_monthly   = std::vector<double>(length, 5.00)
        ,.policy_fee_annual    = std::vector<double>(length, 0.00)
        ,.specamt_load_monthly = std::vector<double>(length, 0.00)
        ,.qab_gio_rate         = std::vector<double>(length, 0.00)
        ,.qab_adb_rate         = std::vector<double>(length, 0.00)
        ,.qab_term_rate        = std::vector<double>(length, 0.00)
        ,.qab_spouse_rate      = std::vector<double>(length, 0.00)
        ,.qab_child_rate       = std::vector<double>(length, 0.00)
        ,.qab_waiver_rate      = std::vector<double>(length, 0.00)
        };

    std::vector<double> i45(length, i_upper_12_over_12_from_i<double>()(0.045));
    std::vector<double> i60(length, i_upper_12_over_12_from_i<double>()(0.060));

    gpt7702 z(sample_q(issue_age), i45, i45, i60, i60, v_parms);

    gpt_scalar_parms s_parms_0 =
        {.duration       =      0
        ,.f3_bft         = 100000.0
        ,.endt_bft       = 100000.0
        ,.target_prem    =      0.0
        ,.chg_sa_base    = 100000.0
        ,.dbopt_7702     = mce_option1_for_7702
        };
    z.initialize_gpt
        (mce_gpt // defn_life_ins
        ,0.0     // fractional_duration
        ,0.0     // inforce_glp
        ,0.0     // inforce_cum_glp
        ,0.0     // inforce_gsp
        ,C0      // inforce_cum_f1A
        ,s_parms_0
        );
    z.update_gpt(s_parms_0, 0.0, C0);
    LMI_TEST(std::fabs( 2074.4029 - z.glp_    ) < 0.01);
    LMI_TEST(std::fabs( 2074.4029 - z.cum_glp_) < 0.01);
    LMI_TEST(std::fabs(24486.3207 - z.gsp_    ) < 0.01);
    LMI_TEST(               C0   == z.forceout_amount_);
    LMI_TEST(               C0   == z.rejected_pmt_   );
    LMI_TEST(               C0   == z.cum_f1A_        );

    for(int j = 46; j < 70; ++j)
        {
        ++s_parms_0.duration;
        z.update_gpt(s_parms_0, 0.0, 20'000'00_cents);
        }
    LMI_TEST(std::fabs(z.cum_glp_ - 25 * z.glp_) < 0.01);
    LMI_TEST(std::fabs(51860.0721 - z.cum_glp_) < 0.01);

    gpt_scalar_parms s_parms_1 =
        {.duration       = 70 - issue_age
        ,.f3_bft         =  50000.0
        ,.endt_bft       =  50000.0
        ,.target_prem    =      0.0
        ,.chg_sa_base    =  50000.0
        ,.dbopt_7702     = mce_option1_for_7702
        };
    z.enqueue_adj_event();
    // Call adjust_guidelines() directly for this test. Normally, a
    // client would instead call this:
//  z.update_gpt(s_parms_1, 0.0, 20'000'00_cents);
    // but a unit test can call this isolated private member function,
    // which does nothing other than what its name implies.
    z.adjust_guidelines(s_parms_1);
    LMI_TEST(std::fabs(-1845.9882 - z.glp_    ) < 0.01);
    LMI_TEST(std::fabs(51860.0721 - z.cum_glp_) < 0.01);
    LMI_TEST(std::fabs(-5267.2627 - z.gsp_    ) < 0.01);
    LMI_TEST(               C0   == z.forceout_amount_);
    LMI_TEST(               C0   == z.rejected_pmt_   );
    LMI_TEST(               C0   == z.cum_f1A_        );
#if 0
std::cout
    << std::fixed << std::setprecision(4)
    << z.glp_ << " z.glp_\n"
    << z.cum_glp_ << " z.cum_glp_\n"
    << z.gsp_ << " z.gsp_\n"
    << z.guideline_limit() << " z.guideline_limit()\n"
    << z.forceout_amount_ << " z.forceout_amount_\n"
    << z.cum_f1A_ << " z.cum_f1A_\n"
    << std::endl
    ;
#endif // 0
}

void gpt_test::test_7702_f_6()
{
    int const issue_age = 45;
    int const length = 100 - issue_age;

    gpt_vector_parms v_parms =
        {.prem_load_target     = std::vector<double>(length, 0.00)
        ,.prem_load_excess     = std::vector<double>(length, 0.00)
        ,.policy_fee_monthly   = std::vector<double>(length, 0.00)
        ,.policy_fee_annual    = std::vector<double>(length, 0.00)
        ,.specamt_load_monthly = std::vector<double>(length, 0.00)
        ,.qab_gio_rate         = std::vector<double>(length, 0.00)
        ,.qab_adb_rate         = std::vector<double>(length, 0.00)
        ,.qab_term_rate        = std::vector<double>(length, 0.00)
        ,.qab_spouse_rate      = std::vector<double>(length, 0.00)
        ,.qab_child_rate       = std::vector<double>(length, 0.00)
        ,.qab_waiver_rate      = std::vector<double>(length, 0.00)
        };

    std::vector<double> i45(length, i_upper_12_over_12_from_i<double>()(0.045));
    std::vector<double> i60(length, i_upper_12_over_12_from_i<double>()(0.060));

    gpt7702 z(sample_q(issue_age), i45, i45, i60, i60, v_parms);

    gpt_scalar_parms s_parms_0 =
        {.duration       =     17
        ,.f3_bft         = 100000.0
        ,.endt_bft       = 100000.0
        ,.target_prem    =      0.0
        ,.chg_sa_base    = 100000.0
        ,.dbopt_7702     = mce_option1_for_7702
        };

    // Inforce arguments demonstrate failure.
    LMI_TEST_THROW
        (z.initialize_gpt
            (mce_gpt  // defn_life_ins
            ,0.1      // fractional_duration
            , -1000.0 // inforce_glp
            ,-10000.0 // inforce_cum_glp
            ,-13000.0 // inforce_gsp
            ,C0       // inforce_cum_f1A
            ,s_parms_0
            )
        ,std::runtime_error
        ,"Assertion 'cum_f1A_ <= guideline_limit()' failed."
        );

    // With the same starting assumptions, but a valid inforce
    // cumulative (c)(1) premiums paid value, no exception is thrown.
    currency const c{-10000'00_cents};
    z.initialize_gpt
        (mce_gpt   // defn_life_ins
        ,0.1       // fractional_duration
        , -1000.0  // inforce_glp
        ,-10000.0  // inforce_cum_glp
        ,-13000.0  // inforce_gsp
        ,c         // inforce_cum_f1A
        ,s_parms_0
        );
    z.update_gpt(s_parms_0, 0.1, C0);
    LMI_TEST_EQUAL( -1000'00_cents, z.rounded_glp()    );
    LMI_TEST_EQUAL(-10000'00_cents, z.rounded_cum_glp());
    LMI_TEST_EQUAL(-13000'00_cents, z.rounded_gsp()    );
    LMI_TEST_EQUAL(        0_cents, z.forceout_amount_ );
    LMI_TEST_EQUAL(        0_cents, z.rejected_pmt_    );
    LMI_TEST_EQUAL(-10000'00_cents, z.cum_f1A_         );
    LMI_TEST_EQUAL(-10000'00_cents, z.guideline_limit());
    // In the real world, this contract would probably be maintained
    // in force as ART under 7702(f)(6), or exchanged, or perhaps even
    // surrendered. Yet its owner could choose to increase the
    // specified amount, making the guideline limit positive.
    gpt_scalar_parms s_parms_1 =
        {.duration       =     18
        ,.f3_bft         = 200000.0
        ,.endt_bft       = 200000.0
        ,.target_prem    =      0.0
        ,.chg_sa_base    =  50000.0
        ,.dbopt_7702     = mce_option1_for_7702
        };
    z.enqueue_adj_event();
    currency forceout = z.update_gpt(s_parms_1, 0.0, C0);
    // f2A value was zero, limiting the forceout to zero.
    LMI_TEST_EQUAL(C0, forceout);
    LMI_TEST_EQUAL(  3943'37_cents, z.rounded_glp()    );
    LMI_TEST_EQUAL( -6056'63_cents, z.rounded_cum_glp());
    LMI_TEST_EQUAL( 32424'69_cents, z.rounded_gsp()    );
    LMI_TEST_EQUAL(        0_cents, z.forceout_amount_ );
    LMI_TEST_EQUAL(        0_cents, z.rejected_pmt_    );
    LMI_TEST_EQUAL(-10000'00_cents, z.cum_f1A_         );
    // That may be unwise, but because it is allowed, the server must
    // still perform the calculations correctly.
    //
    // What happens if a forceout amount exceeded the account value,
    // so that less than the full amount was actually distributed?
    // The guideline limit is of course equal to the GSP now...
    LMI_TEST_EQUAL( 32424'69_cents, z.guideline_limit());
    LMI_TEST_EQUAL(z.rounded_gsp()        , z.guideline_limit());
    // ...but, supposing that only half of the $10000.00 forceout
    // amount was actually distributed, what premium can now be paid?
    currency const limit = z.guideline_limit();
    currency const allowed  = std::max(C0, limit -      z.cum_f1A_);
    currency const allowed0 = std::max(C0, limit - -10000'00_cents);
    currency const allowed1 = std::max(C0, limit - - 5000'00_cents);
    LMI_TEST_EQUAL( 42424'69_cents, allowed );
    LMI_TEST_EQUAL( 42424'69_cents, allowed0);
    LMI_TEST_EQUAL( 37424'69_cents, allowed1);
#if 0
std::cout
    << std::fixed << std::setprecision(4)
    << z.glp_ << " z.glp_\n"
    << z.cum_glp_ << " z.cum_glp_\n"
    << z.gsp_ << " z.gsp_\n"
    << z.guideline_limit() << " z.guideline_limit()\n"
    << z.forceout_amount_ << " z.forceout_amount_\n"
    << z.cum_f1A_ << " z.cum_f1A_\n"
    << allowed  << " allowed\n"
    << allowed0 << " allowed0\n"
    << allowed1 << " allowed1\n"
    << std::endl
    ;
#endif // 0
}

int test_main(int, char*[])
{
    gpt_test::test();
    return EXIT_SUCCESS;
}
