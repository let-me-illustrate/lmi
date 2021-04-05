// Ordinary- and universal-life commutation functions--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "commutation_functions.hpp"

#include "cso_table.hpp"
#include "et_vector.hpp"
#include "math_functions.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"
#include "timer.hpp"                    // TimeAnAliquot()

#include <algorithm>                    // max()
#include <cmath>                        // fabs()
#include <iomanip>                      // setw() etc.
#include <ios>                          // ios_base::fixed()
#include <vector>

namespace
{
/// 1980 CSO Ultimate ANB Male Unismoke

std::vector<double> const& sample_q()
{
    static std::vector<double> const q = cso_table
        (mce_1980cso
        ,oe_orthodox
        ,oe_age_nearest_birthday_ties_younger
        ,mce_male
        ,mce_unismoke
        );
    return q;
}
} // Unnamed namespace.

void mete_olcf
    (std::vector<double> const& q
    ,std::vector<double> const& i
    )
{
    for(int j = 0; j < 100; ++j)
        {
        volatile auto z = OLCommFns(q, i);
        }
}

void mete_ulcf
    (std::vector<double> const& q
    ,std::vector<double> const& ic
    ,std::vector<double> const& ig
    )
{
    for(int j = 0; j < 100; ++j)
        {
        volatile auto z = ULCommFns(q, ic, ig, mce_option1_for_7702, mce_monthly);
        }
}

void mete_reserve
    (ULCommFns const&     ulcf
    ,std::vector<double>& reserve
    )
{
    for(int j = 0; j < 100; ++j)
        {
        volatile double premium = (10.0 * ulcf.aDomega() + ulcf.kM()[0]) / ulcf.aN()[0];
        reserve <<= premium * ulcf.aD() - ulcf.kC();
        reserve <<= fwd_sum(reserve) / ulcf.EaD();
        }
}

/// Exactly reproduce Table 2 from Eckley's paper.
///
/// Table 2 on pages 25-26 of TSA XXIX uses annual functions, and
/// tabulates actuarial functions Ax, ax, Px, and Vx. Its other
/// columns represent assumptions or intermediate results upon which
/// those functions depend.
///
/// Ax and ax are given to a precision of six decimals only, so their
/// maximum roundoff error is 0.0000005: half a unit in the sixth
/// decimal place, which is five units in the seventh. Px and Vx per
/// thousand are given to two decimals only; on a unit basis, their
/// maximum roundoff error is 0.000005: half a unit in the fifth
/// decimal place, which is five units in the sixth. This unit test
/// demonstrates that every number in those four columns is reproduced
/// within its tightest-possible tolerance.

