// Miscellaneous functions--unit test.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "miscellany.hpp"

#include "test_tools.hpp"

#include <cstdio>                       // remove()
#include <fstream>

void test_each_equal()
{
    int                a0[] {0, 0, 0, 0};
    int const          a1[] {0, 1, 1, 1};
    int       volatile a2[] {0, 1, 2, 2};
    int const volatile a3[] {0, 1, 2, 3};

    // There can be no volatile standard container.
    std::vector<int>        v0 {0, 0, 0, 0};
    std::vector<int> const  v1 {0, 1, 1, 1};
    std::vector<int>        v2 {0, 1, 2, 2};
    std::vector<int> const  v3 {0, 1, 2, 3};

    // Test with containers.

    BOOST_TEST( each_equal(a0, 0));
    BOOST_TEST(!each_equal(a1, 0));
    BOOST_TEST(!each_equal(a2, 0));
    BOOST_TEST(!each_equal(a3, 0));

    BOOST_TEST( each_equal(v0, 0));
    BOOST_TEST(!each_equal(v1, 0));
    BOOST_TEST(!each_equal(v2, 0));
    BOOST_TEST(!each_equal(v3, 0));

    // Test with iterators.

    BOOST_TEST( each_equal(v0.begin(), v0.end(), 0));
    BOOST_TEST(!each_equal(v1.begin(), v1.end(), 0));
    BOOST_TEST(!each_equal(v2.begin(), v2.end(), 0));
    BOOST_TEST(!each_equal(v3.begin(), v3.end(), 0));

    // Iterators are more flexible, of course.

    BOOST_TEST( each_equal(v0.begin() + 0, v0.end(), 0));
    BOOST_TEST( each_equal(v1.begin() + 1, v1.end(), 1));
    BOOST_TEST( each_equal(v2.begin() + 2, v2.end(), 2));
    BOOST_TEST( each_equal(v3.begin() + 3, v3.end(), 3));

    // Iterators are also more prone to error. The following examples
    // have undefined behavior.

//  BOOST_TEST( each_equal(v0.begin() + 7, v0.end(), 0));
//  BOOST_TEST( each_equal(v0.begin()    , v3.end(), 0));

    // Test empty ranges. There can be no empty array [8.3.4/1].
    // By arbitrary definition, any value compares equal to an empty
    // range.

    BOOST_TEST( each_equal(v0.end(), v0.end(),     0)); // both end()
    BOOST_TEST( each_equal(v0.end(), v0.end(), 12345)); // both end()
    std::vector<int> v_empty;
    BOOST_TEST( each_equal(v_empty, 23456));
    BOOST_TEST( each_equal(v_empty.begin(), v_empty.end(), 34567));
}

void test_files_are_identical()
{
    char const* f0("unlikely_file_name_0");
    char const* f1("unlikely_file_name_1");

    std::remove(f0);
    std::remove(f1);

    // Nonexistent files.
    BOOST_TEST_THROW
        (files_are_identical(f0, f1)
        ,std::runtime_error
        ,"Unable to open 'unlikely_file_name_0'."
        );

    // Identical empty files.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    }
    BOOST_TEST(files_are_identical(f0, f1));

    // Identical nonempty files.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    ofs0 << "Test";
    ofs1 << "Test";
    }
    BOOST_TEST(files_are_identical(f0, f1));

    // Files whose contents might be identical in text mode but differ
    // in binary mode are nonidentical.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    ofs0 << "Test\r\n";
    ofs1 << "Test\n";
    }
    BOOST_TEST(!files_are_identical(f0, f1));

    std::remove(f0);
    std::remove(f1);
}

void test_minmax()
{
    double const zero = 0.0;
    double const one  = 1.0;

    std::vector<double> w;
    w.push_back(one );
    w.push_back(zero);

    // Test const-correctness.
    std::vector<double> const v = w;
    minmax<double> const m(v);
    BOOST_TEST(zero == m.minimum());
    BOOST_TEST(one  == m.maximum());

    // Motivation for relational operators: to write this...
    BOOST_TEST(zero <= m.minimum() && m.maximum() <= one);
    // ...more compactly:
    BOOST_TEST(  zero <= m && m <= one );
    BOOST_TEST(!(zero <  m || m <  one));
}

