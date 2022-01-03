// Expression templates for arithmetic.
//
// Copyright (C) 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// This file is of historical interest only. It shows various attempts
// to reinvent work that others have done better.

#include "pchfile.hpp"

#include "assert_lmi.hpp"
#include "materially_equal.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <valarray>

// Expression templates
//
// Read these papers by Veldhuizen and Furnish respectively:
//   http://osl.iu.edu/~tveldhui/papers/Expression-Templates/exprtmpl.html
//   http://www.adtmag.com/joop/crarticle.asp?ID=627
// to understand the motivation. The following code is an original
// implementation of ideas in those two papers.
//
// Consider adding two vectors:
//
//   std::transform
//      (v0.begin(), v0.end(), v1.begin(), v2.end(), std::plus<double>());
//
// We want to write this in the simplest way imaginable:
//   v2 = v0 + v1;
// and we want more complicated expressions to be evaluated in one
// pass through the vectors, without redundant loads and stores.
// That can be achieved by deferred evaluation.
//
// Consider std::plus :
//
//   template<typename T>
//   struct plus
//   {
//       T operator()(T const& t0, T const& t1) const {return t0 + t1;}
//   }
//
// Why not just use that? Well, operator()() must be a nonstatic
// member function [13.5.4], so it can't be invoked without an object.
// When we use std::plus as an argument to std::transform(), the
// objects are immediately available. But for expression templates we
// want to defer evaluation instead of operating on the objects
// immediately. And we don't want to carry the objects around either:
// as Furnish points out, that's the problem we're trying to avoid.
//
// Instead, we write this similar code that uses a static member
// function and therefore can be invoked without an object:

struct plus
{
    static double apply(double d0, double d1) {return d0 + d1;}
};

// Following Veldhuizen's presentation here and throughout this file,
// we write specifically for type double, knowing that we can later
// abstract that type. Furnish and Veldhuizen both name this 'apply';
// it can't be named 'operator()' because it's static.
//
// Now consider the binary form of std::transform():
//
// template<typename I0, typename I1, typename R, typename F>
// R transform(I0 alpha0, I0 omega0, I1 alpha1, R result, F f)
// {
//     for(; alpha0 != omega0; ++alpha0, ++alpha1, ++result)
//         *result = f(*alpha0, *alpha1);
//     return result;
// }
//
// which iterates across input iterators, at each step applying a
// binary-operation object to the iterators and assigning the result
// to an output iterator. As Veldhuizen explains under "Optimizing
// Vector Expressions", we want to postpone that activity. Here is
// a class that knows how to do the same thing, but doesn't actually
// do it. It knows what input iterators we want to use because it
// retains local copies of them. It knows how to traverse the
// iterators: that's what its operator++() is for. And the knowledge
// of how to invoke the binary operation on each iterand-pair is
// embodied in operator*().
//
// The concept-names SDF (Scalar Dyadic Function) is borrowed from
// APL. "Dyadic" and "Monadic" are the common APL terms for what the
// C++ literature usually calls binary and unary functions. The APL
// terms emphasize that the functions are scalar (because their
// arguments and results are), although they'll often be applied to
// vectors.

template<typename I0, typename I1, typename SDF>
class binary_expression
{
  public:
    binary_expression(I0 const& i, I1 const& j) :i_(i), j_(j) {}

    double operator*() const {return SDF::apply(*i_, *j_);}
    void operator++()        {++i_; ++j_;}

  private:
    I0 i_;
    I1 j_;
};

// It's instructive to examine how these low-level pieces work:

void demo0()
{
    double u[2] = {1.2, 3.4};
    double v[2] = {5.6, 7.8};

    // The canonical STL way to add two arrays: std::transform().
    // We can use it to add just one pair if we like.
    double r;
    std::transform(u, 1 + u, v, &r, std::plus<double>());
    LMI_TEST(materially_equal(6.8, r));

    // Here's an equivalent using our new code.
    r = *binary_expression<double const*,double const*,plus>(u, v);
    LMI_TEST(materially_equal(6.8, r));

    // This type embodies everything we need to know to add pairs of
    // values during iteration.
    typedef binary_expression<double const*,double const*,plus> Add;
    LMI_TEST(materially_equal(6.8, *Add(u, v)));

    // Since 'Add' is a class type, we can construct an instance and
    // exercise its operator*() and operator++() separately.
    Add a(u, v);
    LMI_TEST(materially_equal(6.8, *a));
    ++a;
    LMI_TEST(materially_equal(11.2, *a));
}

// As this example shows, we have something equivalent to std::plus
// combined with std::transform(), but with iterative evaluation
// removed so that it can easily be done later.

// Let's create a simple array class and add its elements together.
// This class holds an array of exactly two doubles and is obviously
// unsuitable for use beyond this demonstration. The real point is its
// operator=(), which automates the iterator traversal demonstrated
// manually above.

class simple_array0
{
  public:
    simple_array0(int n, double d = 0.0)
        :length_ {n}
        {
        data_ = ::new double[length_];
        for(int j = 0; j < length_; ++j) data_[j] = d;
        }

    ~simple_array0() {delete[] data_;}

    double& operator[](int i)           {return data_[i];}

    typedef binary_expression<double const*,double const*,plus> add_t;
    simple_array0& operator=(add_t e)
        {
        for(double* i = begin(); i < end(); ++i, ++e)
            {*i = *e;}
        return *this;
        }

    double const* begin() const {return           data_;}
    double const* end  () const {return length_ + data_;}

    double*       begin()       {return           data_;}
    double*       end  ()       {return length_ + data_;}

