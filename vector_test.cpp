// Expression templates for arithmetic.
//
// Copyright (C) 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: vector_test.cpp,v 1.2 2005-09-04 17:05:28 chicares Exp $

// This file is of historical interest only. It shows various attempts
// to reinvent work that others have done better.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "materially_equal.hpp"
#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>

// Expression templates
//
// First read
//   http://osl.iu.edu/~tveldhui/papers/Expression-Templates/exprtmpl.html
//   http://www.adtmag.com/joop/crarticle.asp?ID=627
// to understand the motivation. The following code is adapted from
// ideas in those two papers. Also see
// http://web.media.mit.edu/~rahimi/lazy-containers/
// ipdps.eece.unm.edu/1998/papers/113.pdf

// First consider std::plus :
//
//   template <class T>
//   struct plus : public binary_function<T,T,T>
//   {
//       T operator()(const T& x, const T& y) const {return x + y;}
//   };
//
// whose base class binary_function serves only to inject typenames
// for the arguments and the result.
//
// Why don't we just use that? Well, operator()() must be a nonstatic
// member function [13.5.4], so it can't be invoked without an object.
// When we use std::plus as an argument to std::transform(), the
// objects are immediately available. But for expression templates we
// want to defer evaluation instead of operating on the objects
// immediately. And we don't want to carry the objects around either:
// as Furnish points out, that's the problem we're trying to avoid.
//
// Instead, we write this similar code that uses a static member
// function and therefore can be invoked without an object. Following
// Veldhuizen's presentation here and throughout this file, we write
// specifically for type double, knowing that we can abstract that
// type later.

struct adder
{
    static double apply(double a, double b) {return a + b;}
};

// Now consider the binary form of std::transform():
//
//   template<typename I, typename J, typename R, typename BinOp>
//   R transform(I first1, I last1, J first2, R result, BinOp bin_op)
//   {
//       for(;first1 != last1; ++first1, ++first2, ++result)
//           {
//           *result = bin_op(*first1, *first2);
//           }
//       return result;
//   }
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

template<typename I, typename J, typename BinaryOperation>
class binary_expression
{
  public:
    binary_expression(I const& a_i, J const& a_j) :i(a_i), j(a_j) {}

    double operator*() const       {return BinaryOperation::apply(*i, *j);}
    void operator++()              {++i; ++j;}

  private:
    I i;
    J j;
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
    std::cout << r << '\n';
    BOOST_TEST(materially_equal(r, 6.8));

    // Here's an equivalent using our new code.
    std::cout << *binary_expression<double*,double*,adder>(u, v) << '\n';

    // This type embodies everything we need to know to add pairs of
    // values during iteration.
    typedef binary_expression<double*,double*,adder> Add;
    std::cout << *Add(u, v) << '\n';
    BOOST_TEST(materially_equal(*Add(u, v), 6.8));

    // Since 'Add' is a class type, we can construct an instance and
    // exercise its operator*() and operator++().
    Add a(u, v);
    std::cout << "Sum of first  pair: " << *a << '\n';
    BOOST_TEST(materially_equal(*a, 6.8));
    ++a;
    std::cout << "Sum of second pair: " << *a << '\n';
    BOOST_TEST(materially_equal(*a, 11.2));
}

// As this example shows, we have something equivalent to std::plus
// combined with std::transform(), but with iterative evaluation
// removed in such a way that it can easily be done later.

// Now let's create a simple container and add its elements together.
// This container holds an array of two doubles and is obviously
// unsuitable for use beyond this demonstration. The real point is its
// operator=(), which automates the iterator traversal
//    *iterator
//    ++iterator;
//    repeat until done
// demonstrated manually above.

class ArrayOfDouble
{
  public:
    ArrayOfDouble(double a, double b) {data[0] = a; data[1] = b;}

    ArrayOfDouble& operator=(binary_expression<double*,double*,adder> e)
        {for(double* i = begin(); i < end(); ++i, ++e) {*i = *e;} return *this;}

    double* begin() const {return     const_cast<double*>(data);}
    double* end  () const {return n + const_cast<double*>(data);}

  private:
    enum {n = 2};
    double data[n];
};

