// Platform-independent support for report tables: unit test.
//
// Copyright (C) 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "report_table.hpp"

#include "assert_lmi.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"

#include <iomanip>                      // setw()
#include <numeric>                      // accumulate()
#include <sstream>
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
    for(int i = 0; i < lmi::ssize(w); ++i)
        {
        v.push_back({"", w[i], oe_right, e[i] ? oe_elastic : oe_inelastic});
        }
    return v;
}

int sum(std::vector<int> z)
{
    return std::accumulate(z.begin(), z.end(), 0);
}
} // Unnamed namespace.

class report_table_test
{
  public:
    static void test()
        {
        test_apportion();
        test_bloat();
        test_column_widths_generally();
        test_column_widths_for_group_quotes();
        test_column_widths_for_illustrations();
        test_paginator();
        }

  private:
    static void test_apportion();
    static void test_bloat();
    static void test_column_widths_generally();
    static void test_column_widths_for_group_quotes();
    static void test_column_widths_for_illustrations();
    static void test_paginator();
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

    // Test with boolean vectors. This special case of the general
    // algorithm is suitable for apportioning marginal space evenly
    // among columns in a table.

    // All space apportioned--first column gets more.
    BOOST_TEST(std::vector<int>({3, 2, 2}) == apportion({1, 1, 1}, 7));

    // Set apportionable space so that all columns get the same.
    BOOST_TEST(std::vector<int>({2, 2, 2}) == apportion({1, 1, 1}, 6));

    // Set boolean vectors so that some columns get none.
    BOOST_TEST(std::vector<int>({0, 5, 0}) == apportion({0, 1, 0}, 5));
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

void report_table_test::test_column_widths_generally()
{
    std::vector<table_column_info> v;
    std::vector<int> expected;
    std::vector<int> observed;

    // Just enough room for all data with desired margins.
    v = bloat({1, 2, 3}, {0, 0, 0});
    observed = set_column_widths(v, 12, 2, 1);
    expected = {3, 4, 5};
    BOOST_TEST(observed == expected);

    // Same columns: same layout, even if page is much wider.
    v = bloat({1, 2, 3}, {0, 0, 0});
    observed = set_column_widths(v, 99, 2, 1);
    BOOST_TEST(observed == expected);

    // Same columns, but inadequate page width.

    // Tests in this section are overconstrained in that they don't
    // have enough room to print all inelastic columns with a margin
    // of at least one point.

    v = bloat({1, 2, 3}, {0, 0, 0});
    observed = set_column_widths(v, 11, 2, 1);
    expected = {3, 4, 4};
    BOOST_TEST(observed == expected);

    // Just enough room for all data with minimum margins.
    v = bloat({1, 2, 3}, {0, 0, 0});
    observed = set_column_widths(v,  9, 2, 1);
    expected = {2, 3, 4};
    BOOST_TEST(observed == expected);

    // Not enough room for all data with minimum margins.
    v = bloat({1, 2, 3}, {0, 0, 0});
    std::cout << "Expect a diagnostic (printing 2/3 columns):\n  ";
    observed = set_column_widths(v,  8, 2, 1);
    expected = {3, 4, 0};
    BOOST_TEST(observed == expected);

    // Not enough room for all data, even with no margins at all.
    v = bloat({1, 2, 3}, {0, 0, 0});
    std::cout << "Expect a diagnostic (printing 2/3 columns):\n  ";
    observed = set_column_widths(v,  5, 2, 1);
    expected = {2, 3, 0};
    BOOST_TEST(observed == expected);

    // Not enough room for even the first column.
    BOOST_TEST_THROW
        (set_column_widths(v, 1, 2, 1)
        ,std::runtime_error
        ,"Not enough room for even the first column."
        );

    // Report with zero columns.
    v = bloat({}, {});
    BOOST_TEST_THROW
        (set_column_widths(v, 1, 2, 1)
        ,std::runtime_error
        ,"Report table would contain no columns."
        );

    // Minimum margin greater than one.
    v = bloat({1, 2, 3}, {0, 0, 0});
    observed = set_column_widths(v, 16, 5, 3);
    expected = {5, 5, 6};
    BOOST_TEST(observed == expected);

    // An elastic column occupies all available space not claimed by
    // inelastic columns...
    v = bloat({1, 2, 0, 3}, {0, 0, 1, 0});
    observed = set_column_widths(v, 99, 2, 1);
    expected = {3, 4, (99-12), 5};
    BOOST_TEST(observed == expected);
    // ...though its width might happen to be zero (PDF !! but see
    //   https://lists.nongnu.org/archive/html/lmi/2018-07/msg00049.html
    // which questions whether zero should be allowed):
    v = bloat({1, 2, 0, 3}, {0, 0, 1, 0});
    observed = set_column_widths(v, 12, 2, 1);
    expected = {3, 4, 0, 5};
    BOOST_TEST(observed == expected);

    // Multiple elastic columns apportion all unclaimed space among
    // themselves.
    v = bloat({0, 2, 0, 3}, {1, 0, 1, 0});
    observed = set_column_widths(v, 99, 2, 1);
    expected = {45, 4, 45, 5};
    BOOST_TEST(observed == expected);

    // Same, but with nonzero width specified for one elastic column.
    v = bloat({1, 2, 0, 3}, {1, 0, 1, 0});
    observed = set_column_widths(v, 99, 2, 1);
    expected = {46, 4, 44, 5};
    BOOST_TEST(observed == expected);

    // Elastic columns only.
    v = bloat({10, 20, 30}, {1, 1, 1});
    observed = set_column_widths(v, 99, 2, 1);
    expected = {23, 33, 43};
    BOOST_TEST(observed == expected);

    // Same columns, but all inelastic.
    v = bloat({10, 20, 30}, {0, 0, 0});
    observed = set_column_widths(v, 99, 2, 1);
    expected = {12, 22, 32};
    BOOST_TEST(observed == expected);
}

/// Test data for an actual group quote.
///
/// The data used here were intercepted while running an actual
/// group quote. Therefore, they aren't written in a compact way
/// or expanded by bloat().

void report_table_test::test_column_widths_for_group_quotes()
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

