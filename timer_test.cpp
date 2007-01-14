// Measure elapsed time to high resolution--unit test.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: timer_test.cpp,v 1.13 2007-01-14 20:09:07 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "timer.hpp"

#include "test_tools.hpp"

#include <boost/bind.hpp>

#include <climits>
#include <ctime>

void foo()
{
    volatile double d;
    for(unsigned int j = 0; j < 10000; ++j)
        {
        d = std::log10(1U + j * j);
        }
}

class X{};

void goo(int i, X, X const&, X*)
{
    for(int j = 0; j < i; ++j)
        {
        foo();
        }
}

void wait_half_a_second()
{
    std::clock_t first = std::clock();
    for(;;)
        {
        std::clock_t last = std::clock();
        double elapsed = double(last - first) / CLOCKS_PER_SEC;
        if(0.5 <= elapsed)
            {
            break;
            }
        }
}

struct TimerTest
{
    static void TestGreatestNonnegativePowerOfTen();
    static void Test();
};

void TimerTest::TestGreatestNonnegativePowerOfTen()
{
    BOOST_TEST_EQUAL( 0.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen(-1.0e+37));
    BOOST_TEST_EQUAL( 0.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen(-1.0    ));
    BOOST_TEST_EQUAL( 0.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen(-1.0e-37));
    BOOST_TEST_EQUAL( 0.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen(-0.0    ));
    BOOST_TEST_EQUAL( 0.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen( 0.0    ));
    BOOST_TEST_EQUAL( 0.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen( 1.0e-37));

    BOOST_TEST_EQUAL( 1.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen( 1.0    ));

    BOOST_TEST_EQUAL( 1.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen( 9.9    ));
    BOOST_TEST_EQUAL(10.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen(10.0    ));
    BOOST_TEST_EQUAL(10.0  , AliquotTimer<int>::GreatestNonnegativePowerOfTen(10.1    ));

    BOOST_TEST_EQUAL( 1.0e2, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e2   ));
    BOOST_TEST_EQUAL( 1.0e3, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e3   ));
    BOOST_TEST_EQUAL( 1.0e4, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e4   ));
    BOOST_TEST_EQUAL( 1.0e5, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e5   ));
    BOOST_TEST_EQUAL( 1.0e6, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e6   ));
    BOOST_TEST_EQUAL( 1.0e7, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e7   ));
    BOOST_TEST_EQUAL( 1.0e8, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e8   ));
    BOOST_TEST_EQUAL( 1.0e9, AliquotTimer<int>::GreatestNonnegativePowerOfTen(1.0e9   ));

    BOOST_TEST_RELATION(0.1 * LONG_MAX,<=,AliquotTimer<int>::GreatestNonnegativePowerOfTen(LONG_MAX)            );
    BOOST_TEST_RELATION(                  AliquotTimer<int>::GreatestNonnegativePowerOfTen(LONG_MAX),<=,LONG_MAX);

    BOOST_TEST_RELATION(0.1 * LONG_MAX,<=,AliquotTimer<int>::GreatestNonnegativePowerOfTen( 1.0e+37)            );
    BOOST_TEST_RELATION(                  AliquotTimer<int>::GreatestNonnegativePowerOfTen( 1.0e+37),<=,LONG_MAX);
}

// TODO ?? TestGreatestNonnegativePowerOfTen() has been factored out;
// should this monolith be further subdivided?
void TimerTest::Test()
{
    // Coarsely measure resolution of std::clock().
    std::clock_t first = std::clock();
    std::clock_t last;
    double clock_resolution;
    for(;;)
        {
        last = std::clock();
        clock_resolution = double(last - first) / CLOCKS_PER_SEC;
        if(0.0 != clock_resolution)
            {
            break;
            }
        }

    // Use high-resolution timer to measure an interval of about one
    // second.
    Timer timer;
    first = std::clock();
    double interval = 1.0;
    for(;;)
        {
        last = std::clock();
        double elapsed = (last - first) / CLOCKS_PER_SEC;
        if(interval <= elapsed)
            {
            break;
            }
        }
    double observed = timer.stop().elapsed_usec();
    double relative_error = std::fabs(observed - interval) / interval;

    // Test accuracy of high-resolution timer. Finer tests might be
    // devised, but this one catches gross errors.
    BOOST_TEST_RELATION(relative_error,<,2.0*clock_resolution);

    // Already stopped--can't stop again.
    BOOST_TEST_THROW(timer.stop(), std::logic_error, "");

    timer.restart();

    // Already running--can't restart again before stopping.
    BOOST_TEST_THROW(timer.restart(), std::logic_error, "");

    // Still running--can't report interval until stopped.
    BOOST_TEST_THROW(timer.elapsed_usec(), std::logic_error, "");

    std::cout << "  " << TimeAnAliquot(foo) << '\n';

    X x;
    std::cout << "  " << TimeAnAliquot(boost::bind(goo, 10, x, x, &x)) << '\n';

    // Test an operation that has to take longer than the hinted
    // time limit, in order to make sure it executes the operation
    // exactly once.
    //
    // TODO ?? It would be better to use class AliquotTimer directly,
    // so that the elapsed time (and perhaps also the number of
    // iterations) could be queried and tested.
    //
    std::cout << "  " << TimeAnAliquot(wait_half_a_second, 0.1) << '\n';
    // TODO ?? Why does the preceding line measure one iteration
    // after the initial calibration trial?
    std::cout << "  " << TimeAnAliquot(wait_half_a_second, 0.01) << '\n';
}

int test_main(int, char*[])
{
    TimerTest::TestGreatestNonnegativePowerOfTen();
    TimerTest::Test();
    return EXIT_SUCCESS;
}

