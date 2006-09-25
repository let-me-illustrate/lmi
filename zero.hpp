// Root finding by Brent's method.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: zero.hpp,v 1.4 2006-09-25 13:51:22 chicares Exp $

// R. P. Brent, _Algorithms for Minization without Derivatives_
// ISBN 0-13-022335-2

#ifndef lmi_zero_hpp
#define lmi_zero_hpp

#include "config.hpp"

#include "alert.hpp"
#include "round_to.hpp"

#include <cmath>
#include <limits>
#include <utility>

enum root_validity
    {root_is_valid
    ,root_not_bracketed
    };

enum root_bias
    {bias_none   // Return root z with f(z) closest to 0.0 .
    ,bias_lower  // Require f(z) <= 0.0 .
    ,bias_higher // Require  0.0 <= f(z).
    };

// Brent's algorithm with the following GWC modifications.
//
// The original algorithm returns a zero z of the function f in
//   [bound0 bound1]
// (a,b) to within a tolerance
//   6 * epsilon * |z| + 2 * t
// where t is an argument. For life insurance illustrations, the
// tolerance is often one-sided, so that f(z) must be strictly greater
// than or less than zero, and the tolerance is more conveniently
// expressed as a number of decimals. An adjustment is made for the
// constant factor in Brent's t term, so that this implementation's
// tolerance becomes
//   6 * epsilon * |z| + 10^-decimals
//
// The original algorithm returns unrounded values. For life insurance
// illustrations, rounded values are more often wanted, so iterands
// are rounded before each function evaluation. Rounding downstream,
// outside the root-finding algorithm, would not be appropriate: for a
// hypothetical unrounded return value r, f(r) and f(round(r)) might
// easily have different signs.
//
// Often the function f is expensive to evaluate and has important
// side effects. For instance, solving for level premium to endow
// produces yearly account values as a side effect. With the original
// algorithm, those values may correspond either to the upper or the
// lower bound, and thus not necessarily to the zero returned. To
// obtain the correct side effects, the caller would have to evaluate
// the function again--unconditionally, because it doesn't know which
// final bound was returned. Whenever guarantee_side_effects is set to
// a non-default value of true, this implementation guarantees correct
// side effects by reevaluating the function iff necessary.
//
// TODO ?? Expunge this feature?
// However, at least with our current implementation of iterative
// solves, 'guaranteed' side effects are invalidated by our use of the
// 'Solving' mode flag.
//
// Brent states a requirement that the ordinates corresponding to the
// a priori bounds (abscissa arguments) have different signs, but his
// algorithm does not test that requirement. This implementation does
// enforce it, and also handles the special case where both ordinates
// are zero.

typedef std::pair<double, root_validity> root_type;

template<typename FunctionalType>
root_type decimal_root
    (double          bound0
    ,double          bound1
    ,root_bias       bias
    ,int             decimals
    ,FunctionalType& f
    ,bool            guarantee_side_effects = false
    )
{
    static double const epsilon = std::numeric_limits<double>::epsilon();

    int number_of_iterations = 0;

    double t = 0.5 * std::pow(10.0, -decimals);

    round_to<double> const round_(decimals, r_to_nearest);

    double a = round_(bound0);
    double b = round_(bound1);

    double fa = f(a);
    if(0.0 == fa)
        {
        return std::make_pair(a, root_is_valid);
        }

    double fb = f(b);
    double last_evaluated_iterand = b; // Note 1.
    if(0.0 == fb)
        {
        return std::make_pair(b, root_is_valid);
        }

    // f(a) and f(b) must have different signs.
    if((0.0 < fa) == (0.0 < fb))
        {
        return std::make_pair(0.0, root_not_bracketed);
        }

    double fc = fb; // Note 0.
    double c = b;
    double d = b - a;
    double e = d;

    for(;;)
        {
        if((0.0 < fb) == (0.0 < fc))
            {
            c = a;
            fc = fa;
            d = e = b - a;
            }
        if(std::fabs(fc) < std::fabs(fb))
            {
             a =  b;  b =  c;  c =  a;
            fa = fb; fb = fc; fc = fa;
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
                if(guarantee_side_effects && last_evaluated_iterand != b)
                    {
                    f(b);
                    }
                return std::make_pair(b, root_is_valid);
                }
            else if(std::fabs(m) <= 2.0 * epsilon * std::fabs(c) + t)
                {
                if(guarantee_side_effects && last_evaluated_iterand != c)
                    {
                    f(c);
                    }
                return std::make_pair(c, root_is_valid);
                }
            }
        if(std::fabs(e) < tol || std::fabs(fa) <= std::fabs(fb))
            {
            // Bisection.
            d = e = m;
            }
        else
            {
            double p, q;
            double s = fb / fa; // Note 3.
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
            if
                (   p < 1.5 * m * q - std::fabs(tol * q)
                &&  p < std::fabs(0.5 * s * q)
                )
                {
                d = p / q;
                }
            else
                {
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
        b = round_(b);

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
            fb = f(b);
            last_evaluated_iterand = b;
            status()
                << "iteration " << number_of_iterations++
                << " iterand " << b
                << " value " << fb
                << std::flush
                ;
            }
        }
}

