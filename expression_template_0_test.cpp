// Expression templates, investigation 0--unit test.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: expression_template_0_test.cpp,v 1.13 2007-01-03 01:03:27 chicares Exp $

#define USE_UBLAS
// USE_PETE is not unconditionally defined here because PETE is not
// part of our build environment at this time.
#if defined __BORLANDC__
#   undef USE_UBLAS
#   undef USE_PETE
#endif // defined __BORLANDC__

#if defined USE_UBLAS
// BOOST !! Startlingly enough, boost uBLAS depends on this standard
// macro. If it's not defined, then expression templates aren't used,
// which impairs performance significantly and removes an essential
// reason for using this library. However, if it is defined, then it
// must be defined consistently everywhere to avoid ODR problems; but
// that suppresses assertions in other libraries, which it might be
// desirable to leave in production code. It would have been easy to
// accommodate programmers who never want to turn off assertions by
// using a library-specific macro instead.
#   define NDEBUG 1
#endif // defined USE_UBLAS

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "test_tools.hpp"
#include "timer.hpp"

#if !defined __BORLANDC__
#   include <boost/bind.hpp>
#endif // !defined __BORLANDC__

#if defined USE_UBLAS
#   include <boost/numeric/ublas/vector.hpp>
#endif // defined USE_UBLAS

#if defined USE_PETE
// For now at least, install PETE where indicated, and compile with
//   CPPFLAGS='-I /pete/pete-2.1.0/src -DPETE_MAKE_EMPTY_CONSTRUCTORS=0 -DUSE_PETE'
// If PETE proves useful, then we can use the 'freepooma' version,
// either as a library, or by importing the PETE sources.
#   include </pete/pete-2.1.0/examples/Vector/Eval.h>
#endif // defined USE_PETE

#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <valarray>
#include <vector>

// A facility for concise expression and fast execution of simple
// operations on numeric arrays is wanted. This module calculates
// the expression
//   v2 += v0 - 2.0 * v1; // v0, v1, and v2 are arrays.
// using several methods:
//   - straightforward C
//   - STL only
//   - STL and a scalar-expression library
//   - std::valarray
//   - boost uBLAS
//   - PETE
// and serves mainly to demonstrate the verbosity and limitations of
// the STL approaches.

// These variables are global because passing them as arguments is
// quite a chore, and an unnecessary one in this standalone module.

int const length = 100;

// cv*: C vectors.
double cv0[length];
double cv1[length];
double cv2[length];

// sv*: Standard vectors, first test.
std::vector<double> sv0a;
std::vector<double> sv1a;
std::vector<double> sv2a;

// sv*: Standard vectors, second test.
std::vector<double> sv0b;
std::vector<double> sv1b;
std::vector<double> sv2b;

// va*: Standard valarrays.
std::valarray<double> va0;
std::valarray<double> va1;
std::valarray<double> va2;

// ub*: boost uBLAS.
#if defined USE_UBLAS
boost::numeric::ublas::vector<double> ub0;
boost::numeric::ublas::vector<double> ub1;
boost::numeric::ublas::vector<double> ub2;
#endif // defined USE_UBLAS

// ps*: PETE standard vectors.
#if defined USE_PETE
std::vector<double> pv0;
std::vector<double> pv1;
std::vector<double> pv2;
#endif // defined USE_PETE

// These 'mete*' functions perform the same set of operations using
// different implementations.

/// This implementation uses a straightforward technique typical of C.
/// Its shortcoming is that it is all too easy to code it incorrectly,
/// for instance
///    for(int j = 1; i <= length; ++j)
///        v2[j] += v0 - 2.0 * v1[0];
/// and that many such errors can be very difficult to find.

void mete_c()
{
    for(int j = 0; j < length; ++j)
        {
        cv2[j] += cv0[j] - 2.0 * cv1[j];
        }
}

/// This implementation uses plain STL.

void mete_stl_plain()
{
    std::vector<double> tmp0;
    tmp0.reserve(length);
    std::transform
        (sv1a.begin()
        ,sv1a.end()
        ,std::back_inserter(tmp0)
        ,std::bind1st(std::multiplies<double>(), 2.0)
        );
    std::transform
        (sv0a.begin()
        ,sv0a.end()
        ,tmp0.begin()
        ,tmp0.begin()
        ,std::minus<double>()
        );
    std::transform
        (sv2a.begin()
        ,sv2a.end()
        ,tmp0.begin()
        ,sv2a.begin()
        ,std::plus<double>()
        );
}

/// This implementation uses STL in a fancier way. A 'lambda' library
/// handles some of the arithmetic, and a temporary vector is kept as
/// a static local variable (which improves performance noticeably,
/// but introduces new problems--thread safety not least of all).
/// Compared to mete_stl_plain(), this method is somewhat faster and
/// more compact, yet more abstruse.
///
/// An expression-template numeric-array class performs two jobs:
///   it agglutinates expressions, deferring their evaluation; and
///   it applies the agglutinated expression across all elements.
///
/// 'Lambda' libraries do the first job only: agglutination. Using
/// STL facilities like std::for_each or std::transform for the
/// other job, application, painfully restricts arity to two. This
/// model accommodates arbitrarily complicated operations (e.g., a
/// truncated Taylor series), but only for one or two operands: it
/// is not possible to add four vectors (v0 + v1 + v2 + v3).
///
/// Here, two of the std::transform steps in the 'plain' example
/// are combined, avoiding superfluous loads and stores, but still
/// it's impossible to write
///    v2 += v0 - 2.0 * v1;
/// with only one call to std::transform, which at best writes the
/// result of a binary operation to an OutputIterator--and an
/// OutputIterator can't access its own prior value.
///
/// Of course, n-ary analogs of std::transform could be written,
/// but what's really wanted is a much more concise notation.

