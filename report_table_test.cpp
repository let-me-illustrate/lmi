// Platform-independent support for report tables: unit test.
//
// Copyright (C) 2018 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "report_table.hpp"

#include "test_tools.hpp"

#include <numeric>                      // accumulate()

std::vector<int> widths(std::vector<table_column_info> const& z)
{
    std::vector<int> v;
    for(auto const& j : z)
        {
        v.push_back(j.col_width());
        }
    return v;
}

int sum(std::vector<int> z)
{
    return std::accumulate(z.begin(), z.end(), 0);
}

class report_table_test
{
  public:
    static void test()
        {
        test_group_quote();
        test_illustration();
        }

  private:
    static void test_group_quote();
    static void test_illustration();
};

void report_table_test::test_group_quote()
{
    static int const total_width    = 756;
    static int const default_margin = 7;

    std::vector<table_column_info> v =
        {{"", 22, oe_center, oe_inelastic}
        ,{"",  0, oe_left  , oe_elastic  }
        ,{"", 38, oe_center, oe_inelastic}
        ,{"", 48, oe_center, oe_inelastic}
        ,{"", 64, oe_center, oe_inelastic}
        ,{"", 67, oe_center, oe_inelastic}
        ,{"", 64, oe_center, oe_inelastic}
        ,{"", 67, oe_center, oe_inelastic}
        ,{"", 64, oe_center, oe_inelastic}
        ,{"", 67, oe_center, oe_inelastic}
        };

    set_column_widths(total_width, default_margin, v);

    std::vector<int> const observed = widths(v);
    std::vector<int> const expected = {36, 129, 52, 62, 78, 81, 78, 81, 78, 81};
    BOOST_TEST(total_width == sum(expected));
    BOOST_TEST(observed == expected);
}

void report_table_test::test_illustration()
{
    static int const total_width    = 576;
    static int const default_margin = 7;

    // Fits with default margin.

    {
    std::vector<table_column_info> v =
        {{"", 24, oe_right, oe_inelastic}
        ,{"", 38, oe_right, oe_inelastic}
        ,{"", 53, oe_right, oe_inelastic}
        ,{"", 52, oe_right, oe_inelastic}
        ,{"", 31, oe_right, oe_inelastic}
        ,{"", 48, oe_right, oe_inelastic}
        ,{"", 48, oe_right, oe_inelastic}
        ,{"", 53, oe_right, oe_inelastic}
        };

    set_column_widths(total_width, default_margin, v);

    std::vector<int> const observed = widths(v);
    std::vector<int> const expected = {38, 52, 67, 66, 45, 62, 62, 67};
    BOOST_TEST(sum(expected) < total_width);
    BOOST_TEST(observed == expected);
    }

    // Fits with reduced margin.

    {
    std::vector<table_column_info> v =
        {{"", 26, oe_right, oe_inelastic}
        ,{"", 24, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 32, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        };

    set_column_widths(total_width, default_margin, v);

    std::vector<int> const observed = widths(v);
    std::vector<int> const expected = {30, 28, 54, 36, 54, 54, 54, 54, 53, 53, 53, 53};
    BOOST_TEST(total_width == sum(expected));
    BOOST_TEST(observed == expected);
    }

    // Cannot fit.

    {
    std::vector<table_column_info> v =
        {{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        ,{"", 50, oe_right, oe_inelastic}
        };

std::cout << "[Expect a multiline..." << std::endl;
    set_column_widths(total_width, default_margin, v);
std::cout << "...warning message.]" << std::endl;

    // Today, two times the default margin is added to each column,
    // even though the data cannot fit.
    std::vector<int> const observed = widths(v);
    std::vector<int> const expected = {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};
    BOOST_TEST(total_width < sum(expected));
    BOOST_TEST(observed == expected);

#if 0 // Doesn't throw today, but might someday.
    BOOST_TEST_THROW
        (set_column_widths(total_width, default_margin, v)
        ,std::runtime_error
        ,"3 iterations expected, but only 0 completed."
        );
#endif // 0
    }
}

int test_main(int, char*[])
{
    report_table_test::test();
    return EXIT_SUCCESS;
}
