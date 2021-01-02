// Financial functions--unit test.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "financial.hpp"

#include "materially_equal.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <cmath>                        // fabs()
#include <functional>                   // bind()
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

    // For this block of numerical tests, ask for IRR to be calculated
    // to many more decimal places than a normal FPU can represent, to
    // make sure that's handled gracefully and, more importantly, to
    // get the most accurate result achievable, in order to make the
    // NPV at the IRR rate close to zero. The constants to which
    // results are compared were pasted from gnumeric.

    irr(p.begin(), p.end(), b.begin(), results.begin(), 123);

    // Curiously, gnumeric results for durations 7..9 don't match as
    // closely as for earlier durations or for later duration 99.

    BOOST_TEST(materially_equal(99.000000000000000, results[ 0]));
    BOOST_TEST(materially_equal(12.177446878757825, results[ 1]));
    BOOST_TEST(materially_equal( 4.955259671231229, results[ 2]));
    BOOST_TEST(materially_equal( 2.868166276393760, results[ 3]));
    BOOST_TEST(materially_equal( 1.947887419010502, results[ 4]));
    BOOST_TEST(materially_equal( 1.446186004776299, results[ 5]));
    BOOST_TEST(materially_equal( 1.135702229722491, results[ 6]));
    BOOST_TEST(materially_equal( 0.926742991091295, results[ 7], 1e-9));
    BOOST_TEST(materially_equal( 0.777508398535212, results[ 8], 1e-9));
    BOOST_TEST(materially_equal( 0.666120736082859, results[ 9], 1e-9));
    BOOST_TEST(materially_equal( 0.017575671480682, results[99]));

    // For any stream, NPV at the IRR rate should ideally be zero.
    std::vector<double> q{p};
    q.push_back(-b.back());
    // This NPV is -9.777068044058979E-12 in a gnumeric spreadsheet,
    // versus -9.86988e-014 with MinGW-w64 gcc-6.3.0; the 1e-13
    // tolerance is simply the materially_equal() default.
    BOOST_TEST(std::fabs(npv(q.begin(), q.end(), results.back())) <= 1e-13);

    // Trivially, NPV at 0% interest is summation.
    BOOST_TEST(materially_equal(-4950.0L, npv(q.begin(), q.end(), 0.0)));

    // Test const vectors.
    std::vector<double> const cp(p);
    std::vector<double> const cb(b);
    irr(cp.begin(), cp.end(), cb.begin(), results.begin(), 5);

    // Test const references to const vectors.
    std::vector<double> const& crcp(cp);
    std::vector<double> const& crcb(cb);
    irr(crcp.begin(), crcp.end(), crcb.begin(), results.begin(), 5);

    // Unlike the numerical tests above, those below calculate IRR to
    // a more typical five decimals.
    int const decimals = 5;
    double const tolerance = 0.000005;

    // Test specialized irr() for life insurance, reflecting lapse duration.

    irr(p, b, results, p.size(), p.size(), decimals);
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

    irr(p, b, results, 9, p.size(), decimals);
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
    irr(p.begin(), p.begin(), b.begin(), results.begin(), decimals);

    // This version fills 'results' with -100%.
    irr(p, b, results, 0, p.size(), decimals);
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

    // Test nonempty cashflow streams consisting only of zeros.

    std::vector<double> p0(7);       // Payments.
    std::vector<double> b0(7);       // Benefits.
    std::vector<double> r0(7, 3.14); // Results.

    // This version leaves 'results' unchanged. Test it to make
    // sure it doesn't segfault.
    irr(p0.begin(), p0.begin(), b0.begin(), r0.begin(), decimals);
    BOOST_TEST_EQUAL(r0[3], 3.14);

    // SOMEDAY !! The zero polynomial has an infinitude of roots,
    // but, given that we must return only one, wouldn't some
    // value like 0% or -100% be more suitable?
    irr(p0, b0, r0, p0.size(), p0.size(), decimals);
    BOOST_TEST_EQUAL(r0[3], 1000);

    // Test fv().

    static double const i = .05;
    static double const one_plus_i = 1.0 + i;
    std::vector<double> accum_p(p.size());
// Avoid a gcc-9 anomaly--see:
//   https://lists.nongnu.org/archive/html/lmi/2020-04/msg00024.html
//   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94867
#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wnull-dereference"
#endif // defined __GNUC__
    accum_p[0] = p[0] * one_plus_i;
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
    for(int j = 1; j < lmi::ssize(p); ++j)
        {
        accum_p[j] = (accum_p[j - 1] + p[j]) * one_plus_i;
        }
    BOOST_TEST
        (std::fabs(fv(p.begin(), p.end(), i) - accum_p.back())
        <= tolerance
        );

    typedef std::vector<double>::iterator VI;
    std::cout
        << "  Speed test: vector of irrs, length "
        << p.size()
        << ", "
        << decimals
        << " decimals\n    "
        << TimeAnAliquot
            (std::bind
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

    return 0;
}
