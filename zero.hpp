// Root finding by Brent's method.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
///
/// For the evocative term "dithering", see:
///   https://people.eecs.berkeley.edu/~wkahan/Math128/RealRoots.pdf

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
/// [Author's note: I thought this might be a brand-new discovery, as
/// it's mentioned (almost) nowhere in the literature, although it's
/// obvious once you think of it. It turns out to be an independent
/// rediscovery--see Don Clugston's talk:
///   https://dconf.org/2016/talks/clugston.pdf
/// in particular the parts with the following titles:
///   "The Binary Chop That Isn't"
///   "Binary Chop For Real"
/// which I found only after I had implemented this. Clugston (DConf
/// video) remarks: "this is a couple of orders of magnitude better
/// than the industry state of the art...and nobody knows about it."]
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
///
/// A related idea for possible future investigation
///
/// Clugston's comparable discussion of a binary mean for root finding
///   https://dconf.org/2016/talks/clugston.pdf
/// states that
///   "TOMS 748 has a similar problem with linear interpolation"
/// That remark would apply to Brent's method as well. It would be
/// interesting to replace linear interpolation in lmi_root() with
/// std::lerp(), and then write and experiment with a binary64_lerp().

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
/// R. P. Brent, "Algorithms for Minimization without Derivatives"
/// ISBN 0-13-022335-2
///
/// Rationale for choice of algorithm
///
/// "Brent's method combines the sureness of bisection with the speed
/// of a higher-order method when appropriate. We recommend it as the
/// method of choice for general one-dimensional root finding where a
/// function's values only (and not its derivative or functional form)
/// are available." --Press et al., "Numerical Recipes" (3rd ed. 2007)
///
/// Numerous papers claim to improve on Brent's method. TOMS 748
/// purports to require about five percent fewer function evaluations,
/// but that claim is debunked here:
///   https://www.nongnu.org/lmi/toms_748.html
/// Chandrupatla proposed a more stringent criterion for accepting IQI
/// iterates, and a simplified algorithm that uses it. However, actual
/// measurements (tabulated in Note 4) show that, for the 388 solves
/// in lmi's regression test, Brent's method is best for lmi.
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
///
/// Note 4. Brent (AfMWD, page 51) says:
/// | When inverse quadratic interpolation is used the interpolating
/// | parabola cannot be a good approximation to f unless it is
/// | single-valued between (b, f(b)) and (c, f(c)). Thus, it is
/// | natural to accept the point i if it lies between b and c, and
/// | up to three-quarters of the way from b to c: consider the
/// | limiting case where the interpolating parabola has a vertical
/// | tangent at c and f(b) = -f(c). Hence, we reject i [i.e. b + p/q]
/// | if 2|p| ≥ 3|mq|.
/// (That's a 3/4 rule even if it looks like 2/3, because, omitting
/// '±' and '∓' for clarity:
///     i = b + p/q
///     m = (c-b)/2  // half the distance (not the midpoint)
///     p/q < 3(c-b)/4 ⇒ p/q < 3m/2 ⇒ 2p < 3mq
/// .) Brent's method simply uses that "three-quarters" heuristic; the
/// other words serve only to establish its plausibility.
///
/// Instead of casually considering a vertical tangent at a single
/// endpoint, Chandrupatla formulated a rigorous two-sided criterion
/// to reject an IQI iterate outside a region defined by vertical
/// tangents at both endpoints--see the analysis and graph here:
///   https://github.com/SimpleArt/solver/wiki/Methods#chandrupatla
/// which is simple and compelling, and seems to realize Brent's
/// declared intention better. However, for the 388 solves in lmi's
/// regression tests, Brent's heuristic outperforms Chandrupatla's
/// criterion (TOMS 748 is included to complete the table):
///
///   evaluations  max   mean  sample-std-dev    commit     algorithm
///       7332      65   18.9       7.13       028b4541c      Brent
///       7622      76   19.6       7.65       fc8b1a900    TOMS 748
///       9149      59   23.6      11.13       ac5731f52  Chandrupatla
///       8545      66   22.0      12.90   (Brent-Chandrupatla hybrid)
///
/// Chandrupatla's simplified root-finding algorithm (third row)
/// always uses his IQI criterion but never takes a secant step.
/// The last row's "hybrid" is Brent's method with Chandrupatla's
/// criterion for rejecting IQI (see commit ac5731f52). Evidently the
/// "three-quarters" heuristic performs well in many cases where the
/// mathematical rationalization for it does not actually hold true.
/// That is, the heuristic is "unreasonably effective", at least
/// during the initial "flailing" stage described by Kahan:
///   https://people.eecs.berkeley.edu/~wkahan/Math128/RealRoots.pdf
/// so the word "cannot" in the AfMWD quote above is too strong.

