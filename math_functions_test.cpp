// Miscellaneous mathematical operations as function objects--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
#include "miscellany.hpp"               // stifle_warning_for_unused_value()
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

// These 'meteN' functions perform the same set of operations using
// different implementations.

// This implementation naively uses std::pow(); it is both slower and
// less inaccurate than an alternative using std::expm1() and std::log1p().
void mete0()
{
    double volatile x;
    stifle_warning_for_unused_value(x);
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_12_over_12_from_i_naive<double>()(0.04);
        x = i_from_i_upper_12_over_12_naive<double>()(0.04);
        x = d_upper_12_from_i_naive        <double>()(0.04);
        x = net_i_from_gross_naive<double,365>()(0.04, 0.007, 0.003);
        }
}

// This implementation uses production functors.
void mete1()
{
    double volatile x;
    stifle_warning_for_unused_value(x);
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_12_over_12_from_i<double>()(0.04);
        x = i_from_i_upper_12_over_12<double>()(0.04);
        x = d_upper_12_from_i        <double>()(0.04);
        x = net_i_from_gross<double,365>()(0.04, 0.007, 0.003);
        }
}

void mete2()
{
    double volatile x;
    stifle_warning_for_unused_value(x);
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_n_over_n_from_i_T<double,365>()(0.01);
        }
}

void mete3()
{
    long double volatile x;
    stifle_warning_for_unused_value(x);
    for(int j = 0; j < 100000; ++j)
        {
        x = i_upper_n_over_n_from_i_T<long double,365>()(0.01);
        }
}

void assay_speed()
{
    std::cout << "Speed tests:\n";
    std::cout << "  std::pow         " << TimeAnAliquot(mete0) << '\n';
    std::cout << "  std::expm1       " << TimeAnAliquot(mete1) << '\n';
    std::cout << "  double      i365 " << TimeAnAliquot(mete2) << '\n';
    std::cout << "  long double i365 " << TimeAnAliquot(mete3) << '\n';
}

