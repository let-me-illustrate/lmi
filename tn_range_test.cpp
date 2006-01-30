// Trammeled Numeric range type: unit test.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: tn_range_test.cpp,v 1.6 2006-01-30 05:26:14 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "tn_range.hpp"
#include "tn_range.tpp" // Template class implementation.
#include "tn_range_test_aux.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <algorithm> // std::min()
#include <sstream>

template<typename T>
struct nonnegative
    :public trammel_base<T>
{
    T nominal_maximum() const {return std::numeric_limits<T>::max();}
    T nominal_minimum() const {return 0.0;}
    T default_value()   const {return 1.0;}
};

// This should fail to compile:
//   template class tn_range<double, std::string>;

template class tn_range<double, nonnegative<double> >;
typedef tn_range<double, nonnegative<double> > r_nonnegative;

// Range type 'r_int_percentage' is explicitly instantiated in a
// different translation unit. A different template class with the
// same trammel type can be instantiated here:

typedef tn_range<double, percentage_trammel<double> > r_double_percentage;
template class tn_range<double, percentage_trammel<double> >;

// Although character and unsigned types aren't expected to be very
// useful as the first template parameter of template class tn_range,
// instantiating and testing examples here ensures that they work.

typedef tn_range<signed char, percentage_trammel<signed char> > r_signed_char_percentage;
template class tn_range<signed char, percentage_trammel<signed char> >;

typedef tn_range<unsigned int, percentage_trammel<unsigned int> > r_unsigned_int_percentage;
template class tn_range<unsigned int, percentage_trammel<unsigned int> >;

// This range type's bounds aren't exactly representable in binary.
// See the inline discussion where it is tested.

template<typename T>
struct surd
    :public trammel_base<T>
{
    T nominal_maximum() const {return 0.070000000000000001;}
    T nominal_minimum() const {return 0.069999999999999999;}
    T default_value()   const {return 0.07;}
};

template class tn_range<double, surd<double> >;
typedef tn_range<double, surd<double> > r_surd;

r_nonnegative r_zero  ( 0.00000);
r_nonnegative r_one   ( 1.00000);
r_nonnegative r_e     ( 2.71828);
r_nonnegative r_pi    ( 3.14159);

std::string const s_e ("2.71828");
std::string const s_pi("3.14159");

