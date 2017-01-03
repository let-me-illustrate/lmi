// Miscellaneous mathematical operations as function objects.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef math_functors_hpp
#define math_functors_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "et_vector.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_float.hpp>

#include <algorithm>                    // std::max(), std::min()
#include <cmath>                        // C99 expm1(), log1p()
#include <functional>
#include <stdexcept>
#include <vector>

// For Comeau, implement expm1l() and log1pl() using type double, not
// long double, because of an apparent incompatibility in the way
// Comeau and MinGW pass long doubles.

#if !defined LMI_COMPILER_PROVIDES_EXPM1L
#   if defined LMI_COMO_WITH_MINGW
extern "C" double expm1(double);
inline double expm1l(double x) {return expm1(x);}
#   else  // !defined LMI_COMO_WITH_MINGW
extern "C" long double expm1l(long double);
#   endif // !defined LMI_COMO_WITH_MINGW
#endif // !defined LMI_COMPILER_PROVIDES_EXPM1L

#if !defined LMI_COMPILER_PROVIDES_LOG1PL
#   if defined LMI_COMO_WITH_MINGW
extern "C" double log1p(double);
inline double log1pl(double x) {return log1p(x);}
#   else  // !defined LMI_COMO_WITH_MINGW
extern "C" long double log1pl(long double);
#   endif // !defined LMI_COMO_WITH_MINGW
#endif // !defined LMI_COMPILER_PROVIDES_LOG1PL

// For Comeau, implement expm1l() and log1pl() using type double, not
// long double, because of an apparent incompatibility in the way
// Comeau and MinGW pass long doubles.

#if !defined LMI_COMPILER_PROVIDES_EXPM1L
#   if defined LMI_COMO_WITH_MINGW
extern "C" double expm1(double);
inline double expm1l(double x) {return expm1(x);}
#   else  // !defined LMI_COMO_WITH_MINGW
extern "C" long double expm1l(long double);
#   endif // !defined LMI_COMO_WITH_MINGW
#endif // !defined LMI_COMPILER_PROVIDES_EXPM1L

#if !defined LMI_COMPILER_PROVIDES_LOG1PL
#   if defined LMI_COMO_WITH_MINGW
extern "C" double log1p(double);
inline double log1pl(double x) {return log1p(x);}
#   else  // !defined LMI_COMO_WITH_MINGW
extern "C" long double log1pl(long double);
#   endif // !defined LMI_COMO_WITH_MINGW
#endif // !defined LMI_COMPILER_PROVIDES_LOG1PL

// TODO ?? Write functors here for other refactorable uses of
// std::pow() found throughout the program.

// These functors are Adaptable Unary or Binary Functions wherever
// possible.

template<typename T>
struct greater_of
    :public std::binary_function<T,T,T>
{
    T operator()(T const& x, T const& y) const
        {
        return std::max(x, y);
        }
};

template<typename T>
struct lesser_of
    :public std::binary_function<T,T,T>
{
    T operator()(T const& x, T const& y) const
        {
        return std::min(x, y);
        }
};

/// Arithmetic mean.
///
/// Calculate mean as
///   (half of x) plus (half of y)
/// instead of
///   half of (x plus y)
/// because the addition in the latter can overflow. Generally,
/// hardware deals better with underflow than with overflow.
///
/// The domain is restricted to floating point because integers would
/// give surprising results. For instance, the integer mean of one and
/// two would be truncated to one upon either returning an integer or
/// assigning the result to one. Returning a long double in all cases
/// is the best that could be done, but that seems unnatural.

template<typename T>
struct mean
    :public std::binary_function<T, T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& x, T const& y) const
        {return 0.5 * x + 0.5 * y;}
};

// Actuarial functions.
//
// Some inputs are nonsense, like interest rates less than 100%.
// Contemporary compilers usually handle such situations without
// raising a hardware exception. Trapping invalid input would add a
// runtime overhead of about twenty percent (measured with gcc-3.4.2);
// this is judged not to be worthwhile.
//
// Typically, the period 'n' is a constant known at compile time, so
// it is makes sense for it to be a non-type template parameter. That,
// however, makes derivation from std::binary_function nonsensical:
// what is important is not the type of 'n', but its value. But 'n'
// equals twelve in the most common case, for which functors derived
// from std::unary_function are provided.
//
// General preconditions: 0 < 'n'; -1.0 <= 'i'; T is floating point.
//
// Implementation note: greater accuracy and speed are obtained by
// applying the transformation
//   (1+i)^n - 1 <-> expm1l(log1pl(i) * n)
// to naive power-based formulas.

template<typename T, int n>
struct i_upper_n_over_n_from_i
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    BOOST_STATIC_ASSERT(0 < n);
    T operator()(T const& i) const
        {
        if(i < -1.0)
            {
            throw std::domain_error("i is less than -100%.");
            }

        if(-1.0 == i)
            {
            return -1.0;
            }

        static long double const reciprocal_n = 1.0L / n;
        // naively:    (1+i)^(1/n) - 1
        // substitute: (1+i)^n - 1 <-> expm1l(log1pl(i) * n)
        long double z = expm1l(log1pl(i) * reciprocal_n);
        return static_cast<T>(z);
        }
};

