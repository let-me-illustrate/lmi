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
#include <cmath>                        // exp(), fabs(), log(), pow()
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

/// AfMWD eq. 3.3: maximum number of iterations for bisection.
///
/// The return value, k+1, is the exact number of function
/// evaluations unless f vanishes early, as Brent explains in the
/// paragraph following eq. 3.3 .
///
/// static_cast<int> is exact for any number of iterations that
/// can be counted by an 'int'.
///
/// The greatest possible number of bisection steps (where [x] is
/// the greatest integer function) is:
///   log2(DBL_MAX - -DBL_MAX) / DBL_TRUE_MIN
///   = 1 + 1024 + 1074 = 2099
/// Yet an IEEE 754 binary64 entity can have no more than 2^64
/// distinct values; with an appropriate definition of "bisection",
/// about 64 steps should suffice.

int max_n_iter_bolzano(double a, double b, double tol, double zeta)
{
    double delta = 2.0 * epsilon * std::fabs(zeta) + tol;
    double k = std::ceil(std::log2(std::fabs(b - a) / delta));
    return 1 + static_cast<int>(k);
}

/// AfMWD eq. 3.3: maximum number of iterations for Brent's method.
///
/// The greatest possible number of steps is 2099^2 = 4405801.

int max_n_iter_brent(double a, double b, double tol, double zeta)
{
    int k_plus_one = max_n_iter_bolzano(a, b, tol, zeta);
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
///  - the number of iterations doesn't exceed max_n_iter_brent()
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
    int const max_n_iter = max_n_iter_brent(bound0, bound1, tol, exact_root);

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
    INVOKE_LMI_TEST_RELATION(r.n_iter,<=,max_n_iter,file,line);

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
///  - the number of iterations doesn't exceed max_n_iter_brent()
///
/// Also verify that the number of iterations matches the 'n_iter'
/// argument, to make it easier to detect mistaken refactorings.
/// Do this only if 'n_iter' is not zero (the default), and only for
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
    ,int         n_iter = 0
    ,char const* file   = __FILE__
    )
{
    double const tol = 0.5 * std::pow(10.0, -decimals);
    double const maximum_error = max_err(exact_root, tol);
    int const max_n_iter = max_n_iter_brent(bound0, bound1, tol, exact_root);

    double d = brent_zero(f, bound0, bound1, tol);
    double error = d - exact_root;
    INVOKE_LMI_TEST_RELATION(std::fabs(error),<=,maximum_error,file,line);

    root_type r = decimal_root(f, bound0, bound1, bias_none, decimals);
    INVOKE_LMI_TEST(root_is_valid == r.validity, file, line);
    error = r.root - exact_root;
    INVOKE_LMI_TEST_RELATION(std::fabs(error),<=,maximum_error,file,line);
    INVOKE_LMI_TEST_RELATION(r.n_iter,<=,max_n_iter,file,line);

#if defined LMI_X86_64 && defined LMI_POSIX
    if(0 != n_iter)
        {
        INVOKE_LMI_TEST_EQUAL(n_iter, r.n_iter, file, line);
        }
#endif // defined LMI_X86_64 && defined LMI_POSIX
    stifle_warning_for_unused_variable(n_iter);
}

/// Test with all biases, asserting obvious invariants.