// Note 0. For abscissae a, b, c:
//   a and b are a priori bounds;
//   b is the best approximation so far to the true root r;
//   a is the previous value of b, or, initially, equal to c;
//   ordinates f(b) and f(c) are of different sign.
//
// Initializing fc to fb ensures that the first conditional clause in
// the main loop is executed on the first pass, so that the branches
// in the algol original can be rewritten in a structured way.

// Note 1. In order to guarantee side effects, the value of the last
// evaluated iterand is stored. It must equal either b or c, depending
// on whether b and c were swapped. It might be triflingly faster to
// maintain a 'swapped' flag, but that would make it harder to see at
// a glance whether the code is correct because the initialization
// logic would have to be considered as well as the evaluation at the
// end of the loop. Alternatively, one might preserve distinct copies
// of external state embodying side effects for each of b and c, but
// that seems wasteful of space and wouldn't work with singletons.

// Note 2. Here, Brent observes that one might return 0.5 * (b + c),
// equivalent to b + m, but that b is probably a much better
// approximation, so he returns b as soon as the condition
//   !(0.0 != fb && std::fabs(m) <= tol)
// is satisfied. But b might not have the desired bias. In that case,
// 0.5 * (b + c) is not necessarily correct either, because its bias
// is unknown; yet is it appropriate to return c instead?
//
// The bias of c must be correct becaue f(b) and f(c) are known to
// have different signs. And c is within Brent's tolerance in the weak
// sense of his variable tol, which is a worst-case guarantee that
// applies to c as well as b. To see why, suppose the algorithm is
// about to return with tol = 0.005 and
//   f0: z -> z * z * signum(z)
//   b = -0.001   f(b) = -0.000001
//   c =  0.009   f(c) =  0.000081
// The true root is of course zero, and b is closer. But the same
// functional values would be obtained with
//   f1: z -> -0.000001, z < 0.01
//             0.0,      z = 0.0089
//             0.000081, 0.0089 < z <= 1.0
//             1.0,      1.0 < z
// in which case Brent would return b and guarantee that the tolerance
// is satisfied, even though c is much closer than b to the true root.
//
// However, Brent calculates tol in terms of b, guaranteeing a maximum
// error of
//   6 * epsilon * |b| + 2 * t
// when returning b. Unconditionally returning c would give an error
// bound in terms of the local variable b, whose value is unknown to
// the caller, and |b| might exceed |c|. It is irrelevant that the
// return value is multiplied by epsilon, which might often be so
// small as to make the first part of the error term vanish, because
// the return value might far exceed the reciprocal of epsilon. To
// preserve the algorithm's rigorous guarantees, c is returned, when
// bias so dictates, only if Brent's termination criterion is still
// met when reevaluated in terms of c instead of b.
//
// It might appear that the code could be simplified, say, by defining
// tol in terms of max(|b|,|c|), but that would be dangerous: tol is
// used elsewhere to decide whether to revert to bisection, so any
// change in its definition would vitiate the convergence guarantee
// that is Brent's cardinal improvement to Dekker's algorithm, and
// might introduce other errors as well.
//
// At any rate, care is taken to return the same result as Brent's
// original implementation in the bias_none case; to do otherwise
// would violate the principle of least astonishment.

// Note 3. Brent points out that this division is safe because
//   0 < |f(b)| <= |f(a)|
// whenever this line is executed.

// Note 4. Each iterand is rounded, so it might equal an iterand that
// has already been evaluated. In that case, the known value is used,
// because evaluation is assumed to be costly, and in practice one
// bound stays fixed to within rounding (for instance, at the edge of
// a discontinuity) often enough that it is worthwhile to avoid
// superfluous reevaluation.

// A C++ equivalent of Brent's algol60 original, for reference only.
template<typename FunctionalType>
double brent_zero
    (double          a
    ,double          b
    ,double          t
    ,FunctionalType& f
    )
{
    double const epsilon = std::numeric_limits<double>::epsilon();

    // Returns a zero of the function f in [a,b] to within a tolerance
    //   6 * epsilon * |z| + 2 * t
    // f(a) and f(b) must have different signs.
    double fa = f(a);
    double fb = f(b);
    double fc = fb;

    for(;;)
        {
        double c, d, e;
        if((0.0 < fb) == (0.0 < fc))
            {
            c = a;
            fc = fa;
            d = e = b - a;
            }
        if(std::fabs(fc) < std::fabs(fb))
            {
             a =  b;  b =  c;  c =  a;
            fa = fb; fb = fc; fc = fa;
            }
        double tol = 2.0 * epsilon * std::fabs(b) + t;
        double m = 0.5 * (c - b);
        if(0.0 == fb || std::fabs(m) <= tol)
            {
            return b;
            }
        if(std::fabs(e) < tol || std::fabs(fa) <= std::fabs(fb))
            {
            // Bisection.
            d = e = m;
            }
        else
            {
            double p, q;
            double s = fb / fa;
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
            if
                (   p < 1.5 * m * q - std::fabs(tol * q)
                &&  p < std::fabs(0.5 * s * q)
                )
                {
                d = p / q;
                }
            else
                {
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
        fb = f(b);
        }
}

#endif // lmi_zero_hpp

