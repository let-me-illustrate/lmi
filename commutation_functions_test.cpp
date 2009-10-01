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

// $Id: commutation_functions_test.cpp,v 1.18 2009-10-01 15:15:11 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_commfns.hpp"

#include "math_functors.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/bind.hpp>

#include <algorithm>
#include <cmath>      // std::pow()
#include <fstream>
#include <functional>
#include <iomanip>    // std::setw() etc.
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

// TODO ?? Make these tests meaningful, or expunge them.

//============================================================================
void ULCommFns::SelfTest()
{
    std::ofstream os("ulcf.txt", ios_out_trunc_binary());
    static double const COI[30] =   // TSA XXIX, page 32, table 5
        {
        .00018,.00007,.00007,.00006,.00006,.00006,.00006,.00005,.00005,.00005,
        .00005,.00005,.00006,.00007,.00008,.00009,.00010,.00010,.00011,.00011,
        .00011,.00011,.00011,.00011,.00010,.00010,.00010,.00010,.00010,.00010,
        };
/*
[#include "math_functors.hpp"]
    std::vector<double>coi          (COI, COI + lmi_array_size(COI));
    std::vector<double>ic           (coi.size(), i_upper_12_over_12_from_i<double>()(0.10));
    std::vector<double>ig           (coi.size(), i_upper_12_over_12_from_i<double>()(0.04));

    ULCommFns CF
        (coi
        ,ic
        ,ig
        ,OptionB
        ,Monthly
        ,Annual
        ,Monthly
        );
*/
    std::vector<double>coi          (COI, COI + lmi_array_size(COI));
    std::vector<double>ic           (coi.size(), 0.10);
    std::vector<double>ig           (coi.size(), 0.04);
    for(unsigned int j = 0; j < coi.size(); j++)
        {
        coi[j] = 1.0 - std::pow(1.0 - coi[j], 12.0);
        }

    Timer timer;

// 0 extra leaks if #ifdef out remainder of fn
    int const trials = 1000;
    for(int j = 0; j < trials; j++)
        {
        ULCommFns
            (coi
            ,ic
            ,ig
            ,mce_option2
            ,mce_annual
            ,mce_annual
            ,mce_monthly
            );
        }
//timer.stop();
//timer.elapsed_msec_str();
//string xxx = foo();
// 1 extra leaks if #ifdef out remainder of fn
    os
        << "Commutation function calculation time for "
        << trials
        << " trials: "
        << timer.stop().elapsed_msec_str()
//      << timer.elapsed_msec_str()
///     << xxx
        << "\n\n"
        ;

// 1 extra leak if #ifdef out remainder of fn
    ULCommFns CF
        (coi
        ,ic
        ,ig
        ,mce_option2
        ,mce_annual
        ,mce_annual
        ,mce_monthly
        );

    os << "Universal life commutation functions\n";
    os
        << std::setw( 3) << "yr"
        << std::setw( 6) << "i"
        << std::setw( 9) << "q"
        << std::setw(13) << "aD"
        << std::setw(13) << "kD"
        << std::setw(13) << "kC"
        << '\n'
        ;
    for(unsigned int j = 0; j < coi.size(); j++)
        {
        os
            << std::setw(3)  << j
            << std::setiosflags(std::ios_base::fixed)
            << std::setprecision(3)
            << std::setw(6)  << ic[j]
            << std::setprecision(6)
            << std::setw(9)  << coi[j]
            << std::setprecision(9)
            << std::setw(13) << CF.aD()[j]
            << std::setw(13) << CF.kD()[j]
            << std::setw(13) << CF.kC()[j]
            << '\n'
            ;
        }
    os << '\n';
}

//============================================================================
void OLCommFns::SelfTest()
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

// TODO ?? This doesn't actually test its results.

int test_main(int, char*[])
{
    ULCommFns::SelfTest();
    OLCommFns::SelfTest();

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

