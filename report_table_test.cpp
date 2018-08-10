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

#include "assert_lmi.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"

#include <numeric>                      // accumulate()
#include <vector>

// This needs to be defined in the global namespace to make
// std::vector<table_column_info> equality-comparable.

bool operator==(table_column_info const& a, table_column_info const& b)
{
    return
           a.col_header() == b.col_header()
        && a.col_width()  == b.col_width()
        && a.alignment()  == b.alignment()
        && a.is_elastic() == b.is_elastic()
        ;
}

namespace
{
/// Create a std::vector<table_column_info> from vectors of arguments.
///
/// set_column_widths() ignores all table_column_info members except
///   int  col_width_   // read and written
///   bool is_elastic() // read only
/// Initializing all members explicitly makes unit tests verbose; this
/// function lets them be written more compactly.

std::vector<table_column_info> bloat
    (std::vector<int>  const& w
    ,std::vector<bool> const& e
    )
{
    LMI_ASSERT(lmi::ssize(w) == lmi::ssize(e));
    std::vector<table_column_info> v;
    for(int i = 0; i < lmi::ssize(w) ; ++i)
        {
        v.push_back({"", w[i], oe_right, e[i] ? oe_elastic : oe_inelastic});
        }
    return v;
}

int sum(std::vector<int> z)
{
    return std::accumulate(z.begin(), z.end(), 0);
}

std::vector<int> widths(std::vector<table_column_info> const& z)
{
    std::vector<int> v;
    for(auto const& j : z)
        {
        v.push_back(j.col_width());
        }
    return v;
}
} // Unnamed namespace.

class report_table_test
{
  public:
    static void test()
        {
        test_bloat();
        test_generally();
        test_group_quote();
        test_illustration();
        }

  private:
    static void test_bloat();
    static void test_generally();
    static void test_group_quote();
    static void test_illustration();
};

void report_table_test::test_bloat()
{
    std::vector<int>  w = {3, 1, 0, 0, 2};
    std::vector<bool> e = {0, 1, 0, 1, 0};
    std::vector<table_column_info> v =
        {{"",  3, oe_right, oe_inelastic}
        ,{"",  1, oe_right, oe_elastic  }
        ,{"",  0, oe_right, oe_inelastic}
        ,{"",  0, oe_right, oe_elastic  }
        ,{"",  2, oe_right, oe_inelastic}
        };
    BOOST_TEST(bloat(w, e) == v);
}

void report_table_test::test_generally()
{
    std::vector<int>  w = {1, 2, 3};
    std::vector<bool> e = {0, 0, 0};
    std::vector<table_column_info> v = bloat(w, e);
    set_column_widths(13, 1, v);
    std::vector<int> const observed = widths(v);
    std::vector<int> const expected = {3, 4, 5};
    BOOST_TEST(observed == expected);
}

/// Test data for an actual group quote.
///
/// The data used here were intercepted while running an actual
/// group quote. Therefore, they aren't written in a compact way
/// or expanded by bloat().

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

/// Test data for actual illustrations.
///
/// The data used here were intercepted while running several actual
/// illustrations. Therefore, they aren't written in a compact way
/// or expanded by bloat().

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
        ,"Not enough space for all 12 columns."
        );
#endif // 0
    }
}

int test_main(int, char*[])
{
    report_table_test::test();
    return EXIT_SUCCESS;
}
