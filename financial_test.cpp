// Financial functions--unit test.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#if defined __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // defined __BORLANDC__

#include "financial.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

#if !defined __BORLANDC__
#   include <boost/bind.hpp>
#endif // !defined __BORLANDC__

#include <cmath>                        // std::fabs()
#include <iomanip>                      // Formatting of optional detail.
#include <iostream>
#include <vector>

int test_main(int, char*[])
{
    double pmts[3] = {100.0,  200.0,  300.0};
    double bfts[3] = {300.0, 1500.0, 5400.0};

    // The next few tests compare floating-point quantities for exact
    // equality. Often that's inappropriate; however, the quantities
    // are integer-valued and the algorithm is designed to round them
    // exactly.

    irr_helper<double*> xxx(pmts, pmts + 1, bfts[0], 5);
    BOOST_TEST(2.0 == xxx());

    BOOST_TEST(( 2.0 == irr_helper<double*>(pmts, pmts + 1, bfts[0], 5)()));

    BOOST_TEST(( 2.0 == irr_helper<double*>(pmts, pmts + 3, bfts[2], 5)()));

    BOOST_TEST((-1.0 == irr_helper<double*>(pmts, pmts + 3, 0.0    , 5)()));

    // Test with arrays.
    double cash_flows[4] = {pmts[0], pmts[1], pmts[2], -bfts[2]};
    BOOST_TEST(2.0 == irr(cash_flows, 4 + cash_flows, 5));

    // Test with vectors.
    std::vector<double> v(cash_flows, 4 + cash_flows);
    BOOST_TEST(2.0 == irr(v.begin(), v.end(), 0.0, 5));
    BOOST_TEST(2.0 == irr(v.begin(), v.end(), 5));

    std::vector<double> p; // Payments.
    std::vector<double> b; // Benefits.
    for(int j = 1; j < 101; ++j)
        {
        p.push_back(j);
        b.push_back(100 * j);
        }
    std::vector<double> results(p.size());
    irr(p.begin(), p.end(), b.begin(), results.begin(), 5);

/* TODO ?? Reconsider:

Formerly, we used this tolerance:
    double const tolerance = 0.000005;
but the algorithm is good only to within double that tolerance.
Consider making it find irr to within that undoubled tolerance.

std::cout << std::setprecision(12);
std::cout << "results[0]: " << results[0] << '\n';
std::cout << "99.00000 - results[ 0]: " << (99.00000 - results[ 0]) << '\n';
std::cout << "std::fabs(99.00000 - results[ 0]): " << std::fabs(99.00000 - results[ 0]) << '\n';
std::cout << "tolerance = .000005: " << .000005 << '\n';
*/

    double const tolerance = 0.000010;

    BOOST_TEST(std::fabs(99.00000 - results[ 0]) <= tolerance);
    BOOST_TEST(std::fabs(12.17744 - results[ 1]) <= tolerance);
    BOOST_TEST(std::fabs( 4.95525 - results[ 2]) <= tolerance);
    BOOST_TEST(std::fabs( 2.86816 - results[ 3]) <= tolerance);
    BOOST_TEST(std::fabs( 1.94788 - results[ 4]) <= tolerance);
    BOOST_TEST(std::fabs( 1.44618 - results[ 5]) <= tolerance);
    BOOST_TEST(std::fabs( 1.13570 - results[ 6]) <= tolerance);
    BOOST_TEST(std::fabs( 0.92674 - results[ 7]) <= tolerance);
    BOOST_TEST(std::fabs( 0.77750 - results[ 8]) <= tolerance);
    BOOST_TEST(std::fabs( 0.66612 - results[ 9]) <= tolerance);
    BOOST_TEST(std::fabs( 0.01757 - results[99]) <= tolerance);

    // Test const vectors.
    std::vector<double> const cp(p);
    std::vector<double> const cb(b);
    irr(cp.begin(), cp.end(), cb.begin(), results.begin(), 5);

    // Test const references to const vectors.
    std::vector<double> const& crcp(cp);
    std::vector<double> const& crcb(cb);
    irr(crcp.begin(), crcp.end(), crcb.begin(), results.begin(), 5);

    // Test specialized irr() for life insurance, reflecting lapse duration.

    irr(p, b, results, p.size(), p.size(), 5);
    BOOST_TEST(std::fabs(99.00000 - results[ 0]) <= tolerance);
    BOOST_TEST(std::fabs(12.17744 - results[ 1]) <= tolerance);
    BOOST_TEST(std::fabs( 4.95525 - results[ 2]) <= tolerance);
    BOOST_TEST(std::fabs( 2.86816 - results[ 3]) <= tolerance);
    BOOST_TEST(std::fabs( 1.94788 - results[ 4]) <= tolerance);
    BOOST_TEST(std::fabs( 1.44618 - results[ 5]) <= tolerance);
    BOOST_TEST(std::fabs( 1.13570 - results[ 6]) <= tolerance);
    BOOST_TEST(std::fabs( 0.92674 - results[ 7]) <= tolerance);
    BOOST_TEST(std::fabs( 0.77750 - results[ 8]) <= tolerance);
    BOOST_TEST(std::fabs( 0.66612 - results[ 9]) <= tolerance);
    BOOST_TEST(std::fabs( 0.01757 - results[99]) <= tolerance);

    irr(p, b, results, 9, p.size(), 5);
    BOOST_TEST(std::fabs(99.00000 - results[ 0]) <= tolerance);
    BOOST_TEST(std::fabs(12.17744 - results[ 1]) <= tolerance);
    BOOST_TEST(std::fabs( 4.95525 - results[ 2]) <= tolerance);
    BOOST_TEST(std::fabs( 2.86816 - results[ 3]) <= tolerance);
    BOOST_TEST(std::fabs( 1.94788 - results[ 4]) <= tolerance);
    BOOST_TEST(std::fabs( 1.44618 - results[ 5]) <= tolerance);
    BOOST_TEST(std::fabs( 1.13570 - results[ 6]) <= tolerance);
    BOOST_TEST(std::fabs( 0.92674 - results[ 7]) <= tolerance);
    BOOST_TEST(std::fabs( 0.77750 - results[ 8]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 9]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[99]) <= tolerance);

    // Test empty payment interval.

    // This version leaves 'results' unchanged. Test it to make
    // sure it doesn't segfault.
    irr(p.begin(), p.begin(), b.begin(), results.begin(), 5);

    // This version fills 'results' with -100%.
    irr(p, b, results, 0, p.size(), 5);
    BOOST_TEST(std::fabs(-1.00000 - results[ 0]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 1]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 2]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 3]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 4]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 5]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 6]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 7]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 8]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[ 9]) <= tolerance);
    BOOST_TEST(std::fabs(-1.00000 - results[99]) <= tolerance);

    // Test fv().

    static long double const i = .05L;
    static double const one_plus_i = 1.0L + i;
    std::vector<double> accum_p(p.size());
    accum_p[0] = p[0] * one_plus_i;
    for(unsigned int j = 1; j < p.size(); j++)
        {
        accum_p[j] = (accum_p[j - 1] + p[j]) * one_plus_i;
        }
    BOOST_TEST
        (std::fabs(fv(p.begin(), p.end(), i) - accum_p.back())
        <= tolerance
        );

#if !defined __BORLANDC__
    typedef std::vector<double>::iterator VI;
    int const decimals = 5;
    std::cout
        << "  Speed test: vector of irrs, length "
        << p.size()
        << ", "
        << decimals
        << " decimals\n    "
        << TimeAnAliquot
            (boost::bind
                (irr<VI,VI,VI>
                ,p.begin()
                ,p.end()
                ,b.begin()
                ,results.begin()
                ,decimals
                )
            )
        << '\n'
        ;
#endif // !defined __BORLANDC__

    return 0;
}