    std::vector<int> const observed = set_column_widths(v, total_width, default_margin, 1);

    std::vector<int> const expected = {36, 129, 52, 62, 78, 81, 78, 81, 78, 81};
    BOOST_TEST(total_width == sum(expected));
    BOOST_TEST(observed == expected);
}

/// Test data for actual illustrations.
///
/// The data used here were intercepted while running several actual
/// illustrations. Therefore, they aren't written in a compact way
/// or expanded by bloat().

void report_table_test::test_column_widths_for_illustrations()
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

    std::vector<int> const observed = set_column_widths(v, total_width, default_margin, 1);

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

    std::vector<int> const observed = set_column_widths(v, total_width, default_margin, 1);

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

    std::cout << "Expect a diagnostic (printing 11/12 columns):\n  ";
    std::vector<int> const observed = set_column_widths(v, total_width, default_margin, 1);

    // Today, two times the default margin is added to each column,
    // even though the data cannot fit.
    std::vector<int> const expected = {53, 53, 53, 53, 52, 52, 52, 52, 52, 52, 52, 0};
    BOOST_TEST(total_width == sum(expected));
    BOOST_TEST(observed == expected);

#if 0 // Doesn't throw today, but might someday.
    BOOST_TEST_THROW
        (set_column_widths(v, total_width, default_margin, 1)
        ,std::runtime_error
        ,"Not enough space for all 12 columns."
        );
#endif // 0
    }
}

class pagination_demo : public paginator
{
  public:
    pagination_demo() {}
    virtual ~pagination_demo() = default;

    int test(int number_of_rows, int rows_per_group, int max_lines_per_page);

    std::string str() const;

  private:
    void prelude          () override;
    void open_page        () override;
    void print_a_data_row () override;
    void print_a_separator() override;
    void close_page       () override;
    void postlude         () override;

