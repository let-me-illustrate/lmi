// Show progress on lengthy operations--unit test.
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

#include "progress_meter.hpp"

#include "test_tools.hpp"
#include "unwind.hpp"                   // scoped_unwind_toggler

#include <sstream>

class progress_meter_test
{
  public:
    static void test()
        {
        test_normal_usage();
        test_quiet_display_mode();
        test_distinct_metered_operations();
        test_empty_title_and_zero_max_count();
        test_postcondition_failure();
        test_failure_to_culminate();
        }

  private:
    static void test_normal_usage();
    static void test_quiet_display_mode();
    static void test_distinct_metered_operations();
    static void test_empty_title_and_zero_max_count();
    static void test_postcondition_failure();
    static void test_failure_to_culminate();
};

void progress_meter_test::test_normal_usage()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 3;
    std::unique_ptr<progress_meter> meter
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
        LMI_TEST(meter->reflect_progress());
        // Of course, you could do more work here, too, if you like,
        // but that's generally not the best idea.
        }
    meter->culminate();
    LMI_TEST_EQUAL
        ("Some title...\n"
        ,progress_meter_unit_test_stream().str()
        );
}

void progress_meter_test::test_quiet_display_mode()
{
    int const max_count = 3;
    std::unique_ptr<progress_meter> meter
        (create_progress_meter
            (max_count
            ,"ERROR: THIS SHOULD NOT DISPLAY"
            ,progress_meter::e_quiet_display
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        LMI_TEST(meter->reflect_progress());
        }
    meter->culminate();
}

void progress_meter_test::test_distinct_metered_operations()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 3;

    std::unique_ptr<progress_meter> meter0
        (create_progress_meter
            (max_count
            ,"Operation 0"
            ,progress_meter::e_unit_test_mode
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        LMI_TEST(meter0->reflect_progress());
        }
    meter0->culminate();

    // Use a different shared pointer.
    std::unique_ptr<progress_meter> meter1
        (create_progress_meter
            (max_count
            ,"Operation 1"
            ,progress_meter::e_unit_test_mode
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        LMI_TEST(meter1->reflect_progress());
        }
    meter1->culminate();

    // Reuse a shared pointer.
    meter0 = create_progress_meter
        (max_count
        ,"Operation 2"
        ,progress_meter::e_unit_test_mode
        );
    for(int i = 0; i < max_count; ++i)
        {
        LMI_TEST(meter0->reflect_progress());
        }
    meter0->culminate();

    LMI_TEST_EQUAL
        ("Operation 0...\nOperation 1...\nOperation 2...\n"
        ,progress_meter_unit_test_stream().str()
        );
}

void progress_meter_test::test_empty_title_and_zero_max_count()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 0;
    std::unique_ptr<progress_meter> meter
        (create_progress_meter
            (max_count
            ,""
            ,progress_meter::e_unit_test_mode
            )
        );
    for(int i = 0; i < max_count; ++i)
        {
        LMI_TEST(meter->reflect_progress());
        }
    meter->culminate();
    LMI_TEST_EQUAL
        ("\n"
        ,progress_meter_unit_test_stream().str()
        );
}

void progress_meter_test::test_postcondition_failure()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 3;
    std::unique_ptr<progress_meter> meter
        (create_progress_meter
            (max_count
            ,"Some title"
            ,progress_meter::e_unit_test_mode
            )
        );
    LMI_TEST_THROW
        (meter->culminate()
        ,std::runtime_error
        ,"3 iterations expected, but only 0 completed."
        );

    for(int i = 0; i < max_count; ++i)
        {
        try
            {
            if(1 == i)
                {
                scoped_unwind_toggler meaningless_name;
                throw "Thrown and caught before meter incremented.";
                }
            LMI_TEST(meter->reflect_progress());
            }
        catch(...)
            {
            }
        }
    LMI_TEST_THROW
        (meter->culminate()
        ,std::runtime_error
        ,"3 iterations expected, but only 2 completed."
        );

    meter->reflect_progress();
    LMI_TEST_THROW
        (meter->reflect_progress()
        ,std::runtime_error
        ,"Progress meter maximum count exceeded."
        );
}

void progress_meter_test::test_failure_to_culminate()
{
    progress_meter_unit_test_stream().str("");
    int const max_count = 3;
    std::unique_ptr<progress_meter> meter
        (create_progress_meter
            (max_count
            ,"Some title"
            ,progress_meter::e_unit_test_mode
            )
        );
    std::cout
        << "Expect 'Please report this: culminate() not called.':"
        << std::endl
        ;
}

int test_main(int, char*[])
{
    progress_meter_test::test();
    return EXIT_SUCCESS;
}