template<typename FunctionalType>
root_type lmi_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,double          tolerance
    ,std::ostream&   os_trace
    ,int             sprauchling_limit = INT_MAX
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
            // AfMWD says on page 51 [see Note 4]:
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

template<typename FunctionalType>
root_type lmi_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,double          tolerance
    ,int             sprauchling_limit = INT_MAX
    ,root_bias       bias              = bias_none
    )
{
    std::ostream null_ostream(&null_streambuf());
    null_ostream.setstate(std::ios::badbit);
    return lmi_root
        (f
        ,bound0
        ,bound1
        ,tolerance
        ,null_ostream
        ,sprauchling_limit
        ,bias
        );
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
    ,std::ostream&   os_trace
    ,int             sprauchling_limit = INT_MAX
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
        ,os_trace
        ,sprauchling_limit
        ,bias
        );
    z.root = round_dec(z.root);
    os_trace << " function evaluations: " << z.n_eval;
    z.n_eval = lmi::ssize(m);
    os_trace << " " << z.n_eval << " nominal, actual" << std::endl;
    os_trace << " return value: " << z.root << " (rounded)" << std::endl;
    return z;
}

template<typename FunctionalType>
root_type decimal_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,root_bias       bias
    ,int             decimals
    ,int             sprauchling_limit = INT_MAX
    )
{
    std::ostream null_ostream(&null_streambuf());
    null_ostream.setstate(std::ios::badbit);
    return decimal_root
        (f
        ,bound0
        ,bound1
        ,bias
        ,decimals
        ,null_ostream
        ,sprauchling_limit
        );
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
    ,std::ostream&   os_trace
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

template<typename FunctionalType>
double brent_zero
    (FunctionalType& f
    ,double          a
    ,double          b
    ,double          t
    )
{
    std::ostream null_ostream(&null_streambuf());
    null_ostream.setstate(std::ios::badbit);
    return brent_zero
        (f
        ,a
        ,b
        ,t
        ,null_ostream
        );
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

// Source:
//  https://www.netlib.org/toms-2014-06-10/748
// separated into '.f' files manually, and translated thus:
//   f2c -a *.f
// Compiled thus, with minimal editing, for validation:
//   gcc driver.c enclofx.c -lf2c -lm 2>&1 |less
// excluding 'exdrive.c' because it is apparently an alternative
// to 'driver.c'. The code here is a combination of 'driver.c'
// and 'enclofx.c', edited extensively to make it work with lmi.

int tole_(double* b, double* tol, int* neps, double* eps);
int func_(int* /* nprob */, double* x, double* fx);
template<typename FunctionalType>
int rroot_(FunctionalType& f, int* nprob, int* neps, double* eps,
        double* a, double* b, double* root, int* n_eval);
template<typename FunctionalType>
int brackt_(FunctionalType& f, int* nprob, double* a, double* b,
        double* c0, double* fa, double* fb, double* tol,
        int* neps, double* eps, double* d_0, double* fd);
int isign_(double* x);
int newqua_(double* a, double* b, double* d_0,
        double* fa, double* fb, double* fd, double* c0,
        int* k);
int pzero_(double* a, double* b, double* d_0,
        double* e, double* fa, double* fb, double* fd,
        double* fe, double* c0);
int rmp_(double* rel);

inline int tole_(double* b, double* tol, int* neps, double* eps)
{
    /* System generated locals */
    int i1;

    /* Local variables */
    int i0;

/* DETERMINES THE TERMINATION CRITERION. */
/*  B    -- DOUBLE PRECISION. */
/*  NEPS -- INTEGER. */
/*  EPS  -- DOUBLE PRECISION. */
/*  TOL  -- DOUBLE PRECISION. OUTPUT AS THE TERMINATION CRITERION. */
/*           TOL =2*(2*EPS*|B| + 10D-{NEPS}),  IF NEPS IS NOT 1000; */
/*    AND    TOL =2*(2*EPS*|B|),               IF NEPS = 1000. */
    if (*neps == 1000) {
        *tol = 0.;
    } else {
        *tol = 1.;
// GWC modification: change this from 1 to 1/2 iff the same
// tolerance as decimal_root() is wanted.
//      *tol = 0.5;
        i1 = *neps;
        for (i0 = 1; i0 <= i1; ++i0) {
            *tol /= 10.;
/* L10: */
        }
    double const tolx = 0.5 * std::pow(10.0, -*neps);
//  std::cout << "tolerance " << *tol << " should equal " << tolx << std::endl;
    // Use the calculation copied from decimal_root() instead:
    // it differs slightly, and is probably more exact than
    // dividing repeatedly by ten.
    *tol = tolx;
    }
    *tol += std::fabs(*b) * 2. * *eps;
    *tol *= 2.;
//  std::cout << "actual tolerance " << *tol << std::endl;
    return 0;
}

inline int func_(int* /* nprob */, double* x, double* fx)
{
    *fx = std::sin(*x) - *x / 2.;
    return 0;
}

/* *** enclofx.f */
template<typename FunctionalType>
int rroot_(FunctionalType& f, int* nprob, int* neps, double* eps,
        double* a, double* b, double* root, int* n_eval)
{
    /* Table of constant values */

    static int c2 = 2;
    static int c3 = 3;

    /* System generated locals */
    double d_1;

    /* Local variables */
    double c0, d_0, e, u, a0, b0, fa, fb, fd, fe, fu, tol;
    double prof;
    int itnum;

/* FINDS EITHER AN EXACT SOLUTION OR AN APPROXIMATE SOLUTION OF THE */
/* EQUATION F(X)=0 IN THE INTERVAL [A,B]. AT THE BEGINING OF EACH */
/* ITERATION, THE CURRENT ENCLOSING INTERVAL IS RECORDED AS [A0,B0]. */
/* THE FIRST ITERATION IS SIMPLY A SECANT STEP. STARTING WITH THE */
/* SECOND ITERATION, THREE STEPS ARE TAKEN IN EACH ITERATION. FIRST */
/* TWO STEPS ARE EITHER QUADRATIC INTERPOLATION OR CUBIC INVERSE */
/* INTERPOLATION. THE THIRD STEP IS A DOUBLE-SIZE SECANT STEP. IF THE */
/* DIAMETER OF THE ENCLOSING INTERVAL OBTAINED AFTER THOSE THREE STEPS */
/* IS LARGER THAN 0.5*(B0-A0), THEN AN ADDITIONAL BISECTION STEP WILL */
/* BE TAKEN. */
/*  NPROB -- INTEGER. INDICATING THE PROBLEM TO BE SOLVED; */
/*  NEPS  -- INTEGER. USED TO DETERMINE THE TERMINATION CRITERION; */
/*  EPS   -- DOUBLE PRECISION. USED IN THE TERMINATION CRITERION; */
/*  A,B   -- DOUBLE PRECISION. INPUT AS THE INITIAL INTERVAL AND */
/*           OUTPUT AS THE ENCLOSING INTERVAL AT THE TERMINATION; */
/*  ROOT  -- DOUBLE PRECISION. OUTPUT SOLUTION OF THE EQUATION. */

/* INITIALIZATION. SET THE NUMBER OF ITERATION AS 0. CALL SUBROUTINE */
/* "FUNC" TO OBTAIN THE INITIAL FUNCTION VALUES F(A) AND F(B). SET */
/* DUMB VALUES FOR THE VARIABLES "E" AND "FE". */

    itnum = 0;
    fa = f(*a); // f(a, fa);
    fb = f(*b); // f(b, fb);
    *n_eval = 2; // Two evaluations have now been performed.
    e = 1e5;
    fe = 1e5;

/* ITERATION STARTS. THE ENCLOSING INTERVAL BEFORE EXECUTING THE */
/* ITERATION IS RECORDED AS [A0, B0]. */

  L10:
    a0 = *a;
    b0 = *b;

/* UPDATES THE NUMBER OF ITERATION. */

    ++itnum;

/* CALCULATES THE TERMINATION CRITERION. STOPS THE PROCEDURE IF THE */
/* CRITERION IS SATISFIED. */

    if (std::fabs(fb) <= std::fabs(fa)) {
        tole_(b, &tol, neps, eps);
    } else {
        tole_(a, &tol, neps, eps);
    }
    if (*b - *a <= tol) {
        goto L400;
    }

/* FOR THE FIRST ITERATION, SECANT STEP IS TAKEN. */

    if (itnum == 1) {
        c0 = *a - fa / (fb - fa) * (*b - *a);

/* CALL SUBROUTINE "BRACKT" TO GET A SHRINKED ENCLOSING INTERVAL AS */
/* WELL AS TO UPDATE THE TERMINATION CRITERION. STOP THE PROCEDURE */
/* IF THE CRITERION IS SATISFIED OR THE EXACT SOLUTION IS OBTAINED. */

        brackt_(f, nprob, a, b, &c0, &fa, &fb, &tol, neps, eps, &d_0, &fd);
        ++*n_eval;
        if (fa == 0. || *b - *a <= tol) {
            goto L400;
        }
        goto L10;
    }

/* STARTING WITH THE SECOND ITERATION, IN THE FIRST TWO STEPS, EITHER */
/* QUADRATIC INTERPOLATION IS USED BY CALLING THE SUBROUTINE "NEWQUA" */
/* OR THE CUBIC INVERSE INTERPOLATION IS USED BY CALLING THE SUBROUTINE */
/* "PZERO". IN THE FOLLOWING, IF "PROF" IS NOT EQUAL TO 0, THEN THE */
/* FOUR FUNCTION VALUES "FA", "FB", "FD", AND "FE" ARE DISTINCT, AND */
/* HENCE "PZERO" WILL BE CALLED. */

#if defined LMI_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wconditional-uninitialized"
#endif // defined LMI_CLANG
    // clang: "variable 'fd' may be uninitialized when used here"
    prof = (fa - fb) * (fa - fd) * (fa - fe) * (fb - fd) * (fb - fe) * (fd -
            fe);
#if defined LMI_CLANG
#   pragma clang diagnostic pop
#endif // defined LMI_CLANG
    if (itnum == 2 || prof == 0.) {
        newqua_(a, b, &d_0, &fa, &fb, &fd, &c0, &c2);
    } else {
        pzero_(a, b, &d_0, &e, &fa, &fb, &fd, &fe, &c0);
        if ((c0 - *a) * (c0 - *b) >= 0.) {
            newqua_(a, b, &d_0, &fa, &fb, &fd, &c0, &c2);
        }
    }
    e = d_0;
    fe = fd;

/* CALL SUBROUTINE "BRACKT" TO GET A SHRINKED ENCLOSING INTERVAL AS */
/* WELL AS TO UPDATE THE TERMINATION CRITERION. STOP THE PROCEDURE */
/* IF THE CRITERION IS SATISFIED OR THE EXACT SOLUTION IS OBTAINED. */

    brackt_(f, nprob, a, b, &c0, &fa, &fb, &tol, neps, eps, &d_0, &fd);
    ++*n_eval;
    if (fa == 0. || *b - *a <= tol) {
        goto L400;
    }
    prof = (fa - fb) * (fa - fd) * (fa - fe) * (fb - fd) * (fb - fe) * (fd -
            fe);
    if (prof == 0.) {
        newqua_(a, b, &d_0, &fa, &fb, &fd, &c0, &c3);
    } else {
        pzero_(a, b, &d_0, &e, &fa, &fb, &fd, &fe, &c0);
        if ((c0 - *a) * (c0 - *b) >= 0.) {
            newqua_(a, b, &d_0, &fa, &fb, &fd, &c0, &c3);
        }
    }

/* CALL SUBROUTINE "BRACKT" TO GET A SHRINKED ENCLOSING INTERVAL AS */
/* WELL AS TO UPDATE THE TERMINATION CRITERION. STOP THE PROCEDURE */
/* IF THE CRITERION IS SATISFIED OR THE EXACT SOLUTION IS OBTAINED. */

    brackt_(f, nprob, a, b, &c0, &fa, &fb, &tol, neps, eps, &d_0, &fd);
    ++*n_eval;
    if (fa == 0. || *b - *a <= tol) {
        goto L400;
    }
    e = d_0;
    fe = fd;

/* TAKES THE DOUBLE-SIZE SECANT STEP. */

    if (std::fabs(fa) < std::fabs(fb)) {
        u = *a;
        fu = fa;
    } else {
        u = *b;
        fu = fb;
    }
    c0 = u - fu / (fb - fa) * 2. * (*b - *a);
#if defined LMI_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcomma"
#endif // defined LMI_CLANG
    // clang: "possible misuse of comma operator"
    if ((d_1 = c0 - u, std::fabs(d_1)) > (*b - *a) * .5) {
        c0 = *a + (*b - *a) * .5;
    }
#if defined LMI_CLANG
#   pragma clang diagnostic pop
#endif // defined LMI_CLANG

/* CALL SUBROUTINE "BRACKT" TO GET A SHRINKED ENCLOSING INTERVAL AS */
/* WELL AS TO UPDATE THE TERMINATION CRITERION. STOP THE PROCEDURE */
/* IF THE CRITERION IS SATISFIED OR THE EXACT SOLUTION IS OBTAINED. */

    brackt_(f, nprob, a, b, &c0, &fa, &fb, &tol, neps, eps, &d_0, &fd);
    ++*n_eval;
    if (fa == 0. || *b - *a <= tol) {
        goto L400;
    }

/* DETERMINES WHETHER AN ADDITIONAL BISECTION STEP IS NEEDED. AND TAKES */
/* IT IF NECESSARY. */

    if (*b - *a < (b0 - a0) * .5) {
        goto L10;
    }
    e = d_0;
    fe = fd;

/* CALL SUBROUTINE "BRACKT" TO GET A SHRINKED ENCLOSING INTERVAL AS */
/* WELL AS TO UPDATE THE TERMINATION CRITERION. STOP THE PROCEDURE */
/* IF THE CRITERION IS SATISFIED OR THE EXACT SOLUTION IS OBTAINED. */

    d_1 = *a + (*b - *a) * .5;
    brackt_(f, nprob, a, b, &d_1, &fa, &fb, &tol, neps, eps, &d_0, &fd);
    ++*n_eval;
    if (fa == 0. || *b - *a <= tol) {
        goto L400;
    }
    goto L10;

/* TERMINATES THE PROCEDURE AND RETURN THE "ROOT". */

  L400:
    *root = *a;
    return 0;
}

template<typename FunctionalType>
int brackt_(FunctionalType& f, int* nprob, double* a, double* b,
        double* c0, double* fa, double* fb, double* tol,
        int* neps, double* eps, double* d_0, double* fd)
{
    (void)&nprob;
    double fc;

/* GIVEN CURRENT ENCLOSING INTERVAL [A,B] AND A NUMBER C IN (A,B), IF */
/* F(C)=0 THEN SETS THE OUTPUT A=C. OTHERWISE DETERMINES THE NEW */
/* ENCLOSING INTERVAL: [A,B]=[A,C] OR [A,B]=[C,B]. ALSO UPDATES THE */
/* TERMINATION CRITERION CORRESPONDING TO THE NEW ENCLOSING INTERVAL. */
/*  NPROB   -- INTEGER. INDICATING THE PROBLEM TO BE SOLVED; */
/*  A,B     -- DOUBLE PRECISION. [A,B] IS INPUT AS THE CURRENT */
/*             ENCLOSING INTERVAL AND OUTPUT AS THE SHRINKED NEW */
/*             ENCLOSING INTERVAL; */
/*  C       -- DOUBLE PRECISION. USED TO DETERMINE THE NEW ENCLOSING */
/*             INTERVAL; */
/*  D       -- DOUBLE PRECISION. OUTPUT: IF THE NEW ENCLOSING INTERVAL */
/*             IS [A,C] THEN D=B, OTHERWISE D=A; */
/*  FA,FB,FD-- DOUBLE PRECISION. FA=F(A), FB=F(B), AND FD=F(D); */
/*  TOL     -- DOUBLE PRECISION. INPUT AS THE CURRENT TERMINATION */
/*             CRITERION AND OUTPUT AS THE UPDATED TERMINATION */
/*             CRITERION ACCORDING TO THE NEW ENCLOSING INTERVAL; */
/*  NEPS    -- INTEGER. USED TO DETERMINE THE TERMINATION CRITERION; */
/*  EPS     -- DOUBLE PRECISION. USED IN THE TERMINATION CRITERION. */

/* ADJUST C IF (B-A) IS VERY SMALL OR IF C IS VERY CLOSE TO A OR B. */

    *tol *= .7;
    if (*b - *a <= *tol * 2.) {
        *c0 = *a + (*b - *a) * .5;
    } else if (*c0 <= *a + *tol) {
        *c0 = *a + *tol;
    } else {
        if (*c0 >= *b - *tol) {
            *c0 = *b - *tol;
        }
    }

/* CALL SUBROUTINE "FUNC" TO OBTAIN F(C) */

    fc = f(*c0); // f(c0, fc);

/* IF F(C)=0, THEN SET A=C AND RETURN. THIS WILL TERMINATE THE */
/* PROCEDURE IN SUBROUTINE "RROOT" AND GIVE THE EXACT SOLUTION OF */
/* THE EQUATION F(X)=0. */

    if (fc == 0.) {
        *a = *c0;
        *fa = 0.;
        *d_0 = 0.;
        *fd = 0.;
        return 0;
    }

/* IF F(C) IS NOT ZERO, THEN DETERMINE THE NEW ENCLOSING INTERVAL. */

    if (isign_(fa) * isign_(&fc) < 0) {
        *d_0 = *b;
        *fd = *fb;
        *b = *c0;
        *fb = fc;
    } else {
        *d_0 = *a;
        *fd = *fa;
        *a = *c0;
        *fa = fc;
    }

/* UPDATE THE TERMINATION CRITERION ACCORDING TO THE NEW ENCLOSING */
/* INTERVAL. */

    if (std::fabs(*fb) <= std::fabs(*fa)) {
        tole_(b, tol, neps, eps);
    } else {
        tole_(a, tol, neps, eps);
    }

/* END OF THE SUBROUTINE. */

    return 0;
} /* brackt_ */

inline int isign_(double* x)
{
    /* System generated locals */
    int ret_val;

/* INDICATES THE SIGN OF THE VARIABLE "X". */
/*  X     -- DOUBLE PRECISION. */
/*  ISIGN -- INTEGER. */
    if (*x > 0.) {
        ret_val = 1;
    } else if (*x == 0.) {
        ret_val = 0;
    } else {
        ret_val = -1;
    }
    return ret_val;
} /* isign_ */

inline int newqua_(double* a, double* b, double* d_0,
        double* fa, double* fb, double* fd, double* c0,
        int* k)
{
    /* System generated locals */
    int i1;

    /* Local variables */
    int i0;
    double a0, a1, a2, pc, pdc;
    int ierror;

/* USES K NEWTON STEPS TO APPROXIMATE THE ZERO IN (A,B) OF THE */
/* QUADRATIC POLYNOMIAL INTERPOLATING F(X) AT A, B, AND D. SAFEGUARD */
/* IS USED TO AVOID OVERFLOW. */
/*  A,B,D,FA,FB,FD -- DOUBLE PRECISION. D LIES OUTSIDE THE INTERVAL */
/*                    [A,B]. FA=F(A), FB=F(B), AND FD=F(D). F(A)F(B)<0. */
/*  C              -- DOUBLE PRECISION. OUTPUT AS THE APPROXIMATE ZERO */
/*                    IN (A,B) OF THE QUADRATIC POLYNOMIAL. */
/*  K              -- INTEGER. INPUT INDICATING THE NUMBER OF NEWTON */
/*                    STEPS TO TAKE. */

/* INITIALIZATION. FIND THE COEFFICIENTS OF THE QUADRATIC POLYNOMIAL. */

    ierror = 0;
    a0 = *fa;
    a1 = (*fb - *fa) / (*b - *a);
    a2 = ((*fd - *fb) / (*d_0 - *b) - a1) / (*d_0 - *a);

/* SAFEGUARD TO AVOID OVERFLOW. */

  L10:
    if (a2 == 0. || ierror == 1) {
        *c0 = *a - a0 / a1;
        return 0;
    }

/* DETERMINE THE STARTING POINT OF NEWTON STEPS. */

    if (isign_(&a2) * isign_(fa) > 0) {
        *c0 = *a;
    } else {
        *c0 = *b;
    }

/* START THE SAFEGUARDED NEWTON STEPS. */

    i1 = *k;
    for (i0 = 1; i0 <= i1; ++i0) {
        if (ierror == 0) {
            pc = a0 + (a1 + a2 * (*c0 - *b)) * (*c0 - *a);
            pdc = a1 + a2 * (*c0 * 2. - (*a + *b));
            if (pdc == 0.) {
                ierror = 1;
            } else {
                *c0 -= pc / pdc;
            }
        }
/* L20: */
    }
    if (ierror == 1) {
        goto L10;
    }
    return 0;
} /* newqua_ */

inline int pzero_(double* a, double* b, double* d_0,
        double* e, double* fa, double* fb, double* fd,
        double* fe, double* c0)
{
    double d21, d31, d32, q11, q21, q31, q22, q32, q33;

/* USES CUBIC INVERSE INTERPOLATION OF F(X) AT A, B, D, AND E TO */
/* GET AN APPROXIMATE ROOT OF F(X). THIS PROCEDURE IS A SLIGHT */
/* MODIFICATION OF AITKEN-NEVILLE ALGORITHM FOR INTERPOLATION */
/* DESCRIBED BY STOER AND BULIRSCH IN "INTRO. TO NUMERICAL ANALYSIS" */
/* SPRINGER-VERLAG. NEW YORK (1980). */
/*  A,B,D,E,FA,FB,FD,FE -- DOUBLE PRECISION. D AND E LIE OUTSIDE */
/*                         THE INTERVAL [A,B]. FA=F(A), FB=F(B), */
/*                         FD=F(D), AND FE=F(E). */
/*  C                   -- DOUBLE PRECISION. OUTPUT OF THE SUBROUTINE. */

    q11 = (*d_0 - *e) * *fd / (*fe - *fd);
    q21 = (*b - *d_0) * *fb / (*fd - *fb);
    q31 = (*a - *b) * *fa / (*fb - *fa);
    d21 = (*b - *d_0) * *fd / (*fd - *fb);
    d31 = (*a - *b) * *fb / (*fb - *fa);
    q22 = (d21 - q11) * *fb / (*fe - *fb);
    q32 = (d31 - q21) * *fa / (*fd - *fa);
    d32 = (d31 - q21) * *fd / (*fd - *fa);
    q33 = (d32 - q22) * *fa / (*fe - *fa);

/* CALCULATE THE OUTPUT C. */

    *c0 = q31 + q32 + q33;
    *c0 = *a + *c0;
    return 0;
} /* pzero_ */

inline int rmp_(double* rel)
{
    double a, b, beta;

/* CALCULATES THE RELATIVE MACHINE PRECISION (RMP). */
/*  REL -- DOUBLE PRECISION. OUTPUT OF RMP. */

    beta = 2.;
    a = 1.;
  L10:
    b = a + 1.;
    if (b > 1.) {
        a /= beta;
        goto L10;
    }
    *rel = a * beta;
    return 0;
} /* rmp_ */

template<typename FunctionalType>
root_type toms748_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,root_bias       bias
    ,int             decimals
    ,std::ostream&   os_trace
    ,int             sprauchling_limit = INT_MAX
    )
{
    // Arguments that are unused, for now at least:
    (void)&bias;
    (void)&sprauchling_limit;

    int    nprob  {1};    // has no actual meaning
//  int    neps   {1000}; // like setting Brent's 'tol' to zero
    int    neps   {decimals};
    int    n_eval {0};
    double eps    {DBL_EPSILON};
    double a      {bound0};
    double b      {bound1};
    double root   {0.0};
    rroot_(f, &nprob, &neps, &eps, &a, &b, &root, &n_eval);
    os_trace << " " << n_eval << "    evaluations" << std::endl;
    return {root, root_is_valid, n_eval - 2, n_eval};
}

template<typename FunctionalType>
root_type toms748_root
    (FunctionalType& f
    ,double          bound0
    ,double          bound1
    ,root_bias       bias
    ,int             decimals
    ,int             sprauchling_limit = INT_MAX
    )
{
    std::ostream null_ostream(&null_streambuf());
    null_ostream.setstate(std::ios::badbit);
    return toms748_root
        (f
        ,bound0
        ,bound1
        ,bias
        ,decimals
        ,null_ostream
        ,sprauchling_limit
        );
}

#endif // zero_hpp