    mutable std::ostringstream oss_ {};
    mutable int demo_page_          {0};
    mutable int demo_line_          {0};
    mutable int demo_line_count_    {0};
    mutable int demo_row_           {0};
};

int pagination_demo::test
    (int number_of_rows
    ,int rows_per_group
    ,int max_lines_per_page
    )
{
    int const number_of_pages = init
        (number_of_rows
        ,rows_per_group
        ,max_lines_per_page
        );
    oss_.str("");
    oss_.clear();
    demo_page_       = 0;
    demo_line_       = 0;
    demo_line_count_ = 0;
    demo_row_        = 0;
    print();
    return number_of_pages;
}

std::string pagination_demo::str() const
{
    return oss_.str();
}

void pagination_demo::prelude()
{
    oss_ << "Paginated table demonstration begins...\n";
}

void pagination_demo::open_page()
{
    oss_ << "Page " << demo_page_ << '\n';
}

void pagination_demo::print_a_data_row()
{
    oss_
        << "   page "       << std::setw(3) << demo_page_
        << ",  line "       << std::setw(3) << demo_line_
        << ",  line_count " << std::setw(3) << demo_line_count_
        << ",  row "        << std::setw(3) << demo_row_
        << '\n'
        ;
    ++demo_line_;
    ++demo_line_count_;
    ++demo_row_;
}

void pagination_demo::print_a_separator()
{
    oss_
        << "   page "       << std::setw(3) << demo_page_
        << ",  line "       << std::setw(3) << demo_line_
        << ",  line_count " << std::setw(3) << demo_line_count_
        << '\n'
        ;
    ++demo_line_;
    ++demo_line_count_;
}

void pagination_demo::close_page()
{
    demo_line_ = 0;
    ++demo_page_;
}

void pagination_demo::postlude()
{
    oss_ << "...paginated table demonstration ends.\n";
}

std::string test_pagination
    (int number_of_rows
    ,int rows_per_group
    ,int max_lines_per_page
    )
{
    pagination_demo z;
    z.init(number_of_rows, rows_per_group, max_lines_per_page);
    z.print();
    return z.str();
}

