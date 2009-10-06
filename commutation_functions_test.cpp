// Ordinary- and universal-life commutation functions--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: commutation_functions_test.cpp,v 1.27 2009-10-06 02:57:22 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_commfns.hpp"

#include "math_functors.hpp"
#include "miscellany.hpp" // ios_out_trunc_binary()
#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/bind.hpp>

#include <algorithm>
#include <cmath>      // std::fabs()
#include <fstream>
#include <functional>
#include <iomanip>    // std::setw() etc.
#include <ios>        // ios_base::fixed()
#include <iterator>   // std::back_inserter()
#include <vector>

namespace
{
// SOA database 'qx_cso' table 42: "1980 US CSO Male Age nearest"
std::vector<double> const& sample_q()
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
    static std::vector<double> const v(q, q + n);
    return v;
}
} // Unnamed namespace.

void mete_ulcf
    (std::vector<double> const& q
    ,std::vector<double> const& ic
    ,std::vector<double> const& ig
    )
{
    ULCommFns
        (q
        ,ic
        ,ig
        ,mce_option1
        ,mce_monthly
        );
}

void mete_corridor
    (ULCommFns const&     ulcf
    ,std::vector<double>& cvat_corridor
    )
{
    std::transform
        (ulcf.aD().begin()
        ,ulcf.aD().end() - 1
        ,ulcf.kM().begin()
        ,cvat_corridor.begin()
        ,std::divides<double>()
        );
}

// To be refactored soon....
#include "et_vector.hpp"
#include "value_cast.hpp"
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

    static double const COI[65] =
        {  0.00200,   0.00206,   0.00214,   0.00224,   0.00236,   0.00250,   0.00265,   0.00282,   0.00301,   0.00324
        ,  0.00350,   0.00382,   0.00419,   0.00460,   0.00504,   0.00550,   0.00596,   0.00645,   0.00697,   0.00756
        ,  0.00825,   0.00903,   0.00990,   0.01088,   0.01199,   0.01325,   0.01469,   0.01631,   0.01811,   0.02009
        ,  0.02225,   0.02456,   0.02704,   0.02979,   0.03289,   0.03645,   0.04058,   0.04526,   0.05043,   0.05599
        ,  0.06185,   0.06798,   0.07450,   0.08153,   0.08926,   0.09785,   0.10762,   0.11855,   0.13039,   0.14278
        ,  0.15545,   0.16827,   0.18132,   0.19506,   0.21012,   0.22700,   0.24613,   0.26655,   0.28547,   0.31127
        ,  0.40000,   0.50000,   0.60000,   0.70000,   1.00000
        };

    std::vector<double>coi (COI, COI + lmi_array_size(COI));
    std::vector<double>ic  (coi.size(), 0.10);
    std::vector<double>ig  (coi.size(), 0.04);

    ULCommFns CF
        (coi
        ,ic
        ,ig
        ,mce_option1
        ,mce_annual
        );

    std::vector<double> nsp    (coi.size());
    nsp     += (CF.aD().back() + CF.kM()) / CF.aD();
    std::vector<double> annuity(coi.size());
    annuity += (                 CF.aN()) / CF.aD();

    double tolerance = 0.0000005;
    double worst_discrepancy = 0.0;
    for(unsigned int j = 0; j < coi.size(); j++)
        {
        double d0 = fabs(nsp    [j]        - Ax  [j]);
        double d1 = fabs(annuity[j]        - ax  [j]);
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
                << "\n  " << value_cast<std::string>(nsp    [j]) << " " << value_cast<std::string>(Ax[j]) << '\n'
                << "\n  " << value_cast<std::string>(annuity[j]) << " " << value_cast<std::string>(ax[j]) << '\n'
                << std::endl
                ;
            }
        }
    BOOST_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(9)
        << "  " << std::setw(13) << tolerance         << " tolerance\n"
        << "  " << std::setw(13) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
}

/// Exactly reproduce a comprehensive example from Eckley's paper.
///
/// Eckley's paper contains five tables:
///   (1) annual basis; iterative application of a Fackler formula
///   (2) like (1), but current and guaranteed interest rates differ
///   (3) like (2), but option B
///   (4) same as (3), but using commutation functions
///   (5) monthly basis; commutation functions; option B
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

