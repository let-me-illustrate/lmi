// Measure elapsed time to high resolution--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: timer_test.cpp,v 1.2 2005-04-21 15:22:16 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <cmath> // std::fabs()
#include <ctime>

int test_main(int, char*[])
{
    Timer timer;
    std::clock_t first;
    std::clock_t last;
    double elapsed;
    double interval = 1.0;
    double clock_resolution;

    // Coarsely measure resolution of std::clock().
    first = std::clock();
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
    first = std::clock();
    for(;;)
        {
        last = std::clock();
        elapsed = (last - first) / CLOCKS_PER_SEC;
        if(interval <= elapsed)
            {
            break;
            }
        }
    double observed = timer.Stop().Result();
    double relative_error = std::fabs(observed - interval) / interval;

    // Test accuracy of high-resolution timer. Finer tests might be
    // devised, but this one catches gross errors.
    BOOST_TEST_RELATION(relative_error,<,2.0*clock_resolution);

    // Already stopped--can't stop again.
    BOOST_TEST_THROW(timer.Stop(), std::logic_error, "");

    timer.Start();

    // Already running--can't start again.
    BOOST_TEST_THROW(timer.Start(), std::logic_error, "");

    // Still running--can't report interval until stopped.
    BOOST_TEST_THROW(timer.Result(), std::logic_error, "");

    return EXIT_SUCCESS;
}

