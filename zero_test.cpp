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
#include "math_functions.hpp"           // signum()
#include "miscellany.hpp"               // stifle_warning_for_unused_variable()
#include "test_tools.hpp"

#include <cfloat>                       // DECIMAL_DIG
#include <cmath>                        // exp(), fabs(), log(), pow(), sqrt()
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
    return 6.0 * epsilon * std::fabs(zeta) + 2.0 * tol;
}

/// AfMWD eq. 3.3: maximum number of evaluations for bisection.
///
/// The return value, k+1, is the exact number of function
/// evaluations unless f vanishes early, as Brent explains in the
/// paragraph following eq. 3.3 .
///
/// static_cast<int> is exact for any number of evaluations that
/// can be counted by an 'int'.
///
/// The greatest possible number of bisection steps is:
///   log2(DBL_MAX - -DBL_MAX) / DBL_TRUE_MIN
///   = 1 + 1024 + 1074 = 2099
/// Yet an IEEE 754 binary64 entity can have no more than 2^64
/// distinct values; with an appropriate definition of "bisection",
/// about 64 steps should suffice.
///
/// Known defects:
///  - std::fabs(DBL_MAX - -DBL_MAX) overflows.
///  - The denominator can be zero when ζ = 0, because the
///    implementation allows 'tol' to be zero as a shorthand
///    for the hardware minimum. (Specifying DBL_TRUE_MIN would
///    entail a likely speed penalty even on platforms that support
///    denormals, so this shorthand isn't merely a convenience).
/// Such defects in a unit-testing TU needn't be fixed.

int max_n_eval_bolzano(double a, double b, double tol, double zeta)
{
    double delta = 2.0 * epsilon * std::fabs(zeta) + tol;
    double k = std::ceil(std::log2(std::fabs(b - a) / delta));
    return 1 + static_cast<int>(k);
}

/// AfMWD eq. 3.3: maximum number of evaluations for Brent's method.
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
void test_a_function
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
    root_type r = lmi_root(f, bound0, bound1, tol, INT_MAX, os1);
    INVOKE_LMI_TEST(root_is_valid == r.validity, file, line);
    error = r.root - exact_root;
    INVOKE_LMI_TEST_RELATION(std::fabs(error),<=,maximum_error,file,line);
    INVOKE_LMI_TEST_RELATION(r.n_eval,<=,max_n_eval,file,line);

#if !defined LMI_X87
    INVOKE_LMI_TEST_EQUAL(os0.str(),os1.str(),file,line);
#endif // !defined LMI_X87
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
    r = decimal_root(e_function, 0.5, 5.0, bias_none, 9, INT_MAX, oss);
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
    // evaluations is
    //   log2(DBL_MAX - -DBL_MAX) / DBL_TRUE_MIN
    //   = 1 + 1024 + 1074 = 2099
    // for bisection, and 2099^2 = 4405801 for Brent's method with
    // IEEE 754 binary64.
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
    // has an opposite signbit. Note the "UN" in "UNEQUAL" here.

    LMI_TEST_UNEQUAL(0.0, binary64_midpoint( 3.1416, -0.0)); // "UN"!
    LMI_TEST_UNEQUAL(0.0, binary64_midpoint(-3.1416,  0.0)); // "UN"!

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
    root_type r = decimal_root(f, -2.56, 2.56, bias_none, 21, INT_MAX, oss);
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
 return value: +2.09455148154232650981 = b
 function evaluations: +12 +12 nominal, actual
 return value: +2.09455148154232650981 (rounded)
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
    root_type r = decimal_root(f, -4.0, 4.0 / 3.0, bias_none, 15, INT_MAX, oss);
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
    LMI_TEST_RELATION(156,<=,r.n_eval); // weak
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

    std::ostringstream oss;
    r = lmi_root(signum_offset, -1.0e300, 1.0e300, 5.0e-19, INT_MAX, oss);
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
    LMI_TEST_RELATION(64,<=,r.n_eval);
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

    return 0;
}