void TestEckleyTable2()
{
    static double const Ax[65] =
        { 0.052458,  0.055704,  0.059222,  0.063020,  0.067108,  0.071497,  0.076199,  0.081238,  0.086632,  0.092398
        , 0.098540,  0.105072,  0.111980,  0.119262,  0.126926,  0.134992,  0.143491,  0.152477,  0.161984,  0.172052
        , 0.182692,  0.193893,  0.205657,  0.217992,  0.230892,  0.244345,  0.258326,  0.272795,  0.287718,  0.303067
        , 0.318822,  0.334974,  0.351544,  0.368555,  0.385979,  0.403753,  0.421764,  0.439846,  0.457858,  0.475703
        , 0.493351,  0.510833,  0.528213,  0.545524,  0.562767,  0.579890,  0.596800,  0.613301,  0.629220,  0.644478
        , 0.659128,  0.673331,  0.687317,  0.701355,  0.715664,  0.730401,  0.745726,  0.761868,  0.779511,  0.800303
        , 0.825126,  0.847617,  0.869722,  0.896096,  0.935315
        };
    static double const ax[65] =
        {10.454430, 10.420672, 10.384087, 10.344586, 10.302069, 10.256425, 10.207522, 10.155114, 10.099025, 10.039054
        , 9.975175,  9.907247,  9.835400,  9.759662,  9.679952,  9.596069,  9.507682,  9.414227,  9.315348,  9.210637
        , 9.099980,  8.983486,  8.861134,  8.732856,  8.598688,  8.458776,  8.313365,  8.162878,  8.007675,  7.848043
        , 7.684182,  7.516195,  7.343856,  7.166934,  6.985711,  6.800839,  6.613508,  6.425435,  6.238089,  6.052470
        , 5.868894,  5.687038,  5.506229,  5.326138,  5.146733,  4.968557,  4.792568,  4.620797,  4.455047,  4.296105
        , 4.143395,  3.995240,  3.849173,  3.702364,  3.552436,  3.397629,  3.236080,  3.065091,  2.877095,  2.654244
        , 2.386077,  2.134559,  1.872022,  1.534759,  1.000000
        };
    static double const Px[65] =
        {     5.02,      5.35,      5.70,      6.09,      6.51,      6.97,      7.47,      8.00,      8.58,      9.20
        ,     9.88,     10.61,     11.39,     12.22,     13.11,     14.07,     15.09,     16.20,     17.39,     18.68
        ,    20.08,     21.58,     23.21,     24.96,     26.85,     28.89,     31.07,     33.42,     35.93,     38.62
        ,    41.49,     44.57,     47.87,     51.42,     55.25,     59.37,     63.77,     68.45,     73.40,     78.60
        ,    84.06,     89.82,     95.93,    102.42,    109.34,    116.71,    124.53,    132.73,    141.24,    150.01
        ,   159.08,    168.53,    178.56,    189.43,    201.46,    214.97,    230.44,    248.56,    270.94,    301.52
        ,   345.81,    397.09,    464.59,    583.87,    935.31
        };
    // To get end-of-year values, omit Eckley's first element.
    static double const Vx[65] =
        {  /* 0.00, */   3.42,      7.12,     11.11,     15.41,     20.03,     24.98,     30.28,     35.96,     42.02
        ,    48.49,     55.36,     62.63,     70.29,     78.35,     86.84,     95.78,    105.24,    115.24,    125.83
        ,   137.03,    148.82,    161.19,    174.17,    187.75,    201.90,    216.61,    231.84,    247.54,    263.69
        ,   280.26,    297.26,    314.69,    332.59,    350.93,    369.63,    388.58,    407.60,    426.56,    445.33
        ,   463.90,    482.30,    500.58,    518.80,    536.94,    554.96,    572.75,    590.12,    606.87,    622.92
        ,   638.34,    653.28,    668.00,    682.78,    697.84,    713.35,    729.49,    746.49,    765.07,    786.98
        ,   813.15,    836.91,    860.33,    888.39,    930.30,   1000.00
        };

    std::vector<double>coi
        {  0.00200,   0.00206,   0.00214,   0.00224,   0.00236,   0.00250,   0.00265,   0.00282,   0.00301,   0.00324
        ,  0.00350,   0.00382,   0.00419,   0.00460,   0.00504,   0.00550,   0.00596,   0.00645,   0.00697,   0.00756
        ,  0.00825,   0.00903,   0.00990,   0.01088,   0.01199,   0.01325,   0.01469,   0.01631,   0.01811,   0.02009
        ,  0.02225,   0.02456,   0.02704,   0.02979,   0.03289,   0.03645,   0.04058,   0.04526,   0.05043,   0.05599
        ,  0.06185,   0.06798,   0.07450,   0.08153,   0.08926,   0.09785,   0.10762,   0.11855,   0.13039,   0.14278
        ,  0.15545,   0.16827,   0.18132,   0.19506,   0.21012,   0.22700,   0.24613,   0.26655,   0.28547,   0.31127
        ,  0.40000,   0.50000,   0.60000,   0.70000,   1.00000
        };

    std::vector<double>ic  (coi.size(), 0.10);
    std::vector<double>ig  (coi.size(), 0.04);

    ULCommFns CF(coi, ic, ig, mce_option1_for_7702, mce_annual);

    std::vector<double> nsp;
    nsp     <<= (CF.aDomega() + CF.kM()) / CF.aD();

    std::vector<double> annuity;
    annuity <<= (               CF.aN()) / CF.aD();

    std::vector<double> premium;
    premium <<= (CF.aDomega() + CF.kM()) / CF.aN();

    std::vector<double> reserve;
    reserve <<= premium[0] * CF.aD() - CF.kC();
    reserve <<= fwd_sum(reserve) / CF.EaD();

    {
    double tolerance = 0.0000005;
    double worst_discrepancy = 0.0;
    for(int j = 0; j < lmi::ssize(coi); ++j)
        {
        double d0 = std::fabs(nsp    [j] - Ax[j]);
        double d1 = std::fabs(annuity[j] - ax[j]);
        worst_discrepancy = std::max(worst_discrepancy, d0);
        worst_discrepancy = std::max(worst_discrepancy, d1);
        if
            (  tolerance < d0
            || tolerance < d1
            )
            {
            std::cerr
                << "Failed to match Eckley's results at duration "
                << j
                << ".\n"
                << "  differences: " << d0 << ' ' << d1
                << "\n  " << nsp    [j] << " " << Ax[j] << '\n'
                << "\n  " << annuity[j] << " " << ax[j] << '\n'
                << std::endl
                ;
            }
        }
    LMI_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << "Table 2; Ax and ax:\n"
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(9)
        << "  " << std::setw(13) << tolerance         << " tolerance\n"
        << "  " << std::setw(13) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
    }
    {
    double tolerance = 0.000005;
    double worst_discrepancy = 0.0;
    for(int j = 0; j < lmi::ssize(coi); ++j)
        {
        double d0 = std::fabs(premium[j] - Px[j] * .001);
        double d1 = std::fabs(reserve[j] - Vx[j] * .001);
        worst_discrepancy = std::max(worst_discrepancy, d0);
        worst_discrepancy = std::max(worst_discrepancy, d1);
        if
            (  tolerance < d0
            || tolerance < d1
            )
            {
            std::cerr
                << "Failed to match Eckley's results at duration "
                << j
                << ".\n"
                << "  differences: " << d0 << ' ' << d1
                << "\n  " << premium[j] << " " << Px[j] * .001 << '\n'
                << "\n  " << reserve[j] << " " << Vx[j] * .001 << '\n'
                << std::endl
                ;
            }
        }
    LMI_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << "Table 2; Px and Vx:\n"
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(9)
        << "  " << std::setw(13) << tolerance         << " tolerance\n"
        << "  " << std::setw(13) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
    }
}

