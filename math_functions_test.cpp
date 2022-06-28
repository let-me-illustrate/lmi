// Miscellaneous mathematical operations--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "math_functions.hpp"

#include "bin_exp.hpp"
#include "fenv_lmi.hpp"
#include "materially_equal.hpp"
#include "miscellany.hpp"               // stifle_unused_warning()
#include "test_tools.hpp"
#include "timer.hpp"

#include <algorithm>                    // min()
#include <cfloat>                       // DBL_EPSILON
#include <climits>                      // CHAR_BIT
#include <cmath>                        // fabs(), isnan(), pow()
#include <cstdint>
#include <iomanip>
#include <limits>
#include <type_traits>

// Some of these tests may raise hardware exceptions. That means that
// edge cases are tested, not that the code tested is invalid for
// arguments that aren't ill conditioned.

namespace
{
// These naive implementations in terms of pow() are slower and
// less accurate than those in the header tested here.

template<typename T>
struct i_upper_12_over_12_from_i_naive
{
    static_assert(std::is_floating_point_v<T>);
    T operator()(T const& i) const
        {
        long double z = -1.0L + std::pow((1.0L + i), 1.0L / 12.0L);
        return static_cast<T>(z);
        }
};

template<typename T>
struct i_from_i_upper_12_over_12_naive
{
    static_assert(std::is_floating_point_v<T>);
    T operator()(T const& i) const
        {
        long double z = -1.0L + std::pow((1.0L + i), 12.0L);
        return static_cast<T>(z);
        }
};

template<typename T>
struct d_upper_12_from_i_naive
{
    static_assert(std::is_floating_point_v<T>);
    T operator()(T const& i) const
        {
        long double z = 12.0L * (1.0L - std::pow(1.0L + i, -1.0L / 12.0L));
        return static_cast<T>(z);
        }
};

template<typename T, int n>
struct net_i_from_gross_naive
{
    static_assert(std::is_floating_point_v<T>);
    T operator()(T const& i, T const& spread, T const& fee) const
        {
        static long double const reciprocal_n = 1.0L / n;
        long double z = std::pow
            (
            1.0L
            +   std::pow(1.0L + i,      reciprocal_n)
            -   std::pow(1.0L + spread, reciprocal_n)
            -                   fee   * reciprocal_n
            ,n
            );
        return static_cast<T>(z - 1.0L);
        }
};

template<typename T>
struct coi_rate_from_q_naive
{
    static_assert(std::is_floating_point_v<T>);
    T operator()(T const& q, T const& max_coi) const
        {
        if(0.0 == q)
            {
            return 0.0;
            }
        else if(1.0 <= q)
            {
            return max_coi;
            }
        else
            {
            long double monthly_q = 1.0L - std::pow(1.0L - q, 1.0L / 12.0L);
            return std::min
                (max_coi
                ,static_cast<T>(monthly_q / (1.0L - monthly_q))
                );
            }
        }
};

template<typename T, int n>
struct i_upper_n_over_n_from_i_naive
{
    static_assert(std::is_floating_point_v<T>);
    T operator()(T const& i) const
        {
        return T(-1) + std::pow((T(1) + i), T(1) / n);
        }
};
} // Unnamed namespace.

// These 'mete[01]' functions perform the same sets of operations
// using different implementations.

// This implementation naively uses pow(); it is both slower and
// less accurate than an alternative using expm1() and log1p().
void mete0()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_12_over_12_from_i_naive<double>()(0.04);
        x = i_from_i_upper_12_over_12_naive<double>()(0.04);
        x = d_upper_12_from_i_naive        <double>()(0.04);
        x = net_i_from_gross_naive<double,365>()(0.04, 0.007, 0.003);
        }
    stifle_unused_warning(x);
}

