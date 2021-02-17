// Internal Revenue Code section 7702 guideline premium--unit test.
//
// Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "gpt_commutation_functions.hpp"
#include "ihs_irc7702.hpp"

#include "assert_lmi.hpp"
#include "cso_table.hpp"
#include "et_vector.hpp"
#include "irc7702_interest.hpp"         // iglp(), igsp()
#include "materially_equal.hpp"
#include "math_functions.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <numeric>                      // iota()
#include <vector>

namespace
{
/// Convert annual mortality rates to monthly.

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

/// This target premium is chosen to lie between the column minimums
/// and maximums of 'touchstone', so that all paths through the
/// premium-calculation code are exercised.

double const touchstone_target = 10000.0;

/// Guideline premiums for ages [0, 100), using this module's default
/// assumptions and 'touchstone_target'.

double const touchstone[100][3] =
    {   //     GSP                 GLP, DBO 1            GLP, DBO2         age
         {  7038.68351698240,    585.526862159977,    1423.177437435097} //  0
        ,{  6798.19554639087,    582.279184423523,    1454.983214917078} //  1
        ,{  6917.55938814647,    595.654331516075,    1503.925334003016} //  2
        ,{  7054.08836427570,    610.098531559143,    1555.393346766061} //  3
        ,{  7200.42959441599,    625.289620498655,    1609.147412434138} //  4
        ,{  7359.59499719102,    641.377482638437,    1665.394363517762} //  5
        ,{  7534.77264967186,    658.521212308809,    1724.351061338387} //  6
        ,{  7725.75253256339,    676.725078857967,    1786.091846010552} //  7
        ,{  7935.89559393773,    696.159622871104,    1850.848292115368} //  8
        ,{  8163.99574210983,    716.784860802655,    1918.658536904343} //  9
        ,{  8408.78751694222,    738.558854641057,    1989.562952864434} // 10
        ,{  8670.12948570098,    761.493429259317,    2063.655691084516} // 11
        ,{  8943.15168307475,    785.376698055932,    2140.829866947342} // 12
        ,{  9223.90802461156,    810.039562357985,    2221.025457355350} // 13
        ,{  9505.89053285976,    835.188054645305,    2304.076471840259} // 14
        ,{  9786.93746033387,    860.736839659286,    2390.016550335725} // 15
        ,{ 10064.09674883057,    886.594731233553,    2478.881457455330} // 16
        ,{ 10335.79183986005,    912.779767479432,    2570.813425707223} // 17
        ,{ 10606.46561801551,    939.427367607391,    2666.066827908476} // 18
        ,{ 10881.93560436608,    966.857566950968,    2765.066561632714} // 19
        ,{ 11166.04774456913,    995.295455608219,    2868.154802925353} // 20
        ,{ 11464.01637080176,   1025.041342657894,    2975.744576543945} // 21
        ,{ 11780.21027563471,   1056.358083620659,    3088.218208975990} // 22
        ,{ 12119.25033135827,   1089.528374470945,    3205.978738573638} // 23
        ,{ 12483.74838399238,   1124.736519495792,    3329.345194701353} // 24
        ,{ 12876.47313540750,   1162.182018861173,    3458.654561239592} // 25
        ,{ 13300.35865601369,   1202.081288458987,    3594.262976739506} // 26
        ,{ 13756.26463939956,   1244.546633061123,    3736.440115687674} // 27
        ,{ 14244.00139621636,   1289.639330123234,    3885.418226907989} // 28
        ,{ 14764.51444284111,   1337.490074314652,    4041.497806206768} // 29
        ,{ 15317.72104376862,   1388.179505203846,    4204.943822021073} // 30
        ,{ 15904.67615879494,   1441.860768068979,    4376.092191478663} // 31
        ,{ 16524.33383892391,   1498.574118828346,    4555.19126684717 } // 32
        ,{ 17178.91923180180,   1558.559910057550,    4742.66949911027 } // 33
        ,{ 17867.56028901856,   1621.886308664094,    4938.81829564135 } // 34
        ,{ 18591.55843367445,   1688.762387605737,    5144.05870554551 } // 35
        ,{ 19351.26721643064,   1759.352690245432,    5358.78435177983 } // 36
        ,{ 20147.12673213025,   1833.840248511528,    5583.41556319251 } // 37
        ,{ 20978.62057777484,   1912.360234827265,    5818.34595858885 } // 38
        ,{ 21846.34193266791,   1995.133368677091,    6064.05294668874 } // 39
        ,{ 22749.96476442091,   2082.335591042036,    6320.99151825042 } // 40
        ,{ 23690.27901372488,   2174.235376765823,    6589.70614937677 } // 41
        ,{ 24666.18011714967,   2270.988479559321,    6870.66752222614 } // 42
        ,{ 25680.67346367356,   2373.064175530010,    7164.60994754848 } // 43
        ,{ 26733.01333128616,   2480.691139489740,    7472.09234094005 } // 44
        ,{ 27825.53320587390,   2594.354982641396,    7793.89002818164 } // 45
        ,{ 28957.87517174733,   2714.372960009581,    8130.66370509233 } // 46
        ,{ 30132.73718519145,   2841.340030468526,    8483.30199083677 } // 47
        ,{ 31351.18686378591,   2975.772554671284,    8852.64633067760 } // 48
        ,{ 32615.47640454942,   3118.340626160647,    9239.66612712437 } // 49
        ,{ 33925.37360974723,   3269.561842386675,    9645.23247070155 } // 50
        ,{ 35282.72955074919,   3430.199227135144,   10069.64394976404 } // 51
        ,{ 36684.32232853716,   3600.612419051228,   10510.36974551134 } // 52
        ,{ 38128.94087403525,   3781.391207568697,   10972.25718541429 } // 53
        ,{ 39613.93885174853,   3973.035706974506,   11456.25858694856 } // 54
        ,{ 41136.11931963878,   4176.030195457377,   11963.35754058970 } // 55
        ,{ 42695.98388934036,   4391.312735019566,   12494.88289937905 } // 56
        ,{ 44292.80919530735,   4619.78365830111 ,   13052.16853061377 } // 57
        ,{ 45929.5718921291 ,   4862.89582994844 ,   13636.93530368279 } // 58
        ,{ 47607.4216271391 ,   5122.05692743359 ,   14250.88599353768 } // 59
        ,{ 49327.3204391780 ,   5398.82901312474 ,   14895.83704266039 } // 60
        ,{ 51087.0675327450 ,   5694.52747069843 ,   15573.47429994832 } // 61
        ,{ 52883.6498834271 ,   6010.49270177416 ,   16285.54274598828 } // 62
        ,{ 54711.8755883987 ,   6347.85877060754 ,   17033.72432934544 } // 63
        ,{ 56565.1938672560 ,   6707.61038666661 ,   17819.70417095833 } // 64
        ,{ 58438.5603842135 ,   7091.02873646156 ,   18645.45168807685 } // 65
        ,{ 60328.4826730125 ,   7499.76716701808 ,   19513.26040865944 } // 66
        ,{ 62235.7122880404 ,   7936.48566273830 ,   20426.07981227157 } // 67
        ,{ 64161.4476182190 ,   8404.31827131655 ,   21387.17409476018 } // 68
        ,{ 66108.1111518329 ,   8907.15411732119 ,   22400.24884101083 } // 69
        ,{ 68075.2968655369 ,   9448.81780751370 ,   23468.99783484597 } // 70
        ,{ 70056.5435719881 ,  10031.72536519677 ,   24596.65312417418 } // 71
        ,{ 72041.9336211511 ,  10651.63206077490 ,   25786.24096652694 } // 72
        ,{ 74017.1917991849 ,  11315.03532154118 ,   27040.39635816422 } // 73
        ,{ 75965.3671901283 ,  12020.79055649973 ,   28361.55036911865 } // 74
        ,{ 77874.9283195687 ,  12768.50924453628 ,   29753.25818586441 } // 75
        ,{ 79740.9467500571 ,  13559.48673681164 ,   31220.67845789502 } // 76
        ,{ 81564.9188631318 ,  14397.45383224088 ,   32770.90259122696 } // 77
        ,{ 83353.8927677665 ,  15289.23021935768 ,   34413.18965221712 } // 78
        ,{ 85119.4534825334 ,  16245.42797729094 ,   36159.17795669160 } // 79
        ,{ 86867.9022927022 ,  17276.34224247499 ,   38021.03919611617 } // 80
        ,{ 88597.0645980752 ,  18389.30243866668 ,   40010.68845409491 } // 81
        ,{ 90299.0078252165 ,  19588.56033848707 ,   42140.39788964697 } // 82
        ,{ 91957.2671095038 ,  20871.36395800513 ,   44422.07949981734 } // 83
        ,{ 93552.4553979591 ,  22229.02331799561 ,   46869.2271839139  } // 84
        ,{ 95074.4281098265 ,  23655.68252683349 ,   49502.2183174530  } // 85
        ,{ 96522.0685347940 ,  25150.86248432227 ,   52350.8908700057  } // 86
        ,{ 97903.4505017087 ,  26723.33948060062 ,   55458.2004177460  } // 87
        ,{ 99232.0338500975 ,  28391.52743840341 ,   58883.2994927711  } // 88
        ,{100526.4169722035 ,  30187.58145327271 ,   62707.8680603863  } // 89
        ,{101809.7855145883 ,  32162.72117506931 ,   67045.4528454541  } // 90
        ,{103109.8559654121 ,  34396.12222752874 ,   72056.5609689122  } // 91
        ,{104460.6581669420 ,  37013.30252275156 ,   77975.2369131742  } // 92
        ,{105902.8500872039 ,  40216.83342704028 ,   85154.0907265174  } // 93
        ,{107484.6433387535 ,  44350.39801579426 ,   94149.210398289   } // 94
        ,{109230.5635533567 ,  49945.9025675526  , 105840.9285717971   } // 95
        ,{111114.2974785539 ,  57867.8963646683  , 121708.1682942241   } // 96
        ,{112969.7125568349 ,  69513.6146174326  , 144360.4558901023   } // 97
        ,{114149.2609045463 ,  86890.3134223641  , 178406.5103343175   } // 98
        ,{111221.7271446342 , 112671.1356520957  , 228328.2710683905   } // 99
    };

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
        test_preconditions();
        test_premium_calculations();
        assay_speed();
        }

  private:
    static void test_preconditions();
    static void test_premium_calculations();
    static void assay_speed();

    static void initialize(int issue_age);
    static gpt_vector_parms v_parms();
    static gpt_scalar_parms s_parms();
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
/// vector is altered if necessary--such that v[x]+t == v[x+t], so
/// that the same invariant may be tested for premiums.