/// Exactly reproduce Tables 3 and 4 from Eckley's paper.
///
/// Tables 3 and 4 on pages 29-30 of TSA XXIX use annual functions to
/// develop the reserve for a common scenario in two different ways.
/// They both tabulate actuarial functions Px and Vx for an option B
/// solve for endowment at age 65. Their other columns represent
/// assumptions or intermediate results upon which Px and Vx depend.
///
/// Px and Vx per thousand are given to two decimals only; on a unit
/// basis, their maximum roundoff error is 0.000005: half a unit in
/// the fifth decimal place, which is five units in the sixth. This
/// unit test demonstrates that every number in those two columns is
/// reproduced within its tightest-possible tolerance.

void TestEckleyTables3and4()
{
    static double const Px[30] =
        {    14.83,     16.21,     17.74,     19.44,     21.32,     23.43,     25.77,     28.39,     31.32,     34.61
        ,    38.31,     42.50,     47.24,     52.64,     58.81,     65.92,     74.17,     83.82,     95.23,    108.88
        ,   125.45,    145.90,    171.66,    205.02,    249.77,    312.71,    407.47,    565.83,    883.12,   1836.08
        };
    // To get end-of-year values, omit Eckley's first element and append $2000.
    static double const Vx[30] =
        {  /* 0.00, */  14.20,     29.75,     46.77,     65.40,     85.76,    108.01,    132.33,    158.90,    187.94
        ,   219.65,    254.25,    291.99,    333.12,    377.95,    426.80,    480.08,    538.22,    601.68,    670.96
        ,   746.59,    829.10,    919.09,   1017.23,   1124.23,   1240.85,   1367.94,   1506.36,   1657.11,   1821.25
        ,  2000.00
        };

    // Eckley's final COI rate is superfluous.
    std::vector<double>coi
        {  0.00200,   0.00206,   0.00214,   0.00224,   0.00236,   0.00250,   0.00265,   0.00282,   0.00301,   0.00324
        ,  0.00350,   0.00382,   0.00419,   0.00460,   0.00504,   0.00550,   0.00596,   0.00645,   0.00697,   0.00756
        ,  0.00825,   0.00903,   0.00990,   0.01088,   0.01199,   0.01325,   0.01469,   0.01631,   0.01811,   0.02009
        ,/*0.02225 */
        };

    std::vector<double>ic  (coi.size(), 0.10);
    std::vector<double>ig  (coi.size(), 0.04);

    ULCommFns CF(coi, ic, ig, mce_option2_for_7702, mce_annual);

    std::vector<double> premium;
    premium <<= (2.0 * CF.aDomega() + CF.kM()) / CF.aN();

    std::vector<double> reserve;
    reserve <<= premium[0] * CF.aD() - CF.kC();
    reserve <<= fwd_sum(reserve) / CF.EaD();

    double tolerance = 0.000005;
    double worst_discrepancy = 0.0;
    for(int j = 0; j < lmi::ssize(coi); ++j)
        {
        double d0 = std::fabs(premium[j] - Px[j] * .001);
        double d1 = std::fabs(reserve[j] - Vx[j] * .001);
        worst_discrepancy = std::max(worst_discrepancy, d0);
        worst_discrepancy = std::max(worst_discrepancy, d1);
        if
            (  tolerance < d0
            || tolerance < d1
            )
            {
            std::cerr
                << "Failed to match Eckley's results at duration "
                << j
                << ".\n"
                << "  differences: " << d0 << ' ' << d1
                << "\n  " << premium[j] << " " << Px[j] * .001 << '\n'
                << "\n  " << reserve[j] << " " << Vx[j] * .001 << '\n'
                << std::endl
                ;
            }
        }
    LMI_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << "Tables 3 and 4; Px and Vx:\n"
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(9)
        << "  " << std::setw(13) << tolerance         << " tolerance\n"
        << "  " << std::setw(13) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
}