void mete_stl_fancy()
{
#if !defined __BORLANDC__
    static std::vector<double> tmp0(length);
    std::transform
        (sv0b.begin()
        ,sv0b.end()
        ,sv1b.begin()
        ,tmp0.begin()
        ,boost::bind
            (std::minus<double>()
            ,_1
            ,boost::bind
                (std::multiplies<double>()
                ,_2
                ,2.0
                )
            )
        );
    std::transform
        (sv2b.begin()
        ,sv2b.end()
        ,tmp0.begin()
        ,sv2b.begin()
        ,std::plus<double>()
        );
#endif // !defined __BORLANDC__
}

/// This implementation uses std::valarray.

void mete_valarray()
{
    va2 += va0 - 2.0 * va1;
}

/// This implementation uses boost::numeric::ublas::vector.

#if defined USE_UBLAS
void mete_ublas()
{
    ub2 += ub0 - 2.0 * ub1;
}
#endif // defined USE_UBLAS

#if defined USE_PETE
void mete_pete()
{
    pv2 += pv0 - 2.0 * pv1;
}
#endif // defined USE_PETE

void run_one_test(std::string const& s, void(*f)())
{
    double const max_seconds = 10.0;
    std::cout
        << "  Speed test: "
        << s
        << '\n'
        << TimeAnAliquot(f, max_seconds)
        << '\n'
        ;
}

int test_main(int, char*[])
{
    for(int j = 0; j < length; ++j)
        {
        cv0[j] = 0.100 * j;
        cv1[j] = 0.010 * j;
        cv2[j] = 0.001 * j;
        }

    sv0a = std::vector<double>(cv0, cv0 + length);
    sv1a = std::vector<double>(cv1, cv1 + length);
    sv2a = std::vector<double>(cv2, cv2 + length);

    sv0b = std::vector<double>(cv0, cv0 + length);
    sv1b = std::vector<double>(cv1, cv1 + length);
    sv2b = std::vector<double>(cv2, cv2 + length);

    // Don't try to assign to a default-constructed valarray without
    // resizing it first [26.3.2.2/1].
    va0.resize(length);
    va1.resize(length);
    va2.resize(length);

    va0 = std::valarray<double>(cv0, length);
    va1 = std::valarray<double>(cv1, length);
    va2 = std::valarray<double>(cv2, length);

#if defined USE_UBLAS
    ub0.resize(length);
    ub1.resize(length);
    ub2.resize(length);

    std::copy(cv0, cv0 + length, ub0.begin());
    std::copy(cv1, cv1 + length, ub1.begin());
    std::copy(cv2, cv2 + length, ub2.begin());
#endif // defined USE_UBLAS

#if defined USE_PETE
    pv0 = std::vector<double>(cv0, cv0 + length);
    pv1 = std::vector<double>(cv1, cv1 + length);
    pv2 = std::vector<double>(cv2, cv2 + length);
#endif // defined USE_PETE

    double const value01 = 0.001 + 0.100 - 2.0 * 0.010;
    double const value99 = 99.0 * value01;

    mete_c();
    BOOST_TEST_EQUAL(cv2 [ 1], value01);
    BOOST_TEST_EQUAL(cv2 [99], value99);

    mete_stl_plain();
    BOOST_TEST_EQUAL(sv2a[ 1], value01);
    BOOST_TEST_EQUAL(sv2a[99], value99);

#if !defined __BORLANDC__
    mete_stl_fancy();
    BOOST_TEST_EQUAL(sv2b[ 1], value01);
    BOOST_TEST_EQUAL(sv2b[99], value99);
#endif // !defined __BORLANDC__

    mete_valarray();
    BOOST_TEST_EQUAL(va2 [ 1], value01);
    BOOST_TEST_EQUAL(va2 [99], value99);

#if defined USE_UBLAS
    mete_ublas();
    BOOST_TEST_EQUAL(ub2 [ 1], value01);
    BOOST_TEST_EQUAL(ub2 [99], value99);
#endif // defined USE_UBLAS

#if defined USE_PETE
    mete_pete();
    BOOST_TEST_EQUAL(pv2[ 1], value01);
    BOOST_TEST_EQUAL(pv2[99], value99);
#endif // defined USE_PETE

    run_one_test("C"        , mete_c        );
    run_one_test("STL plain", mete_stl_plain);
    run_one_test("STL fancy", mete_stl_fancy);
    run_one_test("valarray" , mete_valarray );
#if defined USE_UBLAS
    run_one_test("uBLAS"    , mete_ublas    );
#endif // defined USE_UBLAS
#if defined USE_PETE
    run_one_test("PETE"     , mete_pete     );
#endif // defined USE_PETE

    return 0;
}

