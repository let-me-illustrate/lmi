// Miscellaneous mathematical operations as function objects--unit test.
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

#include "fenv_lmi.hpp"
#include "materially_equal.hpp"
#include "miscellany.hpp"               // stifle_unused_warning()
#include "stl_extensions.hpp"           // nonstd::power()
#include "test_tools.hpp"
#include "timer.hpp"

#include <algorithm>                    // min()
#include <cmath>                        // isnan(), pow()
#include <iomanip>
#include <limits>
#include <type_traits>

// Some of these tests may raise hardware exceptions. That means that
// edge cases are tested, not that the code tested is invalid for
// arguments that aren't ill conditioned.

namespace
{
// These naive implementations in terms of std::pow() are slower and
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

// This implementation uses std::expm1() and std::log1p() for type T,
// rather than the long double functions used in production.

template<typename T, int n>
struct i_upper_n_over_n_from_i_T
{
    static_assert(std::is_floating_point_v<T>);
    T operator()(T const& i) const
        {
        static T const reciprocal_n = T(1) / n;
        return std::expm1(std::log1p(i) * reciprocal_n);
        }
};
} // Unnamed namespace.

/// This function isn't a unit test per se. Its purpose is to show
/// how a sample calculation is affected by
///   exponential versus power method, and
///   hardware precision (on supported platforms).
///
/// Define f(i,m) = (1+i)^(1/m) - 1, i.e., "i upper m over m".
/// Let i0 = 0.0004 (forty bp).
/// Let i1 = f(i0, 365)
/// Then the number calculated here is f(-i1, 1/365).

void sample_results()
{
    fenv_initialize();
    std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    std::cout.precision(25);
    std::cout
        << "\nDaily rate corresponding to 1% annual interest"
        << ", by various methods:\n"
        << "        000000000111111111122\n"
        << "        123456789012345678901\n"
        << "  " << i_upper_n_over_n_from_i      <long double,365>()(0.01)
        << "  method in production\n"
        ;
#if defined LMI_X87
    fenv_precision(fe_ldblprec);
    std::cout
        << "  " << i_upper_n_over_n_from_i_T    <long double,365>()(0.01)
        << "  long double precision, std::expm1 and std::log1p\n"
        << "  " << i_upper_n_over_n_from_i_naive<long double,365>()(0.01)
        << "  long double precision, std::pow\n"
        ;

    fenv_initialize();
    fenv_precision(fe_dblprec);
#endif // defined LMI_X87
    std::cout
        << "  " << i_upper_n_over_n_from_i_T    <double,365>()(0.01)
        << "  double precision, std::expm1 and std::log1p\n"
        << "  " << i_upper_n_over_n_from_i_naive<double,365>()(0.01)
        << "  double precision, std::pow\n"
        ;

    fenv_initialize();
}

// These 'mete[0123]' functions perform the same set of operations using
// different implementations.

// This implementation naively uses std::pow(); it is both slower and
// less inaccurate than an alternative using std::expm1() and std::log1p().
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

void mete2()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_n_over_n_from_i_T<double,365>()(0.01);
        }
    stifle_unused_warning(x);
}

void mete3()
{
    long double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_n_over_n_from_i_T<long double,365>()(0.01);
        }
    stifle_unused_warning(x);
}

// These 'mete[45]' functions calculate 10^-9 in different ways.
// The SGI extension is about eight times as fast as calling a
// transcendental function; that outcome is not surprising, but
// quantifying it is useful. Of course, it would not be surprising
// to find that a table lookup would be even faster for "reasonable"
// powers of ten.

void mete4()
{
    double volatile base     = 10.0;
    int    volatile exponent = 9;
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        {
        x = 1.0 / nonstd::power(base, exponent);
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

void assay_speed()
{
    std::cout << "Speed tests:\n";
    std::cout << "  std::pow         " << TimeAnAliquot(mete0) << '\n';
    std::cout << "  std::expm1       " << TimeAnAliquot(mete1) << '\n';
    std::cout << "  double      i365 " << TimeAnAliquot(mete2) << '\n';
    std::cout << "  long double i365 " << TimeAnAliquot(mete3) << '\n';
    std::cout << "  10^-9 nonstd     " << TimeAnAliquot(mete4) << '\n';
    std::cout << "  10^-9 std        " << TimeAnAliquot(mete5) << '\n';
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

int test_main(int, char*[])
{
    // Test assign_midpoint().

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

    // Test outward_quotient().

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

    test_signum<bool         >(__FILE__, __LINE__);
    test_signum<signed char  >(__FILE__, __LINE__);
    test_signum<unsigned char>(__FILE__, __LINE__);
    test_signum<int          >(__FILE__, __LINE__);
    test_signum<float        >(__FILE__, __LINE__);
    test_signum<double       >(__FILE__, __LINE__);
    test_signum<long double  >(__FILE__, __LINE__);

    // Actuarial functions.

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
            (0.0032737397821988637 // Not very accurate.
            ,i_upper_12_over_12_from_i_naive<double>()(0.04)
            )
        );
    LMI_TEST
        (materially_equal
            (0.0032737397821988642
            ,i_upper_12_over_12_from_i<double>()(0.04)
            )
        );

    LMI_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_12_over_12_naive<double>()(0.0032737397821988642)
            )
        );
    LMI_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_12_over_12<double>()(0.0032737397821988642)
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

    assay_speed();

    sample_results();

    return 0;
}
