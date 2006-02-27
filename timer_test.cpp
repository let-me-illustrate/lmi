// Measure elapsed time to high resolution--unit test.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: timer_test.cpp,v 1.8 2006-02-27 15:11:08 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/bind.hpp>

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

int test_main(int, char*[])
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

    // Use high-resolution time to measure an interval of about one
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

    std::cout << "  " << aliquot_timer(foo) << '\n';

    X x;
    std::cout << "  " << aliquot_timer(boost::bind(goo, 10, x, x, &x)) << '\n';

    // Test an operation that has to take longer than the hinted
    // time limit, in order to make sure it executes the operation
    // exactly once.
    //
    // TODO ?? It would be better to make aliquot_timer a stateful
    // functor, so that the elapsed time could be queried and tested.
    //
    std::cout << "  " << aliquot_timer(wait_half_a_second, 0.1) << '\n';

    return EXIT_SUCCESS;
}

