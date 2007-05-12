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

// $Id: progress_meter_test.cpp,v 1.8 2007-05-12 23:48:27 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

#include "test_tools.hpp"

#include <sstream>

struct progress_meter_test
{
    static void test_normal_usage();
    static void test_quiet_display_mode();
    static void test_empty_title_and_zero_max_count();
    static void test_invalid_display_mode();
};

void progress_meter_test::test_normal_usage()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 3;
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (max_count
            ,"Some title"
            ,progress_meter::e_unit_test_mode
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        // Intended use: do some work, then update meter at end of block.
        BOOST_TEST(meter->reflect_progress());
        // Of course, you could do more work here, too, if you like.
        }
    BOOST_TEST_EQUAL("Some title...", progress_meter_unit_test_stream().str());
    BOOST_TEST_THROW
        (meter->reflect_progress()
        ,std::logic_error
        ,"progress_meter: max_count_ exceeded."
        );
}

void progress_meter_test::test_quiet_display_mode()
{
    int const max_count = 3;
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (max_count
            ,"ERROR: THIS SHOULD NOT DISPLAY"
            ,progress_meter::e_quiet_display
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        BOOST_TEST(meter->reflect_progress());
        }
    BOOST_TEST_THROW
        (meter->reflect_progress()
        ,std::logic_error
        ,"progress_meter: max_count_ exceeded."
        );
}

void progress_meter_test::test_empty_title_and_zero_max_count()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 0;
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (max_count
            ,""
            ,progress_meter::e_unit_test_mode
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        BOOST_TEST(meter->reflect_progress());
        }
    BOOST_TEST(progress_meter_unit_test_stream().str().empty());
    BOOST_TEST_THROW
        (meter->reflect_progress()
        ,std::logic_error
        ,"progress_meter: max_count_ exceeded."
        );
}

void progress_meter_test::test_invalid_display_mode()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 0;
    BOOST_TEST_THROW
        (boost::shared_ptr<progress_meter> meter
            (create_progress_meter
                (max_count
                ,""
                ,progress_meter::enum_display_mode(99)
                )
            )
        ,std::runtime_error
        ,"Case 99 not found."
        );
}

int test_main(int, char*[])
{
    progress_meter_test::test_normal_usage();
    progress_meter_test::test_empty_title_and_zero_max_count();
    progress_meter_test::test_invalid_display_mode();
    return EXIT_SUCCESS;
}