// Here is some 'syntactic sugar' that lets us add two instances of
// our simple array class together by writing a plus sign between
// their names.

typedef binary_expression<double*,double*,adder> demo1_t;

demo1_t operator+(ArrayOfDouble const& v0, ArrayOfDouble const& v1)
{
    // Geoffrey Furnish notes that this is created on the stack and
    // must therefore be returned by value. TODO ?? Why does that matter?
    return demo1_t(v0.begin(), v1.begin());
}

void demo1()
{
    ArrayOfDouble u(1.2, 3.4);
    ArrayOfDouble v(5.6, 7.8);
    ArrayOfDouble r(0.0, 0.0);
    r = u + v;
    std::cout << "Vector sum: ";
    std::copy(r.begin(), r.end(), std::ostream_iterator<double>(std::cout, " "));
    std::cout << '\n';
    BOOST_TEST(materially_equal(*     r.begin() ,  6.8));
    BOOST_TEST(materially_equal(*(1 + r.begin()), 11.2));
    r = u;
    ArrayOfDouble s(r);
    (void) s; // Avoid gcc 'unused variable' warning.
}

////

/*
binop...add this...
operator ArrayOfDouble() ////
{
  ArrayOfDouble r(0.0, 0.0);
  for(double* i = r.begin(); i < r.end(); ++i, operator++())
    {*i = operator*();}
  return r;
}
*/

/*
template<typename BinaryOperation>
ArrayOfDouble operator=(binary_expression<double*,double*,adder> e)
        {for(double* i = begin(); i < end(); ++i, ++e) {*i = *e;} return *this;}


operator=(), operator+=() etc. must be members

*/


class vec;

class assignable
{
  public:
    virtual void assign_to(vec&) const = 0;
};

class vec
{
    typedef double* iterator_type;

  public:
    vec(int n)
        : length(n)
        {data = new double[n];}

    vec(double d, int n)
        : length(n)
        {
        data = new double[n];
        for(int j = 0; j < n; ++j) data[j] = d;
        }

    ~vec()                              {delete[] data;}

    iterator_type begin() const         {return data;}
    iterator_type end  () const         {return data + length;}
    double& operator[](int i)           {return data[i];}

    vec& operator=(assignable const& x) {x.assign_to(*this); return *this;}

  private:
    double* data;
    int length;
};

template<typename IteratorType>
void perform_assignment(vec& v, IteratorType const& a_root)
{
    IteratorType root = a_root;
    for(double* result = v.begin(); result != v.end(); ++result, ++root)
        {
        *result = *root;
        }
}

template<typename T>
class general_expression
    :public assignable
{
public:
    general_expression(T const& a_t) :t(a_t) {}

    double operator*() const             {return *t; }
    void operator++()                    {++t;}

    virtual void assign_to(vec& x) const {perform_assignment(x, *this);}

private:
    T t;
};

general_expression<binary_expression<double*,double*,adder> >
operator+(vec const& v0, vec const& v1)
{
    return binary_expression<double*,double*,adder>(v0.begin(), v1.begin());
}

void demo2()
{
    int const length = 10;
    vec u(length);
    vec v(length);
    vec w(length);

    for(int j = 0; j < length; ++j)
        {
        u[j] = 1.2 * j;
        v[j] = 3.4 * j;
        }

    w = u + v;

    std::cout << w[1] << '\n';

    BOOST_TEST(materially_equal(w[0],  0.0));
    BOOST_TEST(materially_equal(w[1],  4.6));
    BOOST_TEST(materially_equal(w[2],  9.2));
    BOOST_TEST(materially_equal(w[3], 13.8));
    BOOST_TEST(materially_equal(w[4], 18.4));
    BOOST_TEST(materially_equal(w[5], 23.0));
    BOOST_TEST(materially_equal(w[6], 27.6));
    BOOST_TEST(materially_equal(w[7], 32.2));
    BOOST_TEST(materially_equal(w[8], 36.8));
    BOOST_TEST(materially_equal(w[9], 41.4));
}

int test_main(int, char*[])
{
    demo0();
    demo1();
    demo2();
    return 0;
}

