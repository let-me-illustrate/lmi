// Root finding by Brent's method--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "zero.hpp"

#include "materially_equal.hpp"
#include "test_tools.hpp"

#include <algorithm>

namespace
{
    static double const epsilon = std::numeric_limits<double>::epsilon();
} // Unnamed namespace.

template<typename F>
void test_zero(double bound0, double bound1, int dec, F f, double exact_root)
{
    root_type rn = decimal_root(bound0, bound1, bias_none,   dec, f);
    root_type rl = decimal_root(bound0, bound1, bias_lower,  dec, f);
    root_type rh = decimal_root(bound0, bound1, bias_higher, dec, f);

    LMI_TEST(root_is_valid == rn.second);
    LMI_TEST(root_is_valid == rl.second);
    LMI_TEST(root_is_valid == rh.second);

    LMI_TEST(rl.first <= rn.first && rn.first <= rh.first);

    double tol =
            std::pow(10.0, -dec)
        +   6.0 * epsilon * std::max
                (std::fabs(rl.first), std::fabs(rh.first)
                )
        ;
    LMI_TEST((rh.first - rl.first) <= tol);

    double toll =
            std::pow(10.0, -dec)
        +   6.0 * epsilon * std::fabs(rl.first)
        ;
    LMI_TEST((rl.first - exact_root) <= toll);

    double tolh =
            std::pow(10.0, -dec)
        +   6.0 * epsilon * std::fabs(rh.first)
        ;
    LMI_TEST((rh.first - exact_root) <= tolh);
}

double e_function(double z)
{
    return std::log(z) - 1.0;
}

struct e_functor
{
    double operator()(double z)
        {
        value = z;
        return std::log(z) - 1.0;
        }
    double value;
};

struct e_nineteenth
{
    double operator()(double z) {return std::pow(z, 19);}
};

// This problem once arose in a unit test for irr calculations.
// Minimal test case:
//
//   rounding = near
//   bias     = lower
//   decimals = 5
//
//   lower bound  = 0.12609
//   upper bound  = 0.12611
//   desired root = 0.12610
//
// With a certain compiler, due to a defect in the rounding library,
// the midpoint of the bounds rounded to the lower bound, and the
// function never terminated.

struct e_former_rounding_problem
{
    double operator()(double z) {return z - 0.12610;}
};

int test_main(int, char*[])
{
    // Test use with function.

    root_type r = decimal_root(0.5, 5.0, bias_none, 9, e_function);
    LMI_TEST(root_is_valid == r.second);

    // Test use with function object.

    e_functor e;
    r = decimal_root(0.5, 5.0, bias_none, 9, e);
    LMI_TEST(root_is_valid == r.second);

    // Test failure with interval containing no root.

    r = decimal_root(0.1, 1.0, bias_none, 9, e);
    LMI_TEST(root_not_bracketed == r.second);

    // Test guaranteed side effects.

    // Because the base of natural logarithms is transcendental,
    // Brent's algorithm must terminate with distinct upper and lower
    // bounds.

    r = decimal_root(0.5, 5.0, bias_lower, 9, e, true);
    LMI_TEST(root_is_valid == r.second);
    double e_or_less = r.first;
    r = decimal_root(0.5, 5.0, bias_higher, 9, e, true);
    LMI_TEST(root_is_valid == r.second);
    double e_or_more = r.first;
    LMI_TEST(e_or_less < e_or_more);

    r = decimal_root(0.5, 5.0, bias_lower, 9, e, true);
    LMI_TEST(root_is_valid == r.second);
    LMI_TEST(r.first < std::exp(1.0));
    LMI_TEST(e.value < std::exp(1.0));

    r = decimal_root(0.5, 5.0, bias_higher, 9, e, true);
    LMI_TEST(root_is_valid == r.second);
    LMI_TEST(std::exp(1.0) < r.first);
    LMI_TEST(std::exp(1.0) < e.value);

    // Various tests--see macro definition.

    test_zero(0.5, 5.0, 1, e, std::exp(1.0));
    test_zero(0.5, 5.0, 2, e, std::exp(1.0));
    test_zero(0.5, 5.0, 3, e, std::exp(1.0));
    test_zero(0.5, 5.0, 4, e, std::exp(1.0));
    test_zero(0.5, 5.0, 5, e, std::exp(1.0));
    test_zero(0.5, 5.0, 6, e, std::exp(1.0));
    test_zero(0.5, 5.0, 7, e, std::exp(1.0));
    test_zero(0.5, 5.0, 8, e, std::exp(1.0));

    // Brent's book uses the nineteenth-power function in examples.
    // His example using a tolerance of 1e-20 is subject to underflow
    // on IEEE 754 hardware: distinct bounds can't be that close
    // to the exact value of unity because of machine epsilon. But
    // that doesn't matter, because a term involving machine epsilon
    // is always added to the effective tolerance. An excessively
    // low input tolerance makes the effective tolerance simply
    //   6 * epsilon * |iterand|
    // because the other term vanishes--it does not give more
    // precision than the hardware is capable of, though it's a
    // chasing after wind that costs many iterations.

    e_nineteenth e_19;
    r = decimal_root(-1.0, 4.0, bias_none, -20, e_19);
    LMI_TEST(root_is_valid == r.second);

    test_zero(-1.0, 4.0, -100, e_19, std::exp(1.0));
    test_zero(-1.0, 4.0,    0, e_19, std::exp(1.0));
    test_zero(-1.0, 4.0,  100, e_19, std::exp(1.0));

    e_former_rounding_problem e_frp;
    r = decimal_root(0.12609, 0.12611, bias_lower, 5, e_frp);
#if !defined LMI_COMO_WITH_MINGW
    LMI_TEST(materially_equal(0.12610, r.first));
#else // defined LMI_COMO_WITH_MINGW
    // One would naively expect 0.12610 to be the answer, but it's
    // necessary to inquire which of the two closest representations
    // is meant [C++98 4.8/1]. This particular compiler iterates to
    //          b  = 0.1261 3fc0240b780346dc
    // and then changes its value slightly
    //   round_(b) = 0.1261 3fc0240b780346dd
    // (see documentation of the rounding library and its unit test)
    // resulting in a final iterand whose function value is slightly
    // different from zero, and in the "wrong" direction.
    LMI_TEST
        (   materially_equal(0.12609, r.first)
        ||  materially_equal(0.12610, r.first)
        );
#endif // defined LMI_COMO_WITH_MINGW

    LMI_TEST(root_is_valid == r.second);

    return 0;
}