// This implementation uses production functors.
void mete1()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_12_over_12_from_i<double>()(0.04);
        x = i_from_i_upper_12_over_12<double>()(0.04);
        x = d_upper_12_from_i        <double>()(0.04);
        x = net_i_from_gross<double,365>()(0.04, 0.007, 0.003);
        }
    stifle_unused_warning(x);
}

// These 'mete[23]' functions perform the same operation using
// different types.

// This test uses type 'double'.
void mete2()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_n_over_n_from_i<double,365>()(0.01);
        }
    stifle_unused_warning(x);
}

// This test uses type 'long double'.
void mete3()
{
    long double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_n_over_n_from_i<long double,365>()(0.01);
        }
    stifle_unused_warning(x);
}

// These 'mete[45]' functions calculate 10^-9 in different ways.
// Binary exponentiation is much faster than a transcendental
// calculation; that's not surprising, but worth measuring.

void mete4()
{
    double volatile base     = 10.0;
    int    volatile exponent = -9;
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = bin_exp(base, exponent);
        }
    stifle_unused_warning(x);
}

void mete5()
{
    double volatile base     = 10.0;
    int    volatile exponent = -9;
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = std::pow(base, exponent);
        }
    stifle_unused_warning(x);
}

// These 'mete[6789]' functions test the speed of exmp1() and log1p().

void mete6()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = lmi::expm1(0.0032737397821988637);
        }
    stifle_unused_warning(x);
}

void mete7()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = std::expm1(0.0032737397821988637);
        }
    stifle_unused_warning(x);
}

void mete8()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = lmi::log1p(0.04);
        }
    stifle_unused_warning(x);
}

void mete9()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = std::log1p(0.04);
        }
    stifle_unused_warning(x);
}

void test_assign_midpoint()
{
    constexpr double smallnum = std::numeric_limits<double>::denorm_min();
    constexpr double bignum   = std::numeric_limits<double>::max();

    std::vector<double> v0 {smallnum, bignum, 0.0, 0.0, 1.0, 100.0};
    std::vector<double> v1 {smallnum, bignum, 0.0, 7.0, 2.0, 257.0};
    std::vector<double> v2(v0.size());
    assign_midpoint(v2, v0, v1);
    std::vector<double> v3 {smallnum, bignum, 0.0, 3.5, 1.5, 178.5};
    LMI_TEST_EQUAL(v3[0], v2[0]);
    LMI_TEST_EQUAL(v3[1], v2[1]);
    LMI_TEST_EQUAL(v3[2], v2[2]);
    LMI_TEST_EQUAL(v3[3], v2[3]);
    LMI_TEST_EQUAL(v3[4], v2[4]);
    LMI_TEST_EQUAL(v3[5], v2[5]);
    LMI_TEST_EQUAL(6, v0.size());
}

