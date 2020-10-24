// Trammeled Numeric range type: unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "tn_range.hpp"
#include "tn_range.tpp"                 // Class template implementation.
#include "tn_range_test_aux.hpp"        // Template class instantiation.

#include "test_tools.hpp"

#include <algorithm>                    // min()
#include <sstream>
#include <type_traits>                  // is_same, remove_cv

// This should fail to compile:
//    template class tn_range<double, std::string>;

// This should fail to compile:
//    tn_range<bool, percentage_trammel<bool>> boolean_percentage;

template<typename T>
class nonnegative
    :public trammel_base<T>
{
    T nominal_minimum() const override {return 0.0;}
    T default_value()   const override {return 1.0;}
    T nominal_maximum() const override {return std::numeric_limits<T>::max();}
};

template class tn_range<double, nonnegative<double>>;
typedef tn_range<double, nonnegative<double>> r_nonnegative;

/// One reasonable interpretation of an "unbounded" floating-point
/// range encompasses only normalized values that are nonsingular.

template<typename T>
class unbounded
    :public trammel_base<T>
{
    T nominal_minimum() const override {return -std::numeric_limits<T>::max();}
    T default_value()   const override {return 0.0;}
    T nominal_maximum() const override {return  std::numeric_limits<T>::max();}
};

template class tn_range<double, unbounded<double>>;
typedef tn_range<double, unbounded<double>> r_unbounded;

// Range type 'r_int_percentage' is explicitly instantiated in a
// different translation unit. A different template class with the
// same trammel type can be instantiated here:

typedef tn_range<double, percentage_trammel<double>> r_double_percentage;
template class tn_range<double, percentage_trammel<double>>;

// Although character and unsigned types aren't expected to be very
// useful as the first template parameter of class template tn_range,
// instantiating and testing examples here ensures that they work.

typedef tn_range<signed char, percentage_trammel<signed char>> r_signed_char_percentage;
template class tn_range<signed char, percentage_trammel<signed char>>;

typedef tn_range<unsigned int, percentage_trammel<unsigned int>> r_unsigned_int_percentage;
template class tn_range<unsigned int, percentage_trammel<unsigned int>>;

// This range type's bounds aren't exactly representable in binary.
// See the inline discussion where it is tested.

template<typename T>
class surd
    :public trammel_base<T>
{
    T nominal_minimum() const override {return 0.069999999999999999;}
    T default_value()   const override {return 0.07;}
    T nominal_maximum() const override {return 0.070000000000000001;}
};

template class tn_range<double, surd<double>>;
typedef tn_range<double, surd<double>> r_surd;

/// An exception should be thrown if an attempt is made to use an
/// object of type 'r_absurd', because its limits are crossed.

template<typename T>
class absurd
    :public trammel_base<T>
{
    T nominal_minimum() const override {return 1;}
    T default_value()   const override {return 0;}
    T nominal_maximum() const override {return 0;}
};

template class tn_range<int, absurd<int>>;
typedef tn_range<int, absurd<int>> r_absurd;

struct tn_range_test
{
    template<typename T>
    static void test_auxiliary_functions(char const* file, int line);

    template<typename T>
    static void test_floating_auxiliary_functions(char const* file, int line);

    template<typename T>
    static void test_percentages(char const* file, int line);

    static void test_diagnostics();

    static void test_absurd_limits();

    static void test_nonfundamental_number_type();

    static void test();
};