    void resize_for_testing(int new_size)
        {
        delete[] data_;
        length_ = new_size;
        data_ = ::new double[length_];
        for(int j = 0; j < length_; ++j) data_[j] = j;
        }

  private:
    double* data_;
    int length_;
};

// This syntactic sugar that lets us add two instances of our simple
// array class together by writing a plus sign between their names.

//typedef binary_expression<double*,double*,plus> sdf_addition;
typedef binary_expression<double const*,double const*,plus> sdf_addition;

// Geoffrey Furnish notes that the result is created on the stack and
// must therefore be returned by value. That's unavoidable, but it
// doesn't cost much: it's just a pair of iterators.

sdf_addition operator+(simple_array0 const& v0, simple_array0 const& v1)
{
    return sdf_addition(v0.begin(), v1.begin());
}

void demo1()
{
    int const length = 10;
    simple_array0 u(length);
    simple_array0 v(length);
    simple_array0 w(length);

    for(int j = 0; j < length; ++j)
        {
        u[j] = 1.2 * j;
        v[j] = 3.4 * j;
        }

    w = u + v;

    LMI_TEST(materially_equal(w[0],  0.0));
    LMI_TEST(materially_equal(w[1],  4.6));
    LMI_TEST(materially_equal(w[2],  9.2));
    LMI_TEST(materially_equal(w[3], 13.8));
    LMI_TEST(materially_equal(w[4], 18.4));
    LMI_TEST(materially_equal(w[5], 23.0));
    LMI_TEST(materially_equal(w[6], 27.6));
    LMI_TEST(materially_equal(w[7], 32.2));
    LMI_TEST(materially_equal(w[8], 36.8));
    LMI_TEST(materially_equal(w[9], 41.4));
}

// Obviously class simple_array0 is very limited: for instance, this:
//   simple_array0 s = u + v;
// isn't allowed. (Then again, with std::valarray, it has undefined
// behavior.) Such problems can be solved; one solution for this
// problem would be to add this member function:
//
// operator simple_array0()
// {
//     simple_array0 r(0.0, 0.0);
//     for(double* i = r.begin(); i < r.end(); ++i, operator++())
//         {*i = operator*();}
//     return r;
// }
//
// to class binary_expression.

namespace
{
    // Global variables for timing tests. They could alternatively be
    // passed as arguments, e.g., by using std::bind, but that would
    // increase complexity in return for no real benefit.

    int g_array_length = 1;

    // Number of iterations for 'mete*' functions. This value is a
    // compromise: higher values make this unit test take too long,
    // while lower values may yield measurements that are less than
    // a one-microsecond timer tick.

    int const n_iter = 1000;

    simple_array0 g_u(g_array_length);
    simple_array0 g_v(g_array_length);
    simple_array0 g_w(g_array_length);

    std::valarray<double> g_va_u(g_array_length);
    std::valarray<double> g_va_v(g_array_length);
    std::valarray<double> g_va_w(g_array_length);
} // Unnamed namespace.

void mete_c()
{
    for(int i = 0; i < n_iter; ++i)
        {
        for(int j = 0; j < g_array_length; ++j)
            {
            g_w[j] = g_u[j] + g_v[j];
            }
        }
}

void mete_et()
{
    for(int i = 0; i < n_iter; ++i)
        {
        g_w = g_u + g_v;
        }
}

void mete_va()
{
    for(int i = 0; i < n_iter; ++i)
        {
        g_va_w = g_va_u + g_va_v;
        }
}

void time_one_array_length(int length)
{
    g_array_length = length;

    g_u.resize_for_testing(g_array_length);
    g_v.resize_for_testing(g_array_length);
    g_w.resize_for_testing(g_array_length);

    g_va_u.resize(g_array_length);
    g_va_v.resize(g_array_length);
    g_va_w.resize(g_array_length);
    for(int j = 0; j < g_array_length; ++j)
        {
        g_va_u[j] = j;
        g_va_v[j] = j;
        g_va_w[j] = j;
        }

    int const n = -1 + g_array_length;
    int const max_seconds = 1;
    double const c  = TimeAnAliquot(mete_c , max_seconds).unit_time();
    LMI_TEST_EQUAL(g_w   [n], 2.0 * n);
    double const et = TimeAnAliquot(mete_et, max_seconds).unit_time();
    LMI_TEST_EQUAL(g_w   [n], 2.0 * n);
    double const va = TimeAnAliquot(mete_va, max_seconds).unit_time();
    LMI_TEST_EQUAL(g_va_w[n], 2.0 * n);
    std::cout
        << std::setw( 7) << g_array_length
        << std::setw(15) << std::setprecision(3) << std::scientific << c
        << std::setw(15) << std::setprecision(3) << std::scientific << et
        << std::setw( 7) << std::setprecision(3) << std::fixed << et / c
        << std::setw(15) << std::setprecision(3) << std::scientific << va
        << std::setw( 7) << std::setprecision(3) << std::fixed << va / c
        << std::endl
        ;
}

int test_main(int, char*[])
{
    demo0();
    demo1();

    std::cout
        << "        Time (seconds) for array0 = array1 + array2 by various methods"
        << '\n'
        << " length          C             et       et/C         va       va/c"
        << '\n'
        ;
    time_one_array_length(1);
    time_one_array_length(10);
    time_one_array_length(20);
    time_one_array_length(50);
    time_one_array_length(100);
    time_one_array_length(1000);
    time_one_array_length(10000);
    time_one_array_length(100000);

    return 0;
}