void test_outward_quotient()
{
    LMI_TEST_EQUAL( 1, outward_quotient( 2,  2));
    LMI_TEST_EQUAL( 1, outward_quotient( 1,  2));
    LMI_TEST_EQUAL( 0, outward_quotient( 0,  2));
    LMI_TEST_EQUAL(-1, outward_quotient(-1,  2));
    LMI_TEST_EQUAL(-1, outward_quotient(-2,  2));

    LMI_TEST_EQUAL(-1, outward_quotient( 2, -2));
    LMI_TEST_EQUAL(-1, outward_quotient( 1, -2));
    LMI_TEST_EQUAL( 0, outward_quotient( 0, -2));
    LMI_TEST_EQUAL( 1, outward_quotient(-1, -2));
    LMI_TEST_EQUAL( 1, outward_quotient(-2, -2));

    LMI_TEST_EQUAL( 0ULL, outward_quotient( 0ULL,  2ULL));
    LMI_TEST_EQUAL( 1ULL, outward_quotient( 1ULL,  2ULL));
    LMI_TEST_EQUAL( 1ULL, outward_quotient( 2ULL,  2ULL));

    LMI_TEST_EQUAL( 0, outward_quotient( 0,  3));
    LMI_TEST_EQUAL( 1, outward_quotient( 1,  3));
    LMI_TEST_EQUAL( 1, outward_quotient( 2,  3));
    LMI_TEST_EQUAL( 1, outward_quotient( 3,  3));
    LMI_TEST_EQUAL( 2, outward_quotient( 4,  3));
    LMI_TEST_EQUAL( 2, outward_quotient( 5,  3));
    LMI_TEST_EQUAL( 2, outward_quotient( 6,  3));
    LMI_TEST_EQUAL( 3, outward_quotient( 7,  3));

    LMI_TEST_EQUAL(INT_MIN, outward_quotient(INT_MIN,       1));
    LMI_TEST_EQUAL(      1, outward_quotient(INT_MIN, INT_MIN));
    LMI_TEST_EQUAL(     -1, outward_quotient(      1, INT_MIN));

    LMI_TEST_EQUAL(INT_MAX, outward_quotient(INT_MAX,       1));
    LMI_TEST_EQUAL(      1, outward_quotient(INT_MAX, INT_MAX));
    LMI_TEST_EQUAL(      1, outward_quotient(      1, INT_MAX));

    LMI_TEST_EQUAL(UINT_MAX, outward_quotient(UINT_MAX,       1u));
    LMI_TEST_EQUAL(      1u, outward_quotient(UINT_MAX, UINT_MAX));
    LMI_TEST_EQUAL(      1u, outward_quotient(      1u, UINT_MAX));

    // The language allows "false/true"; this is no sillier.
    LMI_TEST_EQUAL(false, outward_quotient(false, true));

    LMI_TEST_THROW
        (outward_quotient(1, 0)
        ,std::domain_error
        ,"Denominator is zero."
        );

    LMI_TEST_THROW
        (outward_quotient(INT_MIN, -1)
        ,std::domain_error
        ,"Division might overflow."
        );

// Appropriately fails to compile due to conflicting types:
//  outward_quotient( 1, 1u);

// Appropriately fails to compile due to static assertion:
//  outward_quotient(1.0, 1.0);
}

void test_compound_interest()
{
    // Test with 1 == 'n'.

    LMI_TEST
        (materially_equal
            (0.04
            ,i_upper_n_over_n_from_i<double,1>()(0.04)
            )
        );

    LMI_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_n_over_n<double,1>()(0.04)
            )
        );

    LMI_TEST
        (materially_equal
            (0.04 / 1.04
            ,d_upper_n_from_i<double,1>()(0.04)
            )
        );

    // Test interest rate of -100%.

    LMI_TEST_EQUAL(-1.0 , i_upper_12_over_12_from_i_naive<double>()(-1.0));
    LMI_TEST_EQUAL(-1.0 , i_upper_12_over_12_from_i      <double>()(-1.0));
    LMI_TEST_THROW
        (d_upper_12_from_i        <double>()(-1.0)
        ,std::range_error
        ,"i equals -100%."
        );

    // Test nonsensical interest rate of -101%.

    LMI_TEST(std::isnan(i_upper_12_over_12_from_i_naive<double>()(-1.01)));
    LMI_TEST_THROW
        (i_upper_12_over_12_from_i<double>()(-1.01)
        ,std::domain_error
        ,"i is less than -100%."
        );
    LMI_TEST_THROW
        (d_upper_12_from_i        <double>()(-1.01)
        ,std::domain_error
        ,"i is less than -100%."
        );

    // Gross should equal net if decrements are both zero.

    LMI_TEST
        (materially_equal
            (0.04
            ,net_i_from_gross_naive<double,365>()(0.04, 0.0, 0.0)
            )
        );

    LMI_TEST
        (materially_equal
            (0.04
            ,net_i_from_gross<double,365>()(0.04, 0.0, 0.0)
            )
        );

    // Test exponential and power methods for approximate accuracy.

    LMI_TEST
        (materially_equal
            (0.0032737397821988637 // Correctly rounded.
            ,i_upper_12_over_12_from_i_naive<double>()(0.04)
            )
        );
    LMI_TEST
        (materially_equal
            (0.0032737397821988637 // Correctly rounded.
            ,i_upper_12_over_12_from_i<double>()(0.04)
            )
        );

    LMI_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_12_over_12_naive<double>()(0.0032737397821988637)
            )
        );
    LMI_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_12_over_12<double>()(0.0032737397821988637)
            )
        );

    LMI_TEST
        (materially_equal
            (0.039156688577251846
            ,d_upper_12_from_i_naive<double>()(0.04)
            )
        );
    LMI_TEST
        (materially_equal
            (0.039156688577251846
            ,d_upper_12_from_i<double>()(0.04)
            )
        );

    // For small interest rates and frequent compounding, the
    // exponential method is more accurate than the naive power
    // method. In this example, the former has a relative error
    // on the order of 1e-15; the latter, only 1e-13 .

    LMI_TEST
        (materially_equal
            (0.001
            ,net_i_from_gross_naive<double,365>()(0.001, 0.0, 0.0)
            ,1.0e-13
            )
        );
    LMI_TEST
        (!materially_equal
            (0.001
            ,net_i_from_gross_naive<double,365>()(0.001, 0.0, 0.0)
            ,1.0e-14
            )
        );

    LMI_TEST
        (materially_equal
            (0.001
            ,net_i_from_gross<double,365>()(0.001, 0.0, 0.0)
            ,1.0e-15
            )
        );
}