/// Exactly reproduce Table 5 from Eckley's paper.
///
/// Table 5 on page 32 of TSA XXIX is the most advanced example that's
/// applicable to the present work, and the only one that uses monthly
/// functions. Only its last three columns (Dx, Dx12, and Cx12) are
/// crucial; the others represent assumptions or intermediate results
/// upon which the last three depend.
///
/// The table gives results to a precision of six decimals only, so
/// its maximum roundoff error is 0.0000005: half a unit in the sixth
/// decimal place, which is five units in the seventh. This unit test
/// demonstrates that every number in the three crucial columns is
/// reproduced within that tightest-possible tolerance.

void TestEckleyTable5()
{
    static double const Dx[31] =
        {1.000000, 0.909085, 0.826438, 0.751305, 0.683003, 0.620911, 0.564463, 0.513147, 0.466496, 0.424087
        ,0.385533, 0.350483, 0.318621, 0.289655, 0.263322, 0.239382, 0.217620, 0.197835, 0.179850, 0.163499
        ,0.148635, 0.135122, 0.122838, 0.111670, 0.101518, 0.092289, 0.083898, 0.076271, 0.069337, 0.063033
        ,0.057303
        };
    static double const Dx12[31] =
        {0.957613, 0.870553, 0.791410, 0.719462, 0.654054, 0.594594, 0.540538, 0.491397, 0.446724, 0.406112
        ,0.369192, 0.335628, 0.305116, 0.277378, 0.252161, 0.229236, 0.208396, 0.189450, 0.172227, 0.156569
        ,0.142335, 0.129395, 0.117631, 0.106937, 0.097215, 0.088377, 0.080342, 0.073038, 0.066398, 0.060362
        ,0.054874
        };
    static double const Cx12[31] =
        {0.002062, 0.000729, 0.000663, 0.000516, 0.000469, 0.000427, 0.000388, 0.000294, 0.000267, 0.000243
        ,0.000221, 0.000201, 0.000219, 0.000232, 0.000241, 0.000247, 0.000249, 0.000227, 0.000227, 0.000206
        ,0.000187, 0.000170, 0.000155, 0.000141, 0.000116, 0.000106, 0.000096, 0.000087, 0.000079, 0.000072
        ,0.000066
        };

    std::vector<double>coi
        {0.00018,  0.00007,  0.00007,  0.00006,  0.00006,  0.00006,  0.00006,  0.00005,  0.00005,  0.00005
        ,0.00005,  0.00005,  0.00006,  0.00007,  0.00008,  0.00009,  0.00010,  0.00010,  0.00011,  0.00011
        ,0.00011,  0.00011,  0.00011,  0.00011,  0.00010,  0.00010,  0.00010,  0.00010,  0.00010,  0.00010
        ,0.00010
        };

    std::vector<double>ic  (coi.size(), i_upper_12_over_12_from_i<double>()(0.10));
    std::vector<double>ig  (coi.size(), i_upper_12_over_12_from_i<double>()(0.04));

    ULCommFns CF(coi, ic, ig, mce_option2_for_7702, mce_monthly);

    double tolerance = 0.0000005;
    double worst_discrepancy = 0.0;
    for(int j = 0; j < lmi::ssize(coi); ++j)
        {
        double d0 = std::fabs(CF.aD()[j]        - Dx  [j]);
        double d1 = std::fabs(CF.kD()[j] / 12.0 - Dx12[j]);
        double d2 = std::fabs(CF.kC()[j]        - Cx12[j]);
        worst_discrepancy = std::max(worst_discrepancy, d0);
        worst_discrepancy = std::max(worst_discrepancy, d1);
        worst_discrepancy = std::max(worst_discrepancy, d2);
        if
            (  tolerance < d0
            || tolerance < d1
            || tolerance < d2
            )
            {
            std::cerr
                << "Failed to match Eckley's results at duration "
                << j
                << ".\n"
                << "  differences: " << d0 << ' ' << d1 << ' ' << d2 << '\n'
                << std::endl
                ;
            }
        }
    LMI_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << "Table 5; Dx, Dx12, and Cx12:\n"
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(9)
        << "  " << std::setw(13) << tolerance         << " tolerance\n"
        << "  " << std::setw(13) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
}

/// Validate against published numerical results.
///
/// Eckley's paper in TSA XXIX contains five tables:
///   (1) annual basis; iterative application of a Fackler formula
///   (2) like (1), but current and guaranteed interest rates differ
///   (3) like (2), but option B
///   (4) same as (3), but using commutation functions
///   (5) monthly basis; commutation functions; option B
/// Table 1 isn't a useful example for the present purpose.
/// Tables 2, 3, 4, and 5 are all validated here: all their crucial
/// columns are tested, and all numbers in all tested columns match
/// Eckley's published figures exactly.

void ULCommFnsTest()
{
    TestEckleyTable2();
    TestEckleyTables3and4();
    TestEckleyTable5();
}