int test_main(int, char*[])
{
    double      smallnumD = std::numeric_limits<double     >::min();
    double      bignumD   = std::numeric_limits<double     >::max();

    long double smallnumL = std::numeric_limits<long double>::min();
    long double bignumL   = std::numeric_limits<long double>::max();

    BOOST_TEST_EQUAL(2.0, greater_of<double>()(1.0, 2.0));
    BOOST_TEST_EQUAL(1.0, lesser_of <double>()(1.0, 2.0));

    // Test mean<>().

    BOOST_TEST_EQUAL(1.5, mean<double>()(1.0, 2.0));
    BOOST_TEST_EQUAL(smallnumD, mean<double>()(smallnumD, smallnumD));
    BOOST_TEST_EQUAL(bignumD  , mean<double>()(bignumD  , bignumD  ));

    BOOST_TEST_EQUAL(1.5, mean<long double>()(1.0, 2.0));
    BOOST_TEST_EQUAL(smallnumL, mean<long double>()(smallnumL, smallnumL));
    BOOST_TEST_EQUAL(bignumL  , mean<long double>()(bignumL  , bignumL  ));

    // Test outward_quotient().

    BOOST_TEST_EQUAL( 1, outward_quotient( 2,  2));
    BOOST_TEST_EQUAL( 1, outward_quotient( 1,  2));
    BOOST_TEST_EQUAL( 0, outward_quotient( 0,  2));
    BOOST_TEST_EQUAL(-1, outward_quotient(-1,  2));
    BOOST_TEST_EQUAL(-1, outward_quotient(-2,  2));

    BOOST_TEST_EQUAL(-1, outward_quotient( 2, -2));
    BOOST_TEST_EQUAL(-1, outward_quotient( 1, -2));
    BOOST_TEST_EQUAL( 0, outward_quotient( 0, -2));
    BOOST_TEST_EQUAL( 1, outward_quotient(-1, -2));
    BOOST_TEST_EQUAL( 1, outward_quotient(-2, -2));

    BOOST_TEST_EQUAL( 0ULL, outward_quotient( 0ULL,  2ULL));
    BOOST_TEST_EQUAL( 1ULL, outward_quotient( 1ULL,  2ULL));
    BOOST_TEST_EQUAL( 1ULL, outward_quotient( 2ULL,  2ULL));

    BOOST_TEST_EQUAL( 0, outward_quotient( 0,  3));
    BOOST_TEST_EQUAL( 1, outward_quotient( 1,  3));
    BOOST_TEST_EQUAL( 1, outward_quotient( 2,  3));
    BOOST_TEST_EQUAL( 1, outward_quotient( 3,  3));
    BOOST_TEST_EQUAL( 2, outward_quotient( 4,  3));
    BOOST_TEST_EQUAL( 2, outward_quotient( 5,  3));
    BOOST_TEST_EQUAL( 2, outward_quotient( 6,  3));
    BOOST_TEST_EQUAL( 3, outward_quotient( 7,  3));

    BOOST_TEST_EQUAL(INT_MIN, outward_quotient(INT_MIN,       1));
    BOOST_TEST_EQUAL(      1, outward_quotient(INT_MIN, INT_MIN));
    BOOST_TEST_EQUAL(     -1, outward_quotient(      1, INT_MIN));

    BOOST_TEST_EQUAL(INT_MAX, outward_quotient(INT_MAX,       1));
    BOOST_TEST_EQUAL(      1, outward_quotient(INT_MAX, INT_MAX));
    BOOST_TEST_EQUAL(      1, outward_quotient(      1, INT_MAX));

    BOOST_TEST_EQUAL(UINT_MAX, outward_quotient(UINT_MAX,       1u));
    BOOST_TEST_EQUAL(      1u, outward_quotient(UINT_MAX, UINT_MAX));
    BOOST_TEST_EQUAL(      1u, outward_quotient(      1u, UINT_MAX));

    // The language allows "false/true"; this is no sillier.
    BOOST_TEST_EQUAL(false, outward_quotient(false, true));

    BOOST_TEST_THROW
        (outward_quotient(1, 0)
        ,std::domain_error
        ,"Denominator is zero."
        );

    BOOST_TEST_THROW
        (outward_quotient(INT_MIN, -1)
        ,std::domain_error
        ,"Division might overflow."
        );

// Appropriately fails to compile due to conflicting types:
//  outward_quotient( 1, 1u);

// Appropriately fails to compile due to static assertion:
//  outward_quotient(1.0, 1.0);

    // Actuarial functions.

    // Test with 1 == 'n'.

    BOOST_TEST
        (materially_equal
            (0.04
            ,i_upper_n_over_n_from_i<double,1>()(0.04)
            )
        );

    BOOST_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_n_over_n<double,1>()(0.04)
            )
        );

    BOOST_TEST
        (materially_equal
            (0.04 / 1.04
            ,d_upper_n_from_i<double,1>()(0.04)
            )
        );

    // Test interest rate of -100%.

    BOOST_TEST_EQUAL(-1.0 , i_upper_12_over_12_from_i_naive<double>()(-1.0));
    BOOST_TEST_EQUAL(-1.0 , i_upper_12_over_12_from_i      <double>()(-1.0));
    BOOST_TEST_THROW
        (d_upper_12_from_i        <double>()(-1.0)
        ,std::range_error
        ,"i equals -100%."
        );

    // Test nonsensical interest rate of -101%.

    BOOST_TEST(std::isnan(i_upper_12_over_12_from_i_naive<double>()(-1.01)));
    BOOST_TEST_THROW
        (i_upper_12_over_12_from_i<double>()(-1.01)
        ,std::domain_error
        ,"i is less than -100%."
        );
    BOOST_TEST_THROW
        (d_upper_12_from_i        <double>()(-1.01)
        ,std::domain_error
        ,"i is less than -100%."
        );

    // Gross should equal net if decrements are both zero.

    BOOST_TEST
        (materially_equal
            (0.04
            ,net_i_from_gross_naive<double,365>()(0.04, 0.0, 0.0)
            )
        );

    BOOST_TEST
        (materially_equal
            (0.04
            ,net_i_from_gross<double,365>()(0.04, 0.0, 0.0)
            )
        );

    // Test exponential and power methods for approximate accuracy.

    BOOST_TEST
        (materially_equal
            (0.0032737397821988637 // Not very accurate.
            ,i_upper_12_over_12_from_i_naive<double>()(0.04)
            )
        );
    BOOST_TEST
        (materially_equal
            (0.0032737397821988642
            ,i_upper_12_over_12_from_i<double>()(0.04)
            )
        );

    BOOST_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_12_over_12_naive<double>()(0.0032737397821988642)
            )
        );
    BOOST_TEST
        (materially_equal
            (0.04
            ,i_from_i_upper_12_over_12<double>()(0.0032737397821988642)
            )
        );

    BOOST_TEST
        (materially_equal
            (0.039156688577251846
            ,d_upper_12_from_i_naive<double>()(0.04)
            )
        );
    BOOST_TEST
        (materially_equal
            (0.039156688577251846
            ,d_upper_12_from_i<double>()(0.04)
            )
        );

    // For small interest rates and frequent compounding, the
    // exponential method is more accurate than the naive power
    // method. In this example, the former has a relative error
    // on the order of 1e-15; the latter, only 1e-13 .

    BOOST_TEST
        (materially_equal
            (0.001
            ,net_i_from_gross_naive<double,365>()(0.001, 0.0, 0.0)
            ,1.0e-13
            )
        );
    BOOST_TEST
        (!materially_equal
            (0.001
            ,net_i_from_gross_naive<double,365>()(0.001, 0.0, 0.0)
            ,1.0e-14
            )
        );

    BOOST_TEST
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