void test_relative_error()
{
    constexpr double inf {std::numeric_limits<double>::infinity()};
    constexpr double big {std::numeric_limits<double>::max()};

    LMI_TEST_EQUAL(inf, relative_error(0.0, -2.0));
    LMI_TEST_EQUAL(inf, relative_error(0.0, -1.0));
    LMI_TEST_EQUAL(inf, relative_error(0.0, -0.5));
    LMI_TEST_EQUAL(0.0, relative_error(0.0,  0.0));
    LMI_TEST_EQUAL(inf, relative_error(0.0,  0.5));
    LMI_TEST_EQUAL(inf, relative_error(0.0,  1.0));
    LMI_TEST_EQUAL(inf, relative_error(0.0,  2.0));
    LMI_TEST_EQUAL(0.0, relative_error(1.0,  1.0));
    LMI_TEST_EQUAL(2.0, relative_error(1.0, -1.0));
    LMI_TEST_EQUAL(big, relative_error(1.0,  big));
    LMI_TEST_EQUAL(inf, relative_error(big, -big));

    LMI_TEST_EQUAL(2.0F, relative_error(1.0F, -1.0F));
    LMI_TEST_EQUAL(2.0L, relative_error(1.0L, -1.0L));
}

void test_signed_zero()
{
    constexpr double inf  {std::numeric_limits<double>::infinity ()};
    constexpr double qnan {std::numeric_limits<double>::quiet_NaN()};

    LMI_TEST(pos0 == pos0);
    LMI_TEST(pos0 != neg0);
    LMI_TEST(neg0 != pos0);
    LMI_TEST(neg0 == neg0);

    LMI_TEST(pos0 ==  0.0);
    LMI_TEST( 0.0 == pos0);
    LMI_TEST(pos0 != -0.0);
    LMI_TEST(-0.0 != pos0);

    LMI_TEST(neg0 !=  0.0);
    LMI_TEST( 0.0 != neg0);
    LMI_TEST(neg0 == -0.0);
    LMI_TEST(-0.0 == neg0);

    LMI_TEST(pos0 != qnan);
    LMI_TEST(qnan != pos0);
    LMI_TEST(neg0 != qnan);
    LMI_TEST(qnan != neg0);

    LMI_TEST(pos0 !=  inf);
    LMI_TEST( inf != pos0);
    LMI_TEST(pos0 != -inf);
    LMI_TEST(-inf != pos0);

    LMI_TEST(neg0 !=  inf);
    LMI_TEST( inf != neg0);
    LMI_TEST(neg0 != -inf);
    LMI_TEST(-inf != neg0);

    LMI_TEST(pos0 !=  1.0);
    LMI_TEST(neg0 !=  1.0);

    // These intentionally do not compile:
//  LMI_TEST(pos0 == 0);
//  LMI_TEST(pos0 == '0');
//  LMI_TEST(pos0 == nullptr);
}