void gpt_test::initialize(int issue_age)
{
    static double const i12glp = i_upper_12_over_12_from_i<double>()(iglp());
    static double const i12gsp = i_upper_12_over_12_from_i<double>()(igsp());
    q_m = sample_q(issue_age);
    int const length = lmi::ssize(q_m);
    glp_ic               .assign(length,     i12glp);
    glp_ig               .assign(length,     i12glp);
    gsp_ic               .assign(length,     i12gsp);
    gsp_ig               .assign(length,     i12gsp);
    prem_load_target     .assign(length,  0.03    );
    prem_load_excess     .assign(length,  0.02    );
    policy_fee_monthly   .assign(length,  5.0     );
    policy_fee_annual    .assign(length, 37.0     );
    specamt_load_monthly .assign(length,  0.000007);
    qab_gio_rate         .assign(length,  0.000011);
    qab_adb_rate         .assign(length,  0.000013);
    qab_term_rate        .assign(length,  0.000017);
    qab_spouse_rate      .assign(length,  0.000019);
    qab_child_rate       .assign(length,  0.000023);
    qab_waiver_rate      .assign(length,  0.000029);

    std::vector<int>    iota_i(length);
    std::iota(iota_i.begin(), iota_i.end(), issue_age);
    std::vector<double> iota_d(length, 0.0);
    iota_d += 1.0 + 0.001 * iota_i;
    glp_ic               *= iota_d;
    glp_ig               *= iota_d;
    gsp_ic               *= iota_d;
    gsp_ig               *= iota_d;
    prem_load_target     *= iota_d;
    prem_load_excess     *= iota_d;
    policy_fee_monthly   *= iota_d;
    policy_fee_annual    *= iota_d;
    specamt_load_monthly *= iota_d;
    qab_gio_rate         *= iota_d;
    qab_adb_rate         *= iota_d;
    qab_term_rate        *= iota_d;
    qab_spouse_rate      *= iota_d;
    qab_child_rate       *= iota_d;
    qab_waiver_rate      *= iota_d;
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

/// Instantiate plausible specimen scalar parameters.

gpt_scalar_parms gpt_test::s_parms()
{
    gpt_scalar_parms z =
        {     0   // duration
        ,  1000.0 // target
        ,120000.0 // f3bft
        ,100000.0 // endt_bft
        ,100000.0 // chg_sa_amt
        , 20000.0 // qab_gio_amt
        ,100000.0 // qab_adb_amt
        , 25000.0 // qab_term_amt
        , 10000.0 // qab_spouse_amt
        ,  5000.0 // qab_child_amt
        , 50000.0 // qab_waiver_amt
        };
    return z;
}

/// Instantiate GPT commutation functions.

gpt_cf_triad gpt_test::instantiate_cf()
{
    return gpt_cf_triad(q_m, glp_ic, glp_ig, gsp_ic, gsp_ig, v_parms());
}

/// Test gpt_cf_triad::calculate_premium()'s asserted preconditions.

void gpt_test::test_preconditions()
{
    gpt_scalar_parms parms = s_parms();
    initialize(0);
    gpt_cf_triad const z = instantiate_cf();

    // Negative duration.
    parms.duration = -1;
    BOOST_TEST_THROW
        (z.calculate_premium(oe_gsp, mce_option1_for_7702, parms)
        ,std::runtime_error
        ,""
        );
    parms = s_parms(); // Reset.

    // Duration greater than omega minus one.
    parms.duration = lmi::ssize(q_m);
    BOOST_TEST_THROW
        (z.calculate_premium(oe_gsp, mce_option1_for_7702, parms)
        ,std::runtime_error
        ,""
        );
    parms = s_parms(); // Reset.

    // Negative target. (Identical preconditions for other scalar
    // parameters are not redundantly tested here.)
    parms.target = -0.01;
    BOOST_TEST_THROW
        (z.calculate_premium(oe_gsp, mce_option1_for_7702, parms)
        ,std::runtime_error
        ,""
        );
    parms = s_parms(); // Reset.

    // Monthly q shorter than other vector parameters.
    q_m.resize(99);
    BOOST_TEST_THROW(instantiate_cf(), std::runtime_error, "");
    initialize(0); // Reset.

    // Monthly q equal to unity: probably a bad idea, but permitted.
    q_m.back() = 1.0;
    instantiate_cf();
    initialize(0); // Reset.

    // Monthly q greater than unity.
    q_m.back() = 1.001;
    BOOST_TEST_THROW(instantiate_cf(), std::runtime_error, "");
    initialize(0); // Reset.

    // Negative monthly q.
    q_m[0] = -0.001;
    BOOST_TEST_THROW(instantiate_cf(), std::runtime_error, "");
    initialize(0); // Reset.

    // Premium load equal to unity.
    prem_load_target[0] = 1.0;
    BOOST_TEST_THROW(instantiate_cf(), std::runtime_error, "");
    initialize(0); // Reset.

    // Monthly specamt load equal to unity.
    specamt_load_monthly[0] = 1.0;
    BOOST_TEST_THROW(instantiate_cf(), std::runtime_error, "");
    initialize(0); // Reset.

    // Monthly QAB rate equal to unity.
    qab_adb_rate[0] = 1.0;
    BOOST_TEST_THROW(instantiate_cf(), std::runtime_error, "");
    initialize(0); // Reset.

    // Negative premium loads are trapped. They are known to have been
    // used, if rarely, and presumably just reduce guidelines; but
    // it's not worth the trouble to validate premium calculations
    // in advance under rare and questionable circumstances.
    prem_load_excess[0] = -0.01;
    BOOST_TEST_THROW(instantiate_cf(), std::runtime_error, "");
    initialize(0); // Reset.
}

/// The obsolescent GPT class more or less requires this ugliness.

Irc7702* ugliness = nullptr;

/// Instantiate obsolescent GPT class.

Irc7702& gpt_test::instantiate_old(int issue_age)
{
#if defined LMI_COMO_WITH_MINGW
    throw "Code that uses this obsolescent class segfaults with como.";
#else // !defined LMI_COMO_WITH_MINGW
    int const length = lmi::ssize(q_m);
    std::vector<double> const zero(length, 0.0);
    // The old class recognizes only one QAB: ADB. So that all QABs
    // can be exercised with the new class, use a linear combination
    // of all QAB rates as the ADB rate here. Copying literal values
    // elsewhence is brittle, but no great effort is justified for
    // temporary code that tests a class slated for expunction.
    double const adj =
        (   20000.0 * 0.0011
        +  100000.0 * 0.0013
        +   25000.0 * 0.0017
        +   10000.0 * 0.0019
        +    5000.0 * 0.0023
        +   50000.0 * 0.0029
        )
        / (100000.0 * 0.0013)
        ;
    std::vector<double> adj_qab_adb_rate(length);
    assign(adj_qab_adb_rate, qab_adb_rate * adj);
    delete ugliness;
    ugliness =
        (::new Irc7702
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
            ,adj_qab_adb_rate                // a_MlyChgADD
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

void gpt_test::compare_premiums(int issue_age, double target)
{
    gpt_scalar_parms parms = s_parms();
    parms.target = target;

    initialize(issue_age);

    gpt_cf_triad const z = instantiate_cf();

#if !defined LMI_COMO_WITH_MINGW
    double const f3bft    = parms.f3bft   ;
    double const endt_bft = parms.endt_bft;
    // This test of the obsolescent class segfaults with como.
    Irc7702& z_old = instantiate_old(issue_age);
    // Set target (the other arguments don't matter here).
    z_old.Initialize7702(f3bft, endt_bft, mce_option1_for_7702, target);
#endif // !defined LMI_COMO_WITH_MINGW

    int const omega = lmi::ssize(sample_q(0));
    LMI_ASSERT(lmi::ssize(qab_waiver_rate) == omega - issue_age);
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

/// Compare {GSP, GLP opt 1, GLP opt 2} to validated stored values.
///
/// For the standard table provided by sample_q(), test all issue ages
/// and durations, with a target premium ('touchstone_target') chosen
/// to exercise all paths through the premium-calculation code.
///
/// The number of premium triplets tested should be, and is, the 100th
/// triangular number: 101 choose two, which is (100*101)/2 = 5050.
///
/// For the nonce, similarly compare new to obsolescent GPT class.

void gpt_test::test_premium_calculations()
{
    int const omega = lmi::ssize(sample_q(0));

    for(int issue_age = 0; issue_age < omega; ++issue_age)
        {
        compare_premiums(issue_age,     100.0); // < min(touchstone)
        compare_premiums(issue_age,   10000.0); // = touchstone_target
        compare_premiums(issue_age, 1000000.0); // > max(touchstone)
        }

    gpt_scalar_parms parms = s_parms();
    parms.target = touchstone_target;
    int count = 0;
    for(int issue_age = 0; issue_age < omega; ++issue_age)
        {
        initialize(issue_age);
        gpt_cf_triad const z = instantiate_cf();
        for(int duration = 0; duration < omega - issue_age; ++duration)
            {
            parms.duration = duration;
            double const r0 = z.calculate_premium(oe_gsp, mce_option1_for_7702, parms);
            double const r1 = z.calculate_premium(oe_glp, mce_option1_for_7702, parms);
            double const r2 = z.calculate_premium(oe_glp, mce_option2_for_7702, parms);
            int const x_plus_t = issue_age + duration;
            LMI_ASSERT(0 <= x_plus_t && x_plus_t < omega);
            bool const all_materially_equal =
                   materially_equal(r0, touchstone[x_plus_t][0])
                && materially_equal(r1, touchstone[x_plus_t][1])
                && materially_equal(r2, touchstone[x_plus_t][2])
                ;
            BOOST_TEST(all_materially_equal);
            ++count;
            }
        }
    BOOST_TEST(5050 == count);
}

void gpt_test::mete_premiums()
{
    static gpt_scalar_parms const parms = s_parms();
    static gpt_cf_triad const z = instantiate_cf();
    z.calculate_premium(oe_gsp, mce_option1_for_7702, parms);
    z.calculate_premium(oe_glp, mce_option1_for_7702, parms);
    z.calculate_premium(oe_glp, mce_option2_for_7702, parms);
}

/// Measure instantiation speed of old GPT class.
///
/// This simple pass-through function could have been written inline
/// with std::bind, but it's preferable not to drag that in.

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
