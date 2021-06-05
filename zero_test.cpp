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

#include <algorithm>                    // max()
#include <cmath>                        // exp(), fabs(), log(), pow(), signbit()
#include <limits>

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

    LMI_TEST(root_is_valid == rn.validity);
    LMI_TEST(root_is_valid == rl.validity);
    LMI_TEST(root_is_valid == rh.validity);

    LMI_TEST(rl.root <= rn.root && rn.root <= rh.root);

    double tol =
            std::pow(10.0, -dec)
        +   6.0 * epsilon * std::max
                (std::fabs(rl.root), std::fabs(rh.root)
                )
        ;
    LMI_TEST(std::fabs(rh.root - rl.root) <= tol);

    double toll =
            std::pow(10.0, -dec)
        +   6.0 * epsilon * std::fabs(rl.root)
        ;
    LMI_TEST(std::fabs(rl.root - exact_root) <= toll);

    double tolh =
            std::pow(10.0, -dec)
        +   6.0 * epsilon * std::fabs(rh.root)
        ;
    LMI_TEST(std::fabs(rh.root - exact_root) <= tolh);
}

double e_function(double z)
{
    return std::log(z) - 1.0;
}

// A stateful function object.
//
// Commented-out tests below would require that the final state equal
// the root returned by decimal_root(). Those two tests are unlikely
// both to succeed, because decimal_root() returns an iterand chosen
// according to its enum root_bias argument rather than the last
// iterand tested. They exist only for this didactic purpose.

struct e_functor
{
    double operator()(double z)
        {
        e_state = z;
        return std::log(z) - 1.0;
        }
    double e_state;
};

struct e_nineteenth
{
    double operator()(double z) {return std::pow(z, 19);}
};

/// A function that's unfriendly to the secant method.
///
/// This function is based on eq. 2.1 in Brent's fourth chapter, and
/// is designed so that successive secant steps in Dekker's algorithm
/// each move by only the input tolerance.
///
/// Following section 3 of that chapter, define
///   k = [log2((b-a)/t)], [x] being the greatest-integer function
/// Then bisection takes exactly k+1 iterations unless it finds a root
/// earlier by serendipity; and the number of function evaluations
/// required by Brent's method (counting the endpoint evaluations) is
///   (k+1)^2 - 2 [Brent's eq. 3.4]
///
/// For this function, k = [log2(200/1)] = 8, so Brent's method should
/// take no more than 9^2-2 = 79 function evaluations.
///
/// The parameters hardcoded here were chosen to prevent overflow.
/// This is not a dramatic illustration of the superiority to Dekker's
/// method, which would move by a step of 1.0 at each iteration, thus
/// taking about 200 iterations. Brent provides an extended-exponent
/// version for which he says the difference would be 1600 evaluations
/// versus 1.0e12.

double eq_2_1(double x)
{
    double a = -100.0;
    double b =  100.0;
    double t =    1.0; // lowercase delta = Brent's 'tol'
    return
          (x == a)               ? -((b - a - t) / t) * std::pow(2.0, b / t)
        : (x < a + t)            ? 1.0
        : (a + t <= x && x <= b) ? std::pow(2.0, x / t)
        : throw "eq_2_1() out of bounds"
        ;
}

/// A function for which bisection is optimal.
///
/// Return signum(argument + 1/3). Adding the constant makes it less
/// likely that the root will be found by accident, e.g. between
/// bounds such as (0,k) or (-k,k).

double signum_offset(double d)
{
    double z = d + 1.0 / 3.0;
    return (0.0 == z) ? 0.0 : std::signbit(z) ? -1.0 : 1.0;
}

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
    LMI_TEST(root_is_valid == r.validity);

    // Same, with expatiation.

    std::ostringstream oss;
    r = decimal_root(0.5, 5.0, bias_none, 9, e_function, oss);
    std::cout << oss.str() << std::endl;

    // Test use with function object.

    e_functor e;
    r = decimal_root(0.5, 5.0, bias_none, 9, e);
    LMI_TEST(root_is_valid == r.validity);

    // Test failure with improper interval.

    r = decimal_root(1.0, 1.0, bias_none, 9, e);
    LMI_TEST(improper_bounds == r.validity);

    // Test failure with interval containing no root.

    r = decimal_root(0.1, 1.0, bias_none, 9, e);
    LMI_TEST(root_not_bracketed == r.validity);

    // Test different biases.

    // Because the base of natural logarithms is transcendental,
    // Brent's algorithm must terminate with distinct upper and lower
    // bounds: neither can equal the unrepresentable true value.

    r = decimal_root(0.5, 5.0, bias_lower, 9, e);
    LMI_TEST(root_is_valid == r.validity);
    double e_or_less = r.root;
    LMI_TEST(e_or_less < std::exp(1.0));
//  LMI_TEST(e.e_state < std::exp(1.0)); // Not necessarily true.

    r = decimal_root(0.5, 5.0, bias_higher, 9, e);
    LMI_TEST(root_is_valid == r.validity);
    double e_or_more = r.root;
    LMI_TEST(std::exp(1.0) < e_or_more);
//  LMI_TEST(std::exp(1.0) < e.e_state); // Not necessarily true.

    LMI_TEST(e_or_less < e_or_more);

    r = decimal_root(0.5, 5.0, bias_none, 9, e);
    LMI_TEST(root_is_valid == r.validity);
    double e_more_or_less = r.root;

    LMI_TEST(e_more_or_less == e_or_less || e_more_or_less == e_or_more);

    // Various tests--see function-template definition.

    test_zero(-1.0e100, 4.0e100, -100, e, std::exp(1.0));
    test_zero(-1.0    , 4.0    ,    0, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    1, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    2, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    3, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    4, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    5, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    6, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    7, e, std::exp(1.0));
    test_zero( 0.5    , 5.0    ,    8, e, std::exp(1.0));
    test_zero(-1.0    , 4.0    ,  100, e, std::exp(1.0));

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
    r = decimal_root(-1.0, 4.0, bias_none, 20, e_19);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(std::fabs(r.root) <= epsilon);

    double d = brent_zero(-1.0, 4.0, 1.0e-20, e_19);
    LMI_TEST(std::fabs(d) <= epsilon);

    d = brent_zero(-100.0, 100.0, 1.0e-20, eq_2_1);
    LMI_TEST(-100.0 <= d && d <= -100.0 * (1.0 - 6.0 * epsilon));

    r = decimal_root(-100.0, 100.0, bias_none, 20, eq_2_1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(-100.0 <= r.root && r.root <= -100.0 * (1.0 - 6.0 * epsilon));

    r = decimal_root(-1.0, 1.0, bias_none, 13, signum_offset);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));

    e_former_rounding_problem e_frp;
    r = decimal_root(0.12609, 0.12611, bias_lower, 5, e_frp);
#if !defined LMI_COMO_WITH_MINGW
    LMI_TEST(materially_equal(0.12610, r.root));
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
        (   materially_equal(0.12609, r.root)
        ||  materially_equal(0.12610, r.root)
        );
#endif // defined LMI_COMO_WITH_MINGW

    LMI_TEST(root_is_valid == r.validity);

    return 0;
}
