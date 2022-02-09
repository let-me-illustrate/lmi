// Miscellaneous functions--unit test.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "miscellany.hpp"

#include "test_tools.hpp"

#include <cfloat>                       // DBL_MAX
#include <cmath>                        // HUGE_VAL
#include <cstdio>                       // remove()
#include <ctime>                        // clock()
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

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

    LMI_TEST( each_equal(a0, 0));
    LMI_TEST(!each_equal(a1, 0));
    LMI_TEST(!each_equal(a2, 0));
    LMI_TEST(!each_equal(a3, 0));

    LMI_TEST( each_equal(v0, 0));
    LMI_TEST(!each_equal(v1, 0));
    LMI_TEST(!each_equal(v2, 0));
    LMI_TEST(!each_equal(v3, 0));

    // Test with iterators.

    LMI_TEST( each_equal(v0.begin(), v0.end(), 0));
    LMI_TEST(!each_equal(v1.begin(), v1.end(), 0));
    LMI_TEST(!each_equal(v2.begin(), v2.end(), 0));
    LMI_TEST(!each_equal(v3.begin(), v3.end(), 0));

    // Iterators are more flexible, of course.

    LMI_TEST( each_equal(v0.begin() + 0, v0.end(), 0));
    LMI_TEST( each_equal(v1.begin() + 1, v1.end(), 1));
    LMI_TEST( each_equal(v2.begin() + 2, v2.end(), 2));
    LMI_TEST( each_equal(v3.begin() + 3, v3.end(), 3));

    // Iterators are also more prone to error. The following examples
    // have undefined behavior.

//  LMI_TEST( each_equal(v0.begin() + 7, v0.end(), 0));
//  LMI_TEST( each_equal(v0.begin()    , v3.end(), 0));

    // Test empty ranges. There can be no empty array [8.3.4/1].
    // By arbitrary definition, any value compares equal to an empty
    // range.

    LMI_TEST( each_equal(v0.end(), v0.end(),     0)); // both end()
    LMI_TEST( each_equal(v0.end(), v0.end(), 12345)); // both end()
    std::vector<int> v_empty;
    LMI_TEST( each_equal(v_empty, 23456));
    LMI_TEST( each_equal(v_empty.begin(), v_empty.end(), 34567));
}

void test_files_are_identical()
{
    char const* f0("unlikely_file_name_0");
    char const* f1("unlikely_file_name_1");

    std::remove(f0);
    std::remove(f1);

    // Nonexistent files.
    LMI_TEST_THROW
        (files_are_identical(f0, f1)
        ,std::runtime_error
        ,"Unable to open 'unlikely_file_name_0'."
        );

    // Identical empty files.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    }
    LMI_TEST(files_are_identical(f0, f1));

    // Identical nonempty files.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    ofs0 << "Test";
    ofs1 << "Test";
    }
    LMI_TEST(files_are_identical(f0, f1));

    // Files whose contents might be identical in text mode but differ
    // in binary mode are nonidentical.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    ofs0 << "Test\r\n";
    ofs1 << "Test\n";
    }
    LMI_TEST(!files_are_identical(f0, f1));

    std::remove(f0);
    std::remove(f1);
}

void test_sup_inf()
{
    LMI_TEST_EQUAL( false   , infimum <bool  >());
    LMI_TEST_EQUAL( true    , supremum<bool  >());
    LMI_TEST_EQUAL( INT_MIN , infimum <int   >());
    LMI_TEST_EQUAL( INT_MAX , supremum<int   >());
    LMI_TEST_EQUAL(-HUGE_VAL, infimum <double>());
    LMI_TEST_EQUAL( HUGE_VAL, supremum<double>());

    LMI_TEST_EQUAL( 0       , infimum <unsigned int>());
}

