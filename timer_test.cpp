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

// $Id: timer_test.cpp,v 1.21 2007-01-26 01:39:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "timer.hpp"

#include "test_tools.hpp"

#if !defined __BORLANDC__
#   include <boost/bind.hpp>
#endif // !defined __BORLANDC__

#include <cfloat>
#include <climits>
#include <cmath>
#include <ctime>

inline void do_nothing()
{}

void foo()
{
    double volatile d;
    for(unsigned int j = 0; j < 100; ++j)
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

struct TimerTest
{
    static void WaitTenMsec();
    static void TestResolution();
    static void TestExceptions();
    static void TestAliquotTimer();
};

void TimerTest::WaitTenMsec()
{
    Timer timer;
    double limit = 0.01 * timer.frequency_;
    for(;timer.inspect() - timer.time_when_started_ <= limit;) {}
}

void TimerTest::TestResolution()
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
}

void TimerTest::TestExceptions()
{
    Timer timer;

    BOOST_TEST_THROW
        (timer.start()
        ,std::logic_error
        ,"Timer::start() called, but timer was already running."
        );

    timer.stop();
    BOOST_TEST_THROW
        (timer.stop()
        ,std::logic_error
        ,"Timer::stop() called, but timer was not running."
        );

    timer.restart();
    BOOST_TEST_THROW
        (timer.restart()
        ,std::logic_error
        ,"Timer::start() called, but timer was already running."
        );

    BOOST_TEST_THROW
        (timer.elapsed_usec()
        ,std::logic_error
        ,"Timer::elapsed_usec() called, but timer is still running."
        );
}

void TimerTest::TestAliquotTimer()
{
    std::cout << "  " << TimeAnAliquot(do_nothing) << '\n';

    std::cout << "  " << TimeAnAliquot(foo, 0.1) << '\n';

#if !defined __BORLANDC__
    X x;
    std::cout << "  " << TimeAnAliquot(boost::bind(goo, 10, x, x, &x), 0.1) << '\n';
#endif // !defined __BORLANDC__

    // Test an operation that has to take longer than the hinted
    // time limit, in order to make sure it doesn't execute the
    // operation after the initial calibration trial.
    //
    // TODO ?? It would be better to use class AliquotTimer directly,
    // so that the elapsed time (and perhaps also the number of
    // iterations) could be queried and tested.
    //
    std::cout << "  " << TimeAnAliquot(WaitTenMsec,  0.002    ) << '\n';
    std::cout << "  " << TimeAnAliquot(WaitTenMsec,  0.0099999) << '\n';

    // Test some other plausible...
    std::cout << "  " << TimeAnAliquot(WaitTenMsec,  0.099    ) << '\n';
    std::cout << "  " << TimeAnAliquot(WaitTenMsec,  0.101    ) << '\n';
    // ...and implausible (hinted) limits.
    BOOST_TEST_THROW(TimeAnAliquot(WaitTenMsec,  1.0e-100), std::invalid_argument, "");
    BOOST_TEST_THROW(TimeAnAliquot(WaitTenMsec,  0.0     ), std::invalid_argument, "");
    BOOST_TEST_THROW(TimeAnAliquot(WaitTenMsec, -1.0     ), std::invalid_argument, "");
}

int test_main(int, char*[])
{
    TimerTest::TestResolution();
    TimerTest::TestExceptions();
    TimerTest::TestAliquotTimer();
    return EXIT_SUCCESS;
}

