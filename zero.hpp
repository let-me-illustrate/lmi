// Root finding by Brent's method.
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

#ifndef zero_hpp
#define zero_hpp

#include "config.hpp"

#include "math_functions.hpp"           // signum()
#include "miscellany.hpp"               // scoped_ios_format
#include "null_stream.hpp"
#include "round_to.hpp"
#include "ssize_lmi.hpp"

#include <cfloat>                       // DBL_EPSILON, DECIMAL_DIG
#include <climits>                      // INT_MAX
#include <cmath>                        // fabs(), isfinite(), isnan(), pow()
#include <cstdint>                      // uint64_t
#include <cstring>                      // memcpy()
#include <iomanip>                      // setprecision(), setw()
#include <limits>
#include <numeric>                      // midpoint()
#include <ostream>
#include <unordered_map>
#include <utility>                      // forward()

enum root_bias
    {bias_none   // Return root z with f(z) closest to 0.0 .
    ,bias_lower  // Require f(z) <= 0.0 .
    ,bias_higher // Require  0.0 <= f(z).
    };

/// Reason for having been dispatched to a particular "activity".
///
/// This is deliberately defined with enum-key 'enum' rather than
/// 'enum class' or 'enum struct'. It's the same as an 'enum class'
/// except that its enumerators usefully decay to char for printing.
/// Feature comparison:
///
///   this enum  enum class   desirable properties
///   ---------  ----------   --------------------
///      yes        yes       specifies underlying type
///      yes         no       implicitly converts to char

enum root_impetus : char
    {evaluate_bounds                  = 'i'
    ,force_b_and_c_to_bracket_root    = 'j'
    ,force_b_to_be_best_approximation = 'k'
    ,interpolate_linear               = 'L'
    ,interpolate_inverse_quadratic    = 'Q'
    ,interpolate_guaranteed_64_evals  = 'G'
    ,dithering_near_root              = '0'
    ,secant_out_of_bounds             = '1'
    ,parabola_not_single_valued       = '2'
    ,guarantee_linear_convergence     = '3'
    ,pis_aller                        = '4'
    };

enum root_validity
    {root_is_valid
    ,root_not_bracketed
    ,improper_bounds
    };

struct root_type
{
    double        root     {0.0};
    root_validity validity {improper_bounds};
    int           n_iter   {0};
    int           n_eval   {0};
};

/// Specialized binary64 midpoint for root finding.
///
/// A 64-bit double can represent no more than 2^64 distinct values.
/// Disregarding NaNs, they form (a permutation of) an ordered set,
/// any of whose members can be found in 64 binary-search steps.
/// However, bisection using the conventional arithmetic mean takes
///   log2(DBL_MAX - -DBL_MAX) / DBL_TRUE_MIN
///   = 1 + 1024 + 1074 = 2099
/// instead of 64 steps to explore that range fully; and the maximum
/// for Brent's method is the square of that number.
///
/// Consider:
///   DBL_MAX       7fefffffffffffff
///   DBL_MAX/2     7fdfffffffffffff
///   DBL_TRUE_MIN  1000000000000000
///   0.0           0000000000000000
///  -DBL_MAX       ffefffffffffffff
///  -DBL_MAX/2     ffdfffffffffffff
///  -DBL_TRUE_MIN  8000000000000001
///  -0.0           8000000000000000
/// If a root is bounded by [0.0, DBL_MAX], then evaluating the
/// objective function at the arithmetic mean chooses between two
/// partitions
///   [0000000000000000, 7fdfffffffffffff]
///   [7fdfffffffffffff, 7fefffffffffffff]
/// the larger of which contains about 99.95% of the elements.
/// This function instead chooses a pivot that separates half
/// the elements from the other half.
///
/// Precondition: neither argument is an infinity or a NaN;
/// throw if violated.
///
/// The range [0x0, 0xffffffffffffffff] with infinities and NaNs
/// removed is wellordered with respect to only one of the comparisons
/// <(double) and <(uint64_t), but it can be split into two subranges
///   [ DBL_MAX ≡ 0x7fefffffffffffff,  0.0 ≡ 0x0000000000000000]
///   [-DBL_MAX ≡ 0xffefffffffffffff, -0.0 ≡ 0x8000000000000000]
/// that are both wellordered, isomorphically, by those comparisons.
/// Therefore, if the arguments are of opposite sign (both nonzero,
/// one +, the other -) then return +0.0. This can happen only on the
/// first iteration.
///
/// If both arguments are zero, then return +0.0. This case is not
/// expected to arise in practice; treating is specially removes the
/// only violation of the invariant that the result doesn't depend on
/// the order of the arguments.
///
/// Otherwise, calculate and return a binary midpoint. If one argument
/// is a zero, then first change its signbit, if needed, to match the
/// other argument's. Finally, interpret both as unsigned integers,
/// and return their arithmetic mean interpreted as binary64.

