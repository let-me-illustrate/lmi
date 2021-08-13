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

#include "assert_lmi.hpp"
#include "materially_equal.hpp"
#include "math_functions.hpp"           // signum()
#include "miscellany.hpp"               // stifle_warning_for_unused_variable()
#include "test_tools.hpp"

#include <algorithm>                    // max()
#include <cfloat>                       // DECIMAL_DIG
#include <cmath>                        // exp(), fabs(), log(), pow(), sin(), sqrt()
#include <limits>
#include <sstream>

namespace
{
static double const epsilon = std::numeric_limits<double>::epsilon();

/// AfMWD eq. 2.18: maximum error
///
/// As the paragraph following that equation emphasizes, "the effect
/// of rounding errors in the computation of f" must be considered,
/// as Brent's method can "only guarantee to find a zero ζ of the
/// computed function f to an accuracy given by (2.18), and ζ may be
/// nowhere near a root of the mathematically defined function that
/// the user is really interested in!".

double max_err(double zeta, double tol)
{
    LMI_ASSERT(0.0 <= tol);
    return 6.0 * epsilon * std::fabs(zeta) + 2.0 * tol;
}

/// AfMWD eq. 3.2: maximum number of evaluations for bisection.
///
/// The return value, k+1, is the exact number of function
/// evaluations unless f vanishes early, as Brent explains in the
/// paragraph following eq. 3.3 .
///
/// GWC modification: Constrain δ to be no less than ϵ/2, to
/// prevent 'delta' from being zero (which Brent wouldn't allow
/// anyway: he says "t is a positive absolute tolerance" in the
/// paragraph following equation 2.9). Why ϵ/2 instead of ϵ?
/// Because using ϵ/2 gives the correct number of iterations for an
/// input tolerance of zero; using ϵ does not, as is demonstrated
/// by some unit tests below. But why is the correct answer obtained
/// only by using ϵ/2 where ϵ seems natural? Most authors give the
/// number of steps (disregarding two initial endpoint evaluations)
/// as
///   ⌈log2((b-a)/tol)⌉
/// so that the total number of evaluations is
///   2 + ⌈log2((b-a)/tol)⌉
/// whereas Brent gives an answer one lower, presumably because he
/// regards the tolerance as '2t'.
///
/// static_cast<int> is exact for any number of evaluations that
/// can be counted by an 'int'.
///
/// The greatest possible number of bisection steps (i.e., of extra
/// evaluations in addition to the initial two, one at each endpoint)
/// with the lowest possible positive 'tol' is:
///     log2((DBL_MAX - -DBL_MAX) /      DBL_TRUE_MIN)
///   = log2 (DBL_MAX - -DBL_MAX) - log2(DBL_TRUE_MIN)
///   = (1 + 1024) - -1074 = 2099
///   =      1025 +   1074 = 2099
/// Yet an IEEE 754 binary64 entity can have no more than 2^64
/// distinct values; with an appropriate definition of "bisection",
/// about 64 steps should suffice.
///
/// Known defects:
///  - std::fabs(DBL_MAX - -DBL_MAX) overflows.
///  - Brent's eq. 3.2 with the strange ϵ/2 adjustment is shown to
///    give a correct result in certain cases, but a more canonical
///    version would probably be better.
///  - What is the lowest positive value of 'tol' that makes sense?
///    The ϵ/2 adjustment assumes that it is ϵ/2, so that ϵ=2t; but
///    the log2((DBL_MAX - -DBL_MAX) / DBL_TRUE_MIN) calculation
///    conflictingly assumes that it is DBL_TRUE_MIN.
/// Such defects in a unit-testing TU needn't be fixed.

int max_n_eval_bolzano(double a, double b, double tol, double zeta)
{
    LMI_ASSERT(0.0 <= tol);
    double delta = 2.0 * epsilon * std::fabs(zeta) + tol;
    delta = std::max(delta, 0.5 * epsilon);
    double k = std::ceil(std::log2(std::fabs(b - a) / delta));
    return 1 + static_cast<int>(k);
}

/// AfMWD eq. 3.2: maximum number of evaluations for Brent's method.
///
/// The greatest possible number of steps is 2099^2 = 4405801.

int max_n_eval_brent(double a, double b, double tol, double zeta)
{
    int k_plus_one = max_n_eval_bolzano(a, b, tol, zeta);
    return k_plus_one * k_plus_one - 2;
}
} // Unnamed namespace.

/// Test (unrounded) root-finding accuracy and speed.
///
/// Find a root using
///  - a plain translation of Brent's ALGOL procedure 'zero'
///  - lmi's customized version thereof, with default {bias,rounding}
///
/// Verify that
///  - the result is within the max_err() tolerance (ignoring Brent's
///      warning about roundoff in the computed function)
///  - the number of evaluations doesn't exceed max_n_eval_brent()
///  - maximum-precision instrumented traces are identical
/// Identical traces are strong architecture-independent evidence
/// that both implementations behave the same way at every step.
/// This probabilistically approaches a proof that iterates and
/// function evaluations are identical, with a sufficiently
/// comprehensive test suite, though it cannot reliably detect
/// discrepancies such as comparing doubles with '<' rather than '<='
/// when exact equality is extremely unlikely. The alternative of
/// storing a reference dataset (instead of a maintaining a reference
/// implementation) is no more powerful and much balkier.
///
/// Unfortunately, gcc results with x87 are not reproducible because
/// spills from 80- to 64-bit registers are unpredictable: e.g., when
/// solving x^2=2, the first linear interpolation yields
///     1.33333333333333325932  x
///    -0.222222222222222265398 x^2, on one path
///    -0.222222222222222431931 x^2, on another path
/// so this comparison is not made for x87.

template<typename F>
int test_a_function
    (F           f
    ,double      exact_root
    ,double      bound0
    ,double      bound1
    ,double      tolerance
    ,int         line
    ,char const* file   = __FILE__
    )
{
    // Otherwise silly alias for compatibility with test_a_decimal_function().
    double const tol = tolerance;
    double const maximum_error = max_err(exact_root, tol);
    int const max_n_eval = max_n_eval_brent(bound0, bound1, tol, exact_root);

    std::ostringstream os0;
    os0.precision(DECIMAL_DIG);
    double d = brent_zero(f, bound0, bound1, tol, os0);
    double error = d - exact_root;
    INVOKE_LMI_TEST_RELATION(std::fabs(error),<=,maximum_error,file,line);

    std::ostringstream os1;
    os1.precision(DECIMAL_DIG);
    root_type r = lmi_root(f, bound0, bound1, tol, os1);
    INVOKE_LMI_TEST(root_is_valid == r.validity, file, line);
    error = r.root - exact_root;
    INVOKE_LMI_TEST_RELATION(std::fabs(error),<=,maximum_error,file,line);
    INVOKE_LMI_TEST_RELATION(r.n_eval,<=,max_n_eval,file,line);

#if !defined LMI_X87
    INVOKE_LMI_TEST_EQUAL(os0.str(),os1.str(),file,line);
#endif // !defined LMI_X87

    return r.n_eval;
}

/// Test decimal root-finding accuracy and speed.
///
/// Find a root using
///  - a plain translation of Brent's ALGOL procedure 'zero'
///  - lmi's customized version thereof, specifying {bias,rounding}
///
/// Verify that
///  - the result is within the max_err() tolerance (ignoring Brent's
///      warning about roundoff in the computed function)
///  - the number of evaluations doesn't exceed max_n_eval_brent()
///
/// Also verify that the number of evaluations matches the 'n_eval'
/// argument, to make it easier to detect mistaken refactorings.
/// Do this only if 'n_eval' is not zero (the default), and only for
/// a single architecture (here, x86_64-pc-linux-gnu), because the
/// outcome depends on architecture.

template<typename F>
void test_a_decimal_function
    (F           f
    ,double      exact_root
    ,double      bound0
    ,double      bound1
    ,int         decimals
    ,int         line
    ,int         n_eval = 0
    ,char const* file   = __FILE__
    )
{
    double const tol = 0.5 * std::pow(10.0, -decimals);
    double const maximum_error = max_err(exact_root, tol);
    int const max_n_eval = max_n_eval_brent(bound0, bound1, tol, exact_root);

    double d = brent_zero(f, bound0, bound1, tol);
    double error = d - exact_root;
    INVOKE_LMI_TEST_RELATION(std::fabs(error),<=,maximum_error,file,line);

    root_type r = decimal_root(f, bound0, bound1, bias_none, decimals);
    INVOKE_LMI_TEST(root_is_valid == r.validity, file, line);
    error = r.root - exact_root;
    INVOKE_LMI_TEST_RELATION(std::fabs(error),<=,maximum_error,file,line);
    INVOKE_LMI_TEST_RELATION(r.n_eval,<=,max_n_eval,file,line);

#if defined LMI_X86_64 && defined LMI_POSIX
    if(0 != n_eval)
        {
        INVOKE_LMI_TEST_EQUAL(n_eval, r.n_eval, file, line);
        }
#endif // defined LMI_X86_64 && defined LMI_POSIX
    stifle_warning_for_unused_variable(n_eval);
}

/// Test with all biases, asserting obvious invariants.