template<typename T>
void test_auxiliary_functions(char const* file, int line)
{
    INVOKE_BOOST_TEST_EQUAL(-1, signum(T(-1)), file, line);
    INVOKE_BOOST_TEST_EQUAL( 0, signum(T( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL( 1, signum(T( 1)), file, line);

    T const maxT = std::numeric_limits<T>::max();
    INVOKE_BOOST_TEST_EQUAL(-1, signum(-maxT), file, line);
    INVOKE_BOOST_TEST_EQUAL( 1, signum( maxT), file, line);

    INVOKE_BOOST_TEST_EQUAL(true , is_exact_integer(-1), file, line);
    INVOKE_BOOST_TEST_EQUAL(true , is_exact_integer( 0), file, line);
    INVOKE_BOOST_TEST_EQUAL(true , is_exact_integer( 1), file, line);

    // Integer types truncate the argument, always resulting in an
    // exact integer.
    bool volatile exact = std::numeric_limits<T>::is_exact;
    if(!exact)
        {
        INVOKE_BOOST_TEST_EQUAL(false, is_exact_integer(static_cast<T>( 0.5)), file, line);
        INVOKE_BOOST_TEST_EQUAL(false, is_exact_integer(static_cast<T>(1.07)), file, line);
        }

    // The second argument of adjust_bound() must be cast to T if it
    // is negative. Otherwise, an integral promotion [5.3.1/7] might
    // be performed, and that would prevent template resolution. For
    // parallelism, the cast is performed for positive arguments too.

    INVOKE_BOOST_TEST_EQUAL(-1, adjust_bound(T(-1), static_cast<T>( std::numeric_limits<T>::max())), file, line);
    INVOKE_BOOST_TEST_EQUAL( 0, adjust_bound(T( 0), static_cast<T>( std::numeric_limits<T>::max())), file, line);
    INVOKE_BOOST_TEST_EQUAL( 1, adjust_bound(T( 1), static_cast<T>( std::numeric_limits<T>::max())), file, line);

    INVOKE_BOOST_TEST_EQUAL(-1, adjust_bound(T(-1), static_cast<T>(-std::numeric_limits<T>::max())), file, line);
    INVOKE_BOOST_TEST_EQUAL( 0, adjust_bound(T( 0), static_cast<T>(-std::numeric_limits<T>::max())), file, line);
    INVOKE_BOOST_TEST_EQUAL( 1, adjust_bound(T( 1), static_cast<T>(-std::numeric_limits<T>::max())), file, line);

    if(!exact)
        {
        INVOKE_BOOST_TEST_RELATION(-1.07, <=, adjust_bound(T(-1.07), static_cast<T>( std::numeric_limits<T>::max())), file, line);
        INVOKE_BOOST_TEST_RELATION( 1.07, <=, adjust_bound(T( 1.07), static_cast<T>( std::numeric_limits<T>::max())), file, line);

        INVOKE_BOOST_TEST_RELATION(-1.07, >=, adjust_bound(T(-1.07), static_cast<T>(-std::numeric_limits<T>::max())), file, line);
        INVOKE_BOOST_TEST_RELATION( 1.07, >=, adjust_bound(T( 1.07), static_cast<T>(-std::numeric_limits<T>::max())), file, line);


        INVOKE_BOOST_TEST_RELATION(-1.07, <, adjust_bound(T(-1.07), static_cast<T>( std::numeric_limits<T>::max())), file, line);
        INVOKE_BOOST_TEST_RELATION( 1.07, <, adjust_bound(T( 1.07), static_cast<T>( std::numeric_limits<T>::max())), file, line);

        INVOKE_BOOST_TEST_RELATION(-1.07, >, adjust_bound(T(-1.07), static_cast<T>(-std::numeric_limits<T>::max())), file, line);
        INVOKE_BOOST_TEST_RELATION( 1.07, >, adjust_bound(T( 1.07), static_cast<T>(-std::numeric_limits<T>::max())), file, line);
        }

    bool volatile is_iec559 = std::numeric_limits<T>::is_iec559;
    bool volatile has_infinity = std::numeric_limits<T>::has_infinity;
    if(is_iec559 && has_infinity)
        {
        T const infT = std::numeric_limits<T>::infinity();
        INVOKE_BOOST_TEST_EQUAL(-1, signum(-infT), file, line);
        INVOKE_BOOST_TEST_EQUAL( 1, signum( infT), file, line);
        }

#if !defined __BORLANDC__
    // COMPILER !! The borland compiler tends to crash msw xp whenever
    // a quiet NaN is used.
    bool volatile has_quiet_NaN = std::numeric_limits<T>::has_quiet_NaN;
    if(is_iec559 && has_quiet_NaN)
        {
        T const qnanT = std::numeric_limits<T>::quiet_NaN();
        INVOKE_BOOST_TEST_EQUAL( 0, signum(-qnanT), file, line);
        INVOKE_BOOST_TEST_EQUAL( 0, signum( qnanT), file, line);
        }
#endif // Not borland compiler.
}

template<typename T>
void test_percentages(char const* file, int line)
{
    // These workarounds for negative one permit running these tests
    // with unsigned percentage types, and avoid compiler warnings.
#ifdef __BORLANDC__
    // COMPILER !! This definition somehow makes borland happy. See
    //  http://groups.google.com/groups?selm=4196a049%241%40newsgroups.borland.com
    T::number_type work_around_borland_defect;
    &work_around_borland_defect;
#endif // __BORLANDC__
    typedef typename T::number_type N;
    // Use this to test whether -1 is valid.
    N n1 = static_cast<N>(-1);
    // Use this to test trammeling.
    N n10 = std::min(static_cast<N>(0), static_cast<N>(-1));

    T t_percentage;
    t_percentage = 50;
    INVOKE_BOOST_TEST_EQUAL(50, t_percentage, file, line);

    INVOKE_BOOST_TEST(!t_percentage.is_valid( n1), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid(  0), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid(  1), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid(100), file, line);
    INVOKE_BOOST_TEST(!t_percentage.is_valid(101), file, line);

    INVOKE_BOOST_TEST(!t_percentage.is_valid(" -1"), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid("  0"), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid("  1"), file, line);
    INVOKE_BOOST_TEST( t_percentage.is_valid("100"), file, line);
    INVOKE_BOOST_TEST(!t_percentage.is_valid("101"), file, line);

    INVOKE_BOOST_TEST_EQUAL(t_percentage.limits().first ,   0, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.limits().second, 100, file, line);

    INVOKE_BOOST_TEST_EQUAL(t_percentage.trammel(n10),   0, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.trammel(  0),   0, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.trammel(  1),   1, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.trammel(100), 100, file, line);
    INVOKE_BOOST_TEST_EQUAL(t_percentage.trammel(101), 100, file, line);

    T t0(n10);
    INVOKE_BOOST_TEST_EQUAL(  0, t0, file, line);
    T t1(  0);
    INVOKE_BOOST_TEST_EQUAL(  0, t1, file, line);
    T t2(  1);
    INVOKE_BOOST_TEST_EQUAL(  1, t2, file, line);
    T t3(100);
    INVOKE_BOOST_TEST_EQUAL(100, t3, file, line);
    T t4(101);
    INVOKE_BOOST_TEST_EQUAL(100, t4, file, line);

    t_percentage = n10;
    INVOKE_BOOST_TEST_EQUAL(  0, t_percentage, file, line);
    t_percentage =   0;
    INVOKE_BOOST_TEST_EQUAL(  0, t_percentage, file, line);
    t_percentage =   1;
    INVOKE_BOOST_TEST_EQUAL(  1, t_percentage, file, line);
    t_percentage = 100;
    INVOKE_BOOST_TEST_EQUAL(100, t_percentage, file, line);
    t_percentage = 101;
    INVOKE_BOOST_TEST_EQUAL(100, t_percentage, file, line);
}

int test_main(int, char*[])
{
    test_auxiliary_functions<signed char>(__FILE__, __LINE__);
    test_auxiliary_functions<int        >(__FILE__, __LINE__);
    test_auxiliary_functions<float      >(__FILE__, __LINE__);
    test_auxiliary_functions<double     >(__FILE__, __LINE__);
    test_auxiliary_functions<long double>(__FILE__, __LINE__);

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
    // Not tested because it's private.
//    BOOST_TEST_EQUAL(range1.str(), "3.14159");

    // Explicit conversion to enumerator.
    BOOST_TEST_EQUAL(range1.value(), 3.14159);

    // *** Limits.

    // Floating-point limits having an exact binary representation.

    BOOST_TEST_EQUAL(range1.limits().first , 0.0);
    BOOST_TEST_EQUAL(range1.limits().second, std::numeric_limits<double>::max());
    BOOST_TEST( range1.is_valid( 1.0));
    BOOST_TEST( range1.is_valid( 0.0));
    BOOST_TEST( range1.is_valid(-0.0));
    BOOST_TEST(!range1.is_valid(-1.0));
    BOOST_TEST_EQUAL(range1.trammel( -1.0),   0.0);

    // Floating-point limits having no exact binary representation.

    double volatile epsilon = std::numeric_limits<double>::epsilon();

    // For IEEE 754's 64-bit double type, C99's footnote 302 seems to
    // suggest that these numbers should be neighbors. In that case,
    // they and their upper and lower neighbors should be the only
    // four values permitted by range type 'r_surd'.

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

    BOOST_TEST( surd0.is_valid( 0.07 * (1.0 + 1.0 * epsilon)));
    BOOST_TEST( surd0.is_valid( 0.07 / (1.0 + 1.0 * epsilon)));

    // If exactly four values are permissible, then exactly one of
    // these is permissible.
    BOOST_TEST
        (   surd0.is_valid( 0.07 * (1.0 + 2.0 * epsilon))
        ^   surd0.is_valid( 0.07 / (1.0 + 2.0 * epsilon))
        );

    BOOST_TEST(!surd0.is_valid( 0.07 * (1.0 + 3.0 * epsilon)));
    BOOST_TEST(!surd0.is_valid( 0.07 / (1.0 + 3.0 * epsilon)));

    // Validation of strings representing floating-point values.

    BOOST_TEST( range1.is_valid("-0.0"));
    BOOST_TEST( range1.is_valid(" -0."));
    BOOST_TEST( range1.is_valid(" -.0"));
    BOOST_TEST( range1.is_valid("-0e0"));
    BOOST_TEST( range1.is_valid("-0E0"));
    BOOST_TEST(!range1.is_valid("$123"));

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

    return 0;
}