inline double binary64_midpoint(double d0, double d1)
{
    static_assert(std::numeric_limits<double>::is_iec559);

    using u_dbl_int_t = std::uint64_t;
    static_assert(sizeof(u_dbl_int_t) == sizeof(double));

    if(!std::isfinite(d0) || !std::isfinite(d1))
        {throw "binary64_midpoint: non-finite argument";}

    double const s0 = signum(d0);
    double const s1 = signum(d1);
    if(-1.0 == s0 * s1)
        {return 0.0;}
    else if(0.0 == s0 && 0.0 == s1)
        {return 0.0;}
    else if(0.0 == s0)
        {d0 = std::copysign(d0, d1);}
    else if(0.0 == s1)
        {d1 = std::copysign(d1, d0);}
    else {;} // Do nothing.

    u_dbl_int_t u0;
    u_dbl_int_t u1;
    std::memcpy(&u0, &d0, sizeof d0);
    std::memcpy(&u1, &d1, sizeof d1);
    u_dbl_int_t um = std::midpoint(u0, u1);
    double z;
    std::memcpy(&z, &um, sizeof z);
#if 0 // Temporarily useful for acceptance testing.
        scoped_ios_format meaningless_name(std::cout);
        std::cout
            << std::dec
            << std::defaultfloat
            << u0 << " u0\n"
            << u1 << " u1\n"
            << um << " um\n"
            << std::hex
            << u0 << " u0\n"
            << u1 << " u1\n"
            << um << " um\n"
            << d0 << " d0\n"
            << d1 << " d1\n"
            << z  << " z\n"
            << std::hexfloat
            << d0 << " d0\n"
            << d1 << " d1\n"
            << z  << " z\n"
            << std::dec
            << std::defaultfloat
            << std::endl
            << std::endl
            ;
#endif // 0
    return z;
}

