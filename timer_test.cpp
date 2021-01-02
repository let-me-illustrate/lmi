// Measure elapsed time to high resolution--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "timer.hpp"

#include "contains.hpp"
#include "miscellany.hpp"
#include "test_tools.hpp"

#include <cmath>                        // log10()
#include <functional>                   // bind()

inline void do_nothing()
{}

void foo()
{
    double volatile d;
    for(int j = 0; j < 100; ++j)
        {
        d = std::log10(1 + j * j);
        stifle_warning_for_unused_value(d);
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
    static void SleepOneSec();
    static void TestExceptions();
    static void TestAliquotTimer();
};

void TimerTest::WaitTenMsec()
{
    Timer timer;
    double limit = 0.01 * timer.frequency_;
    for(;timer.inspect() - timer.time_when_started_ <= limit;) {}
}

/// Test lmi_sleep().

void TimerTest::SleepOneSec()
{
    lmi_sleep(1);
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
        (timer.elapsed_seconds()
        ,std::logic_error
        ,"Timer::elapsed_seconds() called, but timer is still running."
        );
}

void TimerTest::TestAliquotTimer()
{
    std::cout << "  " << TimeAnAliquot(do_nothing) << '\n';

    std::cout << "  " << TimeAnAliquot(foo, 0.1) << '\n';

    X x;
    std::cout << "  " << TimeAnAliquot(std::bind(goo, 10, x, x, &x), 0.1) << '\n';

    std::string takes_too_long = TimeAnAliquot(WaitTenMsec, 0.0099999).str();
    BOOST_TEST(contains(takes_too_long, "took longer"));
    std::cout << "  " << takes_too_long << '\n';

    std::cout << "  " << TimeAnAliquot(WaitTenMsec, 0.099) << '\n';
    std::cout << "  " << TimeAnAliquot(WaitTenMsec, 0.101) << '\n';

    BOOST_TEST_THROW(TimeAnAliquot(WaitTenMsec,  1.0e-100), std::invalid_argument, "");
    BOOST_TEST_THROW(TimeAnAliquot(WaitTenMsec,  0.0     ), std::invalid_argument, "");
    BOOST_TEST_THROW(TimeAnAliquot(WaitTenMsec, -1.0     ), std::invalid_argument, "");

    std::cout << "  " << TimeAnAliquot(SleepOneSec, 2.000) << '\n';
}

int test_main(int, char*[])
{
    TimerTest::TestExceptions();
    TimerTest::TestAliquotTimer();
    return EXIT_SUCCESS;
}