/// Reproduce published 1954-1958 IET 3% functions almost perfectly.
///
/// The tabular data are from TSA XIII number 37 [1961], Exhibit 4,
/// pages 474, 477-478. Note that N45 contains a typographical error:
///   4767775.863 published
///   4767175.863 intended
/// Within 0.01 for a radix of 1000000, values of Dx, Nx, Cx, and Mx
/// are reproduced. For Dx and Cx, it is further demonstrated that
/// every value matches with a maximum difference of one unit in the
/// last position shown in the table, where half of that difference
/// would be the best achievable; it is no longer known exactly how
/// actuaries performed such arithmetic in 1961, and the one-ulp
/// discrepancy might be our own cumulative roundoff error, but it
/// seems unnecessary to explore the matter further. The published
/// Nx and Mx are demonstrably backward summations of the rounded
/// Dx and Cx shown in the table, whereas ours use the unrounded
/// values that we calculate.

void Test_1954_1958_IET_3pct()
{
    std::vector<double> ell_ex
        {     1000000,       994890,       993477,       992583,       991839,       991214,       990679,       990213,       989797,       989411
        ,      989035,       988639,       988204,       987720,       987177,       986555,       985855,       985076,       984219,       983294
        ,      982311,       981280,       980210,       979122,       978025,       976920,       975797,       974655,       973485,       972278
        ,      971014,       969684,       968278,       966787,       965192,       963474,       961605,       959566,       957330,       954860
        ,      952139,       949130,       945808,       942148,       938116,       933679,       928815,       923493,       917684,       911361
        ,      904480,       897018,       888936,       880198,       870762,       860591,       849644,       837885,       825266,       811748
        ,      797291,       781847,       765373,       747831,       729188,       709427,       688534,       666542,       643453,       619291
        ,      594098,       567922,       540838,       512931,       484284,       454990,       425156,       394902,       364380,       333765
        ,      303269,       273133,       243616,       214996,       187567,       161619,       137421,       115203,        95132,        77311
        ,       61768,        48464,        37300,        28076,        20419,        13999,         8677,         4531,         1744,          349
        };
    static double const Dx[100] =
        {  970873.786,   937779.244,   909172.190,   881897.139,   855569.034,   830126.120,   805512.685,   781683.286,   758596.982,   736214.704
        ,  714499.927,   693411.503,   672918.838,   652999.279,   633631.352,   614788.460,   596458.488,   578628.330,   561286.343,   544426.047
        ,  528040.567,   512122.673,   496664.319,   481663.144,   467110.187,   452992.653,   439293.130,   425999.043,   413094.818,   400565.662
        ,  388393.118,   376564.209,   365066.220,   353887.450,   343013.212,   332429.771,   322121.267,   312075.958,   302280.342,   292718.865
        ,  283383.227,   274259.868,   265339.753,   256614.530,   248074.104,   239709.505,   231515.280,   223484.199,   215610.124,   207887.896
        ,  200309.021,   192870.352,   185565.649,   178389.892,   171337.375,   164403.938,   157585.113,   150877.814,   144277.197,   137780.496
        ,  131385.112,   125087.480,   118885.252,   112777.142,   106762.788,   100844.190,    95023.568,    89309.208,    83704.407,    78214.818
        ,   72847.581,    67609.623,    62510.043,    57557.818,    52760.423,    48125.225,    43659.827,    39371.849,    35270.676,    31366.266
        ,   27670.233,    24194.784,    20951.550,    17951.614,    15205.207,    12720.117,    10500.611,     8546.494,     6851.941,     5406.187
        ,    4193.495,     3194.439,     2386.970,     1744.360,     1231.681,      819.829,      493.354,      250.119,       93.468,       18.159
        };
    static double const Nx[100] =
        {28583343.586, 27612469.800, 26674690.556, 25765518.366, 24883621.227, 24028052.193, 23197926.073, 22392413.388, 21610730.102, 20852133.120
        ,20115918.416, 19401418.489, 18708006.986, 18035088.148, 17382088.869, 16748457.517, 16133669.057, 15537210.569, 14958582.239, 14397295.896
        ,13852869.849, 13324829.282, 12812706.609, 12316042.290, 11834379.146, 11367268.959, 10914276.306, 10474983.176, 10048984.133,  9635889.315
        , 9235323.653,  8846930.535,  8470366.326,  8105300.106,  7751412.656,  7408399.444,  7075969.673,  6753848.406,  6441772.448,  6139492.106
//      , 5846773.241,  5563390.014,  5289130.146,  5023790.393,  4767775.863,  4519101.759,  4279392.254,  4047876.974,  3824392.775,  3608782.651
// Correction (see comment above):                                    ^
        , 5846773.241,  5563390.014,  5289130.146,  5023790.393,  4767175.863,  4519101.759,  4279392.254,  4047876.974,  3824392.775,  3608782.651
        , 3400894.755,  3200585.734,  3007715.382,  2822149.733,  2643759.841,  2472422.466,  2308018.528,  2150433.415,  1999555.601,  1855278.404
        , 1717497.908,  1586112.796,  1461025.316,  1342140.064,  1229362.922,  1122600.134,  1021755.944,   926732.376,   837423.168,   753718.761
        ,  675503.943,   602656.362,   535046.739,   472536.696,   414978.878,   362218.455,   314093.230,   270433.403,   231061.554,   195790.878
        ,  164424.612,   136754.379,   112559.595,    91608.045,    73656.431,    58451.224,    45731.107,    35230.496,    26684.002,    19832.061
        ,   14425.874,    10232.379,     7037.940,     4650.970,     2906.610,     1674.929,      855.100,      361.746,      111.627,       18.159
        };
    static double const Cx[100] =
        {   4816.6651,    1293.0952,     794.3074,     641.7809,     523.4277,     435.0040,     367.8647,     318.8294,     287.2203,     271.6304
        ,    277.7464,     296.2138,     319.9810,     348.5310,     387.6098,     423.5115,     457.5804,     488.7351,     512.1501,     528.4109
        ,    538.0712,     542.1602,     535.2239,     523.9333,     512.3827,     505.5623,     499.1417,     496.4852,     497.2680,     505.5838
        ,    516.4883,     530.0989,     545.7729,     566.8365,     592.7657,     626.0831,     663.1361,     706.0249,     757.1954,     809.8458
        ,    869.4783,     931.9636,     996.8807,    1066.2165,    1139.1400,    1212.3946,    1287.9176,    1364.8262,    1442.3211,    1523.8882
        ,   1604.4255,    1687.1199,    1770.9321,    1856.6950,    1943.0281,    2030.3612,    2117.4412,    2206.1177,    2294.4519,    2382.3605
        ,   2470.8812,    2558.9035,    2645.4327,    2729.5823,    2809.0023,    2883.4123,    2946.6832,    3003.5621,    3051.5968,    3089.1353
        ,   3116.1841,    3130.3680,    3131.5440,    3120.9535,    3098.4864,    3063.6926,    3016.3329,    2954.4200,    2877.1088,    2782.4520
        ,   2669.5200,    2538.5315,    2389.6965,    2223.5447,    2042.2202,    1849.0171,    1648.2732,    1445.6262,    1246.1831,    1055.2307
        ,    876.9151,     714.4271,     573.0865,     461.8727,     375.9772,     302.5966,     228.8660,     149.3660,      72.5858,      17.6305
        };
    static double const Mx[100] =
        { 138349.2156,  133532.5505,  132239.4553,  131445.1479,  130803.3670,  130279.9393,  129844.9353,  129477.0706,  129158.2412,  128871.0209
        , 128599.3905,  128321.6441,  128025.4303,  127705.4493,  127356.9183,  126969.3085,  126545.7970,  126088.2166,  125599.4815,  125087.3314
        , 124558.9205,  124020.8493,  123478.6891,  122943.4652,  122419.5319,  121907.1492,  121401.5869,  120902.4452,  120405.9600,  119908.6920
        , 119403.1082,  118886.6199,  118356.5210,  117810.7481,  117243.9116,  116651.1459,  116025.0628,  115361.9267,  114655.9018,  113898.7064
        , 113088.8606,  112219.3823,  111287.4187,  110290.5380,  109224.3215,  108085.1815,  106872.7869,  105584.8693,  104220.0431,  102777.7220
        , 101253.8338,   99649.4083,   97962.2884,   96191.3563,   94334.6613,   92391.6332,   90361.2720,   88243.8308,   86037.7131,   83743.2612
        ,  81360.9007,   78890.0195,   76331.1160,   73685.6833,   70956.1010,   68147.0987,   65263.6864,   62317.0032,   59313.4411,   56261.8443
        ,  53172.7090,   50056.5249,   46926.1569,   43794.6129,   40673.6594,   37575.1730,   34511.4804,   31495.1475,   28540.7275,   25663.6187
        ,  22881.1667,   20211.6467,   17673.1152,   15283.4187,   13059.8740,   11017.6538,    9168.6367,    7520.3635,    6074.7373,    4828.5542
        ,   3773.3235,    2896.4084,    2181.9813,    1608.8948,    1147.0221,     771.0449,     468.4483,     239.5823,      90.2163,      17.6305
        };

    std::vector<double> E_ell_ex(ell_ex);
    E_ell_ex.erase(E_ell_ex.begin());
    E_ell_ex.push_back(0.0);
    std::vector<double> q;
    q <<= (ell_ex - E_ell_ex) / ell_ex;

    std::vector<double> i(q.size(), 0.03);

    OLCommFns CF(q, i);

    double tolerance = 0.01;
    double worst_discrepancy = 0.0;
    for(int j = 0; j < lmi::ssize(q); ++j)
        {
        double d0 = std::fabs(Dx[0] * CF.D()[j] - Dx[j]);
        double d1 = std::fabs(Dx[0] * CF.N()[j] - Nx[j]);
        double d2 = std::fabs(Dx[0] * CF.C()[j] - Cx[j]);
        double d3 = std::fabs(Dx[0] * CF.M()[j] - Mx[j]);
        worst_discrepancy = std::max(worst_discrepancy, d0);
        worst_discrepancy = std::max(worst_discrepancy, d1);
        worst_discrepancy = std::max(worst_discrepancy, d2);
        worst_discrepancy = std::max(worst_discrepancy, d3);
        if
            (  tolerance < d0
            || tolerance < d1
            || tolerance < d2
            || tolerance < d3
            // One ulp for Dx and Cx: see comment above.
            || 0.001  < d0
            || 0.0001 < d2
            )
            {
            std::cerr
                << "Failed to match published IET results at duration "
                << j
                << ".\n"
                << "  differences: " << d0 << ' ' << d1 << ' ' << d2 << ' ' << d3 << '\n'
                << "  values:\n"
                << std::setiosflags(std::ios_base::fixed)
                << std::setprecision(3) << std::setw(13) << "    Dx " << CF.D()[j] * Dx[0] << '\n'
                << std::setprecision(3) << std::setw(13) << "    Nx " << CF.N()[j] * Dx[0] << '\n'
                << std::setprecision(4) << std::setw(13) << "    Cx " << CF.C()[j] * Dx[0] << '\n'
                << std::setprecision(4) << std::setw(13) << "    Mx " << CF.M()[j] * Dx[0] << '\n'
                << std::endl
                ;
            }
        }
    LMI_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << "1954-1958 IET 3%; Dx, Nx, Cx, and Mx:\n"
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(9)
        << "  " << std::setw(13) << tolerance         << " tolerance\n"
        << "  " << std::setw(13) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
}