/// Return a zero z of a function f within input bounds [a,b].
///
/// Preconditions: bounds are distinct; and either
///   0.0 == f(a), or
///   0.0 == f(b), or
///   f(a) and f(b) have opposite signs;
/// that is, the input bounds include or bracket a root.
///
/// Postcondition: z is within a tolerance
///   6ϵ|z| + 10^(-decimals)
/// of a true zero.
///
/// Brent's algorithm with GWC modifications described below. See:
/// R. P. Brent, _Algorithms for Minimization without Derivatives_
/// ISBN 0-13-022335-2
///
/// Rationale for choice of algorithm
///
/// "Brent's method combines the sureness of bisection with the speed
/// of a higher-order method when appropriate. We recommend it as the
/// method of choice for general one-dimensional root finding where a
/// function's values only (and not its derivative or functional form)
/// are available." --Press et al., _Numerical Recipes_ (3rd ed. 2007)
///
/// Numerous papers claim to improve on Brent's method. Perhaps the
/// best is ACM Algorithm 748 (Transactions on Mathematical Software):
///   https://na.math.kit.edu/alefeld/download/1995_Algorithm_748_Enclosing_Zeros_of_Continuous_Functions.pdf
/// whose Table II compares Brent's algorithm to TOMS748 for fifteen
/// test problems, claiming an advantage of 4-6%. A typical lmi solve
/// takes ten or twenty iterations, so that would represent saving
/// less than one iteration on average. It would be interesting to
/// test TOMS758 with lmi, but there's little hope of any real gain.
///
/// Newton's method has quadratic convergence, in the vicinity of a
/// root, for well-behaved functions (though its performance in the
/// worst case is infinitely poor). Sometimes we're asked why we don't
/// use it, as other illustration systems are said to do. The truth is
/// that they don't really use it, either. For a hundred years' worth
/// of monthiversary processing, a solve requires finding a root of a
/// polynomial of order 1200. Newton's method requires a derivative,
/// which nobody evaluates analytically--the polynomial's coefficients
/// are likely to change with each iteration. There are two obvious
/// ways to approximate the derivative numerically:
///
///  - Use the slope of the last two iterates. That's actually the
///    secant method, whose order of convergence is 1.618..., the
///    golden ratio.
///
///  - Calculate f(x) and f(x+delta), where delta might typically be
///    one cent. Because two evaluations are required, the order of
///    convergence per (costly) function evaluation cannot exceed
///    1.414..., the square root of two, so the secant method would
///    be preferable.
///
/// Once it has localized a root well enough, Brent's method uses
/// inverse quadratic interpolation, whose order of convergence is
/// 1.8393..., faster than either method above. Furthermore, in the
/// worst case, it's "never much slower than bisection" [Brent, op.
/// cit., Chapter 4, soon after equation (2.8)], which is the optimal
/// method for the most ill-conditioned functions. (To see why, seek
/// a faster method than bisection to find the root of
///   f(x) = -1, x < C
///        =  0, x = C
///        =  1, x > C
/// in the a priori interval [a,b] such that f(a) < 0 and f(b) > 0,
/// for an unspecified C in that interval.)
///
/// GWC modifications
///
/// Brent's original algorithm strives to return the closest value to
/// a true root (within a given tolerance). Especially for currency
/// values, it may be necessary to find the least or greatest value
/// r such that f(r) is nonnegative. Those options are governed by a
/// "bias" argument. With 'bias_none', this implementation returns the
/// same result Brent would, so it remains suitable for the "unbiased"
/// case.
///
/// Brent states a requirement that the ordinates corresponding to the
/// a priori bounds (abscissa arguments) have different signs, but his
/// algorithm does not test that requirement. This implementation does
/// enforce it, and also handles the special case where both ordinates
/// are zero.
///
/// For Brent's method, the worst-case number of iterations is the
/// square of the number required by naive bisection, so it may take
/// an unreasonable amount of time for ill-conditioned problems. The
/// optional 'sprauchling_limit' argument specifies the maximum number
/// of evaluations to allow before switching to binary64 bisection,
/// which is guaranteed to converge in 64 further evaluations.
///
/// Notes referred to in the source code
///
/// Note 0. If one of the bounds is a zero, it is returned as soon as
/// that is known. This optimization is justified because it costs so
/// little, even if it happens rarely.
///
/// Note 1. For abscissae a, b, c:
///   a and b are a priori bounds;
///   b is the best approximation so far to the true root r;
///   a is the previous value of b, or, initially, equal to c;
///   ordinates f(b) and f(c) are of different sign.
///
/// Initializing fc to fb ensures that the first conditional clause in
/// the main loop is executed on the first pass, so that the branches
/// in the algol original can be rewritten in a structured way.
///
/// Note 2. Here, Brent observes that one might return 0.5 * (b + c),
/// equivalent to b + m, but that b is probably a much better
/// approximation, so he returns b as soon as the condition
///   !(0.0 != fb && std::fabs(m) <= tol)
/// is satisfied. But b might not have the desired bias. In that case,
/// 0.5 * (b + c) is not necessarily correct either, because its bias
/// is unknown; yet is it appropriate to return c instead? Sometimes.
///
/// The bias of c must be correct because f(b) and f(c) are known to
/// have different signs. And c is within Brent's tolerance in the weak
/// sense of his variable tol, which is a worst-case guarantee that
/// applies to c as well as b. To see why, suppose the algorithm is
/// about to return with tol = 0.005 and
///   f0: z -> z * z * signum(z)
///   b = -0.001   f(b) = -0.000001
///   c =  0.009   f(c) =  0.000081
/// The true root is of course zero, and b is closer. But the same
/// functional values would be obtained with
///   f1: z -> -0.000001, z < 0.01
///             0.0,      z = 0.0089
///             0.000081, 0.0089 < z <= 1.0
///             1.0,      1.0 < z
/// in which case Brent would return b and guarantee that the tolerance
/// is satisfied, even though c is much closer than b to the true root.
///
/// However, Brent calculates tol in terms of b, guaranteeing a maximum
/// error of
///   6ϵ|b| + 2t
/// when returning b. Unconditionally returning c would give an error
/// bound in terms of the local variable b, whose value is unknown to
/// the caller, and |b| might exceed |c|. It is irrelevant that the
/// return value is multiplied by ϵ, which might often be so small as
/// to make the first part of the error term vanish, because the
/// return value might far exceed the reciprocal of ϵ. To preserve
/// the algorithm's rigorous guarantees, c is returned, when bias so
/// dictates, only if Brent's termination criterion is still met when
/// reevaluated in terms of c instead of b.
///
/// It might appear that the code could be simplified, say, by defining
/// tol in terms of max(|b|,|c|), but that would be dangerous: tol is
/// used elsewhere to decide whether to revert to bisection, so any
/// change in its definition would vitiate the convergence guarantee
/// that is Brent's cardinal improvement to Dekker's algorithm, and
/// might introduce other errors as well.
///
/// At any rate, care is taken to return the same result as Brent's
/// original implementation in the bias_none case; to do otherwise
/// would violate the principle of least astonishment.
///
/// Note 3. Brent points out that this division is safe because
///   0 < |f(b)| <= |f(a)|
/// whenever this line is executed.