template<typename T>
void test_signum(char const* file, int line)
{
    T const maxT = std::numeric_limits<T>::max();
    T const minT = std::numeric_limits<T>::lowest();

    INVOKE_LMI_TEST_EQUAL( 0, signum(T( 0)), file, line);
    INVOKE_LMI_TEST_EQUAL( 1, signum(T( 1)), file, line);

    INVOKE_LMI_TEST_EQUAL( 1, signum(maxT), file, line);

    if(minT < 0)
        {
        // The left-hand side is cast to T to avoid gcc 'bool-compare'
        // diagnostics. An 'is_bool' conditional wouldn't prevent the
        // macros from being expanded. See:
        //   https://lists.nongnu.org/archive/html/lmi/2017-05/msg00029.html
        INVOKE_LMI_TEST_EQUAL(T(-1), signum(T(-1)), file, line);
        INVOKE_LMI_TEST_EQUAL(T(-1), signum(minT ), file, line);
        }

    bool volatile is_iec559 = std::numeric_limits<T>::is_iec559;
    bool volatile has_infinity = std::numeric_limits<T>::has_infinity;
    if(is_iec559 && has_infinity)
        {
        T const infT = std::numeric_limits<T>::infinity();
        INVOKE_LMI_TEST_EQUAL(-1, signum(-infT), file, line);
        INVOKE_LMI_TEST_EQUAL( 1, signum( infT), file, line);
        }

    bool volatile has_quiet_NaN = std::numeric_limits<T>::has_quiet_NaN;
    if(is_iec559 && has_quiet_NaN)
        {
        T const qnanT = std::numeric_limits<T>::quiet_NaN();
        INVOKE_LMI_TEST_EQUAL(-1, signum(-qnanT), file, line);
        INVOKE_LMI_TEST_EQUAL( 1, signum( qnanT), file, line);
        }
}

void test_u_abs()
{
    LMI_TEST_EQUAL(0x8000000000000000, u_abs(INT64_MIN));

    constexpr auto int8_min {std::numeric_limits<std::int8_t>::min()};
    constexpr auto int8_max {std::numeric_limits<std::int8_t>::max()};

    std::uint8_t additive_inverse_of_int8_min {u_abs(int8_min)};
    LMI_TEST_EQUAL(128U, additive_inverse_of_int8_min);

    static_assert(8 == CHAR_BIT);

    LMI_TEST_EQUAL  (1   , sizeof       int8_min );
    LMI_TEST_EQUAL  (1   , sizeof u_abs(int8_min));
    LMI_TEST_EQUAL  (128U,        u_abs(int8_min));

    // Incidentally, INT8_MIN is not of type std::int8_t, because it
    // is converted according to the integer promotions.
    LMI_TEST_UNEQUAL(1   , sizeof       INT8_MIN );
    LMI_TEST_UNEQUAL(1   , sizeof u_abs(INT8_MIN));
    LMI_TEST_EQUAL  (128U,        u_abs(INT8_MIN));

    // Test all 256 possibilities.
    for(std::int16_t j = int8_min; j <= int8_max; ++j)
        {
        std::uint16_t u = u_abs(j);
        if(0 <= j)
            {
            LMI_TEST_EQUAL(u,  j);
            }
        if(j <= 0)
            {
            LMI_TEST_EQUAL(u, -j);
            LMI_TEST_EQUAL(0, u + j);
            LMI_TEST_EQUAL(0, j + u);
            }
        }
}

