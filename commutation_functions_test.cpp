// Ordinary- and universal-life commutation functions--unit test.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: commutation_functions_test.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_commfns.hpp"
#include "math_functors.hpp"
#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
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

// TODO ?? This doesn't actually test its results.

int test_main(int, char*[])
{
//    TULCommFns::SelfTest(); TODO ?? Make that test meaningful, or expunge it.

    std::vector<double> q;
    std::transform
        (sample_q().begin()
        ,sample_q().end()
        ,std::back_inserter(q)
        ,std::bind2nd(coi_rate_from_q<double>(), 1.0)
        );

    std::vector<double>ic(q.size(), i_upper_12_over_12_from_i<double>()(0.10));
    std::vector<double>ig(q.size(), i_upper_12_over_12_from_i<double>()(0.04));

    Timer timer;
    int const trials = 1000;
    for(int j = 0; j < trials; j++)
        {
        TULCommFns
            (q
            ,ic
            ,ig
            ,e_dbopt(e_option1)
            ,e_mode(e_annual)
            ,e_mode(e_annual)
            ,e_mode(e_monthly)
            );
        }
    timer.Stop();
    std::cout
        << "Generated UL commutation functions "
        << trials
        << " times in "
        << timer.Report()
        << '\n'
        ;

    TULCommFns ulcf
        (q
        ,ig
        ,ig
        ,e_dbopt(e_option1)
        ,e_mode(e_monthly)
        ,e_mode(e_monthly)
        ,e_mode(e_monthly)
        );
    std::vector<double> cvat_corridor;
    std::transform
        (ulcf.aD().begin()
        ,ulcf.aD().end() - 1
        ,ulcf.kM().begin()
        ,std::back_inserter(cvat_corridor)
        ,std::divides<double>()
        );
    std::cout
        << cvat_corridor[ 0] << " [ 0]; "
        << cvat_corridor[45] << " [45]; "
        << cvat_corridor[99] << " [99]\n"
        ;

    timer.Reset().Start();
    cvat_corridor.resize(q.size());
    for(int j = 0; j < trials; j++)
        {
        std::transform
            (ulcf.aD().begin()
            ,ulcf.aD().end() - 1
            ,ulcf.kM().begin()
            ,cvat_corridor.begin()
            ,std::divides<double>()
            );
        }
    timer.Stop();
    std::cout
        << "Calculated CVAT corridor factors for ages 0 through 99 "
        << trials
        << " times in "
        << timer.Report()
        << '\n'
        ;

    return EXIT_SUCCESS;
}

