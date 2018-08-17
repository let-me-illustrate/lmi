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
        test_apportion();
        test_bloat();
        test_generally();
        test_group_quote();
        test_illustration();
        }

  private:
    static void test_apportion();
    static void test_bloat();
    static void test_generally();
    static void test_group_quote();
    static void test_illustration();
};

void report_table_test::test_apportion()
{
    // Test cases from:
    //   https://en.wikipedia.org/wiki/Largest_remainder_method

    std::vector<int> const votes0 = {47000, 16000, 15800, 12000, 6100, 3100};
    std::vector<int> const seats0 = {5, 2, 1, 1, 1, 0};
    BOOST_TEST(seats0 == apportion(votes0, 10));

    std::vector<int> const votes1 = {1500, 1500, 900, 500, 500, 200};
    std::vector<int> const seats1 = {7, 7, 4, 3, 3, 1};
    BOOST_TEST(seats1 == apportion(votes1, 25));

    std::vector<int> const seats2 = {8, 8, 5, 2, 2, 1};
    BOOST_TEST(seats2 == apportion(votes1, 26));

    // Test with zero total votes, to make sure that division by zero
    // is averted.

    std::vector<int> const votes3 = {0, 0, 0};
    std::vector<int> const seats3 = {0, 0, 0};
    BOOST_TEST(seats3 == apportion(votes3, 7));

    // Test with vectors of one and zero elements.

    std::vector<int> const votes4 = {1};
    std::vector<int> const seats4 = {7};
    BOOST_TEST(seats4 == apportion(votes4, 7));

    std::vector<int> const votes5 = {};
    std::vector<int> const seats5 = {};
    BOOST_TEST(seats5 == apportion(votes5, 7));

    // Test with an equal number of "voters" in each "state".

    std::vector<int> const votes6 = {5, 5, 5};
    std::vector<int> const seats6 = {3, 2, 2};
    BOOST_TEST(seats6 == apportion(votes6, 7));
}

void report_table_test::test_bloat()
{
    std::vector<table_column_info> const v =
        {{"",  3, oe_right, oe_inelastic}
        ,{"",  1, oe_right, oe_elastic  }
        ,{"",  0, oe_right, oe_inelastic}
        ,{"",  0, oe_right, oe_elastic  }
        ,{"",  2, oe_right, oe_inelastic}
        };

    std::vector<int>  const w = {3, 1, 0, 0, 2};
    std::vector<bool> const e = {0, 1, 0, 1, 0};
    BOOST_TEST(v == bloat(w, e));

    // Progressively terser equivalents.

    std::vector<table_column_info> x = bloat({3, 1, 0, 0, 2}, {0, 1, 0, 1, 0});
    BOOST_TEST(v == x);

    auto const y = bloat({3, 1, 0, 0, 2}, {0, 1, 0, 1, 0});
    BOOST_TEST(v == y);

    BOOST_TEST(v == bloat({3, 1, 0, 0, 2}, {0, 1, 0, 1, 0}));
}

void report_table_test::test_generally()
{
    std::vector<table_column_info> v;
    std::vector<int> expected;

    // Width with default margins (12) = maximum available page width.
    v = bloat({1, 2, 3}, {0, 0, 0});
    set_column_widths(12, 2, v);
    expected = {3, 4, 5};
    BOOST_TEST(widths(v) == expected);

    // Same columns: same layout, even if page is much wider (99).
    v = bloat({1, 2, 3}, {0, 0, 0});
    set_column_widths(99, 2, v);
    BOOST_TEST(widths(v) == expected);

    // Same columns, but inadequate page width.

    // PDF !! Begin section subject to revision.

    // Tests in this section are overconstrained in that they don't
    // have enough room to print all inelastic columns with bilateral
    // margins of at least one point.
    //
    // For now, what's tested is the actual behavior of current code
    // in the absence of elastic columns, viz.:
    // - for vector input column widths W[i], speculatively define
    //     X[i] = W[i] + input margin (a positive scalar)
    // - if sum(X) < available page width
    //     then set W=X
    // - if sum(W) < available page width < sum(X)
    //     then apportion any available margin among columns in W
    //     and issue no diagnostic even if some columns get no margin
    // - else, i.e., if available page width < sum(W) < sum(X)
    //     then set W=X
    //     and issue a diagnostic

    std::vector<int> actual;

    v = bloat({1, 2, 3}, {0, 0, 0});
    set_column_widths(11, 2, v);
    actual = {3, 4, 4};
    BOOST_TEST(widths(v) == actual);

    v = bloat({1, 2, 3}, {0, 0, 0});
    set_column_widths( 6, 2, v);
    actual = {1, 2, 3};
    BOOST_TEST(widths(v) == actual);

    // Warning given here:
    v = bloat({1, 2, 3}, {0, 0, 0});
    set_column_widths( 5, 2, v);
    actual = {3, 4, 5};
    BOOST_TEST(widths(v) == actual);

    // PDF !! End section subject to revision.

    // An elastic column occupies all available space not claimed by
    // inelastic columns...
    v = bloat({1, 2, 0, 3}, {0, 0, 1, 0});
    set_column_widths(99, 2, v);
    expected = {3, 4, (99-12), 5};
    BOOST_TEST(widths(v) == expected);
    // ...though its width might happen to be zero (PDF !! but see
    //   https://lists.nongnu.org/archive/html/lmi/2018-07/msg00049.html
    // which questions whether zero should be allowed):
    v = bloat({1, 2, 0, 3}, {0, 0, 1, 0});
    set_column_widths(12, 2, v);
    expected = {3, 4, 0, 5};
    BOOST_TEST(widths(v) == expected);

    // Multiple elastic columns apportion all unclaimed space among
    // themselves.
    v = bloat({1, 2, 0, 3}, {1, 0, 1, 0});
    set_column_widths(99, 2, v);
    expected = {45, 4, 45, 5};
    BOOST_TEST(widths(v) == expected);
}

/// Test data for an actual group quote.
///
/// The data used here were intercepted while running an actual
/// group quote. Therefore, they aren't written in a compact way
/// or expanded by bloat().

void report_table_test::test_group_quote()
{
    static int const total_width    = 756;
    static int const default_margin = 14;

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
    static int const default_margin = 14;

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