/// Test fdlibm expm1() and log1p().
///
/// Testing for exact floating-point equality seems to be a patent
/// mistake, but should work for the fdlibm implementations on
/// any implementation where double is binary64.

void test_expm1_log1p()
{
    std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    std::cout.precision(23);

    // Test several known correctly rounded values. See:
    //   https://lists.nongnu.org/archive/html/lmi/2022-05/msg00030.html
    // which gives the more precise values
    //   1.7456010150169164939897763166603876240737508195959622916673980879...
    //   1.7456010150169166 = 3FFBEDFB5475CB01 correctly rounded binary64
    //  [1.7456010150169163 = 3FFBEDFB5475CB00 lower neighbor--rejected]
    // for e^1.01 - 1 . Similarly, using
    //   https://www.wolframalpha.com/input?i2d=true&i=ln\(40)1.01\(41)
    //   https://babbage.cs.qc.cuny.edu/IEEE-754.old/Decimal.html
    // ln(1 + 0.01) is
    //   0.0099503308531680828482153575442607416886796099400587978646095597...
    //   0.009950330853168083 = 3F8460D6CCCA3677
    // and
    //   https://www.wolframalpha.com/input?i=exp(ln(1.04)/12)-1
    // exp(ln(1 + .04) / 12) - 1 is
    //   0.0032737397821988638592943204158789680534098426263396651605608434...
    //   0.0032737397821988637 = 3F6AD187A99AE58B

    double const g0 = lmi::expm1(1.01);
    double const g1 = lmi::log1p(0.01);
    double const g2 = lmi::expm1(lmi::log1p(0.04) / 12);

    // digits      1 2345678901234567
    LMI_TEST_EQUAL(1.7456010150169166, g0);
    // digits          1234567890123456
    LMI_TEST_EQUAL(0.009950330853168083, g1);
    // digits          12345678901234567
    LMI_TEST_EQUAL(0.0032737397821988637, g2);

    // For sampled (integer/1000000.0) arguments in the open range
    //   ]-0.043348, +0.042151[
    // lmi's fdlibm implementation of expm1() and log1p() matches glibc's
    // except for this single example:
    double const h0 {25610 / 1000000.0};
    double const h1 = lmi::expm1(h0); // 0.02594075354662067622868
    double const h2 = std::expm1(h0); // 0.02594075354662067275924
//  LMI_TEST_EQUAL(h1, h2); fails
    LMI_TEST(materially_equal(h1, h2));
    // and in that example glibc is correct:
    //   https://www.wolframalpha.com/input?i=exp(25610/1000000)-1
    // 0.0259407535466206736037231992174016233440736931692437771090797988...
    //   https://babbage.cs.qc.cuny.edu/IEEE-754.old/Decimal.html
    // 0.025940753546620673 = 3F9A903680771FAF  correctly rounded
    // 0.025940753546620676 = 3F9A903680771FB0  fdlibm
    // 0.025940753546620673 = 3F9A903680771FAF  glibc

    // Monthly equivalent of a -0.999999 = -99.9999% interest rate.
    // The transformation is, generally:
    //   (1+i)^n - 1 <-> expm1(log1p(i) * n)
    // Substituting i = -0.999999 and n = 1/12 :
    //   (1-0.999999)^(1/12) - 1 <-> expm1(log1p(-0.999999) / 12.0)
    // High-precision values:
    //   https://www.wolframalpha.com/input?i=log1p(-0.999999)/12
    //   -1.151292546497022842008995727342182103800550744314386488016
    //   https://www.wolframalpha.com/input?i=expm1(log1p(-0.999999)/12)
    //   -0.683772233983162066800110645556728146628044486067478317314
    //   https://www.wolframalpha.com/input?i=(1-0.999999)^(1/12)-1
    //   -0.683772233983162066800110645556728146628044486067478317314
    // In this ill-conditioned case, we get something like eleven
    // digits of precision--an error of about one million ulp.
    double const i0 = std::log1p(-0.999999) / 12.0;
    LMI_TEST(materially_equal( -1.1512925464970228, i0, 1.0e-11));
    double const i1 = std::expm1(-1.1512925464970228);
    LMI_TEST(materially_equal(-0.68377223398240425, i1, 1.0e-11));
    // (Optionally, to see the platform-dependent actual values:
    // i0 = -1.15129254649462642312585 i1 = -0.68377223398316200331237 MinGW-w32
    // i0 = -1.15129254649462642312585 i1 = -0.68377223398316200331237 glibc
    // [which are curiously identical], uncomment the next line.)
//  std::cout << "i0 = " << i0 << " i1 = " << i1 << std::endl;
    // Worse, we have UB--which UBSan detects when we build fdlibm
    // ourselves, though not when we use the same code via glibc:
    double const i2 = lmi::log1p(-0.999999) / 12.0;
    LMI_TEST(materially_equal( -1.1512925464970228, i2, 1.0e-11));
    // fdlibm_expm1.c:242:13: runtime error: left shift of negative value -2
    double const i3 = lmi::expm1(-1.1512925464970228);
    LMI_TEST(materially_equal(-0.68377223398240425, i3, 1.0e-11));

    // Test fdlibm vs. C RTL for many parameters.
    int    err_count0 {0};
    int    err_count1 {0};
    int    err_count2 {0};
    int    err_count3 {0};
    double err_max0   {0.0};
    double err_max1   {0.0};
    double err_max2   {0.0};
    double err_max3   {0.0};
    std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    std::cout.precision(23);
    int const how_many = 1000000;
    for(int i = 1 - how_many; i < how_many; ++i)
        {
        // interest rate
        double const irate = i / (1.0 * how_many);
        // fdlibm
        double const a0 = lmi::expm1(irate);
        double const a1 = lmi::log1p(irate);
        double const a2 = lmi::expm1(lmi::log1p(irate) / 12);
        double const a3 = lmi::expm1(lmi::log1p(irate) / 365);
        // RTL
        double const b0 = std::expm1(irate);
        double const b1 = std::log1p(irate);
        double const b2 = std::expm1(std::log1p(irate) / 12);
        double const b3 = std::expm1(std::log1p(irate) / 365);
        // relative error
        double const e0 = relative_error(a0, b0);
        double const e1 = relative_error(a1, b1);
        double const e2 = relative_error(a2, b2);
        double const e3 = relative_error(a3, b3);
        // comparison
        if(a0 != b0 || a1 != b1 || a2 != b2 || a3 != b3)
            {
            err_count0 += a0 != b0;
            err_count1 += a1 != b1;
            err_count2 += a2 != b2;
            err_count3 += a3 != b3;
            err_max0 = std::max(err_max0, e0);
            err_max1 = std::max(err_max1, e1);
            err_max2 = std::max(err_max2, e2);
            err_max3 = std::max(err_max3, e3);
// Enable this to show each one:
#if 0
            std::cout << "unequal" << std::endl;
            std::cout
                << i << ' ' << irate << ' ' << how_many << '\n'
                << a0 << ' ' << a1 << ' ' << a2 << ' ' << a3 << '\n'
                << b0 << ' ' << b1 << ' ' << b2 << ' ' << b3 << '\n'
                << e0 << ' ' << e1 << ' ' << e2 << ' ' << e3 << '\n'
                << std::endl
                ;
#endif // 0
            }
        }
    std::cout
        << __func__ << ": " << how_many << " trials\n"
        << "  " << err_count0 << " errors"
                << " [worst " << err_max0 / DBL_EPSILON  << " ulp]"
                << " in expm1()\n"
        << "  " << err_count1 << " errors"
                << " [worst " << err_max1 / DBL_EPSILON  << " ulp]"
                << " in log1p()\n"
        << "  " << err_count2 << " errors"
                << " [worst " << err_max2 / DBL_EPSILON  << " ulp]"
                << " in monthly int\n"
        << "  " << err_count3 << " errors"
                << " [worst " << err_max3 / DBL_EPSILON  << " ulp]"
                << " in daily int\n"
        << std::endl
        ;
}