void test_minmax()
{
    double const zero = 0.0;
    double const one  = 1.0;

    std::vector<double> w;
    w.push_back(one );
    w.push_back(zero);

    // Test default ctor.
    minmax<double> const dbl_minmax {};
    LMI_TEST_EQUAL( HUGE_VAL, dbl_minmax.minimum());
    LMI_TEST_EQUAL(-HUGE_VAL, dbl_minmax.maximum());

    // Test explicit ctor with zero-element argument.
    std::vector<int> const empty_vector {};
    minmax<int> const empty_minmax(empty_vector);
    LMI_TEST_EQUAL(empty_minmax.minimum(), std::numeric_limits<int>::max());
    LMI_TEST_EQUAL(empty_minmax.maximum(), std::numeric_limits<int>::min());

    // Test const-correctness.
    std::vector<double> const v = w;
    minmax<double> const m(v);
    LMI_TEST_EQUAL(zero, m.minimum());
    LMI_TEST_EQUAL(one , m.maximum());

    // Motivation for relational operators: to write this...
    LMI_TEST(zero <= m.minimum() && m.maximum() <= one);
    // ...more compactly:
    LMI_TEST(  zero <= m && m <= one );
    LMI_TEST(!(zero <  m || m <  one));
}

void test_prefix_and_suffix()
{
    std::string s = "";

    LMI_TEST( begins_with(s, ""));
    LMI_TEST( ends_with  (s, ""));

    LMI_TEST(!begins_with(s, "A"));
    LMI_TEST(!ends_with  (s, "Z"));

    LMI_TEST(!begins_with(s, "ABC"));
    LMI_TEST(!ends_with  (s, "XYZ"));

    s = "W";

    LMI_TEST( begins_with(s, ""));
    LMI_TEST( ends_with  (s, ""));

    LMI_TEST(!begins_with(s, "A"));
    LMI_TEST(!ends_with  (s, "Z"));

    LMI_TEST(!begins_with(s, "WW"));
    LMI_TEST(!ends_with  (s, "WW"));

    LMI_TEST( begins_with(s, "W"));
    LMI_TEST( ends_with  (s, "W"));

    s = "LMNOP";

    LMI_TEST( begins_with(s, ""));
    LMI_TEST( ends_with  (s, ""));

    LMI_TEST(!begins_with(s, "A"));
    LMI_TEST(!ends_with  (s, "Z"));

    LMI_TEST( begins_with(s, "L"));
    LMI_TEST( ends_with  (s, "P"));

    LMI_TEST( begins_with(s, "LMN"));
    LMI_TEST( ends_with  (s, "NOP"));

    LMI_TEST( begins_with(s, "LMNOP"));
    LMI_TEST( ends_with  (s, "LMNOP"));

    LMI_TEST(!begins_with(s, "LMNOPQ"));
    LMI_TEST(!ends_with  (s, "KLMNOP"));
}