template<typename FunctionalType>
root_type lmi_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,double          tolerance
    ,int             sprauchling_limit = INT_MAX
    ,std::ostream&   os_trace          = null_stream()
    ,root_bias       bias              = bias_none
    )
{
    int              n_iter  {0};
    int              n_eval  {0};
    root_impetus     impetus {evaluate_bounds};

    os_trace
        << "#it #eval"
        << "            a           fa"
        << "            b           fb"
        << "            c           fc"
        << '\n'
        ;

    // Declarations must precede lambda.
    double a  {bound0};
    double fa {};
    double b  {bound1};
    double fb {};
    double c  {};
    double fc {};

    auto expatiate = [&]()
        {
        os_trace
            <<        std::setw(3)  << n_iter
            << ' ' << std::setw(3)  << n_eval
            << ' '                  << impetus
            << ' ' << std::setw(12) << a << ' ' << std::setw(12) << fa
            << ' ' << std::setw(12) << b << ' ' << std::setw(12) << fb
            << ' ' << std::setw(12) << c << ' ' << std::setw(12) << fc
            << std::endl
            ;
        };

    auto recapitulate = [&]()
        {
        scoped_ios_format meaningless_name(os_trace);
        os_trace
            << n_iter << " iterations, "
            << n_eval << " evaluations; final interval:\n"
            << std::setprecision(DECIMAL_DIG)
            << std::showpos
            << " b "  << std::setw(12) << b
            << " fb " << std::setw(12) << fb << "\n"
            << " c "  << std::setw(12) << c
            << " fc " << std::setw(12) << fc
            << std::endl
            ;
        };

    double t = tolerance;

    if(a == b)
        {
        recapitulate();
        os_trace << " return value: " << a << " = a" << std::endl;
        return {a, improper_bounds, n_iter, n_eval};
        }

    fa = static_cast<double>(f(a));
    ++n_eval;
    if(0.0 == fa) // Note 0.
        {
        recapitulate();
        os_trace << " return value: " << a << " = a" << std::endl;
        return {a, root_is_valid, n_iter, n_eval};
        }

    fb = static_cast<double>(f(b));
    ++n_eval;
    expatiate();
    if(0.0 == fb) // Note 0 [bis].
        {
        recapitulate();
        os_trace << " return value: " << b << " = b" << std::endl;
        return {b, root_is_valid, n_iter, n_eval};
        }

    // f(a) and f(b) must have different signs; neither may be a NaN.
    // Cases where either is zero were already handled above.
    if(std::isnan(fa) || std::isnan(fb) || signum(fa) == signum(fb))
        {
        recapitulate();
        os_trace << " return value: " << 0.0 << " = zero" << std::endl;
        return {0.0, root_not_bracketed, n_iter, n_eval};
        }

    fc = fb; // Note 1.
    c = b;
    double d = b - a;
    double e = d;

    for(;; ++n_iter)
        {
        if((0.0 < fb) == (0.0 < fc))
            {
            c = a;
            fc = fa;
            d = e = b - a;
            impetus = force_b_and_c_to_bracket_root;
            expatiate();
            }
        // If 'c' is a closer approximant than 'b', then swap them,
        // discarding the old value of 'a'.
        if(std::fabs(fc) < std::fabs(fb))
            {
             a =  b;  b =  c;  c =  a;
            fa = fb; fb = fc; fc = fa;
            impetus = force_b_to_be_best_approximation;
            expatiate();
            }
        double tol = 2.0 * DBL_EPSILON * std::fabs(b) + t;
        double m = 0.5 * (c - b);
        double n = std::midpoint(b, c); // "next" iterate
        if(0.0 == fb || std::fabs(m) <= tol) // Note 2.
            {
            if
                (   bias_none   == bias
                ||  bias_lower  == bias && fb <= 0.0
                ||  bias_higher == bias && 0.0 <= fb
                )
                {
                recapitulate();
                os_trace << " return value: " << b << " = b" << std::endl;
                return {b, root_is_valid, n_iter, n_eval};
                }
            else if(std::fabs(m) <= 2.0 * DBL_EPSILON * std::fabs(c) + t)
                {
                recapitulate();
                os_trace << " return value: " << c << " = c" << std::endl;
                return {c, root_is_valid, n_iter, n_eval};
                }
            else
                {
                ; // Do nothing.
                }
            }
        if(sprauchling_limit < n_eval)
            {
            impetus = interpolate_guaranteed_64_evals;
            n = binary64_midpoint(b, c); // "next" iterate
            d = e = n - b;
            }
        else if(std::fabs(e) < tol)
            {
            impetus = dithering_near_root;
            d = e = n - b;
            }
        else if(std::fabs(fa) <= std::fabs(fb))
            {
            impetus = secant_out_of_bounds;
            d = e = n - b;
            }
        else
            {
            double p, q;
            double s = fb / fa; // Note 3.
            if(a == c)
                {
                impetus = interpolate_linear;
                p = 2.0 * m * s;
                q = 1.0 - s;
                }
            else
                {
                impetus = interpolate_inverse_quadratic;
                q = fa / fc;
                double r = fb / fc;
                p = s * (2.0 * m * q * (q - r) - (b - a) * (r - 1.0));
                q = (q - 1.0) * (r - 1.0) * (s - 1.0);
                }
            if(0.0 < p)
                {
                q = -q;
                }
            else
                {
                p = -p;
                }
            s = e;
            e = d;
            // Use the criteria in Brent's ALGOL, which differ
            // slightly from their descriptions in his text.
            //
            // AfMWD says on page 51:
            //   "we reject i [i.e., b + p/q] if 2|p| ≥ 3|mq|"
            // Difference: the ALGOL subtracts tol×|q| [i.e., δ|q|]
            bool const k0 = 2.0 * p < 3.0 * m * q - std::fabs(tol * q);
            // AfMWD says on page 50:
            //   "Let e be the value of p/q at the step before the
            //   last one."
            // (That value is 's', both above and in the ALGOL.)
            //   "If |e| < δ or |p/q| ≥ ½|e| then we do a bisection"
            // Difference: the ALGOL tests |e| < δ elsewhere
            bool const k1 = p < std::fabs(0.5 * s * q);
            // Do not attempt to invert these conditions, e.g.
            // - if(a <  b) x() else y();
            // + if(b <= a) y() else x();
            // because NaNs break such reasoning; instead, make sure
            // the 'else' clause performs bisection.
            if(k0 && k1)
                {
                d = p / q;
                n = b + p / q;
                }
            else
                {
                impetus =
                      k0 ? parabola_not_single_valued
                    : k1 ? guarantee_linear_convergence
                    :      pis_aller
                    ;
                d = e = n - b;
                }
            }
        a = b;
        fa = fb;
        if(tol < std::fabs(d))
            {
            b = n;
            }
        else if(0.0 < m)
            {
            b += tol;
            }
        else
            {
            b -= tol;
            }

        fb = static_cast<double>(f(b));
        ++n_eval;
        expatiate();
        }
}