template<typename F>
void test_bias(double bound0, double bound1, int dec, F f, double exact_root)
{
    double maximum_error = max_err(exact_root, 0.5 * std::pow(10.0, -dec));

    root_type rn = decimal_root(f, bound0, bound1, bias_none,   dec);
    root_type rl = decimal_root(f, bound0, bound1, bias_lower,  dec);
    root_type rh = decimal_root(f, bound0, bound1, bias_higher, dec);

    LMI_TEST(root_is_valid == rn.validity);
    LMI_TEST(root_is_valid == rl.validity);
    LMI_TEST(root_is_valid == rh.validity);

    LMI_TEST(rl.root <= rn.root && rn.root <= rh.root);

    LMI_TEST(std::fabs(rh.root - rl.root) <= maximum_error);
    LMI_TEST(std::fabs(rl.root - exact_root) <= maximum_error);
    LMI_TEST(std::fabs(rh.root - exact_root) <= maximum_error);
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
/// For this function, k = [log2(200/0.5)] = 9, so Brent's method
/// should take no more than 10^2-2 = 98 function evaluations.
///
/// The parameters hardcoded here were chosen to prevent overflow.
/// This is not a dramatic illustration of the superiority to Dekker's
/// method, which would move by a step of 1.0 at each iteration, thus
/// taking about 200 iterations. Brent provides an extended-exponent
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

    // Calculate maximum possible number of iterations according to
    // the documentation for max_n_iter_bolzano(). This calculation
    // would overflow in double precision.
    //
    // log2(DBL_MAX) is 1024, so log2(DBL_MAX - -DBL_MAX) is 1025;
    // and log2(DBL_TRUE_MIN) is 1074; so the maximum number of
    // iterations is
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

    int const max_n_iter = max_n_iter_brent(-1.0e100, 1.0e100, 5.0e-1, pi);
    LMI_TEST_RELATION(r.n_iter,<=,max_n_iter);
    LMI_TEST(materially_equal(1.0e100, std::fabs(r.root)));
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
    LMI_TEST_EQUAL(r.n_iter, 2);

    // Root found on third evaluation of a monomial.
    r = lmi_root(f, -1.0,  1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 3);

    // Root is first bound: found on first evaluation.
    r = lmi_root(f,  0.0, -1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 1);

    // Returns an error status, even though the root coincides with
    // both bounds. Attempting to find a root between identical bounds
    // is presumably an error, which should be reported immediately
    // without evaluating the objective function even once.
    r = lmi_root(f,  0.0,  0.0, tol);
    LMI_TEST(improper_bounds == r.validity);
    LMI_TEST_EQUAL(r.n_iter, 0);

    r = lmi_root(f,  0.0,  1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 1);

    r = lmi_root(f,  1.0, -1.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 3);

    r = lmi_root(f,  1.0,  0.0, tol);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 2);

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
    LMI_TEST_EQUAL(r.n_iter, 2);

    // Root found on third evaluation of a monomial.
    r = decimal_root(f, -1.04,  0.96, bias_none, 1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 3);

    // Root is rounded first bound: found on first evaluation.
    r = decimal_root(f,  0.04, -1.01, bias_none, 1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 1);

    // Bounds identical after rounding: presumptive error.
    r = decimal_root(f, -0.04,  0.04, bias_none, 1);
    LMI_TEST(improper_bounds == r.validity);
    LMI_TEST_EQUAL(r.n_iter, 0);

    // A curious effect of rounding the input bounds.

    // Literal   bounds [0.04, 0.09] bracket no root.
    // Effective bounds [0.0 , 0.1 ] bracket a root.
    // The exact true root, 0.0, is returned, because the literal
    // input bounds are replaced by the rounded effective bounds.
    r = decimal_root(f,  0.04,  0.09, bias_none, 1);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST_EQUAL(r.root, zeta);
    LMI_TEST_EQUAL(r.n_iter, 1);
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

    test_bias(-1.0e100, 4.0e100, -100, e, std::exp(1.0));
    test_bias(-1.0    , 4.0    ,    0, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    1, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    2, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    3, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    4, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    5, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    6, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    7, e, std::exp(1.0));
    test_bias( 0.5    , 5.0    ,    8, e, std::exp(1.0));
    test_bias(-1.0    , 4.0    ,  100, e, std::exp(1.0));
}