template<typename T>
struct i_upper_12_over_12_from_i
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {
        return i_upper_n_over_n_from_i<double,12>()(i);
        }
};

template<typename T, int n>
struct i_from_i_upper_n_over_n
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    BOOST_STATIC_ASSERT(0 < n);
    T operator()(T const& i) const
        {
        // naively:    (1+i)^n - 1
        // substitute: (1+i)^n - 1 <-> expm1l(log1pl(i) * n)
        long double z = expm1l(log1pl(i) * n);
        return static_cast<T>(z);
        }
};

template<typename T>
struct i_from_i_upper_12_over_12
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {
        return i_from_i_upper_n_over_n<double,12>()(i);
        }
};

template<typename T, int n>
struct d_upper_n_from_i
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    BOOST_STATIC_ASSERT(0 < n);
    T operator()(T const& i) const
        {
        if(i < -1.0)
            {
            throw std::domain_error("i is less than -100%.");
            }

        if(-1.0 == i)
            {
            throw std::range_error("i equals -100%.");
            }

        static long double const reciprocal_n = 1.0L / n;
        // naively:    n * (1 - (1+i)^(-1/n))
        // substitute: (1+i)^n - 1 <-> expm1l(log1pl(i) * n)
        long double z = -n * expm1l(log1pl(i) * -reciprocal_n);
        return static_cast<T>(z);
        }
};

template<typename T>
struct d_upper_12_from_i
    :public std::unary_function<T, T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& i) const
        {
        return d_upper_n_from_i<double,12>()(i);
        }
};

/// Annual net from annual gross rate, with two different kinds of
/// decrements. See the interest-rate class for the motivation.
///
/// Additional precondition: arguments are not such as to cause the
/// result to be less than -1.0 .

template<typename T, int n>
struct net_i_from_gross
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    BOOST_STATIC_ASSERT(0 < n);
    T operator()(T const& i, T const& spread, T const& fee) const
        {
        static long double const reciprocal_n = 1.0L / n;
        // naively:
        //   (1
        //   +   (1+     i)^(1/n)
        //   -   (1+spread)^(1/n)
        //   -         fee *(1/n)
        //   )^n - 1
        // substitute: (1+i)^n - 1 <-> expm1l(log1pl(i) * n)
        long double z = expm1l
            (
            n * log1pl
                (   expm1l(reciprocal_n * log1pl(i))
                -   expm1l(reciprocal_n * log1pl(spread))
                -          reciprocal_n * fee
                )
            );
        return static_cast<T>(z);
        }
};

/// Convert q to a monthly COI rate.
///
/// The COI rate is the monthly equivalent of q divided by one minus
/// itself, because deducting the COI charge at the beginning of the
/// month increases the amount actually at risk--see:
///   http://lists.nongnu.org/archive/html/lmi/2009-09/msg00001.html
///
/// The value of 'q' might exceed unity, for example if guaranteed COI
/// rates for simplified issue are 120% of 1980 CSO, so that case is
/// accommodated. A value of zero might arise from a partial-mortality
/// multiplier that equals zero for some or all durations, and that
/// case arises often enough to merit a special optimization.
///
/// Preconditions:
///   'max_coi' is in [0.0, 1.0]
///   'q' is nonnegative
/// An exception is thrown if any precondition is violated.
///
/// If 'q' exceeds unity, then 'max_coi' is returned. Notionally, 'q'
/// is a probability and cannot exceed unity, but it doesn't seem
/// implausible to most actuaries to set q to 125% of 1980 CSO and
/// expect it to limit itself.

template<typename T>
struct coi_rate_from_q
    :public std::binary_function<T,T,T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& q, T const& max_coi) const
        {
        if(!(0.0 <= max_coi && max_coi <= 1.0))
            {
            throw std::runtime_error("Maximum COI rate out of range.");
            }

        if(q < 0.0)
            {
            throw std::domain_error("q is negative.");
            }

        if(0.0 == q)
            {
            return 0.0;
            }
        else if(1.0 <= q)
            {
            return max_coi;
            }
        else
            {
            static long double const reciprocal_12 = 1.0L / 12;
            // naively:    1 - (1-q)^(1/12)
            // substitute: (1+i)^n - 1 <-> expm1l(log1pl(i) * n)
            long double monthly_q = -expm1l(log1pl(-q) * reciprocal_12);
            if(1.0L == monthly_q)
                {
                throw std::logic_error("Monthly q equals unity.");
                }
            monthly_q = monthly_q / (1.0L - monthly_q);
            return std::min(max_coi, static_cast<T>(monthly_q));
            }
        }
};

/// Midpoint for illustration reg.
///
/// Section 7(C)(1)(c)(ii) prescribes an "average" without specifying
/// which average to use. The arithmetic mean is used here because
/// that seems to be the most common practice. On the other hand, a
/// strong case can be made for using the geometric mean, at least
/// with interest and mortality rates.

template<typename T>
void assign_midpoint
    (std::vector<T>      & out
    ,std::vector<T> const& in_0
    ,std::vector<T> const& in_1
    )
{
    LMI_ASSERT(in_0.size() == in_1.size());
    out.resize(in_0.size());
    assign(out, apply_binary(mean<T>(), in_0, in_1));
}

#endif // math_functors_hpp