template<typename F>
void test_bias
    (double bound0
    ,double bound1
    ,int dec
    ,F f
    ,double exact_root
    ,int         line
    ,char const* file   = __FILE__
   )
{
    double maximum_error = max_err(exact_root, 0.5 * std::pow(10.0, -dec));

    root_type rn = decimal_root(f, bound0, bound1, bias_none,   dec);
    root_type rl = decimal_root(f, bound0, bound1, bias_lower,  dec);
    root_type rh = decimal_root(f, bound0, bound1, bias_higher, dec);

    INVOKE_LMI_TEST_RELATION(root_is_valid,==,rn.validity,file,line);
    INVOKE_LMI_TEST_RELATION(root_is_valid,==,rl.validity,file,line);
    INVOKE_LMI_TEST_RELATION(root_is_valid,==,rh.validity,file,line);

    INVOKE_LMI_TEST_RELATION(rl.root,<=,rn.root,file,line);
    INVOKE_LMI_TEST_RELATION(rn.root,<=,rh.root,file,line);

    INVOKE_LMI_TEST_RELATION(std::fabs(rh.root - rl.root),<=,maximum_error,file,line);
    INVOKE_LMI_TEST_RELATION(std::fabs(rl.root - exact_root),<=,maximum_error,file,line);
    INVOKE_LMI_TEST_RELATION(std::fabs(rh.root - exact_root),<=,maximum_error,file,line);
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
/// Bisection takes exactly k+1 evaluations unless it finds a root
/// earlier by serendipity; and the number of function evaluations
/// required by Brent's method (counting the endpoint evaluations) is
///   (k+1)^2 - 2 [Brent's eq. 3.4]
///
/// For this function, k = [log2(200/0.5)] = 9, so Brent's method
/// should take no more than 10^2-2 = 98 function evaluations.
///
/// The parameters hardcoded here were chosen to prevent overflow.
/// This is not a dramatic illustration of the superiority to Dekker's
/// method, which would move by a step of 1.0 at each evaluation, thus
/// taking about 200 evaluations. Brent provides an extended-exponent
/// version for which he says the difference would be 1600 evaluations
/// versus 1.0e12 for a tolerance of 1.0e-12.

double eq_2_1(double x)
{
    double a = -100.0;
    double b =  100.0;
    double t =    0.5; // lowercase delta = Brent's 'tol'
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
    return signum(d + 1.0 / 3.0);
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

void test_fundamentals()
{
    // Test use with function.

    root_type r = decimal_root(e_function, 0.5, 5.0, bias_none, 9);
    LMI_TEST(root_is_valid == r.validity);

    // Same, with expatiation.

    std::ostringstream oss;
    r = decimal_root(e_function, 0.5, 5.0, bias_none, 9, oss);
    std::cout << oss.str() << std::endl;

    // Test use with function object.

    e_functor e;
    r = decimal_root(e, 0.5, 5.0, bias_none, 9);
    LMI_TEST(root_is_valid == r.validity);

    // Test failure with improper interval.

    r = decimal_root(e, 1.0, 1.0, bias_none, 9);
    LMI_TEST(improper_bounds == r.validity);

    // Test failure with interval containing no root.

    r = decimal_root(e, 0.1, 1.0, bias_none, 9);
    LMI_TEST(root_not_bracketed == r.validity);

    // Calculate maximum possible number of evaluations according to
    // the documentation for max_n_eval_bolzano(). This calculation
    // would overflow in double precision.
    //
    // log2(DBL_MAX) is 1024, so log2(DBL_MAX - -DBL_MAX) is 1025;
    // and log2(DBL_TRUE_MIN) is 1074; so the maximum number of
    // evaluations with IEEE 754 binary64 is
    //   1025 + 1074 = 2099 for bisection, and
    //   2099^2 = 4405801 for Brent's method
    long double max = DBL_MAX;
    long double min = DBL_TRUE_MIN;
    int max_iter = static_cast<int>(std::ceil(std::log2((max - -max) / min)));
    LMI_TEST_EQUAL(1 + 1024 + 1074, max_iter);
    LMI_TEST_EQUAL(2099, max_iter);
}

void test_binary64_midpoint()
{
    // Make sure double is binary64.
    static_assert(std::numeric_limits<double>::is_iec559);

    // Make doubly sure it has infinity...
    static_assert(std::numeric_limits<double>::has_infinity);
    constexpr double inf = std::numeric_limits<double>::infinity();

    // ...and qNaN.
    static_assert(std::numeric_limits<double>::has_quiet_NaN);
    constexpr double qnan = std::numeric_limits<double>::quiet_NaN();

    // Make sure the signs of non-finite values are detected correctly.

    LMI_TEST_EQUAL( 0.0, signum( 0.0));
    LMI_TEST_EQUAL( 0.0, signum(-0.0));

    LMI_TEST_EQUAL( 1.0, signum( inf));
    LMI_TEST_EQUAL(-1.0, signum(-inf));

    LMI_TEST_EQUAL( 1.0, signum( qnan));
    LMI_TEST_EQUAL(-1.0, signum(-qnan));

    // Both zero: return positive zero, regardless of signbit.
    // Thus, the midpoint of two zeros doesn't depend on the order
    // in which they're given.

    double const zpp = binary64_midpoint( 0.0,  0.0);
    double const zpn = binary64_midpoint( 0.0, -0.0);
    double const znp = binary64_midpoint(-0.0,  0.0);
    double const znn = binary64_midpoint(-0.0, -0.0);

    LMI_TEST_EQUAL(0.0, zpp);
    LMI_TEST_EQUAL(0.0, zpn);
    LMI_TEST_EQUAL(0.0, znp);
    LMI_TEST_EQUAL(0.0, znn);

    LMI_TEST_EQUAL(false, std::signbit(zpp));
    LMI_TEST_EQUAL(false, std::signbit(zpn));
    LMI_TEST_EQUAL(false, std::signbit(znp));
    LMI_TEST_EQUAL(false, std::signbit(znn));

    // One argument >0, the other <0: return zero.

    LMI_TEST_EQUAL(0.0, binary64_midpoint( 3.1416, -2.718));
    LMI_TEST_EQUAL(0.0, binary64_midpoint(-3.1416,  2.718));

    // Do not return zero when one argument is zero and the other
    // has an opposite signbit.

    double const d0 = binary64_midpoint( 3.1416, -0.0);
    double const d1 = binary64_midpoint(-3.1416,  0.0);
    LMI_TEST_UNEQUAL(0.0, d0); // do not return zero
    LMI_TEST_UNEQUAL(0.0, d1); // do not return zero
    // Actual values are calculated...
    LMI_TEST(materially_equal( 1.91739e-154, d0, 1.0e-5));
    LMI_TEST(materially_equal(-1.91739e-154, d1, 1.0e-5));
    // ...as though the zero's signbit matched the other argument's:
    double const e0 = binary64_midpoint( 3.1416,  0.0);
    double const e1 = binary64_midpoint(-3.1416, -0.0);
    LMI_TEST_EQUAL(d0, e0);
    LMI_TEST_EQUAL(d1, e1);

    // One argument zero, the other nonzero:  binary midpoint, i.e.,
    //   std::midpoint(*(std::uint64_t)(&x), *(std::uint64_t)(&y))
    // after forcing the zero to match the other argument's signbit.

    // 0000000000000000 <-> 0.0
    // 3ff0000000000000 <-> 1.0
    // 1ff8000000000000 <-> 1.11875e-154 <-> 0x1.8p-512
    LMI_TEST(materially_equal(1.11875e-154, binary64_midpoint(0.0,  1.00), 1.0e-5));

    LMI_TEST(materially_equal(5.59376e-155, binary64_midpoint(0.0,  0.25), 1.0e-5));

    LMI_TEST(materially_equal(1.09631e-104, binary64_midpoint(0.0, 1.0e100), 1.0e-5));
    LMI_TEST(materially_equal(1.09631e-104, binary64_midpoint(1.0e100, 0.0), 1.0e-5));
    LMI_TEST(materially_equal(0.000106605 , binary64_midpoint(0.0, 1.0e300), 1.0e-5));

    LMI_TEST(materially_equal( 2.65703e-154, binary64_midpoint( 0.0,  6.25), 1.0e-5));
    LMI_TEST(materially_equal( 2.65703e-154, binary64_midpoint(-0.0,  6.25), 1.0e-5));
    LMI_TEST(materially_equal(-2.65703e-154, binary64_midpoint( 0.0, -6.25), 1.0e-5));
    LMI_TEST(materially_equal(-2.65703e-154, binary64_midpoint(-0.0, -6.25), 1.0e-5));

    // Both arguments nonzero and same sign: binary midpoint, i.e.,
    //   std::midpoint((std::uint64_t)x, (std::uint64_t)y)

    LMI_TEST(materially_equal( 3.75, binary64_midpoint( 3.0,  5.0)));
    LMI_TEST(materially_equal(-3.75, binary64_midpoint(-3.0, -5.0)));

    LMI_TEST(materially_equal( 1.00028e3  , binary64_midpoint( 1.0e0  ,  1.0e6  ), 1.0e-5));

    LMI_TEST(materially_equal( 1.00223e50 , binary64_midpoint( 1.0e0  ,  1.0e100), 1.0e-5));
    LMI_TEST(materially_equal( 1.00894e200, binary64_midpoint( 1.0e100,  1.0e300), 1.0e-5));

    LMI_TEST(materially_equal( 0.973197   , binary64_midpoint( 1.0e-100, 1.0e100), 1.0e-5));

    // Identical arguments: return value equals both.

    LMI_TEST_EQUAL( 1.0e100, binary64_midpoint( 1.0e100,  1.0e100));
    LMI_TEST_EQUAL(-1.0e100, binary64_midpoint(-1.0e100, -1.0e100));

    // Illustration solves typically search in [0, 1.0e9]. For
    // binary64 bisection (only):
    //  - using 1.0e9 instead of DBL_MAX saves only about one function
    //    evaluation (but risks overflow), so an even more implausible
    //    upper limit like 1.0e18 would cost little;
    //  - sometimes 0.0 is the correct answer, but the next higher
    //    currency amount is $0.01, which is very far from zero: the
    //    interval [1.0e-2, 1.0e9] can be searched exhaustively in
    //    about fifty-seven function evaluations.
    std::uint64_t bignum        = 0x7FEFFFFFFFFFFFFF;
    std::uint64_t one_e_300     = 0x7E37E43C8800759C;
    std::uint64_t one_billion   = 0x41CDCD6500000000;
    std::uint64_t one_hundredth = 0x3F847AE147AE147B;
    LMI_TEST_EQUAL(4741671816366391296, one_billion);
    LMI_TEST_EQUAL(4576918229304087675, one_hundredth);
    LMI_TEST(materially_equal(62.9993, std::log2(bignum       ), 1.0e-4));
    LMI_TEST(materially_equal(62.9798, std::log2(one_e_300    ), 1.0e-4));
    LMI_TEST(materially_equal(62.0401, std::log2(one_billion  ), 1.0e-4));
    LMI_TEST(materially_equal(61.9891, std::log2(one_hundredth), 1.0e-4));
    LMI_TEST(materially_equal(57.1931, std::log2(one_billion - one_hundredth), 1.0e-4));
    // The same [0, 1.0e9] interval could be searched exhaustively for
    // integral cents in fewer iterations using the arithmetic mean:
    LMI_TEST(materially_equal(3.49808e-150, binary64_midpoint(0.0, 1.0e9), 1.0e-5));
    LMI_TEST_EQUAL(39, max_n_eval_bolzano(0.0, 1.0e9, 0.005, 1.0e9));
    LMI_TEST_EQUAL(39, max_n_eval_bolzano(0.0, 1.0e9, 0.005, 0.0));

    // Examples from Don Clugston:
    //   https://dconf.org/2016/talks/clugston.pdf

    // Reproduce results for arithmetic mean:
    double x0 = 1e-100;
    double x1 = 1e100;
    double x2 = std::midpoint(x0, x1);
    double x3 = std::midpoint(x0, x2);
    double x4 = std::midpoint(x0, x3);
    double x5 = std::midpoint(x0, x4);
    LMI_TEST_EQUAL( 5.0e99, x2);
    LMI_TEST_EQUAL( 2.5e99, x3);
    LMI_TEST_EQUAL(1.25e99, x4);
    LMI_TEST_EQUAL(6.25e98, x5);

    // Clugston's "midpoint in implementation space" in D is:
    //   ulong x0_raw = reinterpret!ulong(x0);
    //   ulong x1_raw = reinterpret!ulong(x1);
    //   auto midpoint = reinterpret!double( x0_raw + x1_raw ) / 2;
    // and this would seem to be a faithful translation from D to C
    // that avoids "pun-and-alias" issues:
    auto binary_chop_for_real = [](double d00, double d01)
        {
        std::uint64_t u00, u01, um;
        std::memcpy(&u00, &d00, sizeof d00);
        std::memcpy(&u01, &d01, sizeof d01);
        um = std::midpoint(u00, u01);
        double z;
        std::memcpy(&z, &um, sizeof z);
        return z;
        };

    // Clugston reports that
    //   "Midpoints are 5e0, 2.5e-50, 1.2e-75, 6e-88, 3e-94"
    // but the observed values here don't quite agree...
    double y0 = 1e-100;
    double y1 = 1e100;
    double y2 = binary_chop_for_real(y0, y1);
    double y3 = binary_chop_for_real(y0, y2);
    double y4 = binary_chop_for_real(y0, y3);
    double y5 = binary_chop_for_real(y0, y4);
    LMI_TEST(materially_equal(0.973197   , y2, 1.0e-5));
    LMI_TEST(materially_equal(9.87906e-51, y3, 1.0e-5));
    LMI_TEST(materially_equal(9.94306e-76, y4, 1.0e-5));
    LMI_TEST(materially_equal(3.20308e-88, y5, 1.0e-5));

    // ...Instead, they agree with lmi's binary64_midpoint():
    double z0 = 1e-100;
    double z1 = 1e100;
    double z2 = binary64_midpoint(z0, z1);
    double z3 = binary64_midpoint(z0, z2);
    double z4 = binary64_midpoint(z0, z3);
    double z5 = binary64_midpoint(z0, z4);
    LMI_TEST(materially_equal(0.973197   , z2, 1.0e-5));
    LMI_TEST(materially_equal(9.87906e-51, z3, 1.0e-5));
    LMI_TEST(materially_equal(9.94306e-76, z4, 1.0e-5));
    LMI_TEST(materially_equal(3.20308e-88, z5, 1.0e-5));
}

/// A function whose value almost everywhere in (-1.0e100, 1.0e100)
/// is a "signed" NaN. It's dubious to think of NaNs as possessing
/// signedness, yet they do have a sign bit.
///
///   f(x) =
///     -1.0,             x <= -1.0e100
///     -NaN, -1.0e100 <  x <  π
///      0.0,             x =  π
///     +NaN,        π <  x <  +1.0e100
///     +1.0, +1.0e100 <= x

double NaN_signed(double z)
{
    static_assert(std::numeric_limits<double>::has_quiet_NaN);
    // SOMEDAY !! Use std::numbers::pi when it becomes available.
    constexpr double pi {3.14159265358979323851};
    constexpr double qnan = std::numeric_limits<double>::quiet_NaN();
    if(z <= -1.0e100)
        {return -1.0;}
    else if(pi == z)
        {return  0.0;}
    else if(1.0e100 <= z)
        {return  1.0;}
    else if(pi < z)
        {return qnan;}
    else
        {return -qnan;}
}

/// Test NaN-valued functions.
///
/// On the IBM 360 hardware Brent used, there is no NaN (see Goldberg,
/// "What Every Computer Scientist Should Know...":
///  | On some floating-point hardware every bit pattern represents a
///  | valid floating-point number. The IBM System/370 is an example
/// ), so it's important to test worst-case convergence for functions
/// that may return a NaN.
///
/// The "root" found is one of the endpoints. Reason: as of 2021-07
/// at least, the bracketing interval is narrowed to [1.0e100, NaN].

void test_NaNs()
{
    constexpr double pi {3.14159265358979323851};

    LMI_TEST_EQUAL( 1, signum(NaN_signed(4.0)));
    LMI_TEST_EQUAL(-1, signum(NaN_signed(3.0)));

    root_type r = lmi_root(NaN_signed, -1.0e100, 1.0e100, 5.0e-1);
    LMI_TEST_EQUAL(root_is_valid, r.validity);

    int const max_n_eval = max_n_eval_brent(-1.0e100, 1.0e100, 5.0e-1, pi);
    LMI_TEST_RELATION(r.n_eval,<=,max_n_eval);
    LMI_TEST(materially_equal(1.0e100, std::fabs(r.root)));

    // If the function's value is a NaN at either input bound, then
    // no root is bracketed.
    r = lmi_root(NaN_signed,  -1.0e100, 2.0 * pi, 5.0e-1);
    LMI_TEST_EQUAL(root_not_bracketed, r.validity);
    r = lmi_root(NaN_signed, -2.0 * pi,  1.0e100, 5.0e-1);
    LMI_TEST_EQUAL(root_not_bracketed, r.validity);
    r = lmi_root(NaN_signed, -2.0 * pi, 2.0 * pi, 5.0e-1);
    LMI_TEST_EQUAL(root_not_bracketed, r.validity);
}

/// Find a root that coincides with one or both bounds.
///
/// In this special case, lmi_root() returns the root as soon as
/// possible. The reference implementation does not.

void test_root_at_a_bound()
{
    auto f = [](double x) {return x;};
    double tol = 1.0e-15;
    double zeta = 0.0;
    root_type r;

    // No root in bounding interval.
    r = lmi_root(f, -1.0, -1.0, tol);
    LMI_TEST(improper_bounds == r.validity);

    // Root is second bound: found on second evaluation.
    r = lmi_root(f, -1.0,  0.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 2);

    // Root found on third evaluation of a monomial.
    r = lmi_root(f, -1.0,  1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 3);

    // Root is first bound: found on first evaluation.
    r = lmi_root(f,  0.0, -1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 1);

    // Returns an error status, even though the root coincides with
    // both bounds. Attempting to find a root between identical bounds
    // is presumably an error, which should be reported immediately
    // without evaluating the objective function even once.
    r = lmi_root(f,  0.0,  0.0, tol);
    LMI_TEST(improper_bounds == r.validity);
    LMI_TEST_EQUAL(r.n_eval, 0);

    r = lmi_root(f,  0.0,  1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 1);

    r = lmi_root(f,  1.0, -1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 3);

    r = lmi_root(f,  1.0,  0.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 2);

    r = lmi_root(f,  1.0,  1.0, tol);
    LMI_TEST(improper_bounds == r.validity);

    // Repeat representative cases with decimal rounding.

    // No root in bounding interval.
    r = decimal_root(f, -0.96, -1.04, bias_none, 1);
    LMI_TEST(improper_bounds == r.validity);

    // Root is rounded second bound: found on second evaluation.
    r = decimal_root(f, -1.03,  0.04, bias_none, 1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 2);

    // Root found on third evaluation of a monomial.
    r = decimal_root(f, -1.04,  0.96, bias_none, 1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 3);

    // Root is rounded first bound: found on first evaluation.
    r = decimal_root(f,  0.04, -1.01, bias_none, 1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 1);

    // Bounds identical after rounding: presumptive error.
    r = decimal_root(f, -0.04,  0.04, bias_none, 1);
    LMI_TEST(improper_bounds == r.validity);
    LMI_TEST_EQUAL(r.n_eval, 0);

    // A curious effect of rounding the input bounds.

    // Literal   bounds [0.04, 0.09] bracket no root.
    // Effective bounds [0.0 , 0.1 ] bracket a root.
    // The exact true root, 0.0, is returned, because the literal
    // input bounds are replaced by the rounded effective bounds.
    r = decimal_root(f,  0.04,  0.09, bias_none, 1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_eval, 1);
}

void test_biases()
{
    // Test different biases.

    // Because the base of natural logarithms is transcendental,
    // Brent's algorithm terminates with distinct upper and lower
    // bounds: neither can equal the unrepresentable true value
    // (though perhaps zero is returned for some argument).
    e_functor e;

    // The last iterate evaluated is retained in member 'e.state'.
    // It is one endpoint of the final bounding interval, but not
    // necessarily the endpoint that is returned according to the
    // "bias" argument; the commented-out 'e.state' tests below serve
    // to suggest this.

    root_type r = decimal_root(e, 0.5, 5.0, bias_lower, 9);
    LMI_TEST(root_is_valid == r.validity);
    double e_or_less = r.root;
    LMI_TEST(e_or_less < std::exp(1.0));
//  LMI_TEST(e.e_state < std::exp(1.0)); // Not necessarily true.

    r = decimal_root(e, 0.5, 5.0, bias_higher, 9);
    LMI_TEST(root_is_valid == r.validity);
    double e_or_more = r.root;
    LMI_TEST(std::exp(1.0) < e_or_more);
//  LMI_TEST(std::exp(1.0) < e.e_state); // Not necessarily true.

    LMI_TEST(e_or_less < e_or_more);

    r = decimal_root(e, 0.5, 5.0, bias_none, 9);
    LMI_TEST(root_is_valid == r.validity);
    double e_more_or_less = r.root;

    LMI_TEST(e_more_or_less == e_or_less || e_more_or_less == e_or_more);

    // Various tests--see function-template definition.

    // Rounding to -100 decimals makes the maximum error 1e+100,
    // which probably isn't useful in practice.
    test_bias(0.0, 4.0e100, -100, e, std::exp(1.0), __LINE__);
    test_bias(0.0, 4.0    ,    0, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    1, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    2, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    3, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    4, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    5, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    6, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    7, e, std::exp(1.0), __LINE__);
    test_bias(0.5, 5.0    ,    8, e, std::exp(1.0), __LINE__);
    // Rounding to 100 decimals shouldn't round at all; the
    // effective maximum error is 6ϵ × e = 3.62148e-15 .
    test_bias(0.0, 4.0    ,  100, e, std::exp(1.0), __LINE__);
}

/// Test the worked-out example given here:
///   https://blogs.mathworks.com/cleve/2016/01/04/testing-zero-finders/
/// All iterates are identical for x86_64-pc-linux-gnu except where
/// marked with absolute difference as a multiple of ϵ=DBL_EPSILON:
///
///    i686-w64-mingw32     x86_64-pc-linux-gnu
///   --------lmi-------     --------lmi-------     -----mathworks----
///   2.5600000000000001     2.5600000000000001     2.5600000000000001
///   1.0980323260716793     1.0980323260716793     1.0980323260716793
///   1.7832168816106038     1.7832168816106038     1.7832168816106038
///   2.2478393639958032 -2ϵ 2.2478393639958032 -2ϵ 2.2478393639958036
///   2.0660057758331045     2.0660057758331045     2.0660057758331045
///   2.0922079131171945     2.0922079131171945     2.0922079131171945
///   2.0945566700001779     2.0945566700001779     2.0945566700001779
///   2.0945514746903116 +2ϵ 2.0945514746903111     2.0945514746903111
///   2.0945514815423065     2.0945514815423065     2.0945514815423065
///   2.0945514815423265     2.0945514815423265     2.0945514815423265
///   2.0945514815423274     2.0945514815423274     2.0945514815423274
///
/// "The reason I call x^3-2x-5=0 a celebrated equation is because it
/// was the one on which Wallis chanced to exhibit Newton's method
/// when he first published it; in consequence of which every numerical
/// solver has felt bound in duty to make it one of his examples."
///   -- De Morgan, letter to Whewell, 1861-01-20

void test_celebrated_equation()
{
    auto f = [](double x) {return x * x * x - 2.0 * x - 5.0;};
    std::ostringstream oss;
    oss.precision(17);
    root_type r = decimal_root(f, -2.56, 2.56, bias_none, 21, oss);
    LMI_TEST(root_is_valid == r.validity);
    // This constant is from the cited blog; lmi yields this,
    // which agrees to sixteen significant digits:
    //                 2.09455148154232650981
    LMI_TEST(std::fabs(2.094551481542327 - r.root) <= 1.0e-15);

#if defined LMI_X86_64 && defined LMI_POSIX
    // This is fragile, but serviceable for now.

    // "1 + " skips the newline:
    std::string const verified = 1 + R"--cut-here--(
#it #eval            a           fa            b           fb            c           fc
  0   2 i -2.5600000000000001 -16.657216000000002 2.5600000000000001 6.6572160000000018            0            0
  0   2 j -2.5600000000000001 -16.657216000000002 2.5600000000000001 6.6572160000000018 -2.5600000000000001 -16.657216000000002
  0   3 L 2.5600000000000001 6.6572160000000018 1.0980323260716793 -5.8721945393772152 -2.5600000000000001 -16.657216000000002
  1   3 j 2.5600000000000001 6.6572160000000018 1.0980323260716793 -5.8721945393772152 2.5600000000000001 6.6572160000000018
  1   4 L 1.0980323260716793 -5.8721945393772152 1.7832168816106038 -2.8960493667789873 2.5600000000000001 6.6572160000000018
  2   5 Q 1.7832168816106038 -2.8960493667789873 2.2478393639958032 1.862163113956667 2.5600000000000001 6.6572160000000018
  3   5 j 1.7832168816106038 -2.8960493667789873 2.2478393639958032 1.862163113956667 1.7832168816106038 -2.8960493667789873
  3   6 L 2.2478393639958032 1.862163113956667 2.0660057758331045 -0.3135140955237814 1.7832168816106038 -2.8960493667789873
  4   6 j 2.2478393639958032 1.862163113956667 2.0660057758331045 -0.3135140955237814 2.2478393639958032 1.862163113956667
  4   7 L 2.0660057758331045 -0.3135140955237814 2.0922079131171945 -0.026123094109737011 2.2478393639958032 1.862163113956667
  5   8 Q 2.0922079131171945 -0.026123094109737011 2.0945566700001779 5.7910818359374616e-05 2.2478393639958032 1.862163113956667
  6   8 j 2.0922079131171945 -0.026123094109737011 2.0945566700001779 5.7910818359374616e-05 2.0922079131171945 -0.026123094109737011
  6   9 L 2.0945566700001779 5.7910818359374616e-05 2.0945514746903111 -7.6478343657981895e-08 2.0922079131171945 -0.026123094109737011
  7   9 j 2.0945566700001779 5.7910818359374616e-05 2.0945514746903111 -7.6478343657981895e-08 2.0945566700001779 5.7910818359374616e-05
  7  10 L 2.0945514746903111 -7.6478343657981895e-08 2.0945514815423065 -2.2382096176443156e-13 2.0945566700001779 5.7910818359374616e-05
  8  11 Q 2.0945514815423065 -2.2382096176443156e-13 2.0945514815423265 -8.8817841970012523e-16 2.0945566700001779 5.7910818359374616e-05
  9  12 Q 2.0945514815423265 -8.8817841970012523e-16 2.0945514815423274 9.7699626167013776e-15 2.0945566700001779 5.7910818359374616e-05
 10  12 j 2.0945514815423265 -8.8817841970012523e-16 2.0945514815423274 9.7699626167013776e-15 2.0945514815423265 -8.8817841970012523e-16
 10  12 k 2.0945514815423274 9.7699626167013776e-15 2.0945514815423265 -8.8817841970012523e-16 2.0945514815423274 9.7699626167013776e-15
10 iterations, 12 evaluations; final interval:
 b +2.09455148154232650981 fb -8.88178419700125232339e-16
 c +2.09455148154232739799 fc +9.76996261670137755573e-15
 return value: 2.0945514815423265 = b
 function evaluations: 12 12 nominal, actual
 return value: 2.0945514815423265 (rounded)
)--cut-here--";

    LMI_TEST_EQUAL(verified, oss.str());
#endif // defined LMI_X86_64 && defined LMI_POSIX
}

/// Test the worked-out example given here:
///   https://en.wikipedia.org/wiki/Brent%27s_method#Example
/// which seems correct up to here:
///   "In the fourth iteration [sixth evaluation], we use inverse
///   quadratic interpolation between
///       (a3, f(a3)) = (−4, −25)           [Brent's 'c']
///   and (b2, f(b2)) = (1.14205, 0.083582) [Brent's 'a']
///   and (b3, f(b3)) = (−1.42897, 9.26891) [Brent's 'b'].
///   This yields 1.15448 [which is rejected]"
/// But |fa| <= |fb|, so a secant would transgress the bounding
/// interval, and the IQI parabola would not be single-valued in that
/// interval; therefore, Brent immediately bisects without considering
/// whether that IQI iterate is three-quarters of the way from b to c.
/// That may seem unimportant because bisection is chosen either way;
/// but later...
///   "In the sixth iteration [eight evaluation] ...
///   linear interpolation ... −2.95064"
/// ...it goes astray:
///   "But since the iterate did not change in the previous step,
///   we reject this result and fall back to bisection."
/// Brent's algorithm has no such rejection rule; it performs a linear
/// interpolation and accepts the -2.95064 result.
///
/// The last several steps have parenthetical "corrections" that are
/// invalid; they seem to have been added by another author.

void test_wikipedia_example()
{
    auto f = [](double x) {return (x + 3.0) * (x - 1.0) * (x - 1.0);};
    std::ostringstream oss;
    root_type r = decimal_root(f, -4.0, 4.0 / 3.0, bias_none, 15, oss);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(std::fabs(-3.0 - r.root) <= 1.0e-15);
    // Display this to investigate further:
//  std::cout << oss.str() << std::endl;
}

void test_various_functions()
{
    auto f00 = [](double x) {return x * x * x - 2.0 * x - 5.0;};
    auto root_00 = 2.09455148154232650981;
    test_a_decimal_function(f00, root_00, -2.56, 2.56, 17     , __LINE__, 12);
    test_a_function        (f00, root_00, -2.56, 2.56, 1.0e-15, __LINE__);

    auto f01 = [](double x) {return std::pow(x, 19);};
    auto root_01 = 0.0;
    // For now, test_a_[decimal_]function() tests that the error is
    // within tolerance, ignoring roundoff in the computed function.
    // That may very often be useful, but it can produce spurious
    // failures, as in these three commented-out lines:
//  test_a_decimal_function(f01, root_01, -1.0, 4.0, 20, __LINE__, 169);
//  test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-20, __LINE__);
//  test_a_decimal_function(f01, root_01, -1.0, 4.0, 19, __LINE__, 171);
//  test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-19, __LINE__);
//  test_a_decimal_function(f01, root_01, -1.0, 4.0, 18, __LINE__, 168);
//  test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-18, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 17, __LINE__, 149);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-17, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 16, __LINE__, 140);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-16, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 15, __LINE__, 127);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-15, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 14, __LINE__, 125);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-14, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 12, __LINE__,  93);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-12, __LINE__);

    auto f02 = [](double x) {return std::pow(x - 1.7, 17.0);};
    auto root_02 = 1.7;
    test_a_decimal_function(f02, root_02, 0.0, 2.0, 17     , __LINE__, 148);
    test_a_function        (f02, root_02, 0.0, 2.0, 1.0e-15, __LINE__);

    auto f03 = [](double x) {return std::cos(x) - 0.999;};
// This would seem preferable:
//  auto root_03 = std::acosl(0.999L);
// but gcc-10.2.0 (x86_64-pc-linux-gnu) rejects it:
//    error: ‘acosl’ is not a member of ‘std’
// despite C++20 [cmath.syn], so use this instead:
    auto root_03 = std::acos(0.999);
// Regardless of that theoretical ζ, the computed ζ′ is not exact
// to more than about fifteen decimals.
    test_a_decimal_function(f03, root_03, -0.01, 0.8, 15     , __LINE__, 16);
    test_a_function        (f03, root_03, -0.01, 0.8, 1.0e-15, __LINE__);

    // The next three examples are from _Computational Physics_,
    // Philipp O. J. Scherer, 2nd ed., ISBN 978-3-319-00400-6,
    // page 96; number of evaluations estimated from graphs as two
    // plus apparent number of iterations, to account for required
    // evaluation of both initial bounds; and separately measured
    // by writing functions based on Scherer's pseudocode (with
    // numerous corrections for his faulty Brent algorithm; use
    //   git switch --detach ac5731f52
    // to reproduce the tests with that code).
    //
    // (The LMI_* conditionals for evaluation counts may seem
    // haphazard; by design, they're just adequate to prevent error
    // messages for secondary (msw) platforms, where they probably
    // indicate either x87 discrepancies or msw library defects).

    root_type r {};

    // Scherer, Fig. 6.10, iteration counts for a 2ϵ tolerance:
    //    10              Scherer's Chandrupatla algorithm
    //    (9)             Chandrupatla estimated from Scherer's graph
    //    11 [22, x87]    Scherer's (not quite) Brent algorithm
    //   (12)             Brent estimated from Scherer's graph
    //    11              lmi_root(): Brent's method, validated
    //    63              binary64_midpoint() bisection
    auto f04 = [](double x) {return std::pow(x, 2.0) - 2.0;};
    auto root_04 = std::sqrt(2.0);
    test_a_decimal_function (f04, root_04,  -1.0, 2.0, 17     , __LINE__, 11);
    test_a_function         (f04, root_04,  -1.0, 2.0, 0.0    , __LINE__);
#if defined ac5731f52
    r = scherer_chandrupatla(f04,           -1.0, 2.0, 0.0              );
    LMI_TEST_EQUAL(10, r.n_eval);
    r = scherer_brent       (f04,           -1.0, 2.0, 0.0              );
#   if !defined LMI_X87
    LMI_TEST_EQUAL(11, r.n_eval);
#   else  // defined LMI_X87
    LMI_TEST_EQUAL(22, r.n_eval);
#   endif // defined LMI_X87
#endif // defined ac5731f52
    r = lmi_root            (f04,           -1.0, 2.0, 0.0              );
    LMI_TEST_EQUAL(11, r.n_eval);
    r = lmi_root            (f04,           -1.0, 2.0, 0.0, 0           );
    LMI_TEST_EQUAL(63, r.n_eval); // sprauchling_limit 0

    // Scherer, Fig. 6.11, iteration counts for a 2ϵ tolerance:
    //    62              Scherer's Chandrupatla algorithm
    //   (61)             Chandrupatla estimated from Scherer's graph
    //   130              Scherer's (not quite) Brent algorithm
    //  (128)             Brent estimated from Scherer's graph
    //   130              lmi_root(): Brent's method, validated
    //    62              binary64_midpoint() bisection
    auto f05 = [](double x) {return std::pow((x - 1.0), 3.0);};
    auto root_05 = 1.0;
    test_a_decimal_function (f05, root_05,   0.0, 1.8, 17     , __LINE__, 130);
    test_a_function         (f05, root_05,   0.0, 1.8, 0.0    , __LINE__);
#if defined ac5731f52
    r = scherer_chandrupatla(f05,            0.0, 1.8, 0.0              );
    LMI_TEST_EQUAL(62, r.n_eval);
    r = scherer_brent       (f05,            0.0, 1.8, 0.0              );
    LMI_TEST_EQUAL(130, r.n_eval);
#endif // defined ac5731f52
    r = lmi_root            (f05,            0.0, 1.8, 0.0              );
    LMI_TEST_EQUAL(130, r.n_eval);
    r = lmi_root            (f05,            0.0, 1.8, 0.0, 0           );
    LMI_TEST_EQUAL(62, r.n_eval); // sprauchling_limit 0

    // Scherer, Fig. 6.12, iteration counts for a 1.0e-12 tolerance
    // (roundoff error in the computed function precludes using 2ϵ):
    //    44 [45, x87]    Scherer's Chandrupatla algorithm
    //   (33)             Chandrupatla estimated from Scherer's graph
    //   105 [119, x87]   Scherer's (not quite) Brent algorithm
    //  (126)             Brent estimated from Scherer's graph
    //   117              lmi_root(): Brent's method, validated
    //     3              binary64_midpoint() bisection
    auto f06 = [](double x) {return std::pow(x, 25.0);};
    auto root_06 = 0.0;
    test_a_decimal_function (f06, root_06,  -1.0, 2.0, 12     , __LINE__, 107);
    test_a_function         (f06, root_06,  -1.0, 2.0, 5.0e-13, __LINE__);
#if defined ac5731f52
    r = scherer_chandrupatla(f06,           -1.0, 2.0, 5.0e-13          );
#   if !defined LMI_X87
    LMI_TEST_EQUAL(44, r.n_eval);
#   else  // defined LMI_X87
    LMI_TEST_EQUAL(45, r.n_eval);
#   endif // defined LMI_X87
    r = scherer_brent       (f06,           -1.0, 2.0, 5.0e-13          );
#   if defined LMI_X86_64 && defined LMI_POSIX
    LMI_TEST_EQUAL(105, r.n_eval);
#   endif // defined LMI_X86_64 && defined LMI_POSIX
#endif // defined ac5731f52
    r = lmi_root            (f06,           -1.0, 2.0, 5.0e-13          );
#if defined LMI_X86_64 && defined LMI_POSIX
    LMI_TEST_EQUAL(117, r.n_eval);
#endif // defined LMI_X86_64 && defined LMI_POSIX
    r = lmi_root            (f06,           -1.0, 2.0, 5.0e-13, 0       );
    // This is not a fair test: 0.0, an exact root, is the
    // first iterate with binary64_midpoint().
    LMI_TEST_EQUAL(3, r.n_eval); // sprauchling_limit 0

    // Despite its apparent insipidity, this is actually a very
    // interesting test: after the first iterate has been calculated
    // as -0.05 by linear interpolation, the values are:
    //   a, fa   1.9   -0.39
    //   b, fb  -0.05  -3.9975
    //   c, fc  -2.1    0.41
    // so fb and fc have the same sign while |fc| < |fb|, which is
    // an uncommon conjunction of circumstances that this unit test
    // contrives to engender. Usually the second test is met only if
    // the first is, too.
    auto f07 = [](double x) {return x * x - 4.0;};
    auto root_07 = -2.0;
    test_a_decimal_function(f07, root_07,  1.9, -2.1, 15     , __LINE__);
    test_a_decimal_function(f07, root_07, -2.1 , 1.9, 15     , __LINE__);
    test_a_function        (f07, root_07,  1.9, -2.1, 1.0e-15, __LINE__);
    test_a_function        (f07, root_07, -2.1 , 1.9, 1.0e-15, __LINE__);

    // Here, the input tolerance is legitimately much smaller than ϵ.
    // Brent describes 'tol' as positive, so it shouldn't be zero, yet
    // it has no absolute minimum (other than DBL_TRUE_MIN) because
    // it is a relative error. Thus, when ζ = 0, δ has no minimum at
    // all (other than DBL_TRUE_MIN).
    auto f08 = [](double x) {return 1.23 * (x - 1.0e-100);};
    auto root_08 = 1.0e-100;
    test_a_function(f08, root_08, 1.0e-10, 1.0e-200, 1.0e-150, __LINE__);
}

void test_hodgepodge()
{
    // Brent's book uses the nineteenth-power function in examples.
    // His example using a tolerance of 1e-20 is subject to underflow
    // on IEEE 754 hardware: distinct bounds can't be that close
    // to the exact value of unity because of machine epsilon. But
    // that doesn't matter, because a term involving machine epsilon
    // is always added to the effective tolerance. An excessively
    // low input tolerance makes the effective tolerance simply
    //   6ϵ|iterand|
    // because the other term vanishes--it does not give more
    // precision than the hardware is capable of, though it's a
    // chasing after wind that costs many evaluations.

    e_nineteenth e_19;

    // Number of evaluations:
    //   195 Brent's table 4.1 (IBM 360)
    //   171 x86_64 brent_zero (IEEE 754)
    //   169 x86_64 decimal_root (differs slightly due to rounding)
    double d = brent_zero(e_19, -1.0, 4.0, 1.0e-20);
    LMI_TEST(std::fabs(d) <= epsilon);

    root_type r = decimal_root(e_19, -1.0, 4.0, bias_none, 20);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(std::fabs(r.root) <= epsilon);
    double t = 0.5 * std::pow(10.0, -20.0);
    double zeta = 0.0;
    // (This isn't quite true:
//  LMI_TEST(1.0e-20 == max_err(zeta, t));
    // because the RHS might be 9.99999999999999945153e-21, e.g.)
    //
    // Brent's equation 2.18 gives the guaranteed maximum error as
    //   6ϵ|ζ| + 2t
    // where, because ζ is exactly zero, the ϵ term vanishes.
    // However, the result (for x86_64-pc-linux-gnu) is 5.89e-18,
    // which exceeds that guaranteed maximum error. Why?
//  LMI_TEST(std::fabs(r.root) <= max_err(zeta, t)); // fails
    // Because 5.89e-18^19 is just slightly less than DBL_TRUE_MIN,
    // so the computed function becomes zero: see the documentation
    // for max_err().

    // Assertions labelled 'weak' test the number of evaluations
    // against empirical measurements (with various architectures)
    // rather than a theoretical maximum. Perhaps they'll always
    // succeed, because floating-point behavior is determinate;
    // but small variations betoken no catastrophe.
    LMI_TEST_RELATION(153,<=,r.n_eval); // weak
    LMI_TEST_RELATION(r.n_eval,<=,166); // weak

    d = brent_zero(eq_2_1, -100.0, 100.0, 0.5);
    zeta = -100.0;
    double eq_2_1_upper = zeta + max_err(zeta, 0.5);
    LMI_TEST(-100.0 <= d && d <= eq_2_1_upper);

    r = decimal_root(eq_2_1, -100.0, 100.0, bias_none, 0);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(-100.0 <= r.root && r.root <= eq_2_1_upper);
    LMI_TEST(10 == max_n_eval_bolzano(-100.0, 100.0, 0.5, -100.0));
    LMI_TEST(98 == max_n_eval_brent  (-100.0, 100.0, 0.5, -100.0));
    LMI_TEST(r.n_eval <= 98);
    LMI_TEST_EQUAL(11, r.n_eval); // weak
    // Number of evaluations required:
    //   23 for brent_zero() [above]
    //   20 for decimal_root()
    // Presumably the difference is due to rounding.

    r = decimal_root(eq_2_1, -100.0, 100.0, bias_none, 20);
    LMI_TEST(root_is_valid == r.validity);
    // Rounding 'x' (of type 'double') to twenty decimals doesn't
    // affect its value near -100.0; the final bracketing values
    // (for x86_64-pc-linux-gnu) are
    //      x                       f(x)
    //    -99.9999999999999147349   1
    //   -100                      -6.41168279659337119941e+62
    // in whose vicinity the error term in Brent's equation 2.18
    //   |ζ′-ζ| ≤ 6ϵ|ζ| + 2t
    // with t=0.5*10^-20 becomes
    //   600e 1.33226762955018784851e-13
    //   + 2t 0.00000010000000000000e-13 (same as 1.0e-20)
    // where the ϵ term overwhelms the t term.
    t = 0.5 * std::pow(10.0, -20.0);
    LMI_TEST(-100.0 <= r.root && r.root <= zeta + max_err(zeta, t));

    LMI_TEST(  53 == max_n_eval_bolzano(-100.0, 100.0, 0.0, -100.0));
    LMI_TEST(2807 == max_n_eval_brent  (-100.0, 100.0, 0.0, -100.0));
    LMI_TEST(r.n_eval <= 2807);
    LMI_TEST_EQUAL(67, r.n_eval); // weak

    r = decimal_root(signum_offset, -1.0, 1.0, bias_none, 13);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    zeta = 1.0 / 3.0;
    double tol = 0.5 * 1.0e-13;
    LMI_TEST_EQUAL(  47, max_n_eval_bolzano(-1.0, 1.0, tol, zeta));
    LMI_TEST_EQUAL(2207, max_n_eval_brent  (-1.0, 1.0, tol, zeta));
    LMI_TEST(r.n_eval <= 2207);
    // Here, decimal_root() always chooses the bisection technique.
    LMI_TEST(46 <= r.n_eval && r.n_eval <= 47); // weak

    r = decimal_root(signum_offset, -1.0, 1.0, bias_none, 16);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    tol = 0.5 * 1.0e-16;
    LMI_TEST_EQUAL(  55, max_n_eval_bolzano(-1.0, 1.0, tol, zeta));
    LMI_TEST_EQUAL(3023, max_n_eval_brent  (-1.0, 1.0, tol, zeta));
    LMI_TEST(r.n_eval <= 3023);
    // Here, decimal_root() always chooses the bisection technique.
    LMI_TEST_EQUAL(55, r.n_eval); // weak

    r = lmi_root(signum_offset, -1.0, 1.0, 0.0);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    tol = 0.0;
    LMI_TEST_EQUAL(  55, max_n_eval_bolzano(-1.0, 1.0, tol, zeta));
    LMI_TEST_EQUAL(3023, max_n_eval_brent  (-1.0, 1.0, tol, zeta));
    LMI_TEST(r.n_eval <= 3023);
    // Here, lmi_root() always chooses the bisection technique,
    // as a secant step would transgress the bounds. This example
    // demonstrates that the ϵ/2 minimum in max_n_eval_bolzano()
    // is correct: if ϵ were not divided by two there, then the
    // "maximum" number of iterations would be 54, anomalously
    // less than the actual 55 here.
    LMI_TEST_EQUAL(55, r.n_eval); // weak

    // Here is an easier way to see that the ϵ/2 minimum is correct.
    // Consider nine equally-spaced point centered around zero:
    //   a=-4ϵ -3ϵ -2ϵ -1ϵ 0ϵ 1ϵ 2ϵ 3ϵ 4ϵ=b
    // Bisection in [a,b] requires evaluating f(a) and f(b), so there
    // are two initial evaluations. To find a root requires evaluating
    // f at 0, then at -2ϵ or 2ϵ, then at one of the odd multiples of
    // ϵ; that's three more evaluations, for a total of five.
    LMI_TEST_EQUAL(5, max_n_eval_bolzano(-4.0 * epsilon, 4.0 * epsilon, 0.0, 0.0));

    std::ostringstream oss;
    r = lmi_root(signum_offset, -1.0e300, 1.0e300, 5.0e-19, oss);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    LMI_TEST(r.n_eval <= 3023);
    LMI_TEST_EQUAL(1052, r.n_eval); // weak
    // Display this to investigate further:
//  std::cout << oss.str() << std::endl;

    // Find a root of this irksome function in 64 evaluations,
    // to maximal precision, in an enormous interval.
    r = lmi_root(signum_offset, -1.0e300, 1.0e300, 5.0e-19, 0);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    LMI_TEST_RELATION(r.n_eval,==,64);

    // Similarly test decimal_root, using a narrower interval because
    // round_to() cannot handle 1.0e300 (defectively, perhaps).
    r = decimal_root(signum_offset, -1.0e30, 1.0e30, bias_none, 16, 0);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    LMI_TEST_RELATION(r.n_eval,<=,64);
}

void test_former_rounding_problem()
{
    e_former_rounding_problem e_frp;
    root_type r = decimal_root(e_frp, 0.12609, 0.12611, bias_lower, 5);
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
}

void test_toms748()
{
    // begin test adapted from 'driver.f'
    // (scoped to sequester an oddly imprecise 'pi')
    {
    auto f = [](double x) {return std::sin(x) - x / 2.0;};
    /* Local variables */
    double a, b, eps;
    int neps;
    double root;
    int nprob;

    int    n_eval {0};

/* THIS PROGRAM CALCULATES A ROOT OF A CONTINUOUS FUNCTION F(X) */
/* IN AN INTERVAL I=[A, B] PROVIDED THAT F(A)F(B)<0. THE FUNCTION F(X) */
/* AND THE INITIAL INTERVAL [A, B] ARE TO BE SUPPLIED BY THE USER IN */
/* THE SUBROUTINES "FUNC" AND "INIT". THE OUTPUT "ROOT" EITHER SATISFIES */
/* THAT F(ROOT)=0 OR IS AN APPROXIMATE SOLUTION OF THE EQUATION F(X)=0 */
/* SUCH THAT "ROOT" IS INCLUDED IN AN INTERVAL [AC, BC] WITH */
/*      F(AC)F(BC)<0, */
/* AND */
/*      BC-AC <= TOL = 2*TOLE(AC,BC). */
/* PRECISION CHOSEN AS THE RELATIVE MACHINE PRECISION, */
/* AND "UC" IS EQUAL TO EITHER "AC" OR "BC" WITH THE CONDITION */
/*      |F(UC)| = MIN{ |F(AC)|, |F(BC)| }. */

/* INPUT OF THE PROGRAM: */
/*  NPROB -- INTEGER. POSITIVE INTEGER STANDING FOR "NUMBER OF PROBLEM", */
/*           INDICATING THE PROBLEM TO BE SOLVED. */
/*  N     -- PROBLEM DEPENDENT PARAMETER */
/* OUTPUT OF THE PROGRAM: */
/*  ROOT  -- DOUBLE PRECISION. EXACT OR APPROXIMATE SOLUTION OF THE */
/*           EQUATION F(X)=0. */

nprob = 1;
a = 0;
b = 1;

/* USE MACHINE PRECISION */

    rmp_(&eps);
    neps = 1000;
// TOMS748 calculation matches DBL_EPSILON:
//std::cout << eps << " = calculated ϵ" << std::endl;
//std::cout << DBL_EPSILON << " = DBL_EPSILON" << std::endl;

/* CALL SUBROUTINE "INIT" TO GET THE INITIAL INTERVAL. */

// test problem #1 bounds (hardcoded)
    double pi;
    pi = 3.1416; // How very odd to use such a coarse approximation!
    a = pi / 2.;
    b = pi;

/* CALL SUBROUTINE "RROOT" TO HAVE THE PROBLEM SOLVED. */

    rroot_(f, &nprob, &neps, &eps, &a, &b, &root, &n_eval);

/* PRINT OUT THE RESULT ON THE SCREEN. */

//  s_stop("", (ftnlen)0);
//std::cout << "Number of evaluations = " << n_eval << std::endl;
//std::cout.precision(21);
//std::cout << "Computed root = " << root << std::endl;
    } // end test adapted from 'driver.f'

    constexpr double pi {3.14159265358979323851};

    double bound0   {pi / 2.0};
    double bound1   {pi};
    int    decimals {7};

    double const tol = 0.5 * std::pow(10.0, -decimals);

    auto f = [](double x) {return std::sin(x) - x / 2.0;};

    std::cout.precision(21);

    root_type r = lmi_root(f, bound0, bound1, 0.0);
    double const validated = r.root;
    std::cout
        << "high-precision value " << validated
        << "; observed error " << f(validated)
        << std::endl
        ;

    r = lmi_root(f, bound0, bound1, tol);
    std::cout
        << "lmi_root()    : root " << r.root
        << " #eval " << r.n_eval
        << std::endl
        ;

    r = decimal_root(f, bound0, bound1, bias_none, decimals);
    std::cout
        << "decimal_root(): root " << r.root
        << " #eval " << r.n_eval
        << std::endl
        ;

    r = toms748_root(f, bound0, bound1, bias_none, decimals);
    std::cout
        << "TOMS748       : root " << r.root
        << " #eval " << r.n_eval
        << "\n                             ^"
        << "\n    doesn't round to 1.8954943,"
        << "\n  but within        ±0.00000005 of true root:"
        << "\n      TOMS748   " << r.root
        << "\n    - validated " << validated
        << "\n    = error     " << std::fixed << std::fabs(r.root - validated)
        << "\n              < 0.00000005"
        << std::endl
        ;
}

/// TOMS 748 test suite.
///
/// Alefeld et al. present fifteen numbered problems in Table I,
/// which expand to twenty-eight numbered problems indexed by 'NPROB'
/// in their FORTRAN. A total of 154 tests results from the outer
/// product of these problems and a variable parameter 'n'.
///
/// They ran their tests on an "AT&T 3B2-1000 Model 80". The WE32000
/// doesn't have hardware floating point. They don't say what kind of
/// coprocessor was used, if any; most likely it would have been a
/// WE32106, which was designed to conform to a contemporary draft of
/// IEEE 754 and seems to have been similar to x87, notably performing
/// all floating-point calculations in 96-bit extended precision.
/// However, they say "macheps is the relative machine precision which
/// in our case is 1.9073486328 x 10[e]-16", as opposed to 2.22e-16
/// for IEEE 754 binary64, so presumably they had no floating-point
/// hardware. (But their "macheps" isn't even an integral power of
/// two, so maybe that's just an error from which no conclusion can
/// be drawn.)
///
/// The number of evaluations in Table II ("BR" column = Brent) is
/// nearly reproduced:
///
///    tol    x87  Alefeld  x86_64
///   1e-07  2809    2804    2807
///   1e-10  2909    2905    2907
///   1e-15  3015    2975    2974
///   0      3038    3008    2991
///
/// especially for x86_64. This would seem to suggest that Alefeld
/// et al. used a machine without a WE32106 coprocessor, which should
/// have been closer to the x87 values above.
///
/// The roots presented in their 'testout' file are limited to
/// fourteen significant decimals. Those limited-precision roots are
/// given as comments below, next to more precise values that are
/// given to DECIMAL_DIG figures for reproducibility although of
/// course only DBL_DIG are accurate.

void test_alefeld_examples(int alefeld_count, double tol)
{
    double pi_alefeld {3.1416e0}; // This is the value Alefeld uses.

    double bound0 {};
    double bound1 {};

    double n {0.0};

    int n_eval {};

    // Table I #1 = FORTRAN #1
    auto f01n01 = [](double x) {return std::sin(x) - x / 2.0;};
    // Alefeld:     1.8954942670340;
    double r01n01 = 1.89549426703398093963;
    bound0 = pi_alefeld / 2.0e0;
    bound1 = pi_alefeld;
    n_eval += test_a_function(f01n01, r01n01, bound0, bound1, tol, __LINE__);

    // Table I #2 = FORTRAN #2-11
    auto f02 = [](double x)
        {
        double sum = 0.0;
        for(int i = 1; i <= 20; ++i)
            {
            sum += std::pow(2.0 * i - 5.0, 2.0) / std::pow(x - i * i, 3.0);
            }
        return -2.0 * sum;
        };
    // Alefeld:     3.0229153472731;
    double r02n02 = 3.0229153472730572183;
    bound0 =  1.0e0 *  1.0e0 + 1.0e-9;
    bound1 =  2.0e0 *  2.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n02, bound0, bound1, tol, __LINE__);

    // Test the same function with different intervals.

    // Alefeld:     6.6837535608081
    double r02n03 = 6.68375356080807847547;
    bound0 =  2.0e0 *  2.0e0 + 1.0e-9;
    bound1 =  3.0e0 *  3.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n03, bound0, bound1, tol, __LINE__);

    // Alefeld:     11.238701655002
    double r02n04 = 11.2387016550022114103;
    bound0 =  3.0e0 *  3.0e0 + 1.0e-9;
    bound1 =  4.0e0 *  4.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n04, bound0, bound1, tol, __LINE__);

    // Alefeld:     19.676000080623
    double r02n05 = 19.6760000806234103266;
    bound0 =  4.0e0 *  4.0e0 + 1.0e-9;
    bound1 =  5.0e0 *  5.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n05, bound0, bound1, tol, __LINE__);

    // Alefeld:     29.828227326505
    double r02n06 = 29.8282273265047557231;
    bound0 =  5.0e0 *  5.0e0 + 1.0e-9;
    bound1 =  6.0e0 *  6.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n06, bound0, bound1, tol, __LINE__);

    // Alefeld:     41.906116195289
    double r02n07 = 41.9061161952894138949;
    bound0 =  6.0e0 *  6.0e0 + 1.0e-9;
    bound1 =  7.0e0 *  7.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n07, bound0, bound1, tol, __LINE__);

    // Alefeld:     55.953595800143
    double r02n08 = 55.95359580014309131;
    bound0 =  7.0e0 *  7.0e0 + 1.0e-9;
    bound1 =  8.0e0 *  8.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n08, bound0, bound1, tol, __LINE__);

    // Alefeld:     71.985665586588
    double r02n09 = 71.9856655865877996803;
    bound0 =  8.0e0 *  8.0e0 + 1.0e-9;
    bound1 =  9.0e0 *  9.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n09, bound0, bound1, tol, __LINE__);

    // Alefeld:     90.008868539167
    double r02n10 = 90.0088685391666700752;
    bound0 =  9.0e0 *  9.0e0 + 1.0e-9;
    bound1 = 10.0e0 * 10.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n10, bound0, bound1, tol, __LINE__);

    // Alefeld:     110.02653274833
    double r02n11 = 110.026532748330197364;
    bound0 = 10.0e0 * 10.0e0 + 1.0e-9;
    bound1 = 11.0e0 * 11.0e0 - 1.0e-9;
    n_eval += test_a_function(f02, r02n11, bound0, bound1, tol, __LINE__);

    // Table I #3 = FORTRAN #12-14
    auto f03n12 = [](double x) {return  -40.0 * x * std::exp(-1.0 * x);};
    // Alefeld:  0.0;
    double r03 = 0.0;
    bound0 = -9.0e0;
    bound1 = 31.0e0;
    n_eval += test_a_function(f03n12, r03, bound0, bound1, tol, __LINE__);

    auto f03n13 = [](double x) {return -100.0 * x * std::exp(-2.0 * x);};
    n_eval += test_a_function(f03n13, r03, bound0, bound1, tol, __LINE__);

    auto f03n14 = [](double x) {return -200.0 * x * std::exp(-3.0 * x);};
    n_eval += test_a_function(f03n14, r03, bound0, bound1, tol, __LINE__);

    // Table I #4 = FORTRAN #15-17
    //
    // For the set of problems identified as #4 in Alefeld's table I,
    // there are really two variable parameters, 'n' and 'a'. The urge
    // to treat 'a' as a variable is resisted to avoid confusion, as
    // 'a' is both a parameter and a bound in the "[a,b]" column.
    auto f04n15 = [&n](double x) {return std::pow(x, n) - 0.2;};
    n = 4.0;
    // Alefeld:      0.66874030497642
    double r04n15a = 0.668740304976422006433;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n15, r04n15a, bound0, bound1, tol, __LINE__);

    n = 6.0;
    // Alefeld:      0.76472449133173
    double r04n15b = 0.764724491331730038546;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n15, r04n15b, bound0, bound1, tol, __LINE__);

    n = 8.0;
    // Alefeld:      0.81776543395794
    double r04n15c = 0.817765433957942544652;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n15, r04n15c, bound0, bound1, tol, __LINE__);

    n = 10.0;
    // Alefeld:      0.85133992252078
    double r04n15d = 0.851339922520784608828;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n15, r04n15d, bound0, bound1, tol, __LINE__);

    n = 12.0;
    // Alefeld:      0.87448527222117
    double r04n15e = 0.874485272221167897477;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n15, r04n15e, bound0, bound1, tol, __LINE__);

    auto f04n16 = [n](double x) {return std::pow(x, n) - 1.0;};
    n = 4.0;
    // Alefeld:      1.0000000000000
    double r04n16a = 1.0;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n16, r04n16a, bound0, bound1, tol, __LINE__);

    n = 6.0;
    // Alefeld:      1.0000000000000
    double r04n16b = 1.0;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n16, r04n16b, bound0, bound1, tol, __LINE__);

    n = 8.0;
    // Alefeld:      1.0000000000000
    double r04n16c = 1.0;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n16, r04n16c, bound0, bound1, tol, __LINE__);

    n = 10.0;
    // Alefeld:      1.0000000000000
    double r04n16d = 1.0;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n16, r04n16d, bound0, bound1, tol, __LINE__);

    n = 12.0;
    // Alefeld:      1.0000000000000
    double r04n16e = 1.0;
    bound0 = 0.0e0;
    bound1 = 5.0e0;
    n_eval += test_a_function(f04n16, r04n16e, bound0, bound1, tol, __LINE__);

    auto f04n17 = [n](double x) {return std::pow(x, n) - 1.0;};
    n = 8.0;
    // Alefeld:      1.0000000000000
    double r04n17a = 1.0;
    bound0 = -0.95e0;
    bound1 =  4.05e0;
    n_eval += test_a_function(f04n17, r04n17a, bound0, bound1, tol, __LINE__);

    n = 10.0;
    // Alefeld:      1.0000000000000
    double r04n17b = 1.0;
    bound0 = -0.95e0;
    bound1 =  4.05e0;
    n_eval += test_a_function(f04n17, r04n17b, bound0, bound1, tol, __LINE__);

    n = 12.0;
    // Alefeld:      1.0000000000000
    double r04n17c = 1.0;
    bound0 = -0.95e0;
    bound1 =  4.05e0;
    n_eval += test_a_function(f04n17, r04n17c, bound0, bound1, tol, __LINE__);

    n = 14.0;
    // Alefeld:      1.0000000000000
    double r04n17d = 1.0;
    bound0 = -0.95e0;
    bound1 =  4.05e0;
    n_eval += test_a_function(f04n17, r04n17d, bound0, bound1, tol, __LINE__);

    // Table I #5 = FORTRAN #18
    auto f05n18 = [](double x) {return std::sin(x) - 0.5;};
    // Alefeld:     0.52359877559830;
    double r05n18 = 0.523598775598298815659;
    bound0 = 0.0e0;
    bound1 = 1.5e0;
    n_eval += test_a_function(f05n18, r05n18, bound0, bound1, tol, __LINE__);

    // Table I #6 = FORTRAN #19
    auto f06n19 = [&n](double x)
        {
        return 2.0 * x * std::exp(-n) - 2.0 * std::exp(-n * x) + 1.0;
        };
    n = 1.0;
    // Alefeld:      0.42247770964124
    double r06n19a = 0.422477709641236709448;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f06n19, r06n19a, bound0, bound1, tol, __LINE__);

    n = 2.0;
    // Alefeld:      0.30669941048320
    double r06n19b = 0.306699410483203704914;
    n_eval += test_a_function(f06n19, r06n19b, bound0, bound1, tol, __LINE__);

    n = 3.0;
    // Alefeld:      0.22370545765466
    double r06n19c = 0.223705457654662959177;
    n_eval += test_a_function(f06n19, r06n19c, bound0, bound1, tol, __LINE__);

    n = 4.0;
    // Alefeld:      0.17171914751951
    double r06n19d = 0.171719147519508369415;
    n_eval += test_a_function(f06n19, r06n19d, bound0, bound1, tol, __LINE__);

    n = 5.0;
    // Alefeld:      0.13825715505682
    double r06n19e = 0.13825715505682406592;
    n_eval += test_a_function(f06n19, r06n19e, bound0, bound1, tol, __LINE__);

    n = 20.0;
    // Alefeld:        3.4657359020854e-02
    double r06n19f = 0.0346573590208538451218;
    n_eval += test_a_function(f06n19, r06n19f, bound0, bound1, tol, __LINE__);

    n = 40.0;
    // Alefeld:        1.7328679513999e-02
    double r06n19g = 0.0173286795139986349312;
    n_eval += test_a_function(f06n19, r06n19g, bound0, bound1, tol, __LINE__);

    n = 60.0;
    // Alefeld:        1.1552453009332e-02
    double r06n19h = 0.0115524530093324209745;
    n_eval += test_a_function(f06n19, r06n19h, bound0, bound1, tol, __LINE__);

    n = 80.0;
    // Alefeld:         8.6643397569993e-03
    double r06n19i = 0.00866433975699931746561;
    n_eval += test_a_function(f06n19, r06n19i, bound0, bound1, tol, __LINE__);

    n = 100.0;
    // Alefeld:         6.9314718055995e-03
    double r06n19j = 0.00693147180559945241124;
    n_eval += test_a_function(f06n19, r06n19j, bound0, bound1, tol, __LINE__);

    // Table I #7 = FORTRAN #20
    auto f07n20 = [&n](double x)
        {
        return
              (1.0 + std::pow(1.0 - n, 2.0)) * x
            - std::pow((1.0 - n * x), 2.0)
            ;
        };
    n = 5.0;
    // Alefeld:        3.8402551840622e-02
    double r07n20a = 0.0384025518406218985268;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f07n20, r07n20a, bound0, bound1, tol, __LINE__);

    n = 10.0;
    // Alefeld:         9.9000099980005e-03
    double r07n20b = 0.00990000999800050122956;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f07n20, r07n20b, bound0, bound1, tol, __LINE__);

    n = 20.0;
    // Alefeld:         2.4937500390620e-03
    double r07n20c = 0.00249375003906201174464;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f07n20, r07n20c, bound0, bound1, tol, __LINE__);

    // Table I #8 = FORTRAN #21
    auto f08n21 = [&n](double x)
        {
        return std::pow(x, 2.0) - std::pow(1.0 - x, n);
        };
    n =  2.0;
    // Alefeld:      0.50000000000000
    double r08n21a = 0.5;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f08n21, r08n21a, bound0, bound1, tol, __LINE__);

    n =  5.0;
    // Alefeld:      0.34595481584824
    double r08n21b = 0.345954815848241947762;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f08n21, r08n21b, bound0, bound1, tol, __LINE__);

    n = 10.0;
    // Alefeld:      0.24512233375331
    double r08n21c = 0.245122333753307247717;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f08n21, r08n21c, bound0, bound1, tol, __LINE__);

    n = 15.0;
    // Alefeld:      0.19554762353657
    double r08n21d = 0.19554762353656562901;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f08n21, r08n21d, bound0, bound1, tol, __LINE__);

    n = 20.0;
    // Alefeld:      0.16492095727644
    double r08n21e = 0.164920957276440960371;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f08n21, r08n21e, bound0, bound1, tol, __LINE__);

    // Table I #9 = FORTRAN #22
    auto f09n22 = [&n](double x)
        {
        return (1.0 + std::pow(1.0 - n, 4.0)) * x - std::pow(1.0 - n * x, 4.0);
        };
    n =  1.0;
    // Alefeld:      0.27550804099948
    double r09n22a = 0.27550804099948439374;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f09n22, r09n22a, bound0, bound1, tol, __LINE__);

    n =  2.0;
    // Alefeld:      0.13775402049974
    double r09n22b = 0.13775402049974219687;
    n_eval += test_a_function(f09n22, r09n22b, bound0, bound1, tol, __LINE__);

    n =  4.0;
    // Alefeld:        1.0305283778156e-02
    double r09n22c = 0.0103052837781564439468;
    n_eval += test_a_function(f09n22, r09n22c, bound0, bound1, tol, __LINE__);

    n =  5.0;
    // Alefeld:         3.6171081789041e-03
    double r09n22d = 0.00361710817890406339387;
    n_eval += test_a_function(f09n22, r09n22d, bound0, bound1, tol, __LINE__);

    n =  8.0;
    // Alefeld:          4.1087291849640e-04
    double r09n22e = 0.000410872918496395320848;
    n_eval += test_a_function(f09n22, r09n22e, bound0, bound1, tol, __LINE__);

    n = 15.0;
    // Alefeld:      2.5989575892908e-05
    double r09n22f = 2.59895758929076292133e-05;
    n_eval += test_a_function(f09n22, r09n22f, bound0, bound1, tol, __LINE__);

    n = 20.0;
    // Alefeld:      7.6685951221853e-06
    double r09n22g = 7.66859512218533888794e-06;
    n_eval += test_a_function(f09n22, r09n22g, bound0, bound1, tol, __LINE__);

    // Table I #10 = FORTRAN #23
    auto f10n23 = [&n](double x)
        {
        return std::exp(-n * x) * (x - 1.0) + std::pow(x, n);
        };
    n =  1.0;
    // Alefeld:      0.40105813754155
    double r10n23a = 0.401058137541547010674;
    bound0 = 0.0e0;
    bound1 = 1.0e0;
    n_eval += test_a_function(f10n23, r10n23a, bound0, bound1, tol, __LINE__);

    n =  5.0;
    // Alefeld:      0.51615351875793
    double r10n23b = 0.516153518757933582606;
    n_eval += test_a_function(f10n23, r10n23b, bound0, bound1, tol, __LINE__);

    n = 10.0;
    // Alefeld:      0.53952222690842
    double r10n23c = 0.539522226908415780677;
    n_eval += test_a_function(f10n23, r10n23c, bound0, bound1, tol, __LINE__);

    n = 15.0;
    // Alefeld:      0.54818229434066
    double r10n23d = 0.548182294340655240639;
    n_eval += test_a_function(f10n23, r10n23d, bound0, bound1, tol, __LINE__);

    n = 20.0;
    // Alefeld:      0.55270466667849
    double r10n23e = 0.552704666678487832598;
    n_eval += test_a_function(f10n23, r10n23e, bound0, bound1, tol, __LINE__);

    // Table I #11 = FORTRAN #24
    auto f11n24 = [&n](double x)
        {
        return (n * x - 1.0) / ((n - 1.0) * x);
        };
    n =  2.0;
    // Alefeld:      0.50000000000000
    double r11n24a = 0.5;
    bound0 = 1.0e-2;
    bound1 = 1.0e0;
    n_eval += test_a_function(f11n24, r11n24a, bound0, bound1, tol, __LINE__);

    n =  5.0;
    // Alefeld:      0.20000000000000
    double r11n24b = 0.2;
    n_eval += test_a_function(f11n24, r11n24b, bound0, bound1, tol, __LINE__);

    n = 15.0;
    // Alefeld:        6.6666666666667e-02
    double r11n24c = 0.066666666666666667;
    n_eval += test_a_function(f11n24, r11n24c, bound0, bound1, tol, __LINE__);

    n = 20.0;
    // Alefeld:         5.0000000000000e-02
    double r11n24d = 0.05;
    n_eval += test_a_function(f11n24, r11n24d, bound0, bound1, tol, __LINE__);

    // Table I #12 = FORTRAN #25
    //
    // Presumably due to inaccuracy in std::pow(), the calculated
    // zero differs from the theoretical one (equal to 'n') by at
    // least one ulp for some values of 'n'.
    auto f12n25 = [&n](double x)
        {
        return std::pow(x, 1.0 / n) - std::pow(n, 1.0 / n);
        };
    n =  2.0;
    // Alefeld:       2.0000000000000
    double r12n25a =  2.0;
    bound0 =   1.0e0;
    bound1 = 100.0e0;
    n_eval += test_a_function(f12n25, r12n25a, bound0, bound1, tol, __LINE__);

    n =   3.0;
    // Alefeld:       3.0000000000000
    double r12n25b =  3.0;
    n_eval += test_a_function(f12n25, r12n25b, bound0, bound1, tol, __LINE__);

    n =   4.0;
    // Alefeld:       4.0000000000000
    double r12n25c =  4.0;
    n_eval += test_a_function(f12n25, r12n25c, bound0, bound1, tol, __LINE__);

    n =   5.0;
    // Alefeld:       5.0000000000000
    double r12n25d =  5.0;
    n_eval += test_a_function(f12n25, r12n25d, bound0, bound1, tol, __LINE__);

    n =   6.0;
    // Alefeld:       6.0000000000000
    double r12n25e =  6.0;
    n_eval += test_a_function(f12n25, r12n25e, bound0, bound1, tol, __LINE__);

    n =   7.0;
    // Alefeld:       7.0000000000000
    double r12n25f =  7.0;
    n_eval += test_a_function(f12n25, r12n25f, bound0, bound1, tol, __LINE__);

    n =   9.0;
    // Alefeld:       9.0000000000000
    double r12n25g =  9.0;
    n_eval += test_a_function(f12n25, r12n25g, bound0, bound1, tol, __LINE__);

    n =  11.0;
    // Alefeld:      11.000000000000
    double r12n25h = 11.0;
    n_eval += test_a_function(f12n25, r12n25h, bound0, bound1, tol, __LINE__);

    n =  13.0;
    // Alefeld:      13.000000000000
    double r12n25i = 13.0;
    n_eval += test_a_function(f12n25, r12n25i, bound0, bound1, tol, __LINE__);

    n =  15.0;
    // Alefeld:      15.000000000000
    double r12n25j = 15.0;
    n_eval += test_a_function(f12n25, r12n25j, bound0, bound1, tol, __LINE__);

    n =  17.0;
    // Alefeld:      17.000000000000
    double r12n25k = 16.9999999999999715783;
    n_eval += test_a_function(f12n25, r12n25k, bound0, bound1, tol, __LINE__);

    n =  19.0;
    // Alefeld:      19.000000000000
    double r12n25l = 19.0;
    n_eval += test_a_function(f12n25, r12n25l, bound0, bound1, tol, __LINE__);

    n =  21.0;
    // Alefeld:      21.000000000000
    double r12n25m = 21.0000000000000355271;
    n_eval += test_a_function(f12n25, r12n25m, bound0, bound1, tol, __LINE__);

    n =  23.0;
    // Alefeld:      23.000000000000
    double r12n25n = 23.0000000000000568434;
    n_eval += test_a_function(f12n25, r12n25n, bound0, bound1, tol, __LINE__);

    n =  25.0;
    // Alefeld:      25.000000000000
    double r12n25o = 25.000000000000024869;
    n_eval += test_a_function(f12n25, r12n25o, bound0, bound1, tol, __LINE__);

    n =  27.0;
    // Alefeld:      27.000000000000
    double r12n25p = 26.9999999999999573674;
    n_eval += test_a_function(f12n25, r12n25p, bound0, bound1, tol, __LINE__);

    n =  29.0;
    // Alefeld:      29.000000000000
    double r12n25q = 28.9999999999999076294;
    n_eval += test_a_function(f12n25, r12n25q, bound0, bound1, tol, __LINE__);

    n =  31.0;
    // Alefeld:      31.000000000000
    double r12n25r = 31.0000000000000568434;
    n_eval += test_a_function(f12n25, r12n25r, bound0, bound1, tol, __LINE__);

    n =  33.0;
    // Alefeld:      33.000000000000
    double r12n25s = 33.0000000000000852651;
    n_eval += test_a_function(f12n25, r12n25s, bound0, bound1, tol, __LINE__);

    // Table I #13 = FORTRAN #26
    //
    // This is a dodgy test that requires special handling.
    //
    // Alefeld says: "If we code xe^-x^-2 in Fortran 77 as x(e^-1/x^2)
    // then all algorithms that solve this problem within 1000 itera-
    // tions deliver values around 0.02 as the exact solution, because
    // the result of the computation of 0.02(e^-1/0.02^2) on our
    // machine is equal to 0. However, when we code xe^-x^-2 as
    // x/e^(1/x^2), all algorithms give correct solutions." However,
    // while the FORTRAN code that accompanies the paper uses the
    // recommended formula:
    //   FX=X/DEXP(1.0D0/(X*X))
    // the accompanying 'testout' file gives the root as
    //   COMPUTED ROOT =    2.2317679157465D-02
    // which demonstrates exactly the difficulty that was to be
    // avoided. A similar outcome is seen with a 'long double'
    // calculation (which of course varies by architecture).
    //
    // Therefore, whereas all the other TOMS 748 tests here invoke
    // test_a_function(), this one merely invokes lmi_root(). It
    // records the number of evaluations, without which Alefeld's
    // totals in Table I could not be (approximately) reproduced,
    // and ignores the meaningless false root.
    auto f13n26 = [](double x)
        {
        return
              (0.0 == x)
            ? 0.0
            // Alefeld recommends against this:
        //  : x * std::exp(-std::pow(x, -2.0))
            //   which returns 0.0154471909812447585897
            // Instead, Alefeld recommends:
            : x / std::exp(1.0 / (x * x))
            //   which returns 0.0151677361684193559577
            // Even this:
        //  : static_cast<double>(x / std::exp(1.0L / (x * x)))
            //   which returns 0.0152105118173455761132
            // returns an artifact of underflow rather than a
            // true root.
            ;
        };
    // Alefeld:     2.2317679157465e-02 [not a true root]
