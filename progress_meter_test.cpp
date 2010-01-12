// Show progress on lengthy operations--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

#include "test_tools.hpp"

#include <sstream>

class progress_meter_test
{
  public:
    static void test()
        {
        progress_meter_test::test_normal_usage();
        progress_meter_test::test_quiet_display_mode();
        progress_meter_test::test_distinct_metered_operations();
        progress_meter_test::test_empty_title_and_zero_max_count();
        progress_meter_test::test_invalid_display_mode();
        progress_meter_test::test_postcondition_failure();
        }

  private:
    static void test_normal_usage();
    static void test_quiet_display_mode();
    static void test_distinct_metered_operations();
    static void test_empty_title_and_zero_max_count();
    static void test_invalid_display_mode();
    static void test_postcondition_failure();
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
        // Intended use: do some work, then call reflect_progress() at
        // end of block, then call culminate() right after block ends.
        BOOST_TEST(meter->reflect_progress());
        // Of course, you could do more work here, too, if you like,
        // but that's generally not the best idea.
        }
    meter->culminate();
    BOOST_TEST_EQUAL
        ("Some title...\n"
        ,progress_meter_unit_test_stream().str()
        );
    BOOST_TEST_THROW
        (meter->reflect_progress()
        ,std::runtime_error
        ,"Progress meter maximum count exceeded."
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
    meter->culminate();
}

void progress_meter_test::test_distinct_metered_operations()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 3;

    boost::shared_ptr<progress_meter> meter0
        (create_progress_meter
            (max_count
            ,"Operation 0"
            ,progress_meter::e_unit_test_mode
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        BOOST_TEST(meter0->reflect_progress());
        }
    meter0->culminate();

    boost::shared_ptr<progress_meter> meter1
        (create_progress_meter
            (max_count
            ,"Operation 1"
            ,progress_meter::e_unit_test_mode
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        BOOST_TEST(meter1->reflect_progress());
        }
    meter1->culminate();

    BOOST_TEST_EQUAL
        ("Operation 0...\nOperation 1...\n"
        ,progress_meter_unit_test_stream().str()
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
    meter->culminate();
    BOOST_TEST_EQUAL
        ("\n"
        ,progress_meter_unit_test_stream().str()
        );
    BOOST_TEST_THROW
        (meter->reflect_progress()
        ,std::runtime_error
        ,"Progress meter maximum count exceeded."
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

void progress_meter_test::test_postcondition_failure()
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
    std::cout
        << "Expect '3 iterations expected, but only 0 completed.':"
        << std::endl
        ;
    meter->culminate();

    for(int i = 0; i < max_count; ++i)
        {
        try
            {
            if(1 == i)
                {
                throw "Thrown and caught before meter incremented.";
                }
            BOOST_TEST(meter->reflect_progress());
            }
        catch(...)
            {
            }
        }
    std::cout
        << "Expect '3 iterations expected, but only 2 completed.':"
        << std::endl
        ;
    meter->culminate();
}

int test_main(int, char*[])
{
    progress_meter_test::test();
    return EXIT_SUCCESS;
}