/// Return a rounded zero z of a function f within input bounds [a,b].
///
/// Intended to be used where f would round its argument anyway.
///
/// Brent's algorithm returns a zero z of the function f in
///   [bound0 bound1]
/// (a,b) to within a tolerance
///   6ϵ|z| + 2t
/// where t is an argument. For financial applications that traffic in
/// rounded currency values, the tolerance is a function of the number
/// of decimals to which values are rounded, thus:
///   6ϵ|z| + 10^(-decimals)
/// For such applications, this tolerance is often one-sided (governed
/// by the 'bias' argument), so that f(z) must be strictly greater
/// than or less than zero for return value z.
///
/// Design consideration: where should rounding be performed?
///
/// An earlier version of lmi_root() rounded each iterate 'b' just
/// before calling f to evaluate the function at that value, so no
/// separate decimal_root() was required. (Instead, a function object
/// to perform appropriate rounding was passed as an argument, which
/// defaulted to std::identity() if rounding was not wanted.) This
/// version provides a separate decimal_root() which interposes that
/// rounding in the fr lambda that it passes to lmi_root(), so that
/// when lmi_root() evaluates f(b), what it actually calls is:
///   fr(b) ≡ f(rounding_function(b))
/// Thus, lmi_root()'s internal 'b' (the point of departure for the
/// next succeeding iterate) is not identical to the value at which
/// f is evaluated. In theory, the relationship between 'b' and 'fb'
/// is thereby vitiated, which may slow convergence in the vicinity
/// of a root; but it doesn't matter at all in the intended use case,
/// where
///   f(b) ≡ fr(b) ≡ f(rounding_function(b))
/// because the external f rounds its argument in exactly the same
/// (idempotent) manner anyway.
///
/// Consequently, lmi_root() may call this modified f with successive
/// approximations that round to the same value. To avoid superfluous
/// evaluations, a map of {b, f(b)} is stored; when f is costly to
/// evaluate and the number of evaluations is not too large, the map's
/// overhead is negligible.
///
/// Another reason to avoid rounding each iteration inside lmi_root()
/// is that it is incompatible with offering binary64_midpoint() as an
/// alternative to the arithmetic mean. Suppose that the unrounded
/// true root is a small number close to zero, the a priori bounds are
/// [0,1.0e100], and iterates are to be rounded to a reasonable number
/// of decimals (say, |decimals| ≤ DBL_DIG). Then the lower bound, if
/// rounded, would tend to stay fixed at zero, because
///   1.09631e-104 ≈ binary64_midpoint(0.0, 1.0e100) [rounds to zero]
///   1.11875e-154 ≈ binary64_midpoint(0.0, 1.0e0)   [rounds to zero]
/// and convergence would (slowly) proceed by reducing the (remote)
/// upper bound. A smallest possible nonzero value exists:
///   double const least_positive = std::pow(10.0, -decimals);
///   (here, equal to two times the tolerance passed to lmi_root())
/// but it can't be approached from the bottom. This raises the
/// question whether a (not yet rounded) iterate x such that
///   0.0 < x < least_positive
/// should be forced to 'least_positive'. The answer is "no". With no
/// such deliberate forcing, Brent's method increments 'b' by ±'tol',
/// thus updating the lower bound, and evaluates the function at that
/// new point (which is exactly 'tol' if 'b' was zero). If this new
/// iterate rounds to 'least_positive', then that outcome arose
/// naturally without writing any code to force it. Otherwise, it
/// rounds to zero, so the lower bound was adjusted without the cost
/// of another function evaluation (because of caching here).