void report_table_test::test_paginator()
{
    // Instead of testing classes prepaginator or paginator directly,
    // use pagination_demo::test(), which instantiates paginator (and
    // hence prepaginator) and exercises other code as well before
    // returning the page count.
    pagination_demo p;

    // Original tests: vary only the number of data rows.

    // Edge cases.
    // Arguably zero rows should mean zero pages.
    BOOST_TEST_EQUAL(1, p.test( 0, 5, 28));
    BOOST_TEST_EQUAL(1, p.test( 1, 5, 28));
    // Just a trivial sanity test.
    BOOST_TEST_EQUAL(1, p.test(17, 5, 28));
    // 4 full groups + incomplete last group.
    BOOST_TEST_EQUAL(1, p.test(24, 5, 28));
    // 5 full groups don't fit on one page.
    BOOST_TEST_EQUAL(2, p.test(25, 5, 28));
    // 4 + 4 groups + incomplete last one.
    BOOST_TEST_EQUAL(2, p.test(44, 5, 28));
    // 9 full groups don't fit on two pages.
    BOOST_TEST_EQUAL(3, p.test(45, 5, 28));

    // Test preconditions.

    // Negative number of data rows.
    BOOST_TEST_THROW
        (p.test(-1, 1, 1)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    // Zero rows per group.
    BOOST_TEST_THROW
        (p.test(1, 0, 1)
        ,std::logic_error
        ,"Rows per group must be positive."
        );

    // Negative number of rows per group.
    BOOST_TEST_THROW
        (p.test(1, -1, 1)
        ,std::logic_error
        ,"Rows per group must be positive."
        );

    // Insufficient room to print even one group.
    BOOST_TEST_THROW
        (p.test(1, 7, 3)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    // A single row of data.
    BOOST_TEST_EQUAL(1, p.test(1, 1, 1));
    BOOST_TEST_EQUAL(1, p.test(1, 1, 3));
    BOOST_TEST_EQUAL(1, p.test(1, 3, 3));
    BOOST_TEST_EQUAL(1, p.test(1, 3, 7));

    // One-row groups:

    // Page length an odd number.
    BOOST_TEST_EQUAL(1, p.test(1, 1, 5));
    BOOST_TEST_EQUAL(1, p.test(3, 1, 5));
    BOOST_TEST_EQUAL(2, p.test(4, 1, 5));
    BOOST_TEST_EQUAL(2, p.test(6, 1, 5));
    BOOST_TEST_EQUAL(3, p.test(7, 1, 5));

    // Same, but next even length: same outcome.
    BOOST_TEST_EQUAL(1, p.test(1, 1, 6));
    BOOST_TEST_EQUAL(1, p.test(3, 1, 6));
    BOOST_TEST_EQUAL(2, p.test(4, 1, 6));
    BOOST_TEST_EQUAL(2, p.test(6, 1, 6));
    BOOST_TEST_EQUAL(3, p.test(7, 1, 6));

    // Two-row groups.

    // Page length four.
    BOOST_TEST_EQUAL(1, p.test(1, 2, 4));
    BOOST_TEST_EQUAL(1, p.test(3, 2, 4));
    BOOST_TEST_EQUAL(2, p.test(4, 2, 4));
    BOOST_TEST_EQUAL(2, p.test(5, 2, 4));
    BOOST_TEST_EQUAL(3, p.test(6, 2, 4));

    // Page length five: no room for widow and orphan control.
    BOOST_TEST_EQUAL(1, p.test(1, 2, 5));
    BOOST_TEST_EQUAL(1, p.test(4, 2, 5));
    BOOST_TEST_EQUAL(2, p.test(5, 2, 5));
    BOOST_TEST_EQUAL(2, p.test(8, 2, 5));
    BOOST_TEST_EQUAL(3, p.test(9, 2, 5));

    // Same, but next even length: same outcome.
    BOOST_TEST_EQUAL(1, p.test(1, 2, 6));
    BOOST_TEST_EQUAL(1, p.test(4, 2, 6));
    BOOST_TEST_EQUAL(2, p.test(5, 2, 6));
    BOOST_TEST_EQUAL(2, p.test(8, 2, 6));
    BOOST_TEST_EQUAL(3, p.test(9, 2, 6));

    // Page length seven: one extra data row possible on last page.
    BOOST_TEST_EQUAL(1, p.test(1, 2, 7));
    BOOST_TEST_EQUAL(1, p.test(4, 2, 7));
    BOOST_TEST_EQUAL(1, p.test(5, 2, 7));
    BOOST_TEST_EQUAL(2, p.test(6, 2, 7));
    BOOST_TEST_EQUAL(2, p.test(8, 2, 7));
    BOOST_TEST_EQUAL(2, p.test(9, 2, 7));

    std::cout << "Zero rows" << std::endl;
    std::cout << test_pagination(0, 2, 7) << std::endl;
    std::cout << "Last page: zero full groups and one partial" << std::endl;
    std::cout << test_pagination(1, 2, 7) << std::endl;
    std::cout << "Last page: two full groups and one partial" << std::endl;
    std::cout << test_pagination(5, 2, 7) << std::endl;
    std::cout << "Last page: one full group" << std::endl;
    std::cout << test_pagination(6, 2, 7) << std::endl;
    std::cout << "Last page: one full group and one partial" << std::endl;
    std::cout << test_pagination(7, 2, 7) << std::endl;
    std::cout << "Last page: full page" << std::endl;
    std::cout << test_pagination(8, 2, 7) << std::endl;
    std::cout << "Last page: full page, plus an antiwidowed partial group" << std::endl;
    std::cout << test_pagination(9, 2, 7) << std::endl;
}

int test_main(int, char*[])
{
    report_table_test::test();
    return EXIT_SUCCESS;
}