/// Test the worked-out example given here:
///   https://blogs.mathworks.com/cleve/2016/01/04/testing-zero-finders/
/// All iterates are identical for x86_64-pc-linux-gnu except where
/// marked with absolute difference as a multiple of ϵ=DBL_EPSILON:
///
///    i686-w64-mingw32     x86_64-pc-linux-gnu
///   --------lmi-------     --------lmi-------     -----mathworks----
///   2.5600000000000001     2.5600000000000001     2.5600000000000001
///   1.0980323260716796  +ϵ 1.0980323260716793     1.0980323260716793
///   1.783216881610604   +ϵ 1.783216881610604   +ϵ 1.7832168816106038
///   2.2478393639958036     2.2478393639958036     2.2478393639958036
///   2.0660057758331045     2.0660057758331045     2.0660057758331045
///   2.0922079131171945     2.0922079131171945     2.0922079131171945
///   2.0945566700001774 -2ϵ 2.0945566700001779     2.0945566700001779
///   2.0945514746903111     2.0945514746903111     2.0945514746903111
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
#eval            a           fa            b           fb            c           fc
  2 i -2.5600000000000001 -16.657216000000002 2.5600000000000001 6.6572160000000018            0            0
  2 j -2.5600000000000001 -16.657216000000002 2.5600000000000001 6.6572160000000018 -2.5600000000000001 -16.657216000000002
  3 L 2.5600000000000001 6.6572160000000018 1.0980323260716793 -5.8721945393772152 -2.5600000000000001 -16.657216000000002
  3 j 2.5600000000000001 6.6572160000000018 1.0980323260716793 -5.8721945393772152 2.5600000000000001 6.6572160000000018
  4 L 1.0980323260716793 -5.8721945393772152 1.783216881610604 -2.8960493667789873 2.5600000000000001 6.6572160000000018
  5 Q 1.783216881610604 -2.8960493667789873 2.2478393639958036 1.8621631139566732 2.5600000000000001 6.6572160000000018
  5 j 1.783216881610604 -2.8960493667789873 2.2478393639958036 1.8621631139566732 1.783216881610604 -2.8960493667789873
  6 L 2.2478393639958036 1.8621631139566732 2.0660057758331045 -0.3135140955237814 1.783216881610604 -2.8960493667789873
  6 j 2.2478393639958036 1.8621631139566732 2.0660057758331045 -0.3135140955237814 2.2478393639958036 1.8621631139566732
  7 L 2.0660057758331045 -0.3135140955237814 2.0922079131171945 -0.026123094109737011 2.2478393639958036 1.8621631139566732
  8 Q 2.0922079131171945 -0.026123094109737011 2.0945566700001779 5.7910818359374616e-05 2.2478393639958036 1.8621631139566732
  8 j 2.0922079131171945 -0.026123094109737011 2.0945566700001779 5.7910818359374616e-05 2.0922079131171945 -0.026123094109737011
  9 L 2.0945566700001779 5.7910818359374616e-05 2.0945514746903111 -7.6478343657981895e-08 2.0922079131171945 -0.026123094109737011
  9 j 2.0945566700001779 5.7910818359374616e-05 2.0945514746903111 -7.6478343657981895e-08 2.0945566700001779 5.7910818359374616e-05
 10 L 2.0945514746903111 -7.6478343657981895e-08 2.0945514815423065 -2.2382096176443156e-13 2.0945566700001779 5.7910818359374616e-05
 11 Q 2.0945514815423065 -2.2382096176443156e-13 2.0945514815423265 -8.8817841970012523e-16 2.0945566700001779 5.7910818359374616e-05
 12 Q 2.0945514815423265 -8.8817841970012523e-16 2.0945514815423274 9.7699626167013776e-15 2.0945566700001779 5.7910818359374616e-05
 12 j 2.0945514815423265 -8.8817841970012523e-16 2.0945514815423274 9.7699626167013776e-15 2.0945514815423265 -8.8817841970012523e-16
 12 k 2.0945514815423274 9.7699626167013776e-15 2.0945514815423265 -8.8817841970012523e-16 2.0945514815423274 9.7699626167013776e-15