template<typename FunctionalType>
root_type decimal_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,root_bias       bias
    ,int             decimals
    ,int             sprauchling_limit = INT_MAX
    ,std::ostream&   os_trace          = null_stream()
    )
{
    round_to<double> const round_dec {decimals, r_to_nearest};

    std::unordered_map<double,double> m;

    auto fr = [&](double x) // f(), rounded
        {
        double const r = round_dec(x);
        auto const i = m.find(r);
        if(m.end() != i)
            {
            os_trace
                << "Superfluous evaluation at "
                << x
                << ", which rounds to "
                << r
                << ", avoided."
                << std::endl
                ;
            return i->second;
            }
        else
            {
            return m[r] = static_cast<double>(f(r));
            }
        };

    auto z = lmi_root
        (fr
        ,round_dec(bound0)
        ,round_dec(bound1)
        ,0.5 * std::pow(10.0, -decimals)
        ,sprauchling_limit
        ,os_trace
        ,bias
        );
    z.root = round_dec(z.root);
    os_trace << " function evaluations: " << z.n_eval;
    z.n_eval = lmi::ssize(m);
    os_trace << " " << z.n_eval << " nominal, actual" << std::endl;
    os_trace << " return value: " << z.root << " (rounded)" << std::endl;
    return z;
}

