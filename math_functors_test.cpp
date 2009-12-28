// Miscellaneous mathematical operations as function objects--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: math_functors_test.cpp,v 1.14 2009-04-05 10:34:24 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "math_functors.hpp"

#include "fenv_lmi.hpp"
#include "materially_equal.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <algorithm> // std::min()
#include <cmath>     // std::pow()
#include <functional>
#include <iomanip>
#include <limits>

// Some of these tests may raise hardware exceptions. That means that
// edge cases are tested, not that the code tested is invalid for
// arguments that aren't ill conditioned.

namespace
{
/// C99's isnan macro can't be written correctly in portable C++.
/// This implementation is portable, but compilers needn't implement
/// it correctly.

template<typename T>
bool lmi_isnan(T t)
{
    volatile T t0(t);
    return t0 != t0;
}

// These naive implementations in terms of std::pow() are slower and
// less accurate than those in the header tested here.

template<typename T>
struct i_upper_12_over_12_from_i_naive
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {
        long double z = -1.0L + std::pow((1.0L + i), 1.0L / 12.0L);
        return static_cast<T>(z);
        }
};

template<typename T>
struct i_from_i_upper_12_over_12_naive
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {
        long double z = -1.0L + std::pow((1.0L + i), 12.0L);
        return static_cast<T>(z);
        }
};

template<typename T>
struct d_upper_12_from_i_naive
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {
        long double z = 12.0L * (1.0L - std::pow(1.0L + i, -1.0L / 12.0L));
        return static_cast<T>(z);
        }
};

template<typename T, int n>
struct net_i_from_gross_naive
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
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
    :public std::binary_function<T,T,T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
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
} // Unnamed namespace.

/// This function isn't a unit test per se. Its purpose is to show
/// how a sample calculation is affected by
///   exponential versus power method, and
///   hardware precision.
///
/// Define f(i,m) = (1+i)^(1/m) - 1, i.e., "i upper m over m".
/// Let i0 = 0.0004 (forty bp).
/// Let i1 = f(i0, 365)
/// Then the number calculated here is f(-i1, 1/365).

void sample_results()
{
    fenv_initialize();
    fenv_precision(fe_ldblprec);
    std::cout
        << "\n  annual rate corresponding to a 0.004 daily spread"
        << ", by various methods\n"
        << std::setprecision(20)
        << "    long double precision, expm1l and log1pl\n      "
        << net_i_from_gross<double,365>()(0.0, 0.004, 0.0) << '\n'
        << "    long double precision, pow\n      "
        << net_i_from_gross_naive<double,365>()(0.0, 0.004, 0.0) << '\n'
        ;

    fenv_initialize();
    fenv_precision(fe_dblprec);
    std::cout
        << std::setprecision(20)
        << "    double precision, expm1l and log1pl\n      "
        << net_i_from_gross<double,365>      ()(0.0, 0.004, 0.0) << '\n'
        << "    double precision, pow\n      "
        << net_i_from_gross_naive<double,365>()(0.0, 0.004, 0.0) << '\n'
        ;

    fenv_initialize();
}

// These 'meteN' functions perform the same set of operations using
// different implementations.

// This implementation naively uses std::pow(); it is both slower and
// less inaccurate than an alternative using expm1l() and log1pl().
void mete0()
{
    volatile double x;
    x = i_upper_12_over_12_from_i_naive<double>()(0.04);
    x = i_from_i_upper_12_over_12_naive<double>()(0.04);
    x = d_upper_12_from_i_naive        <double>()(0.04);
    x = net_i_from_gross_naive<double,365>()(0.04, 0.007, 0.003);
}

// This implementation uses production functors.
void mete1()
{
    volatile double x;
    x = i_upper_12_over_12_from_i<double>()(0.04);
    x = i_from_i_upper_12_over_12<double>()(0.04);
    x = d_upper_12_from_i        <double>()(0.04);
    x = net_i_from_gross<double,365>()(0.04, 0.007, 0.003);
}

void assay_speed()
{
    std::cout << "  Speed test: pow   \n    " << TimeAnAliquot(mete0) << '\n';
    std::cout << "  Speed test: expm1l\n    " << TimeAnAliquot(mete1) << '\n';
}

int test_main(int, char*[])
{
    double      smallnumD = std::numeric_limits<double     >::min();
    double      bignumD   = std::numeric_limits<double     >::max();

    long double smallnumL = std::numeric_limits<long double>::min();
    long double bignumL   = std::numeric_limits<long double>::max();

    BOOST_TEST_EQUAL(2.0, greater_of<double>()(1.0, 2.0));
    BOOST_TEST_EQUAL(1.0, lesser_of <double>()(1.0, 2.0));

    BOOST_TEST_EQUAL(1.5, mean<double>()(1.0, 2.0));
    BOOST_TEST_EQUAL(smallnumD, mean<double>()(smallnumD, smallnumD));
    BOOST_TEST_EQUAL(bignumD  , mean<double>()(bignumD  , bignumD  ));

    BOOST_TEST_EQUAL(1.5, mean<long double>()(1.0, 2.0));
    BOOST_TEST_EQUAL(smallnumL, mean<long double>()(smallnumL, smallnumL));
    BOOST_TEST_EQUAL(bignumL  , mean<long double>()(bignumL  , bignumL  ));

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

    // Test nonsensical interest rate of -101%.

    BOOST_TEST(lmi_isnan(i_upper_12_over_12_from_i_naive<double>()(-1.01)));
    BOOST_TEST(lmi_isnan(i_upper_12_over_12_from_i<double>()      (-1.01)));

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