void OLCommFnsTest()
{
    Test_1954_1958_IET_3pct();
}

/// Comprehensive UL example.
///
/// Calculate year-by-year option B account value for a no-load UL
/// contract; compare to results imported from a spreadsheet, with
/// a comparison tolerance of 1.0e-13 (cf. 'materially_equal.hpp').
///
/// This example calculates and uses a premium to endow for ten times
/// the specified amount. It is worth pointing out that the 7702
/// corridor (calculated using option A and four percent) would be
/// entered (at ages 33 through 92 inclusive), and the actual account
/// value would therefore differ. That's one important reason why UL
/// commutation functions cannot replace a general monthiversary loop.
///
/// The monthly COI rate is limited to one-eleventh: one-twelfth
/// divided by one minus itself, because deducting the COI charge at
/// the beginning of the month increases the amount actually at risk:
///   https://lists.nongnu.org/archive/html/lmi/2009-09/msg00001.html
/// It is interesting to substitute a limit of unity and observe the
/// effect on account value in the last few years before maturity;
/// that exercise shows why a COI limit of unity is impractical.
///
/// A no-load account value is the same thing as a terminal reserve
/// calculated on a monthly basis. The negative first value may seem
/// surprising at first glance, but see Donald B. Warren's article
/// "A Discussion of Negative Reserves" in _The Actuary_, Volume 2,
/// Number 8, October 1968, page 4, which says negative reserves can
/// occur "in the first policy year at age 0 on a whole life plan".