/// An instrumented translation of Brent's reference implementation.
///
/// Deviation from the original ALGOL:
///
/// The ALGOL original calculates and stores a correction term (called
/// 'i' on page 49 of AfMWD, but 'd' in the ALGOL) for bisection as
/// well as for other interpolation techniques, then adds it to 'b'
/// when appropriate. This can lead to a catastrophic cancellation,
/// as in this actual example:
///   -1.02311777153193876348e+49 b
///   -0.0106034417457945805141   c
///   -3.18454409903526645858e+23 binary64_midpoint(c, b)
///    1.02311777153193876348e+49 binary64_midpoint(c, b) - b
///    0.0                   b + (binary64_midpoint(c, b) - b)
/// which iterates to a new point outside the known [c,b] bounds. Even
/// though no such drastic example has been seen with the arithmetic
/// mean that Brent uses, less drastic examples occur in unit tests.
/// The catastrophic cancellation is conditionally avoided by storing
/// the next iterate in new variable 'n' (for "next") whenever 'd' is
/// calculated, and then assigning it directly to 'b' instead of
/// incrementing 'b' by 'd'.

template<typename FunctionalType>
double brent_zero
    (FunctionalType& f
    ,double          a
    ,double          b
    ,double          t
    ,std::ostream&   os_trace = null_stream()
    )
{
    // Returns a zero of the function f in the given interval [a,b],
    // to within a tolerance 6ϵ|ζ| + 2t, where ϵ is the relative
    // machine precision and t is a positive tolerance. Assumes
    // that f(a) and f(b) have different signs.

    int          n_iter  {0};
    int          n_eval  {0};
    root_impetus impetus {evaluate_bounds};

    os_trace
        << "#it #eval"
        << "            a           fa"
        << "            b           fb"
        << "            c           fc"
        << '\n'
        ;

    double c, d, e, fa, fb, fc, tol, m, n, p, q, r, s;

    auto expatiate = [&]()
        {
        os_trace
            <<        std::setw(3)  << n_iter
            << ' ' << std::setw(3)  << n_eval
            << ' '                  << impetus
            << ' ' << std::setw(12) << a << ' ' << std::setw(12) << fa
            << ' ' << std::setw(12) << b << ' ' << std::setw(12) << fb
            << ' ' << std::setw(12) << c << ' ' << std::setw(12) << fc
            << std::endl
            ;
        };

    auto recapitulate = [&]()
        {
        scoped_ios_format meaningless_name(os_trace);
        os_trace
            << n_iter << " iterations, "
            << n_eval << " evaluations; final interval:\n"
            << std::setprecision(DECIMAL_DIG)
            << std::showpos
            << " b "  << std::setw(12) << b
            << " fb " << std::setw(12) << fb << "\n"
            << " c "  << std::setw(12) << c
            << " fc " << std::setw(12) << fc
            << std::endl
            ;
        };

    fa = f(a);
    ++n_eval;
    fb = f(b);
    ++n_eval;
    c = fc = 0.0; // Zero-initialize before calling expatiate().
    expatiate();
  interpolate:
    c = a; fc = fa; d = e = b - a;
    impetus = force_b_and_c_to_bracket_root;
    expatiate();
  extrapolate:
    if(std::fabs(fc) < std::fabs(fb))
        {
         a =  b;  b =  c;  c =  a;
        fa = fb; fb = fc; fc = fa;
        impetus = force_b_to_be_best_approximation;
        expatiate();
        }
    tol = 2.0 * DBL_EPSILON * std::fabs(b) + t;
    m = 0.5 * (c - b);
    n = std::midpoint(b, c);
    if(tol < std::fabs(m) && 0.0 != fb)
        {
        // See if a bisection is forced.
        if(std::fabs(e) < tol)
            {
            impetus = dithering_near_root;
            d = e = n - b;
            }
        else if(std::fabs(fa) <= std::fabs(fb))
            {
            impetus = secant_out_of_bounds;
            d = e = n - b;
            }
        else
            {
            s = fb / fa;
            if(a == c)
                {
                // Linear interpolation.
                impetus = interpolate_linear;
                p = 2.0 * m * s;
                q = 1.0 - s;
                }
            else
                {
                // Inverse quadratic interpolation.
                impetus = interpolate_inverse_quadratic;
                q = fa / fc;
                r = fb / fc;
                p = s * (2.0 * m * q * (q - r) - (b - a) * (r - 1.0));
                q = (q - 1.0) * (r - 1.0) * (s - 1.0);
                }
            if(0.0 < p)
                {
                q = -q;
                }
            else
                {
                p = -p;
                }
            s = e;
            e = d;
            bool const k0 = 2.0 * p < 3.0 * m * q - std::fabs(tol * q);
            bool const k1 = p < std::fabs(0.5 * s * q);
            if(k0 && k1)
                {
                d = p / q;
                n = b + p / q;
                }
            else
                {
                impetus =
                      k0 ? parabola_not_single_valued
                    : k1 ? guarantee_linear_convergence
                    :      pis_aller
                    ;
                d = e = n - b;
                }
            }
        a = b; fa = fb;
        if(tol < std::fabs(d))
            {
#if 0  // See "catastrophic cancellation" above.
            b += d;
#else  // 1
            b = n;
#endif // 1
            }
        else if(0.0 < m)
            {
            b += tol;
            }
        else
            {
            b -= tol;
            }
        fb = f(b);
        ++n_eval;
        expatiate();
        ++n_iter;
        if((0.0 < fb) == (0.0 < fc))
            {goto interpolate;}
        else
            {goto extrapolate;}
        }
    recapitulate();
    os_trace << " return value: " << b << " = b" << std::endl;
    return b;
}

