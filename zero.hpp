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

#include "null_stream.hpp"
#include "round_to.hpp"

#include <cfloat>                       // DBL_EPSILON, DECIMAL_DIG
#include <cmath>                        // fabs(), pow()
#include <functional>                   // function(), identity()
#include <iomanip>                      // setprecision(), setw()
#include <limits>
#include <ostream>
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
};

namespace detail
{
using RoundT = std::function<double(double)>;
} // namespace detail

/// Workaround for clang--see:
///    https://lists.nongnu.org/archive/html/lmi/2021-07/msg00001.html
/// It is hoped that this can be replaced by std::identity soon.

struct lmi_identity
{
    using is_transparent = void;

    template<typename T>
    constexpr T&& operator()(T&& t) const noexcept
        {return std::forward<T>(t);}
};

/// Return a zero z of a function f within input bounds [a,b].
///
/// Preconditions: bounds are distinct after rounding; and either
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
/// Brent's original algorithm returns unrounded values. To support
/// financial applications that traffic in rounded currency values,
/// iterands are optionally rounded before each function evaluation.
/// Rounding must be performed within the root-finding algorithm: for
/// an unrounded return value r, f(r) and f(round(r)) might easily
/// have different signs. The default rounding function returns its
/// argument unchanged, so this implementation remains suitable for
/// the unrounded case.
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
///
/// Note 4. Each iterand is rounded, so it might equal an iterand that
/// has already been evaluated. In that case, the known value is used,
/// because evaluation is assumed to be costly, and in practice one
/// bound stays fixed to within rounding (for instance, at the edge of
/// a discontinuity) often enough that it is worthwhile to avoid
/// superfluous reevaluation.

template<typename FunctionalType>
root_type lmi_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,double          tolerance
    ,std::ostream&   os_trace  = null_stream()
    ,root_bias       bias      = bias_none
//  ,detail::RoundT  round_dec = std::identity()
    ,detail::RoundT  round_dec = lmi_identity()
    )
{
    constexpr double epsilon {std::numeric_limits<double>::epsilon()};

    int              n_iter  {0};
    root_impetus     impetus {evaluate_bounds};

    os_trace
        << "#eval"
        << "            a           fa"
        << "            b           fb"
        << "            c           fc"
        << '\n'
        ;

    // Declarations must precede lambda.
    double a  {};
    double fa {};
    double b  {};
    double fb {};
    double c  {};
    double fc {};

    auto expatiate = [&]()
        {
        os_trace
            <<        std::setw(3)  << n_iter
            << ' '                  << impetus
            << ' ' << std::setw(12) << a << ' ' << std::setw(12) << fa
            << ' ' << std::setw(12) << b << ' ' << std::setw(12) << fb
            << ' ' << std::setw(12) << c << ' ' << std::setw(12) << fc
            << std::endl
            ;
        };

    auto recapitulate = [&]()
        {
        os_trace
            << n_iter
            << " iterations; final interval:\n"
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

    a = round_dec(bound0);
    b = round_dec(bound1);

    if(a == b)
        {
        recapitulate();
        return {a, improper_bounds, n_iter};
        }

    fa = static_cast<double>(f(a));
    ++n_iter;
    if(0.0 == fa) // Note 0.
        {
        recapitulate();
        return {a, root_is_valid, n_iter};
        }

    fb = static_cast<double>(f(b));
    ++n_iter;
    expatiate();
    if(0.0 == fb) // Note 0 [bis].
        {
        recapitulate();
        return {b, root_is_valid, n_iter};
        }

    // f(a) and f(b) must have different signs.
    if((0.0 < fa) == (0.0 < fb))
        {
        recapitulate();
        return {0.0, root_not_bracketed, n_iter};
        }

    fc = fb; // Note 1.
    c = b;
    double d = b - a;
    double e = d;

    for(;;)
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
        double tol = 2.0 * epsilon * std::fabs(b) + t;
        double m = 0.5 * (c - b);
        if(0.0 == fb || std::fabs(m) <= tol) // Note 2.
            {
            if
                (   bias_none   == bias
                ||  bias_lower  == bias && fb <= 0.0
                ||  bias_higher == bias && 0.0 <= fb
                )
                {
                recapitulate();
                return {b, root_is_valid, n_iter};
                }
            else if(std::fabs(m) <= 2.0 * epsilon * std::fabs(c) + t)
                {
                recapitulate();
                return {c, root_is_valid, n_iter};
                }
            else
                {
                ; // Do nothing.
                }
            }
        if(std::fabs(e) < tol)
            {
            impetus = dithering_near_root;
            d = e = m;
            }
        else if(std::fabs(fa) <= std::fabs(fb))
            {
            impetus = secant_out_of_bounds;
            d = e = m;
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
                }
            else
                {
                impetus =
                      k0 ? parabola_not_single_valued
                    : k1 ? guarantee_linear_convergence
                    :      pis_aller
                    ;
                d = e = m;
                }
            }
        a = b;
        fa = fb;
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
        b = round_dec(b);

        if(b == a) // Note 4.
            {
            fb = fa;
            }
        else if(b == c)
            {
            fb = fc;
            }
        else
            {
            fb = static_cast<double>(f(b));
            ++n_iter;
            expatiate();
            }
        }
}

