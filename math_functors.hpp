// Miscellaneous mathematical operations as function objects.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: math_functors.hpp,v 1.7 2005-10-31 18:09:52 chicares Exp $

#ifndef math_functors_hpp
#define math_functors_hpp

#include "config.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <algorithm>
#include <cmath>
#include <functional>

#if !defined LMI_COMPILER_PROVIDES_EXPM1
extern "C" double expm1(double);
#endif // !defined LMI_COMPILER_PROVIDES_EXPM1

#if !defined LMI_COMPILER_PROVIDES_LOG1P
extern "C" double log1p(double);
#endif // !defined LMI_COMPILER_PROVIDES_LOG1P

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

// Calculate mean as
//   (half of x) plus (half of y)
// instead of
//   half of (x plus y)
// because the addition in the latter can overflow. Generally,
// hardware deals better with underflow than with overflow.
//
// The domain is restricted to floating point because integers would
// give surprising results. For instance, the integer mean of one and
// two would be truncated to one upon either returning an integer or
// assigning the result to one. Returning a long double in all cases
// is the best that could be done, but that seems unnatural.
//
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
// what is of interest is not the type of 'n', but its value. But 'n'
// equals twelve in the most common case, for which functors derived
// from std::unary_function are provided.
//
// General preconditions: 0 < 'n'; -1.0 <= 'i'; T is floating point.
//
// Implementation note: greater accuracy and speed are obtained by
// applying the transformation
//   (1+i)^n - 1 <-> expm1(log1p(i) * n)
// to naive power-based formulas.

template<typename T, int n>
struct i_upper_n_over_n_from_i
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    BOOST_STATIC_ASSERT(0 < n);
    T operator()(T const& i) const
        {
        static long double const reciprocal_n = 1.0L / n;
        return expm1(log1p(i) * reciprocal_n);
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
        return expm1(log1p(i) * n);
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
        static long double const reciprocal_n = 1.0L / n;
        return -n * expm1(log1p(i) * -reciprocal_n);
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

// Annual net from annual gross rate, with two different kinds of
// decrements. See the interest-rate class for the motivation.
//
// Additional precondition: arguments are not such as to cause the
// result to be less than -1.0 .
//
template<typename T, int n>
struct net_i_from_gross
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    BOOST_STATIC_ASSERT(0 < n);
    T operator()(T const& i, T const& spread, T const& fee) const
        {
        static long double const reciprocal_n = 1.0L / n;
        return expm1
            (
            n * log1p
                (   expm1(reciprocal_n * log1p(i))
                -   expm1(reciprocal_n * log1p(spread))
                -         reciprocal_n * fee
                )
            );
        }
};

// Convert q to a monthly COI rate. The COI charge is assessed against
// all insureds who are alive at the beginning of the month. Assuming
// that deaths occur at the end of the month, the monthly-equivalent
// q should be divided by one minus itself to obtain the COI rate.
//
// The value of 'q' might exceed unity, for example if guaranteed COI
// rates for simplified issue are 120% of 1980 CSO, so that case is
// accommodated. A value of zero might arise from a partial-mortality
// multiplier that equals zero for some or all durations, and that
// case arises often enough to merit a special optimization. Negative
// values of the arguments are not plausible and are not tested.
//
template<typename T>
struct coi_rate_from_q
    :public std::binary_function<T,T,T>
{
    BOOST_STATIC_ASSERT(boost::is_float<T>::value);
    T operator()(T const& q, T const& max_coi) const
        {
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
            long double monthly_q = -expm1(log1p(-q) * reciprocal_12);
            return std::min
                (max_coi
                ,static_cast<T>(monthly_q / (1.0L - monthly_q))
                );
            }
        }
};

#endif  // math_functors_hpp