void test_scale_power()
{
    LMI_TEST_THROW
        (scale_power(0, 0.0, 0.0)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    LMI_TEST_THROW
        (scale_power(9, 1.0, -1.0)
        ,std::runtime_error
        ,lmi_test::what_regex("^Assertion.*failed")
        );

    using D = std::numeric_limits<double>;
    if(D::has_quiet_NaN)
        {
        LMI_TEST_THROW
            (scale_power(9, D::quiet_NaN(), D::quiet_NaN())
            ,std::runtime_error
            ,lmi_test::what_regex("^Assertion.*failed")
            );
        }

    if(D::has_infinity)
        {
        LMI_TEST_EQUAL( 0, scale_power( 9,   D::infinity(),   D::infinity()));
        }

    // Test positive and negative zeros.

    LMI_TEST_EQUAL( 0, scale_power( 9,               0.0,               0.0));
    LMI_TEST_EQUAL( 0, scale_power( 9,               0.0,     999'999'999.0));
    LMI_TEST_EQUAL( 0, scale_power( 9,     -99'999'999.0,               0.0));

    LMI_TEST_EQUAL( 0, scale_power( 9,              -0.0,              -0.0));
    LMI_TEST_EQUAL( 0, scale_power( 9,              -0.0,     999'999'999.0));
    LMI_TEST_EQUAL( 0, scale_power( 9,     -99'999'999.0,              -0.0));

    LMI_TEST_EQUAL( 0, scale_power( 9,               0.0,              -0.0));
    LMI_TEST_EQUAL( 0, scale_power( 9,              -0.0,               0.0));

    // Test values for which rounding toward infinity crosses a threshold.

    // 999,999,999.0 rounds to 999,999,999
    LMI_TEST_EQUAL( 0, scale_power( 9,     -99'999'999.0,     999'999'999.0));
    // However:
    // 999,999.999.9 may round to 1,000,000,000; and
    // -99,999,999.9 may round to  -100,000,000
    //   which is equally wide if widths are reckoned as [,]:0 and [-0-9]:1
    //   (as for a monospace font with comma-less formatting).
    // Test with such a maximal value on LHS, RHS, and both sides.
    LMI_TEST_EQUAL( 3, scale_power( 9,     -99'999'999.9,         999'999.9));
    LMI_TEST_EQUAL( 3, scale_power( 9,        -999'999.9,     999'999'999.9));
    LMI_TEST_EQUAL( 3, scale_power( 9,     -99'999'999.9,     999'999'999.9));

    // Test values of like sign in threshold neighborhood.

    // both positive, below threshold
    LMI_TEST_EQUAL( 6, scale_power( 6,               0.1, 999'999'999'999.0));
    LMI_TEST_EQUAL( 6, scale_power( 6,     123'456'789.0, 999'999'999'999.0));
    LMI_TEST_EQUAL( 6, scale_power( 6, 999'999'999'999.0, 999'999'999'999.0));

    // both positive, threshold
    LMI_TEST_EQUAL( 9, scale_power( 6,               0.1, 999'999'999'999.1));
    LMI_TEST_EQUAL( 9, scale_power( 6,     123'456'789.0, 999'999'999'999.1));
    LMI_TEST_EQUAL( 9, scale_power( 6, 999'999'999'999.0, 999'999'999'999.1));
    LMI_TEST_EQUAL( 9, scale_power( 6, 999'999'999'999.1, 999'999'999'999.1));

    // both negative, below threshold
    LMI_TEST_EQUAL( 6, scale_power( 6, -99'999'999'999.0,              -0.1));
    LMI_TEST_EQUAL( 6, scale_power( 6, -99'999'999'999.0,    -123'456'789.0));
    LMI_TEST_EQUAL( 6, scale_power( 6, -99'999'999'999.0, -99'999'999'999.0));

    // both negative, threshold
    LMI_TEST_EQUAL( 9, scale_power( 6, -99'999'999'999.1,              -0.1));
    LMI_TEST_EQUAL( 9, scale_power( 6, -99'999'999'999.1,    -123'456'789.0));
    LMI_TEST_EQUAL( 9, scale_power( 6, -99'999'999'999.1, -99'999'999'999.0));
    LMI_TEST_EQUAL( 9, scale_power( 6, -99'999'999'999.1, -99'999'999'999.1));

    // Test threshold neighborhood for the scale_power=9 setting that
    // is still hardcoded in lmi as this is written in 2018-03.

    LMI_TEST_EQUAL( 0, scale_power( 9, 0.0,                   999'999'999.0));
    LMI_TEST_EQUAL( 3, scale_power( 9, 0.0,                   999'999'999.1));
    LMI_TEST_EQUAL( 3, scale_power( 9, 0.0,               999'999'999'999.0));
    LMI_TEST_EQUAL( 6, scale_power( 9, 0.0,               999'999'999'999.1));
    LMI_TEST_EQUAL( 6, scale_power( 9, 0.0,           999'999'999'999'999.0));
    LMI_TEST_EQUAL( 9, scale_power( 9, 0.0,           999'999'999'999'999.1));

    // In the last test above, the threshold is not     999'999'999'999'999.01
    // as the scale_power() algorithm, which rounds fractions toward
    // infinity, would have it (with infinite-precision real numbers),
    // because 'binary64' doesn't have seventeen exact decimal digits.
    // In this range, successive values show a granularity of 1/8:

    LMI_TEST_EQUAL(          999'999'999'999'999     ,999'999'999'999'999.0);
    LMI_TEST_EQUAL(          999'999'999'999'999.125 ,999'999'999'999'999.1);
    LMI_TEST_EQUAL(          999'999'999'999'999.25  ,999'999'999'999'999.2);
    LMI_TEST_EQUAL(          999'999'999'999'999.25  ,999'999'999'999'999.3);
    LMI_TEST_EQUAL(          999'999'999'999'999.375 ,999'999'999'999'999.4);
    LMI_TEST_EQUAL(          999'999'999'999'999.5   ,999'999'999'999'999.5);
    LMI_TEST_EQUAL(          999'999'999'999'999.625 ,999'999'999'999'999.6);
    LMI_TEST_EQUAL(          999'999'999'999'999.75  ,999'999'999'999'999.7);
    LMI_TEST_EQUAL(          999'999'999'999'999.75  ,999'999'999'999'999.8);
    LMI_TEST_EQUAL(          999'999'999'999'999.875 ,999'999'999'999'999.9);

    // As more decimal triplets are added, exactness erodes. Between
    //                                    2^52 =      4'503'599'627'370'496
    // 2^52 and 2^53, all 'binary64' values are exact integers, and
    // no significant digit is accurate after the decimal point.
    //                                    2^53 =      9'007'199'254'740'992
    // That double's integer successor is not representable exactly;
    // the same is true of this 80-bit extended precision value:
    //                                    2^64 = 18'446'744'073'709'551'616
    // Thus, this test would fail for a 'binary64' double:
//  LMI_TEST_EQUAL( 9, scale_power( 9, 0.0,       999'999'999'999'999'999.0));
    // that value being indistinguishable from    1'000'000'000'000'000'000.0
    // of which neither is representable exactly. The next several
    // tests accidentally "work" as one might naively imagine...
    LMI_TEST_EQUAL(12, scale_power( 9, 0.0,       999'999'999'999'999'999.1));
    LMI_TEST_EQUAL(12, scale_power( 9, 0.0,     1'000'000'000'000'000'000.0));
    LMI_TEST_EQUAL(15, scale_power( 9, 0.0, 1'000'000'000'000'000'000'000.0));
    // ...but this one happens not to...
//  LMI_TEST_EQUAL(18, scale_power(9,0, 1'000'000'000'000'000'000'000'000.0));
    // ...just as this "worked" above...
//  LMI_TEST_EQUAL( 9, scale_power( 9, 0.0,           999'999'999'999'999.1));
    // but wouldn't have "worked" with the value        999'999'999'999'999.01

    // Test threshold neighborhood for scale_power=8.

    LMI_TEST_EQUAL( 0, scale_power( 8, 0.0,                    99'999'999.0));
    LMI_TEST_EQUAL( 3, scale_power( 8, 0.0,                    99'999'999.1));
    LMI_TEST_EQUAL( 3, scale_power( 8, 0.0,                99'999'999'999.0));
    LMI_TEST_EQUAL( 6, scale_power( 8, 0.0,                99'999'999'999.1));
    LMI_TEST_EQUAL( 6, scale_power( 8, 0.0,            99'999'999'999'999.0));
    LMI_TEST_EQUAL( 9, scale_power( 8, 0.0,            99'999'999'999'999.1));

    // Test threshold neighborhood for scale_power=7.

    LMI_TEST_EQUAL( 0, scale_power( 7, 0.0,                     9'999'999.0));
    LMI_TEST_EQUAL( 3, scale_power( 7, 0.0,                     9'999'999.1));
    LMI_TEST_EQUAL( 3, scale_power( 7, 0.0,                 9'999'999'999.0));
    LMI_TEST_EQUAL( 6, scale_power( 7, 0.0,                 9'999'999'999.1));
    LMI_TEST_EQUAL( 6, scale_power( 7, 0.0,             9'999'999'999'999.0));
    LMI_TEST_EQUAL( 9, scale_power( 7, 0.0,             9'999'999'999'999.1));

    // Test threshold neighborhood for scale_power=6.

    LMI_TEST_EQUAL( 0, scale_power( 6, 0.0,                       999'999.0));
    LMI_TEST_EQUAL( 3, scale_power( 6, 0.0,                       999'999.1));
    LMI_TEST_EQUAL( 3, scale_power( 6, 0.0,                   999'999'999.0));
    LMI_TEST_EQUAL( 6, scale_power( 6, 0.0,                   999'999'999.1));
    LMI_TEST_EQUAL( 6, scale_power( 6, 0.0,               999'999'999'999.0));
    LMI_TEST_EQUAL( 9, scale_power( 6, 0.0,               999'999'999'999.1));
    LMI_TEST_EQUAL( 9, scale_power( 6, 0.0,           999'999'999'999'999.0));
    LMI_TEST_EQUAL(12, scale_power( 6, 0.0,           999'999'999'999'999.1));
}

void test_trimming()
{
    char const*const superfluous = " ;";

    std::string s = "";
    ltrim(s, superfluous); rtrim(s, superfluous);
    LMI_TEST_EQUAL(s, "");

    s = " ";
    ltrim(s, superfluous); rtrim(s, superfluous);
    LMI_TEST_EQUAL(s, "");

    s = " ;; ";
    ltrim(s, superfluous); rtrim(s, superfluous);
    LMI_TEST_EQUAL(s, "");

    s = "a";
    ltrim(s, superfluous); rtrim(s, superfluous);
    LMI_TEST_EQUAL(s, "a");

    s = "; ;a; ;";
    ltrim(s, superfluous); rtrim(s, superfluous);
    LMI_TEST_EQUAL(s, "a");

    s = "a ; a ; ";
    rtrim(s, superfluous); // Shows what rtrim() does.
    LMI_TEST_EQUAL(s, "a ; a");
    ltrim(s, superfluous); rtrim(s, superfluous); // Does nothing, correctly.
    LMI_TEST_EQUAL(s, "a ; a");

    s = "; a ; a";
    ltrim(s, superfluous); // Shows what ltrim() does.
    LMI_TEST_EQUAL(s, "a ; a");
    ltrim(s, superfluous); rtrim(s, superfluous); // Does nothing, correctly.
    LMI_TEST_EQUAL(s, "a ; a");
}

void test_scoped_ios_format()
{
    std::ostringstream oss;
    oss << -2.71828 << ' ' << 3.14159 << std::endl;
    std::string const s(oss.str());

    {
    scoped_ios_format meaningless_name(oss);
    oss << std::setfill('0');
    oss << std::setprecision(3);
    oss << std::setw(12);
    oss << std::fixed;
    oss << std::hex;
    oss << std::hexfloat;
    oss << std::left;
    oss << std::showpos;
    oss << -2.71828 << ' ' << 3.14159 << std::endl;
    }

    oss.str("");
    oss.clear();

    oss << -2.71828 << ' ' << 3.14159 << std::endl;
    LMI_TEST_EQUAL(oss.str(), s);
}

class partly_unused
{
  public:
    partly_unused
        (int used
        ,int unused
        )
        :used_   {used}
        ,unused_ {unused}
        {
        // Suppress clang '-Wunused-private-field' warnings:
        stifle_unused_warning(unused_);
        }
    int used() {return used_;}

  private:
    int used_;
    int unused_;
};

int         return_temporary_0() {return {};}
std::string return_temporary_1() {return {};}

void test_stifle_unused_warning()
{
    // Variable neither initialized nor used.
    int a;
    stifle_unused_warning(a);

    // Variable initialized but not used.
    int b {2};
    stifle_unused_warning(b);

    // Variable initialized and used, but only conditionally.
    int c;
#if defined some_undefined_condition
    c = 3;
    std::cout << c << " This should not print" << std::endl;
#endif // defined some_undefined_condition
    stifle_unused_warning(c);

    // Leaving a variable uninitialized at declaration...
    int volatile d;
    // ...would not elicit an unused-value warning here--it has
    // no unused value because it has no value at all yet:
//  stifle_unused_warning(d);
    // ...so an unused-value warning must be stifled later...
    for(int i = 0; i < 7; ++i)
        {
        d = static_cast<int>(std::clock());
        }
    // ...e.g., here (for clang at least)--see:
    //   https://lists.nongnu.org/archive/html/lmi/2021-04/msg00058.html
    // and
    //   https://lists.nongnu.org/archive/html/lmi/2021-10/msg00050.html
    stifle_unused_warning(d);

    // Same as immediately preceding case, except that the variable
    // is initialized at declaration...
    int volatile e {};
    // ...so the warning can be stifled before the loop:
    stifle_unused_warning(e);
    for(int i = 0; i < 7; ++i)
        {
        e = static_cast<int>(std::clock());
        }

    partly_unused {0, 1};

    stifle_unused_warning(return_temporary_0());
    stifle_unused_warning(return_temporary_1());
}

int test_main(int, char*[])
{
    test_each_equal();
    test_files_are_identical();
    test_sup_inf();
    test_minmax();
    test_prefix_and_suffix();
    test_scale_power();
    test_trimming();
    test_scoped_ios_format();
    test_stifle_unused_warning();

    return 0;
}