void test_page_count()
{
    // Original tests: vary only the number of data rows.

    // Edge case (0 rows is not allowed).
    BOOST_TEST_EQUAL(1, page_count( 1, 5, 28));
    // Just a trivial sanity test.
    BOOST_TEST_EQUAL(1, page_count(17, 5, 28));
    // 4 full groups + incomplete last group.
    BOOST_TEST_EQUAL(1, page_count(24, 5, 28));
    // 5 full groups don't fit on one page.
    BOOST_TEST_EQUAL(2, page_count(25, 5, 28));
    // 4 + 4 groups + incomplete last one.
    BOOST_TEST_EQUAL(2, page_count(44, 5, 28));
    // 9 full groups don't fit on two pages.
    BOOST_TEST_EQUAL(3, page_count(45, 5, 28));

    // Test preconditions.

    // No data rows.
    BOOST_TEST_THROW
        (page_count(0, 1, 1)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    // Zero rows per group.
    BOOST_TEST_THROW
        (page_count(1, 0, 1)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    // Insufficient room to print even one group.
    BOOST_TEST_THROW
        (page_count(1, 7, 3)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    // A single row of data.
    BOOST_TEST_EQUAL(1, page_count(1, 1, 1));
    BOOST_TEST_EQUAL(1, page_count(1, 1, 3));
    BOOST_TEST_EQUAL(1, page_count(1, 3, 3));
    BOOST_TEST_EQUAL(1, page_count(1, 3, 7));

    // One-row groups:

    // Page length an odd number.
    BOOST_TEST_EQUAL(1, page_count(1, 1, 5));
    BOOST_TEST_EQUAL(1, page_count(3, 1, 5));
    BOOST_TEST_EQUAL(2, page_count(4, 1, 5));
    BOOST_TEST_EQUAL(2, page_count(6, 1, 5));
    BOOST_TEST_EQUAL(3, page_count(7, 1, 5));

    // Same, but next even length: same outcome.
    BOOST_TEST_EQUAL(1, page_count(1, 1, 6));
    BOOST_TEST_EQUAL(1, page_count(3, 1, 6));
    BOOST_TEST_EQUAL(2, page_count(4, 1, 6));
    BOOST_TEST_EQUAL(2, page_count(6, 1, 6));
    BOOST_TEST_EQUAL(3, page_count(7, 1, 6));

    // Two-row groups.

    // Page length four.
    BOOST_TEST_EQUAL(1, page_count(1, 2, 4));
    BOOST_TEST_EQUAL(1, page_count(3, 2, 4));
    BOOST_TEST_EQUAL(2, page_count(4, 2, 4));
    BOOST_TEST_EQUAL(2, page_count(5, 2, 4));
    BOOST_TEST_EQUAL(3, page_count(6, 2, 4));

    // Page length five: no room for widow and orphan control.
    BOOST_TEST_EQUAL(1, page_count(1, 2, 5));
    BOOST_TEST_EQUAL(1, page_count(4, 2, 5));
    BOOST_TEST_EQUAL(2, page_count(5, 2, 5));
    BOOST_TEST_EQUAL(2, page_count(8, 2, 5));
    BOOST_TEST_EQUAL(3, page_count(9, 2, 5));

    // Same, but next even length: same outcome.
    BOOST_TEST_EQUAL(1, page_count(1, 2, 6));
    BOOST_TEST_EQUAL(1, page_count(4, 2, 6));
    BOOST_TEST_EQUAL(2, page_count(5, 2, 6));
    BOOST_TEST_EQUAL(2, page_count(8, 2, 6));
    BOOST_TEST_EQUAL(3, page_count(9, 2, 6));

    // Page length seven: one extra data row possible on last page.
    BOOST_TEST_EQUAL(1, page_count(1, 2, 7));
    BOOST_TEST_EQUAL(1, page_count(4, 2, 7));
    BOOST_TEST_EQUAL(1, page_count(5, 2, 7));
    BOOST_TEST_EQUAL(2, page_count(6, 2, 7));
    BOOST_TEST_EQUAL(2, page_count(8, 2, 7));
    BOOST_TEST_EQUAL(2, page_count(9, 2, 7));
}

void test_prefix_and_suffix()
{
    std::string s = "";

    BOOST_TEST( begins_with(s, ""));
    BOOST_TEST( ends_with  (s, ""));

    BOOST_TEST(!begins_with(s, "A"));
    BOOST_TEST(!ends_with  (s, "Z"));

    BOOST_TEST(!begins_with(s, "ABC"));
    BOOST_TEST(!ends_with  (s, "XYZ"));

    s = "W";

    BOOST_TEST( begins_with(s, ""));
    BOOST_TEST( ends_with  (s, ""));

    BOOST_TEST(!begins_with(s, "A"));
    BOOST_TEST(!ends_with  (s, "Z"));

    BOOST_TEST(!begins_with(s, "WW"));
    BOOST_TEST(!ends_with  (s, "WW"));

    BOOST_TEST( begins_with(s, "W"));
    BOOST_TEST( ends_with  (s, "W"));

    s = "LMNOP";

    BOOST_TEST( begins_with(s, ""));
    BOOST_TEST( ends_with  (s, ""));

    BOOST_TEST(!begins_with(s, "A"));
    BOOST_TEST(!ends_with  (s, "Z"));

    BOOST_TEST( begins_with(s, "L"));
    BOOST_TEST( ends_with  (s, "P"));

    BOOST_TEST( begins_with(s, "LMN"));
    BOOST_TEST( ends_with  (s, "NOP"));

    BOOST_TEST( begins_with(s, "LMNOP"));
    BOOST_TEST( ends_with  (s, "LMNOP"));

    BOOST_TEST(!begins_with(s, "LMNOPQ"));
    BOOST_TEST(!ends_with  (s, "KLMNOP"));
}

void test_scale_power()
{
    BOOST_TEST_THROW
        (scale_power(0, 0.0, 0.0)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    BOOST_TEST_THROW
        (scale_power(9, 1.0, -1.0)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    BOOST_TEST_EQUAL( 0, scale_power( 9,               0.0,               0.0));
    BOOST_TEST_EQUAL( 0, scale_power( 9,               0.0,     999'999'999.0));
    BOOST_TEST_EQUAL( 0, scale_power( 9,     -99'999'999.0,               0.0));
    BOOST_TEST_EQUAL( 0, scale_power( 9,     999'999'999.0,     999'999'999.0));

    // 999,999,999.0 rounds to 999,999,999
    BOOST_TEST_EQUAL( 0, scale_power( 9,     -99'999'999.0,     999'999'999.0));

    // 999,999.999.9 may round to 1,000,000,000; and
    // -99,999,999.9 may round to  -100,000,000
    //   which is equally wide if widths are reckoned as [,]:0 and [-0-9]:1
    //   (as for a monospace font with comma-less formatting).
    // Test with such a maximal value on LHS, RHS, and both sides.
    BOOST_TEST_EQUAL( 3, scale_power( 9,     -99'999'999.9,         999'999.9));
    BOOST_TEST_EQUAL( 3, scale_power( 9,        -999'999.9,     999'999'999.9));
    BOOST_TEST_EQUAL( 3, scale_power( 9,     -99'999'999.9,     999'999'999.9));

    BOOST_TEST_EQUAL( 3, scale_power( 9,    -999'999'999.0,   1'999'999'999.0));

    // Test threshold values for the scale_power=9 setting that is
    // still hardcoded in lmi as this is written in 2018-03.

    BOOST_TEST_EQUAL( 0, scale_power( 9, 0.0,                   999'999'999.0));
    BOOST_TEST_EQUAL( 3, scale_power( 9, 0.0,                   999'999'999.1));
    BOOST_TEST_EQUAL( 3, scale_power( 9, 0.0,               999'999'999'999.0));
    BOOST_TEST_EQUAL( 6, scale_power( 9, 0.0,               999'999'999'999.1));
    BOOST_TEST_EQUAL( 6, scale_power( 9, 0.0,           999'999'999'999'999.0));
    BOOST_TEST_EQUAL( 9, scale_power( 9, 0.0,           999'999'999'999'999.1));

    // In the last test above, the threshold is not     999'999'999'999'999.01
    // as a logarithm-based algorithm that rounds fractions toward
    // infinity would have it (with infinite-precision real numbers),
    // because 'binary64' doesn't have seventeen exact decimal digits.
    // In this range, successive values show a granularity of 1/8:

    BOOST_TEST_EQUAL(          999'999'999'999'999     ,999'999'999'999'999.0);
    BOOST_TEST_EQUAL(          999'999'999'999'999.125 ,999'999'999'999'999.1);
    BOOST_TEST_EQUAL(          999'999'999'999'999.25  ,999'999'999'999'999.2);
    BOOST_TEST_EQUAL(          999'999'999'999'999.25  ,999'999'999'999'999.3);
    BOOST_TEST_EQUAL(          999'999'999'999'999.375 ,999'999'999'999'999.4);
    BOOST_TEST_EQUAL(          999'999'999'999'999.5   ,999'999'999'999'999.5);
    BOOST_TEST_EQUAL(          999'999'999'999'999.625 ,999'999'999'999'999.6);
    BOOST_TEST_EQUAL(          999'999'999'999'999.75  ,999'999'999'999'999.7);
    BOOST_TEST_EQUAL(          999'999'999'999'999.75  ,999'999'999'999'999.8);
    BOOST_TEST_EQUAL(          999'999'999'999'999.875 ,999'999'999'999'999.9);

    // As more decimal triplets are added, exactness erodes. Between
    //                                    2^52 =      4'503'599'627'370'496
    // 2^52 and 2^53, all 'binary64' values are exact integers, and
    // no significant digit is accurate after the decimal point.
    //                                    2^53 =      9'007'199'254'740'992
    // That double's integer successor is not representable exactly;
    // the same is true of this 80-bit extended precision value:
    //                                    2^64 = 18'446'744'073'709'551'616
    // Thus, this test would fail for a 'binary64' double:
//  BOOST_TEST_EQUAL( 9, scale_power( 9, 0.0,       999'999'999'999'999'999.0));
    // that value being indistinguishable from    1'000'000'000'000'000'000.0
    // of which neither is representable exactly. The next several
    // tests accidentally "work" as one might naively imagine...
    BOOST_TEST_EQUAL(12, scale_power( 9, 0.0,       999'999'999'999'999'999.1));
    BOOST_TEST_EQUAL(12, scale_power( 9, 0.0,     1'000'000'000'000'000'000.0));
    BOOST_TEST_EQUAL(15, scale_power( 9, 0.0, 1'000'000'000'000'000'000'000.0));
    // ...but this one happens not to...
//  BOOST_TEST_EQUAL(18, scale_power(9,0, 1'000'000'000'000'000'000'000'000.0));
    // ...just as this "worked" above...
//  BOOST_TEST_EQUAL( 9, scale_power( 9, 0.0,           999'999'999'999'999.1));
    // but wouldn't have "worked" with the value        999'999'999'999'999.01

    // Test threshold values for scale_power=8.

    BOOST_TEST_EQUAL( 0, scale_power( 8, 0.0,                    99'999'999.0));
    BOOST_TEST_EQUAL( 3, scale_power( 8, 0.0,                    99'999'999.1));
    BOOST_TEST_EQUAL( 3, scale_power( 8, 0.0,                99'999'999'999.0));
    BOOST_TEST_EQUAL( 6, scale_power( 8, 0.0,                99'999'999'999.1));
    BOOST_TEST_EQUAL( 6, scale_power( 8, 0.0,            99'999'999'999'999.0));
    BOOST_TEST_EQUAL( 9, scale_power( 8, 0.0,            99'999'999'999'999.1));

    // Test threshold values for scale_power=7.

    BOOST_TEST_EQUAL( 0, scale_power( 7, 0.0,                     9'999'999.0));
    BOOST_TEST_EQUAL( 3, scale_power( 7, 0.0,                     9'999'999.1));
    BOOST_TEST_EQUAL( 3, scale_power( 7, 0.0,                 9'999'999'999.0));
    BOOST_TEST_EQUAL( 6, scale_power( 7, 0.0,                 9'999'999'999.1));
    BOOST_TEST_EQUAL( 6, scale_power( 7, 0.0,             9'999'999'999'999.0));
    BOOST_TEST_EQUAL( 9, scale_power( 7, 0.0,             9'999'999'999'999.1));
}

void test_trimming()
{
    char const*const superfluous = " ;";

    std::string s = "";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "");

    s = " ";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "");

    s = " ;; ";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "");

    s = "a";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "a");

    s = "; ;a; ;";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "a");

    s = "a ; a ; ";
    rtrim(s, superfluous); // Shows what rtrim() does.
    BOOST_TEST_EQUAL(s, "a ; a");
    ltrim(s, superfluous); rtrim(s, superfluous); // Does nothing, correctly.
    BOOST_TEST_EQUAL(s, "a ; a");

    s = "; a ; a";
    ltrim(s, superfluous); // Shows what ltrim() does.
    BOOST_TEST_EQUAL(s, "a ; a");
    ltrim(s, superfluous); rtrim(s, superfluous); // Does nothing, correctly.
    BOOST_TEST_EQUAL(s, "a ; a");
}

int test_main(int, char*[])
{
    test_each_equal();
    test_files_are_identical();
    test_minmax();
    test_page_count();
    test_prefix_and_suffix();
    test_scale_power();
    test_trimming();

    return 0;
}