12 iterations; final interval:
 b +2.09455148154232650981 fb -8.88178419700125232339e-16
 c +2.09455148154232739799 fc +9.76996261670137755573e-15
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
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 17, __LINE__, 163);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-17, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 16, __LINE__, 156);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-16, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 15, __LINE__, 142);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-15, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 14, __LINE__, 128);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-14, __LINE__);
    test_a_decimal_function(f01, root_01, -1.0, 4.0, 12, __LINE__, 112);
    test_a_function        (f01, root_01, -1.0, 4.0, 0.5 * 1.0e-12, __LINE__);

    auto f02 = [](double x) {return std::pow(x - 1.7, 17.0);};
    auto root_02 = 1.7;
    test_a_decimal_function(f02, root_02, 0.0, 2.0, 17     , __LINE__, 145);
    test_a_function        (f02, root_02, 0.0, 2.0, 1.0e-15, __LINE__);

    auto f03 = [](double x) {return std::cos(x) - 0.999;};
    auto root_03 = 0.044725087168733454;
    test_a_decimal_function(f03, root_03, -0.01, 0.8, 17     , __LINE__, 16);
    test_a_function        (f03, root_03, -0.01, 0.8, 1.0e-15, __LINE__);

    auto f04 = [](double x) {return std::pow((x - 1.0), 3);};
    auto root_04 = 1.0;
    test_a_decimal_function(f04, root_04, 0.0 , 1.8, 17     , __LINE__, 130);
    test_a_function        (f04, root_04, 0.0 , 1.8, 1.0e-15, __LINE__);

    auto f05 = [](double x) {return std::pow(x, 2.0) - 2.0;};
    auto root_05 = 1.4142135623730951;
    test_a_decimal_function(f05, root_05, 0.0 , 2.0, 17     , __LINE__, 10);
    test_a_function        (f05, root_05, 0.0 , 2.0, 1.0e-15, __LINE__);

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
    auto f06 = [](double x) {return x * x - 4.0;};
    auto root_06 = -2.0;
    test_a_decimal_function(f06, root_06,  1.9, -2.1, 15     , __LINE__);
    test_a_decimal_function(f06, root_06, -2.1 , 1.9, 15     , __LINE__);
    test_a_function        (f06, root_06,  1.9, -2.1, 1.0e-15, __LINE__);
    test_a_function        (f06, root_06, -2.1 , 1.9, 1.0e-15, __LINE__);

    // Here, the input tolerance is legitimately much smaller than ϵ.
    // Brent describes 'tol' as positive, so it shouldn't be zero, yet
    // it has no absolute minimum (other than DBL_TRUE_MIN) because
    // it is a relative error. Thus, when ζ = 0, δ has no minimum at
    // all (other than DBL_TRUE_MIN).
    auto f07 = [](double x) {return 1.23 * (x - 1.0e-100);};
    auto root_07 = 1.0e-100;
    test_a_function(f07, root_07, 1.0e-10, 1.0e-200, 1.0e-150, __LINE__);
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
    // chasing after wind that costs many iterations.

    e_nineteenth e_19;

    // Number of iterations:
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

    // Assertions labelled 'weak' test the number of iterations
    // against empirical measurements (with various architectures)
    // rather than a theoretical maximum. Perhaps they'll always
    // succeed, because floating-point behavior is determinate;
    // but small variations betoken no catastrophe.
    LMI_TEST(169 <= r.n_iter && r.n_iter <= 173); // weak

    d = brent_zero(eq_2_1, -100.0, 100.0, 0.5);
    zeta = -100.0;
    double eq_2_1_upper = zeta + max_err(zeta, 0.5);
    LMI_TEST(-100.0 <= d && d <= eq_2_1_upper);

    r = decimal_root(eq_2_1, -100.0, 100.0, bias_none, 0);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(-100.0 <= r.root && r.root <= eq_2_1_upper);
    LMI_TEST(10 == max_n_iter_bolzano(-100.0, 100.0, 0.5, -100.0));
    LMI_TEST(98 == max_n_iter_brent  (-100.0, 100.0, 0.5, -100.0));
    LMI_TEST(r.n_iter <= 98);
    LMI_TEST_EQUAL(20, r.n_iter); // weak
    // Number of iterations required:
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

    LMI_TEST(  53 == max_n_iter_bolzano(-100.0, 100.0, 0.0, -100.0));
    LMI_TEST(2807 == max_n_iter_brent  (-100.0, 100.0, 0.0, -100.0));
    LMI_TEST(r.n_iter <= 2807);
    LMI_TEST_EQUAL(66, r.n_iter); // weak

    r = decimal_root(signum_offset, -1.0, 1.0, bias_none, 13);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    zeta = 1.0 / 3.0;
    double tol = 0.5 * 1.0e-13;
    LMI_TEST_EQUAL(  47, max_n_iter_bolzano(-1.0, 1.0, tol, zeta));
    LMI_TEST_EQUAL(2207, max_n_iter_brent  (-1.0, 1.0, tol, zeta));
    LMI_TEST(r.n_iter <= 2207);
    // Here, decimal_root() always chooses the bisection technique.
    LMI_TEST(46 <= r.n_iter && r.n_iter <= 47); // weak

    r = decimal_root(signum_offset, -1.0, 1.0, bias_none, 16);
    LMI_TEST(root_is_valid == r.validity);
    LMI_TEST(materially_equal(-1.0 / 3.0, r.root));
    tol = 0.5 * 1.0e-16;
    LMI_TEST_EQUAL(  55, max_n_iter_bolzano(-1.0, 1.0, tol, zeta));
    LMI_TEST_EQUAL(3023, max_n_iter_brent  (-1.0, 1.0, tol, zeta));
    LMI_TEST(r.n_iter <= 3023);
    // Here, decimal_root() always chooses the bisection technique.
    LMI_TEST_EQUAL(55, r.n_iter); // weak
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