template<typename T>
void tn_range_test::test_auxiliary_functions(char const* file, int line)
{
    T const maxT = std::numeric_limits<T>::max();
    T const minT = std::numeric_limits<T>::lowest();

    INVOKE_BOOST_TEST(!is_strictly_between_extrema(maxT), file, line);
    if(!std::is_same_v<bool,typename std::remove_cv<T>::type>)
        {
        INVOKE_BOOST_TEST( is_strictly_between_extrema<T>(1), file, line);
        }

    INVOKE_BOOST_TEST_EQUAL( 0, signum(T( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL( 1, signum(T( 1)), file, line);

    INVOKE_BOOST_TEST_EQUAL( 1, signum(maxT), file, line);

    INVOKE_BOOST_TEST_EQUAL(true , is_exact_integer(T( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL(true , is_exact_integer(T( 1)), file, line);

    if(minT < 0)
        {
        // The left-hand side is cast to T to avoid gcc 'bool-compare'
        // diagnostics. An 'is_bool' conditional wouldn't prevent the
        // macros from being expanded. See:
        //   https://lists.nongnu.org/archive/html/lmi/2017-05/msg00029.html
        INVOKE_BOOST_TEST_EQUAL(T(-1), signum(T(-1)), file, line);
        INVOKE_BOOST_TEST_EQUAL(T(-1), signum(minT ), file, line);
        INVOKE_BOOST_TEST_EQUAL(true , is_exact_integer(T(-1)), file, line);
        }

    // Integer types truncate the argument, always resulting in an
    // exact integer.
    bool volatile exact = std::numeric_limits<T>::is_exact;
    if(!exact)
        {
        INVOKE_BOOST_TEST_EQUAL(false, is_exact_integer(T( 0.5)), file, line);
        INVOKE_BOOST_TEST_EQUAL(false, is_exact_integer(T(1.07)), file, line);
        }

    bool volatile is_iec559 = std::numeric_limits<T>::is_iec559;
    bool volatile has_infinity = std::numeric_limits<T>::has_infinity;
    if(is_iec559 && has_infinity)
        {
        T const infT = std::numeric_limits<T>::infinity();
        INVOKE_BOOST_TEST_EQUAL(-1, signum(-infT), file, line);
        INVOKE_BOOST_TEST_EQUAL( 1, signum( infT), file, line);
        }

    bool volatile has_quiet_NaN = std::numeric_limits<T>::has_quiet_NaN;
    if(is_iec559 && has_quiet_NaN)
        {
        T const qnanT = std::numeric_limits<T>::quiet_NaN();
        INVOKE_BOOST_TEST_EQUAL(-1, signum(-qnanT), file, line);
        INVOKE_BOOST_TEST_EQUAL( 1, signum( qnanT), file, line);
        }
}

template<typename T>
void tn_range_test::test_floating_auxiliary_functions(char const* file, int line)
{
    INVOKE_BOOST_TEST_EQUAL(-1, adjust_bound(T(-1),  std::numeric_limits<T>::max()), file, line);
    INVOKE_BOOST_TEST_EQUAL( 0, adjust_bound(T( 0),  std::numeric_limits<T>::max()), file, line);
    INVOKE_BOOST_TEST_EQUAL( 1, adjust_bound(T( 1),  std::numeric_limits<T>::max()), file, line);

    INVOKE_BOOST_TEST_EQUAL(-1, adjust_bound(T(-1), -std::numeric_limits<T>::max()), file, line);
    INVOKE_BOOST_TEST_EQUAL( 0, adjust_bound(T( 0), -std::numeric_limits<T>::max()), file, line);
    INVOKE_BOOST_TEST_EQUAL( 1, adjust_bound(T( 1), -std::numeric_limits<T>::max()), file, line);

    bool volatile exact = std::numeric_limits<T>::is_exact;
    if(!exact)
        {
        // An inexact bound should be adjusted to a different value,
        // and it should be adjusted in the appropriate direction.
        // Test those requirements separately to make the cause of any
        // failure more readily apparent.

        // Test direction of adjustment.

        INVOKE_BOOST_TEST_RELATION(T(-1.07), <=, adjust_bound(T(-1.07),  std::numeric_limits<T>::max()), file, line);
        INVOKE_BOOST_TEST_RELATION(T( 1.07), <=, adjust_bound(T( 1.07),  std::numeric_limits<T>::max()), file, line);

        INVOKE_BOOST_TEST_RELATION(T(-1.07), >=, adjust_bound(T(-1.07), -std::numeric_limits<T>::max()), file, line);
        INVOKE_BOOST_TEST_RELATION(T( 1.07), >=, adjust_bound(T( 1.07), -std::numeric_limits<T>::max()), file, line);

        // Test inequality of original and adjusted values.

        INVOKE_BOOST_TEST_RELATION(T(-1.07), <,  adjust_bound(T(-1.07),  std::numeric_limits<T>::max()), file, line);
        INVOKE_BOOST_TEST_RELATION(T( 1.07), <,  adjust_bound(T( 1.07),  std::numeric_limits<T>::max()), file, line);

        INVOKE_BOOST_TEST_RELATION(T(-1.07), >,  adjust_bound(T(-1.07), -std::numeric_limits<T>::max()), file, line);
        INVOKE_BOOST_TEST_RELATION(T( 1.07), >,  adjust_bound(T( 1.07), -std::numeric_limits<T>::max()), file, line);
        }

    bool volatile is_iec559 = std::numeric_limits<T>::is_iec559;
    bool volatile has_infinity = std::numeric_limits<T>::has_infinity;
    if(is_iec559 && has_infinity)
        {
        T const infT = std::numeric_limits<T>::infinity();
        INVOKE_BOOST_TEST_EQUAL(-infT, adjust_bound(T(-infT),  std::numeric_limits<T>::max()), file, line);
        INVOKE_BOOST_TEST_EQUAL( infT, adjust_bound(T( infT),  std::numeric_limits<T>::max()), file, line);
        INVOKE_BOOST_TEST_EQUAL(-infT, adjust_bound(T(-infT), -std::numeric_limits<T>::max()), file, line);
        INVOKE_BOOST_TEST_EQUAL( infT, adjust_bound(T( infT), -std::numeric_limits<T>::max()), file, line);
        }
}

template<typename T>
void tn_range_test::test_percentages(char const* file, int line)
{
    // These workarounds for negative one permit running these tests
    // with unsigned percentage types, and avoid compiler warnings.
    typedef typename T::number_type N;
    // Use this to test whether -1 is valid.
    N n1 = static_cast<N>(-1);
    // Use this to test trammeling.
    N n10 = std::min(static_cast<N>(0), static_cast<N>(-1));

    T t_percentage;
    t_percentage = 50;
    INVOKE_BOOST_TEST_EQUAL(N(50), t_percentage, file, line);

    INVOKE_BOOST_TEST(!t_percentage.is_valid( n1), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid(  0), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid(  1), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid(100), file, line);
    INVOKE_BOOST_TEST(!t_percentage.is_valid(101), file, line);

    INVOKE_BOOST_TEST(!t_percentage.diagnose_invalidity(" -1").empty(), file, line);
    INVOKE_BOOST_TEST( t_percentage.diagnose_invalidity("  0").empty(), file, line);
    INVOKE_BOOST_TEST( t_percentage.diagnose_invalidity("  1").empty(), file, line);
    INVOKE_BOOST_TEST( t_percentage.diagnose_invalidity("100").empty(), file, line);
    INVOKE_BOOST_TEST(!t_percentage.diagnose_invalidity("101").empty(), file, line);

    INVOKE_BOOST_TEST_EQUAL(t_percentage.minimum(),   0, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.maximum(), 100, file, line);

    INVOKE_BOOST_TEST_EQUAL(t_percentage.curb(n10),   0, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.curb(  0),   0, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.curb(  1),   1, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.curb(100), 100, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.curb(101), 100, file, line);

    T t0(n10);
    INVOKE_BOOST_TEST_EQUAL(N(  0), t0, file, line);
    T t1(  0);
    INVOKE_BOOST_TEST_EQUAL(N(  0), t1, file, line);
    T t2(  1);
    INVOKE_BOOST_TEST_EQUAL(N(  1), t2, file, line);
    T t3(100);
    INVOKE_BOOST_TEST_EQUAL(N(100), t3, file, line);
    T t4(101);
    INVOKE_BOOST_TEST_EQUAL(N(100), t4, file, line);

    t_percentage = n10;
    INVOKE_BOOST_TEST_EQUAL(N(  0), t_percentage, file, line);
    t_percentage =   0;
    INVOKE_BOOST_TEST_EQUAL(N(  0), t_percentage, file, line);
    t_percentage =   1;
    INVOKE_BOOST_TEST_EQUAL(N(  1), t_percentage, file, line);
    t_percentage = 100;
    INVOKE_BOOST_TEST_EQUAL(N(100), t_percentage, file, line);
    t_percentage = 101;
    INVOKE_BOOST_TEST_EQUAL(N(100), t_percentage, file, line);
}

void tn_range_test::test()
{
    test_auxiliary_functions<bool         >(__FILE__, __LINE__);
    test_auxiliary_functions<signed char  >(__FILE__, __LINE__);
    test_auxiliary_functions<unsigned char>(__FILE__, __LINE__);
    test_auxiliary_functions<int          >(__FILE__, __LINE__);
    test_auxiliary_functions<float        >(__FILE__, __LINE__);
    test_auxiliary_functions<double       >(__FILE__, __LINE__);
    test_auxiliary_functions<long double  >(__FILE__, __LINE__);

    test_floating_auxiliary_functions<float      >(__FILE__, __LINE__);
    test_floating_auxiliary_functions<double     >(__FILE__, __LINE__);
    test_floating_auxiliary_functions<long double>(__FILE__, __LINE__);

    r_nonnegative const r_zero  ( 0.00000);
    r_nonnegative const r_one   ( 1.00000);
    r_nonnegative const r_e     ( 2.71828);
    r_nonnegative const r_pi    ( 3.14159);

    std::string const s_e ("2.71828");
    std::string const s_pi("3.14159");

    // *** Construction.

    // Default ctor.
    r_nonnegative range0;
    BOOST_TEST_EQUAL(range0, r_one);

    // Copy ctor.
    r_nonnegative range1(range0);
    BOOST_TEST_EQUAL(range1, r_one);

    // Construct from Number.
    r_nonnegative range2(3.14159);
    BOOST_TEST_EQUAL(range2, r_pi);

    // Construct from std::string.
    r_nonnegative range3(s_e);
    BOOST_TEST_EQUAL(range3, r_e);

    // Construct from char const* implicitly converted to std::string.
    r_nonnegative range4("2.71828");
    BOOST_TEST_EQUAL(range4, r_e);

    // *** Assignment.

    // Copy assignment operator.
    range1 = range3;
    BOOST_TEST_EQUAL(range1, r_e);

    // Assign from Number.
    range1 = 0.0;
    BOOST_TEST_EQUAL(range1, r_zero);

    // Assign from std::string.
    range1 = s_e;
    BOOST_TEST_EQUAL(range1, r_e);

    // Assign from char const* implicitly converted to std::string.
    range1 = "3.14159";
    BOOST_TEST_EQUAL(range1, r_pi);

    // *** Equivalence.

    // Equivalence to same type.
    BOOST_TEST_EQUAL(range1, range2);

    // Equivalence to Number.
    BOOST_TEST_EQUAL(range1, 3.14159);

    // Equivalence to std::string.
    BOOST_TEST_EQUAL(range1, s_pi);

    // Equivalence to char const* implicitly converted to std::string.
    BOOST_TEST_EQUAL(range1, "3.14159");

    // Because comparison to std::string is a matter of numerical
    // equivalence, test several equivalent representations.
    BOOST_TEST_EQUAL(range1, "3.1415900");
    BOOST_TEST_EQUAL(range1, "3.14159e0");
    BOOST_TEST_EQUAL(range1, ".314159E1");

    // *** Operators added by boost::equality_comparable.

    // Inequivalence to same type.
    BOOST_TEST_UNEQUAL(range4,  range1);

    // [In]equivalence to Number.
    BOOST_TEST_EQUAL  (3.14159, range1);
    BOOST_TEST_UNEQUAL(range1, 2.71828);
    BOOST_TEST_UNEQUAL(2.71828, range1);

    // [In]equivalence to std::string.
    BOOST_TEST_EQUAL  (s_pi,    range1);
    BOOST_TEST_UNEQUAL(range1,     s_e);
    BOOST_TEST_UNEQUAL(s_e,     range1);

    // *** Explicit conversions.

    // Explicit conversion to std::string.
    BOOST_TEST_EQUAL(range1.str(), "3.14159");

    // Explicit conversion to number.
    BOOST_TEST_EQUAL(range1.value(), 3.14159);

    // *** Type identification.
    BOOST_TEST(typeid(double) == range1.value_type());

    // *** Limits.

    // Floating-point limits having an exact binary representation.

    BOOST_TEST_EQUAL(range1.minimum(), 0.0);
    BOOST_TEST_EQUAL(range1.maximum(), std::numeric_limits<double>::max());
    BOOST_TEST( range1.is_valid( 1.0));
    BOOST_TEST( range1.is_valid( 0.0));
    BOOST_TEST( range1.is_valid(-0.0));
    BOOST_TEST(!range1.is_valid(-1.0));
    BOOST_TEST_EQUAL(range1.curb( -1.0), 0.0);

    // Floating-point limits having no exact binary representation.

    double volatile epsilon = std::numeric_limits<double>::epsilon();

    // For IEEE 754's 64-bit double type, C99's footnote 302 seems to
    // suggest that these numbers should be neighbors. In that case,
    // they and their upper and lower neighbors should be the only
    // four values permitted by range type 'r_surd', out of these
    // five plausible candidates (to DECIMAL_DIG precision):
    //      000000000011111111112
    //      123456789012345678901
    //   0.0699999999999999789058 = 0.07 / (1.0 + 2.0 * epsilon)
    //   0.0699999999999999927836 = 0.07 / (1.0 + 1.0 * epsilon)
    //   0.0700000000000000066613 = 0.07
    //   0.0700000000000000205391 = 0.07 * (1.0 + 1.0 * epsilon)
    //   0.0700000000000000344169 = 0.07 * (1.0 + 2.0 * epsilon)

    double volatile hi = 0.070000000000000001;
    double volatile lo = 0.069999999999999999;
    double volatile z  = lo * (1.0 + epsilon);
    BOOST_TEST(z == hi);

    r_surd surd0;

    BOOST_TEST(!surd0.is_valid( 0.0699999999999999  ));
    BOOST_TEST( surd0.is_valid( 0.069999999999999999));
    BOOST_TEST( surd0.is_valid( 0.070000000000000000));
    BOOST_TEST( surd0.is_valid( 0.070000000000000001));
    BOOST_TEST(!surd0.is_valid( 0.0700000000000001  ));

    BOOST_TEST( surd0.is_valid( 0.0700000000000000205391)); // 0.07 * (1+ε)
    BOOST_TEST( surd0.is_valid( 0.0699999999999999927836)); // 0.07 / (1+ε)

    // If exactly four values are permissible, then exactly one of
    // these is permissible.
    BOOST_TEST
        (   surd0.is_valid( 0.0700000000000000344169) // 0.07 * (1+2ε)
        ^   surd0.is_valid( 0.0699999999999999789058) // 0.07 / (1+2ε)
        );

    BOOST_TEST(!surd0.is_valid( 0.07 * (1.0 + 3.0 * epsilon)));
    BOOST_TEST(!surd0.is_valid( 0.07 / (1.0 + 3.0 * epsilon)));

    // Validation of strings representing floating-point values.

    BOOST_TEST( range1.diagnose_invalidity("-0.0").empty());
    BOOST_TEST( range1.diagnose_invalidity(" -0.").empty());
    BOOST_TEST( range1.diagnose_invalidity(" -.0").empty());
    BOOST_TEST( range1.diagnose_invalidity("-0e0").empty());
    BOOST_TEST( range1.diagnose_invalidity("-0E0").empty());
    BOOST_TEST(!range1.diagnose_invalidity("$123").empty());

    // *** Stream operators.
    r_nonnegative const pi(r_pi);
    r_nonnegative range5;
    BOOST_TEST_UNEQUAL(pi, range5);
    std::stringstream ss;
    ss << pi;
    ss >> range5;
    BOOST_TEST_EQUAL(pi, range5);

    BOOST_TEST_UNEQUAL(r_e, range5);
    ss.str("");
    ss.clear();
    ss << r_e;
    ss >> range5;
    BOOST_TEST_EQUAL(r_e, range5);

    // Test a numeric-range class instantiated in a different
    // translation unit.

    test_percentages<r_int_percentage>         (__FILE__, __LINE__);

    // Test character and unsigned types.

    test_percentages<r_signed_char_percentage> (__FILE__, __LINE__);
    test_percentages<r_unsigned_int_percentage>(__FILE__, __LINE__);

    // Test a numeric-range class with a floating-point underlying
    // type and neither limit equal to the representable extrema.

    test_percentages<r_double_percentage>     (__FILE__, __LINE__);

    test_diagnostics();

    test_absurd_limits();

    test_nonfundamental_number_type();
}

void tn_range_test::test_diagnostics()
{
    r_unbounded r;
    std::string v;
    std::string s;

    // Test range [-1.07, 1.07].

    r.minimum(-1.07);
    r.maximum( 1.07);

    v = r.diagnose_invalidity("  0.0");
    BOOST_TEST_EQUAL("", v);

    v = r.diagnose_invalidity(" 1.07");
    BOOST_TEST_EQUAL("", v);

    v = r.diagnose_invalidity("-1.07");
    BOOST_TEST_EQUAL("", v);

    v = r.diagnose_invalidity(" 1.070000000000001");
    s = " 1.070000000000001 is too high: value must be between -1.07 and 1.07 inclusive.";
    BOOST_TEST_EQUAL(s, v);

    v = r.diagnose_invalidity("-1.070000000000001");
    s = "-1.070000000000001 is too low: value must be between -1.07 and 1.07 inclusive.";
    BOOST_TEST_EQUAL(s, v);

    v = r.diagnose_invalidity(" 1e999999");
    s = " 1e999999 is not normalized.";
    BOOST_TEST_EQUAL(s, v);

    v = r.diagnose_invalidity("-1e999999");
    s = "-1e999999 is not normalized.";
    BOOST_TEST_EQUAL(s, v);

    v = r.diagnose_invalidity("$123");
    s = "'$123' is ill formed.";
    BOOST_TEST_EQUAL(s, v);

    // Test range [2.0, DBL_MAX]. Changing first the minimum and then
    // the maximum would not work: the new minimum of 2.0 exceeds the
    // old maximum of 1.07 and would therefore be rejected. This is a
    // motivating case for member function minimum_and_maximum().

    r.minimum_and_maximum(2.0, std::numeric_limits<double>::max());

    v = r.diagnose_invalidity(" 1.07");
    s = " 1.07 is too low: 2 is the lower limit.";
    BOOST_TEST_EQUAL(s, v);

    v = r.diagnose_invalidity("-1e999999");
    s = "-1e999999 is not normalized.";
    BOOST_TEST_EQUAL(s, v);

    v = r.diagnose_invalidity("1e999999");
    s = "1e999999 is not normalized.";
    BOOST_TEST_EQUAL(s, v);

    // Test range [-DBL_MAX, 2.0].

    r.minimum(-std::numeric_limits<double>::max());
    r.maximum(2.0);

    v = r.diagnose_invalidity(" 2.718281828");
    s = " 2.718281828 is too high: 2 is the upper limit.";
    BOOST_TEST_EQUAL(s, v);

    // Test range [-DBL_MAX, DBL_MAX].

    r.minimum(-std::numeric_limits<double>::max());
    r.maximum( std::numeric_limits<double>::max());

    v = r.diagnose_invalidity("1e999999");
    s = "1e999999 is not normalized.";
    BOOST_TEST_EQUAL(s, v);
}

void tn_range_test::test_absurd_limits()
{
    absurd<int> a;

    BOOST_TEST_THROW
        (a.assert_sanity()
        ,std::runtime_error
        ,"Lower bound 1 exceeds upper bound 0 ."
        );

    BOOST_TEST_THROW
        (r_absurd b;
        ,std::runtime_error
        ,"Lower bound 1 exceeds upper bound 0 ."
        );

    r_double_percentage p0;
    r_double_percentage p1;

    p0.minimum(1.0);
    p0.maximum(2.0);

    BOOST_TEST_EQUAL(p0.minimum(),   1.0);
    BOOST_TEST_EQUAL(p0.maximum(),   2.0);

    BOOST_TEST_THROW
        (p0.minimum(-1.0)
        ,std::runtime_error
        ,"Cannot change lower bound to -1, which is less than infimum 0 ."
        );

    BOOST_TEST_THROW
        (p0.minimum(3.0)
        ,std::runtime_error
        ,"Cannot change lower bound to 3, which is greater than upper bound 2 ."
        );

    BOOST_TEST_THROW
        (p0.maximum(0.0)
        ,std::runtime_error
        ,"Cannot change upper bound to 0, which is less than lower bound 1 ."
        );

    BOOST_TEST_THROW
        (p0.maximum(101.0)
        ,std::runtime_error
        ,"Cannot change upper bound to 101, which is greater than supremum 100 ."
        );

    // Make sure p1's limits aren't affected by the change in p0's.

    BOOST_TEST_EQUAL(p1.minimum(),   0.0);
    BOOST_TEST_EQUAL(p1.maximum(), 100.0);
}

void tn_range_test::test_nonfundamental_number_type()
{
    r_range_udt r0;
}

int test_main(int, char*[])
{
    tn_range_test::test();
    return 0;
}
