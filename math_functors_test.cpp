// Miscellaneous mathematical operations as function objects--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: math_functors_test.cpp,v 1.1 2005-04-05 12:36:37 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "math_functors.hpp"

#include "materially_equal.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <iomanip>
#include <limits>

// Some of these tests may raise hardware exceptions. That means that
// edge cases are tested, not that the code tested is invalid for
// arguments that aren't ill conditioned.

namespace
{
long double const days_per_year = 365.0L;
long double const years_per_day = 1.0L / days_per_year;

// C99's isnan macro can't be written correctly in portable C++.
// This implementation is portable, but compilers needn't implement
// it correctly.
//
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
        {return -1.0L + std::pow((1.0L + i), 1.0L / 12.0L);}
};

template<typename T>
struct i_from_i_upper_12_over_12_naive
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {return -1.0L + std::pow((1.0L + i), 12.0L);}
};

template<typename T>
struct d_upper_12_from_i_naive
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {return 12.0L * (1.0L - std::pow(1.0L + i, -1.0L / 12.0L));}
};

template<typename T, int n>
struct net_i_from_gross_naive
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i, T const& spread, T const& fee) const
        {
        static long double const reciprocal_n = 1.0L / n;
        return
            std::pow(
                1.0L
                +   std::pow(1.0L + i,      reciprocal_n)
                -   std::pow(1.0L + spread, reciprocal_n)
                -                   fee   * reciprocal_n
                ,n
                )
            -   1.0L;
        }
};
} // Unnamed namespace.

// This function isn't a unit test per se. Its purpose is to show
// how a sample calculation is affected by
//   exponential versus power method, and
//   hardware precision.
//
void sample_results()
{
    std::cout
        << "\n-0.004 upper 365 by various methods\n"
        << std::setprecision(20)
        << "  64-bit mantissa, expm1 and log1p\n"
        << net_i_from_gross<double,365>()(0.0, 0.004, 0.0) << '\n'
        << "  64-bit mantissa, pow\n"
        << net_i_from_gross_naive<double,365>()(0.0, 0.004, 0.0) << '\n'
        ;
#if defined __GNUC__ && defined LMI_X86
    volatile unsigned short int control_word = 0x027f;
    asm volatile("fldcw %0" : : "m" (control_word));
    std::cout
        << std::setprecision(20)
        << "  53-bit mantissa, expm1 and log1p\n"
        << net_i_from_gross<double,365>      ()(0.0, 0.004, 0.0) << '\n'
        << "  53-bit mantissa, pow\n"
        << net_i_from_gross_naive<double,365>()(0.0, 0.004, 0.0) << '\n'
        ;
    control_word = 0x037f;
    asm volatile("fldcw %0" : : "m" (control_word));
#endif // defined __GNUC__ && defined LMI_X86
}

int test_main(int, char*[])
{
    double smallnumD = std::numeric_limits<double>::min();
    double bignumD   = std::numeric_limits<double>::max();

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

    // Test run time.

    int const iterations = 1000000;
    volatile double x;

    Timer timer0;
    for(int j = 0; j < iterations; ++j)
        {
        x = i_upper_12_over_12_from_i_naive<double>()(0.04);
        x = i_from_i_upper_12_over_12_naive<double>()(0.04);
        x = d_upper_12_from_i_naive        <double>()(0.04);
        x = net_i_from_gross_naive<double,365>()(0.04, 0.007, 0.003);
        }
    timer0.Stop();
    std::cout
        << timer0.Report() << " for " << iterations
        << " runs with power method\n"
        ;

    Timer timer1;
    for(int j = 0; j < iterations; ++j)
        {
        x = i_upper_12_over_12_from_i<double>()(0.04);
        x = i_from_i_upper_12_over_12<double>()(0.04);
        x = d_upper_12_from_i        <double>()(0.04);
        x = net_i_from_gross<double,365>()(0.04, 0.007, 0.003);
        }
    timer1.Stop();
    std::cout
        << timer1.Report() << " for " << iterations
        << " runs with C++ exponential method\n"
        ;

    Timer timer2;
    for(int j = 0; j < iterations; ++j)
        {
        static long double const one_twelfth = 1.0L / 12.0L;
        x = expm1(log1p(0.04) * one_twelfth);
        x = expm1(log1p(0.04) * 12.0L);
        x = -12.0L * expm1(log1p(0.04) * -one_twelfth);
        x = expm1
            (
            days_per_year * log1p
                (   expm1(years_per_day * log1p(0.04))
                -   expm1(years_per_day * log1p(0.007))
                -         years_per_day *       0.003
                )
            );
        }
    timer2.Stop();
    std::cout
        << timer2.Report() << " for " << iterations
        << " runs with C exponential method\n"
        ;

    sample_results();

    return 0;
}