/// This function isn't a unit test per se. Its purpose is to show
/// how a sample calculation is affected by
///   exponential versus power method,
///   floating-point type (double vs. long double), and
///   hardware precision (on supported platforms).
///
/// All methods and precisions are tested with the same constant input
/// interest rate, which is declared as 'double', as though it were
/// read as such from a data file containing the given string-literal.
/// The intention here is to use exactly the same value in all cases;
/// using a long double string literal for long double scenarios would
/// introduce a confounder.

void sample_results()
{
    constexpr double intrate {0.04};
    fenv_initialize();
    std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    std::cout.precision(23);
    std::cout
        << "Monthly rate corresponding to 1% annual interest"
        << ", by various methods:\n"
        << "      000000000111111111122\n"
        << "      123456789012345678901\n"
        << "  " << i_upper_n_over_n_from_i      <long double,12>()(intrate)
        << "  long double prec, expm1 and log1p (production)\n"
        ;
#if defined LMI_X87
    fenv_precision(fe_ldblprec);
#endif // defined LMI_X87
    std::cout
        << "  " << i_upper_n_over_n_from_i_naive<long double,12>()(intrate)
        << "  long double prec, pow\n"
        ;
#if defined LMI_X87
    fenv_initialize();
    fenv_precision(fe_dblprec);
#endif // defined LMI_X87
    std::cout
        << "  0.0032737397821988637 (correctly rounded binary64)\n"
        << "  " << i_upper_n_over_n_from_i      <double,12>()(intrate)
        << "  double prec, expm1 and log1p (production)\n"
        << "  " << i_upper_n_over_n_from_i_naive<double,12>()(intrate)
        << "  double prec, pow\n"
        << std::endl;
        ;

    fenv_initialize();
}