#if 0
#   if !defined LMI_X87
    double r13n26 = 0.0151677361684193559577;
#   else  // defined LMI_X87
    double r13n26 = 0.0156404232768098810924;
#   endif // defined LMI_X87
#endif // 0
    bound0 = -1.0e0;
    bound1 =  4.0e0;
#if 0
    n_eval += test_a_function(f13n26, r13n26, bound0, bound1, tol, __LINE__);
#endif // 0
    root_type r = lmi_root(f13n26, bound0, bound1, tol);
    n_eval += r.n_eval;

    // Table I #14 = FORTRAN #27
    auto f14n27 = [&n](double x)
        {
        return
              (0.0 <= x)
            ? (n / 20.0) * (x / 1.5 + std::sin(x) - 1.0)
            : -n / 20.0
            ;
        };
    n =  1.0;
    // Alefeld:      0.62380651896161
    double r14n27a = 0.623806518961612321839;
    bound0 = -10000;
    bound1 = pi_alefeld / 2.0e0;
    n_eval += test_a_function(f14n27, r14n27a, bound0, bound1, tol, __LINE__);

    n =  2.0;
    // Alefeld:      0.62380651896161
    double r14n27b = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27b, bound0, bound1, tol, __LINE__);

    n =  3.0;
    // Alefeld:      0.62380651896161
    double r14n27c = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27c, bound0, bound1, tol, __LINE__);

    n =  4.0;
    // Alefeld:      0.62380651896161
    double r14n27d = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27d, bound0, bound1, tol, __LINE__);

    n =  5.0;
    // Alefeld:      0.62380651896161
    double r14n27e = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27e, bound0, bound1, tol, __LINE__);

    n =  6.0;
    // Alefeld:      0.62380651896161
    double r14n27f = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27f, bound0, bound1, tol, __LINE__);

    n =  7.0;
    // Alefeld:      0.62380651896161
    double r14n27g = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27g, bound0, bound1, tol, __LINE__);

    n =  8.0;
    // Alefeld:      0.62380651896161
    double r14n27h = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27h, bound0, bound1, tol, __LINE__);

    n =  9.0;
    // Alefeld:      0.62380651896161
    double r14n27i = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27i, bound0, bound1, tol, __LINE__);

    n = 10.0;
    // Alefeld:      0.62380651896161
    double r14n27j = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27j, bound0, bound1, tol, __LINE__);

    n = 11.0;
    // Alefeld:      0.62380651896161
    double r14n27k = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27k, bound0, bound1, tol, __LINE__);

    n = 12.0;
    // Alefeld:      0.62380651896161
    double r14n27l = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27l, bound0, bound1, tol, __LINE__);

    n = 13.0;
    // Alefeld:      0.62380651896161
    double r14n27m = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27m, bound0, bound1, tol, __LINE__);

    n = 14.0;
    // Alefeld:      0.62380651896161
    double r14n27n = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27n, bound0, bound1, tol, __LINE__);

    n = 15.0;
    // Alefeld:      0.62380651896161
    double r14n27o = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27o, bound0, bound1, tol, __LINE__);

    n = 16.0;
    // Alefeld:      0.62380651896161
    double r14n27p = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27p, bound0, bound1, tol, __LINE__);

    n = 17.0;
    // Alefeld:      0.62380651896161
    double r14n27q = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27q, bound0, bound1, tol, __LINE__);

    n = 18.0;
    // Alefeld:      0.62380651896161
    double r14n27r = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27r, bound0, bound1, tol, __LINE__);

    n = 19.0;
    // Alefeld:      0.62380651896161
    double r14n27s = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27s, bound0, bound1, tol, __LINE__);

    n = 20.0;
    // Alefeld:      0.62380651896161
    double r14n27t = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27t, bound0, bound1, tol, __LINE__);

    n = 21.0;
    // Alefeld:      0.62380651896161
    double r14n27u = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27u, bound0, bound1, tol, __LINE__);

    n = 22.0;
    // Alefeld:      0.62380651896161
    double r14n27v = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27v, bound0, bound1, tol, __LINE__);

    n = 23.0;
    // Alefeld:      0.62380651896161
    double r14n27w = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27w, bound0, bound1, tol, __LINE__);

    n = 24.0;
    // Alefeld:      0.62380651896161
    double r14n27x = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27x, bound0, bound1, tol, __LINE__);

    n = 25.0;
    // Alefeld:      0.62380651896161
    double r14n27y = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27y, bound0, bound1, tol, __LINE__);

    n = 26.0;
    // Alefeld:      0.62380651896161
    double r14n27z = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27z, bound0, bound1, tol, __LINE__);

    n = 27.0;
    // Alefeld:      0.62380651896161
    double r14n27A = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27A, bound0, bound1, tol, __LINE__);

    n = 28.0;
    // Alefeld:      0.62380651896161
    double r14n27B = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27B, bound0, bound1, tol, __LINE__);

    n = 29.0;
    // Alefeld:      0.62380651896161
    double r14n27C = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27C, bound0, bound1, tol, __LINE__);

    n = 30.0;
    // Alefeld:      0.62380651896161
    double r14n27D = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27D, bound0, bound1, tol, __LINE__);

    n = 31.0;
    // Alefeld:      0.62380651896161
    double r14n27E = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27E, bound0, bound1, tol, __LINE__);

    n = 32.0;
    // Alefeld:      0.62380651896161
    double r14n27F = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27F, bound0, bound1, tol, __LINE__);

    n = 33.0;
    // Alefeld:      0.62380651896161
    double r14n27G = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27G, bound0, bound1, tol, __LINE__);

    n = 34.0;
    // Alefeld:      0.62380651896161
    double r14n27H = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27H, bound0, bound1, tol, __LINE__);

    n = 35.0;
    // Alefeld:      0.62380651896161
    double r14n27I = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27I, bound0, bound1, tol, __LINE__);

    n = 36.0;
    // Alefeld:      0.62380651896161
    double r14n27J = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27J, bound0, bound1, tol, __LINE__);

    n = 37.0;
    // Alefeld:      0.62380651896161
    double r14n27K = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27K, bound0, bound1, tol, __LINE__);

    n = 38.0;
    // Alefeld:      0.62380651896161
    double r14n27L = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27L, bound0, bound1, tol, __LINE__);

    n = 39.0;
    // Alefeld:      0.62380651896161
    double r14n27M = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27M, bound0, bound1, tol, __LINE__);

    n = 40.0;
    // Alefeld:      0.62380651896161
    double r14n27N = 0.623806518961612321839;
    n_eval += test_a_function(f14n27, r14n27N, bound0, bound1, tol, __LINE__);

    // Table I #15 = FORTRAN #28
    auto f15n28 = [&n](double x)
        {
        double const k = 0.002 / (1.0 + n);
        return
            // FORTRAN says 'k<x', but Alefeld says 'k<=x'
              (k < x  ) ? std::exp(1.0) - 1.859
            : (x < 0.0) ?                -0.859
            :             std::exp((n + 1.0) * x / .002) - 1.859
            ;
        };
    n =   20.0;
    // Alefeld:      5.9051305594220e-05
    double r15n28a = 5.90513055942197166237e-05;
    bound0 = -10000;
    bound1 = 1.0e-4;
    n_eval += test_a_function(f15n28, r15n28a, bound0, bound1, tol, __LINE__);

    n =  21.0;
    // Alefeld:      5.6367155339937e-05
    double r15n28b = 5.63671553399369966875e-05;
    n_eval += test_a_function(f15n28, r15n28b, bound0, bound1, tol, __LINE__);

    n =  22.0;
    // Alefeld:      5.3916409455592e-05
    double r15n28c = 5.39164094555919128212e-05;
    n_eval += test_a_function(f15n28, r15n28c, bound0, bound1, tol, __LINE__);

    n =  23.0;
    // Alefeld:      5.1669892394942e-05
    double r15n28d = 5.16698923949422605161e-05;
    n_eval += test_a_function(f15n28, r15n28d, bound0, bound1, tol, __LINE__);

    n =  24.0;
    // Alefeld:      4.9603096699145e-05
    double r15n28e = 4.9603096699144567656e-05;
    n_eval += test_a_function(f15n28, r15n28e, bound0, bound1, tol, __LINE__);

    n =  25.0;
    // Alefeld:      4.7695285287639e-05
    double r15n28f = 4.76952852876390018884e-05;
    n_eval += test_a_function(f15n28, r15n28f, bound0, bound1, tol, __LINE__);

    n =  26.0;
    // Alefeld:      4.5928793239949e-05
    double r15n28g = 4.59287932399486594501e-05;
    n_eval += test_a_function(f15n28, r15n28g, bound0, bound1, tol, __LINE__);

    n =  27.0;
    // Alefeld:      4.4288479195665e-05
    double r15n28h = 4.42884791956647908559e-05;
    n_eval += test_a_function(f15n28, r15n28h, bound0, bound1, tol, __LINE__);

    n =  28.0;
    // Alefeld:      4.2761290257883e-05
    double r15n28i = 4.27612902578832391001e-05;
    n_eval += test_a_function(f15n28, r15n28i, bound0, bound1, tol, __LINE__);

    n =  29.0;
    // Alefeld:      4.1335913915954e-05
    double r15n28j = 4.13359139159538029919e-05;
    n_eval += test_a_function(f15n28, r15n28j, bound0, bound1, tol, __LINE__);

    n =  30.0;
    // Alefeld:      4.0002497338020e-05
    double r15n28k = 4.00024973380198143745e-05;
    n_eval += test_a_function(f15n28, r15n28k, bound0, bound1, tol, __LINE__);

    n =  31.0;
    // Alefeld:      3.8752419296207e-05
    double r15n28l = 3.8752419296206693693e-05;
    n_eval += test_a_function(f15n28, r15n28l, bound0, bound1, tol, __LINE__);

    n =  32.0;
    // Alefeld:      3.7578103559958e-05
    double r15n28m = 3.75781035599579977917e-05;
    n_eval += test_a_function(f15n28, r15n28m, bound0, bound1, tol, __LINE__);

    n =  33.0;
    // Alefeld:      3.6472865219959e-05
    double r15n28n = 3.64728652199592355424e-05;
    n_eval += test_a_function(f15n28, r15n28n, bound0, bound1, tol, __LINE__);

    n =  34.0;
    // Alefeld:      3.5430783356532e-05
    double r15n28o = 3.54307833565318272637e-05;
    n_eval += test_a_function(f15n28, r15n28o, bound0, bound1, tol, __LINE__);

    n =  35.0;
    // Alefeld:      3.4446594929961e-05
    double r15n28p = 3.44465949299614979757e-05;
    n_eval += test_a_function(f15n28, r15n28p, bound0, bound1, tol, __LINE__);

    n =  36.0;
    // Alefeld:      3.3515605877800e-05
    double r15n28q = 3.35156058778003841008e-05;
    n_eval += test_a_function(f15n28, r15n28q, bound0, bound1, tol, __LINE__);

    n =  37.0;
    // Alefeld:      3.2633616249437e-05
    double r15n28r = 3.26336162494372057554e-05;
    n_eval += test_a_function(f15n28, r15n28r, bound0, bound1, tol, __LINE__);

    n =  38.0;
    // Alefeld:      3.1796856858426e-05
    double r15n28s = 3.17968568584259944827e-05;
    n_eval += test_a_function(f15n28, r15n28s, bound0, bound1, tol, __LINE__);

    n =  39.0;
    // Alefeld:      3.1001935436965e-05
    double r15n28t = 3.10019354369653454676e-05;
    n_eval += test_a_function(f15n28, r15n28t, bound0, bound1, tol, __LINE__);

    n =  40.0;
    // Alefeld:      3.0245790670210e-05
    double r15n28u = 3.02457906702100933871e-05;
    n_eval += test_a_function(f15n28, r15n28u, bound0, bound1, tol, __LINE__);

    n =  100.0;
    // Alefeld:      1.2277994232462e-05
    double r15n28v = 1.22779942324615231084e-05;
    n_eval += test_a_function(f15n28, r15n28v, bound0, bound1, tol, __LINE__);

    n =  200.0;
    // Alefeld:      6.1695393904409e-06
    double r15n28w = 6.16953939044086532173e-06;
    n_eval += test_a_function(f15n28, r15n28w, bound0, bound1, tol, __LINE__);

    n =  300.0;
    // Alefeld:      4.1198585298293e-06
    double r15n28x = 4.11985852982928247635e-06;
    n_eval += test_a_function(f15n28, r15n28x, bound0, bound1, tol, __LINE__);

    n =  400.0;
    // Alefeld:      3.0924623877272e-06
    double r15n28y = 3.09246238772721767043e-06;
    n_eval += test_a_function(f15n28, r15n28y, bound0, bound1, tol, __LINE__);

    n =  500.0;
    // Alefeld:      2.4752044261050e-06
    double r15n28z = 2.4752044261050178947e-06;
    n_eval += test_a_function(f15n28, r15n28z, bound0, bound1, tol, __LINE__);

    n =  600.0;
    // Alefeld:      2.0633567678513e-06
    double r15n28A = 2.06335676785127107013e-06;
    n_eval += test_a_function(f15n28, r15n28A, bound0, bound1, tol, __LINE__);

    n =  700.0;
    // Alefeld:      1.7690120078154e-06
    double r15n28B = 1.76901200781542650599e-06;
    n_eval += test_a_function(f15n28, r15n28B, bound0, bound1, tol, __LINE__);

    n =  800.0;
    // Alefeld:      1.5481615698859e-06
    double r15n28C = 1.54816156988591015938e-06;
    n_eval += test_a_function(f15n28, r15n28C, bound0, bound1, tol, __LINE__);

    n =  900.0;
    // Alefeld:      1.3763345366022e-06
    double r15n28D = 1.37633453660223511171e-06;
    n_eval += test_a_function(f15n28, r15n28D, bound0, bound1, tol, __LINE__);

    n = 1000.0;
    // Alefeld:      1.2388385788997e-06
    double r15n28E = 1.23883857889971445027e-06;
    n_eval += test_a_function(f15n28, r15n28E, bound0, bound1, tol, __LINE__);

    std::cout
        << "  evaluations: " << n_eval
        << " (vs. " << alefeld_count << " Alefeld Table II)"
        << "; tol " << tol
        << std::endl
        ;
}

int test_main(int, char*[])
{
    test_fundamentals();
    test_binary64_midpoint();
    test_NaNs();
    test_root_at_a_bound();
    test_biases();
    test_celebrated_equation();
    test_wikipedia_example();
    test_various_functions();
    test_hodgepodge();
    test_former_rounding_problem();
    test_toms748();

    std::cout << "TOMS 748 tests: " << std::endl;
    test_alefeld_examples(2804, 1.0e-7);
    test_alefeld_examples(2905, 1.0e-10);
    test_alefeld_examples(2975, 1.0e-15);
    test_alefeld_examples(3008, 0.0);

    return 0;
}