void Test_1980_CSO_Male_ANB()
{
    static double const Vx[100] =
        { -0.000473738046570238,  0.002247295730502500,  0.005241698768037170,  0.008456078393915220,  0.011926553664244400
        ,  0.015691769467939000,  0.019761995744489900,  0.024179297721835100,  0.028947251189598200,  0.034069685539281000
        ,  0.039561057270306300,  0.045395407505165400,  0.051555313767552100,  0.058001406638023400,  0.064732980666241200
        ,  0.071749271581902600,  0.079070180589403500,  0.086737735362197100,  0.094828018387171400,  0.103401718256485000
        ,  0.112534146880374000,  0.122295523126232000,  0.132760980422963000,  0.143990173553492000,  0.156046931411632000
        ,  0.168999548728467000,  0.182900369141144000,  0.197795038675394000,  0.213742765788671000,  0.230796542052218000
        ,  0.249023433690621000,  0.268474476756488000,  0.289235369742500000,  0.311366720949798000,  0.334954107313755000
        ,  0.360078731819492000,  0.386827479699908000,  0.415282949068700000,  0.445543880422912000,  0.477705552320072000
        ,  0.511880273535237000,  0.548167462221615000,  0.586714977324977000,  0.627639523709730000,  0.671097094012277000
        ,  0.717223451381436000,  0.766194995364631000,  0.818179696136827000,  0.873367657041346000,  0.931931091773772000
        ,  0.994075027674959000,  1.059956390662080000,  1.129763798156960000,  1.203678136735230000,  1.281882375519170000
        ,  1.364613829788600000,  1.452105427192480000,  1.544648036036500000,  1.642521430146680000,  1.746014332704110000
        ,  1.855383042188360000,  1.970880381760790000,  2.092734253416150000,  2.221156076811710000,  2.356381500538860000
        ,  2.498672040708780000,  2.648359273853140000,  2.805786375699680000,  2.971319414972850000,  3.145274051622660000
        ,  3.327846632421630000,  3.519146679140350000,  3.719167032899280000,  3.927805107918910000,  4.145027867087800000
        ,  4.370910301296330000,  4.605645006036610000,  4.849530703628860000,  5.102948373912990000,  5.366096833315370000
        ,  5.638870088173620000,  5.920892749892140000,  6.211406544675630000,  6.509423574587240000,  6.814138523878080000
        ,  7.124963331396270000,  7.441556798621410000,  7.763670987547250000,  8.091088523083430000,  8.423497506567250000
        ,  8.760319112455520000,  9.100519387440430000,  9.442178781312970000,  9.781775452586110000, 10.110922159784500000
        , 10.411786134120300000, 10.645096296084700000, 10.716449549065400000, 10.370818742354000000, 10.000000000000000000
        };

    std::vector<double> q(sample_q());
    q <<= apply_binary(coi_rate_from_q<double>(), q, 1.0 / 11.0);

    std::vector<double>ic(q.size(), i_upper_12_over_12_from_i<double>()(0.07));
    std::vector<double>ig(q.size(), i_upper_12_over_12_from_i<double>()(0.03));

    ULCommFns ulcf(q, ic, ig, mce_option2_for_7702, mce_monthly);

    double premium = (10.0 * ulcf.aDomega() + ulcf.kM()[0]) / ulcf.aN()[0];
    std::vector<double> reserve;
    reserve <<= premium * ulcf.aD() - ulcf.kC();
    reserve <<= fwd_sum(reserve) / ulcf.EaD();

    double tolerance = 1.0e-13;
    double worst_discrepancy = 0.0;
    for(int j = 0; j < lmi::ssize(q); ++j)
        {
        double d0 = std::fabs(reserve[j] - Vx[j]);
        worst_discrepancy = std::max(worst_discrepancy, d0);
        if
            (  tolerance < d0
            )
            {
            std::cerr
                << "Failed to match stored results at duration "
                << j
                << ".\n"
                << "  difference: " << d0
                << "\n  " << reserve[j] << " " << Vx[j] << '\n'
                << std::endl
                ;
            }
        }
    LMI_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << "Yearly account values:\n"
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(17)
        << "  " << std::setw(21) << tolerance         << " tolerance\n"
        << "  " << std::setw(21) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
}

