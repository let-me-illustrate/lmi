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

// $Id: commutation_functions_test.cpp,v 1.25 2009-10-03 17:32:15 chicares Exp $

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
        ,mce_annual
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
        ,mce_annual
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