void ULCommFnsTest()
{
    TestEckleyTable2(); // To be refactored soon....
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

    static double const COI[31] =
        {0.00018,  0.00007,  0.00007,  0.00006,  0.00006,  0.00006,  0.00006,  0.00005,  0.00005,  0.00005
        ,0.00005,  0.00005,  0.00006,  0.00007,  0.00008,  0.00009,  0.00010,  0.00010,  0.00011,  0.00011
        ,0.00011,  0.00011,  0.00011,  0.00011,  0.00010,  0.00010,  0.00010,  0.00010,  0.00010,  0.00010
        ,0.00010
        };

    std::vector<double>coi (COI, COI + lmi_array_size(COI));
    std::vector<double>ic  (coi.size(), i_upper_12_over_12_from_i<double>()(0.10));
    std::vector<double>ig  (coi.size(), i_upper_12_over_12_from_i<double>()(0.04));

    ULCommFns CF
        (coi
        ,ic
        ,ig
        ,mce_option2
        ,mce_monthly
        );

    double tolerance = 0.0000005;
    double worst_discrepancy = 0.0;
    for(unsigned int j = 0; j < coi.size(); j++)
        {
        double d0 = fabs(CF.aD()[j]        - Dx  [j]);
        double d1 = fabs(CF.kD()[j] / 12.0 - Dx12[j]);
        double d2 = fabs(CF.kC()[j]        - Cx12[j]);
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
    BOOST_TEST_RELATION(worst_discrepancy,<,tolerance);
    std::cout
        << std::setiosflags(std::ios_base::fixed)
        << std::setprecision(9)
        << "  " << std::setw(13) << tolerance         << " tolerance\n"
        << "  " << std::setw(13) << worst_discrepancy << " worst_discrepancy\n"
        << std::endl
        ;
}

void OLCommFnsTest()
{
    std::ofstream os("olcf.txt", ios_out_trunc_binary());

    static double const Q[100] =    // I think this is unisex unismoke ANB 80CSO
        {
        .00354,.00097,.00091,.00089,.00085,.00083,.00079,.00077,.00073,.00072,
        .00071,.00072,.00078,.00087,.00097,.00110,.00121,.00131,.00139,.00144,
        .00148,.00149,.00150,.00149,.00149,.00147,.00147,.00146,.00148,.00151,
        .00154,.00158,.00164,.00170,.00179,.00188,.00200,.00214,.00231,.00251,
        .00272,.00297,.00322,.00349,.00375,.00406,.00436,.00468,.00503,.00541,
        .00583,.00630,.00682,.00742,.00807,.00877,.00950,.01023,.01099,.01181,
        .01271,.01375,.01496,.01639,.01802,.01978,.02164,.02359,.02558,.02773,
        .03016,.03296,.03629,.04020,.04466,.04955,.05480,.06031,.06606,.07223,
        .07907,.08680,.09568,.10581,.11702,.12911,.14191,.15541,.16955,.18445,
        .20023,.21723,.23591,.25743,.28381,.32074,.37793,.47661,.65644,1.0000,
        };

    std::vector<double>q                (Q, Q + lmi_array_size(Q));
    std::vector<double>i                (100, 0.04);

    OLCommFns CF(q, i);

    os << "Ordinary life commutation functions\n";
    os
        << std::setw( 3) << "yr"
        << std::setw( 6) << "i"
        << std::setw( 9) << "q"
        << std::setw(13) << "c"
        << std::setw(13) << "d"
        << std::setw(13) << "m"
        << std::setw(13) << "n"
        << '\n'
        ;
    for(unsigned int j = 0; j < q.size(); j++)
        {
        os
            << std::setw(3)  << j
            << std::setiosflags(std::ios_base::fixed)
            << std::setprecision(3)
            << std::setw(6)  << i[j]
            << std::setprecision(6)
            << std::setw(9)  << q[j]
            << std::setprecision(9)
            << std::setw(13) << CF.C()[j]
            << std::setw(13) << CF.D()[j]
            << std::setw(13) << CF.M()[j]
            << std::setw(13) << CF.N()[j]
            << '\n'
            ;
        }
    os << '\n';
}

int test_main(int, char*[])
{
    ULCommFnsTest();
    OLCommFnsTest();

    // ET !! q = coi_rate_from_q(sample_q, 1.0);
    std::vector<double> q;
    std::transform
        (sample_q().begin()
        ,sample_q().end()
        ,std::back_inserter(q)
        ,std::bind2nd(coi_rate_from_q<double>(), 1.0)
        );

    std::vector<double>ic(q.size(), i_upper_12_over_12_from_i<double>()(0.10));
    std::vector<double>ig(q.size(), i_upper_12_over_12_from_i<double>()(0.04));

    ULCommFns ulcf
        (q
        ,ig
        ,ig
        ,mce_option1
        ,mce_monthly
        );
    std::vector<double> cvat_corridor;
    std::transform
        (ulcf.aD().begin()
        ,ulcf.aD().end() - 1
        ,ulcf.kM().begin()
        ,std::back_inserter(cvat_corridor)
        ,std::divides<double>()
        );
    // TODO ?? Add tests to validate values. Consider using examples
    // published in Eckley's paper.

    std::cout
        << "  Speed test: generate UL commutation functions\n    "
        << TimeAnAliquot
            (boost::bind
                (mete_ulcf
                ,q
                ,ic
                ,ig
                )
            )
        << '\n'
        ;

    std::cout
        << "  Speed test: calculate CVAT corridor factors\n    "
        << TimeAnAliquot
            (boost::bind
                (mete_corridor
                ,boost::ref(ulcf)
                ,cvat_corridor
                )
            )
        << '\n'
        ;

    return EXIT_SUCCESS;
}

