// Show progress on lengthy operations--unit test.
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

// $Id: progress_meter_test.cpp,v 1.6 2007-05-11 01:28:16 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

#include "test_tools.hpp"

struct progress_meter_test
{
    static void test();
};

void progress_meter_test::test()
{
    int const max_count = 3;
    boost::shared_ptr<progress_meter> meter(create_progress_meter(max_count));
    for(int i = 0; i < max_count; ++i)
        {
        // Intended use: do some work, then update meter at end of block.
        BOOST_TEST(meter->reflect_progress());
        // Of course, you could do more work here, too, if you like.
        }

    BOOST_TEST_THROW(meter->reflect_progress(), std::logic_error, "");
}

int test_main(int, char*[])
{
    progress_meter_test::test();
    return EXIT_SUCCESS;
}