/// Return a rounded zero z of a function f within input bounds [a,b].
///
/// Brent's original algorithm returns a zero z of the function f in
///   [bound0 bound1]
/// (a,b) to within a tolerance
///   6ϵ|z| + 2t
/// where t is an argument. For financial applications, the tolerance
/// is often one-sided, so that f(z) must be strictly greater than or
/// less than zero, and the tolerance is more conveniently expressed
/// as a number of decimals. An adjustment is made for the constant
/// factor in Brent's t term, so that this implementation's tolerance
/// becomes
///   6ϵ|z| + 10^(-decimals)
/// instead.

template<typename FunctionalType>
root_type decimal_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,root_bias       bias
    ,int             decimals
    ,std::ostream&   os_trace = null_stream()
    )
{
    round_to<double> const round_dec {decimals, r_to_nearest};

    return lmi_root
        (f
        ,bound0
        ,bound1
        ,0.5 * std::pow(10.0, -decimals)
        ,os_trace
        ,bias
        ,detail::RoundT(round_dec)
        );
}

/// An instrumented translation of Brent's reference implementation.

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
    root_impetus impetus {evaluate_bounds};

    os_trace
        << "#eval"
        << "            a           fa"
        << "            b           fb"
        << "            c           fc"
        << '\n'
        ;

    double c, d, e, fa, fb, fc, tol, m, p, q, r, s;

    auto expatiate = [&]()
        {
        os_trace
            <<        std::setw(3)  << n_iter
            << ' '                  << impetus
            << ' ' << std::setw(12) << a << ' ' << std::setw(12) << fa
            << ' ' << std::setw(12) << b << ' ' << std::setw(12) << fb
            << ' ' << std::setw(12) << c << ' ' << std::setw(12) << fc
            << std::endl
            ;
        };

    auto recapitulate = [&]()
        {
        os_trace
            << n_iter
            << " iterations; final interval:\n"
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
    ++n_iter;
    fb = f(b);
    ++n_iter;
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
    if(tol < std::fabs(m) && 0.0 != fb)
        {
        // See if a bisection is forced.
        if(std::fabs(e) < tol)
            {
            impetus = dithering_near_root;
            d = e = m;
            }
        else if(std::fabs(fa) <= std::fabs(fb))
            {
            impetus = secant_out_of_bounds;
            d = e = m;
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
                }
            else
                {
                impetus =
                      k0 ? parabola_not_single_valued
                    : k1 ? guarantee_linear_convergence
                    :      pis_aller
                    ;
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
        ++n_iter;
        expatiate();
        if((0.0 < fb) == (0.0 < fc))
            {goto interpolate;}
        else
            {goto extrapolate;}
        }
    recapitulate();
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