/// A C++ translation of Brent's algol60 reference implementation.

template<typename FunctionalType>
double brent_zero_reference
    (FunctionalType& f
    ,double          a
    ,double          b
    ,double          t
    )
{
    // Returns a zero of the function f in the given interval [a,b],
    // to within a tolerance 6ϵ|ζ| + 2t, where ϵ is the relative
    // machine precision and t is a positive tolerance. Assumes
    // that f(a) and f(b) have different signs.
    double c, d, e, fa, fb, fc, tol, m, p, q, r, s;
    fa = f(a);
    fb = f(b);
  interpolate:
    c = a; fc = fa; d = e = b - a;
  extrapolate:
    if(std::fabs(fc) < std::fabs(fb))
        {
         a =  b;  b =  c;  c =  a;
        fa = fb; fb = fc; fc = fa;
        }
    tol = 2.0 * DBL_EPSILON * std::fabs(b) + t;
    m = 0.5 * (c - b);
    if(tol < std::fabs(m) && 0.0 != fb)
        {
        // See if a bisection is forced.
        if(std::fabs(e) < tol || std::fabs(fa) <= std::fabs(fb))
            {
            d = e = m;
            }
        else
            {
            s = fb / fa;
            if(a == c)
                {
                // Linear interpolation.
                p = 2.0 * m * s;
                q = 1.0 - s;
                }
            else
                {
                // Inverse quadratic interpolation.
                q = fa / fc;
                r = fb / fc;
                p = s * (2.0 * m * q * (q - r) - (b - a) * (r - 1.0));
                q = (q - 1.0) * (r - 1.0) * (s - 1.0);
                }
            if(0.0 < p)
                {
                q = -q;
                }
            else
                {
                p = -p;
                }
            s = e;
            e = d;
            if
                (  2.0 * p < 3.0 * m * q - std::fabs(tol * q)
                && p < std::fabs(0.5 * s * q)
                )
                {
                d = p / q;
                }
            else
                {
                d = e = m;
                }
            }
        a = b; fa = fb;
        if(tol < std::fabs(d))
            {
            b += d;
            }
        else if(0.0 < m)
            {
            b += tol;
            }
        else
            {
            b -= tol;
            }
        fb = f(b);
        if((0.0 < fb) == (0.0 < fc))
            {goto interpolate;}
        else
            {goto extrapolate;}
        }
    return b;
}

#endif // zero_hpp