void assay_speed()
{
    std::vector<double> q(sample_q());
    q <<= apply_binary(coi_rate_from_q<double>(), q, 1.0 / 11.0);

    std::vector<double>ic(q.size(), i_upper_12_over_12_from_i<double>()(0.07));
    std::vector<double>ig(q.size(), i_upper_12_over_12_from_i<double>()(0.03));

    ULCommFns ulcf(q, ic, ig, mce_option2_for_7702, mce_monthly);

    std::vector<double> reserve;
    reserve.reserve(q.size());

    auto f0 = [&q, &ic]         {mete_olcf(q, ic);};
    auto f1 = [&q, &ic, ig]     {mete_ulcf(q, ic, ig);};
    auto f2 = [&ulcf, &reserve] {mete_reserve(ulcf, reserve);};
    std::cout
        << "\n  Speed tests..."
        << "\n  olcf construct: " << TimeAnAliquot(f0)
        << "\n  ulcf construct: " << TimeAnAliquot(f1)
        << "\n  ulcf reserve  : " << TimeAnAliquot(f2)
        << std::endl
        ;
}

/// Test UL commutation functions in extreme cases.
///
/// For example, ic and ig can both be zero, and qc may round to zero
/// for a Frasierized survivorship contract.

void TestLimits()
{
    std::vector<double> zero(10, 0.0);
    ULCommFns ulcf(zero, zero, zero, mce_option1_for_7702, mce_monthly);
    LMI_TEST_EQUAL(1.0, ulcf.aDomega());
    LMI_TEST_EQUAL(0.0, ulcf.kC().back());
}

int test_main(int, char*[])
{
    ULCommFnsTest();
    OLCommFnsTest();
    Test_1980_CSO_Male_ANB();
    TestLimits();
    assay_speed();

    return EXIT_SUCCESS;
}