void assay_speed()
{
    std::cout << "Speed tests:\n";
    std::cout << "  pow              " << TimeAnAliquot(mete0) << '\n';
    std::cout << "  expm1 and log1p  " << TimeAnAliquot(mete1) << '\n';
    std::cout << "  double      i365 " << TimeAnAliquot(mete2) << '\n';
    std::cout << "  long double i365 " << TimeAnAliquot(mete3) << '\n';
    std::cout << "  10^-9 nonstd     " << TimeAnAliquot(mete4) << '\n';
    std::cout << "  10^-9 std        " << TimeAnAliquot(mete5) << '\n';
    std::cout << "  lmi::expm1()     " << TimeAnAliquot(mete6) << '\n';
    std::cout << "  std::expm1()     " << TimeAnAliquot(mete7) << '\n';
    std::cout << "  lmi::log1p()     " << TimeAnAliquot(mete8) << '\n';
    std::cout << "  std::log1p()     " << TimeAnAliquot(mete9) << '\n';
    std::cout << std::flush;
}

int test_main(int, char*[])
{
    std::cout << LMI_CONTEXT << '\n' << std::endl;

    test_assign_midpoint();

    test_outward_quotient();

    test_compound_interest();

    test_relative_error();

    test_signed_zero();

    test_signum<bool         >(__FILE__, __LINE__);
    test_signum<signed char  >(__FILE__, __LINE__);
    test_signum<unsigned char>(__FILE__, __LINE__);
    test_signum<int          >(__FILE__, __LINE__);
    test_signum<float        >(__FILE__, __LINE__);
    test_signum<double       >(__FILE__, __LINE__);
    test_signum<long double  >(__FILE__, __LINE__);

    test_u_abs();

    test_expm1_log1p();

    sample_results();

    assay_speed();

    return 0;
}
