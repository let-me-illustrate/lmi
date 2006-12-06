// Expression templates, investigation 0--unit test.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: expression_template_0_test.cpp,v 1.5 2006-12-06 16:23:17 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/bind.hpp>

#include <functional>
#include <iterator>
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

/// This implementation uses STL in a naive way.

void mete_stl_naive()
{
    std::vector<double> tmp0;
    // Omitting the call to reserve() greatly impairs performance.
    tmp0.reserve(100);
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

/// This implementation uses STL in a smarter way.
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
/// Here, two of the std::transform steps in the 'naive' example
/// are combined, avoiding superfluous loads and stores, but still
/// it's impossible to write
///    v2 += v0 - 2.0 * v1;
/// with only one call to std::transform, which at best writes the
/// result of a binary operation to an OutputIterator--and an
/// OutputIterator can't access its own prior value.
///
/// Of course, n-ary analogs of std::transform could be written,
/// but what's really wanted is a much more concise notation.

void mete_stl_smart()
{
    // Writing 'static' here is an optimization, though of course it
    // is not consonant with thread safety.
    static std::vector<double> tmp0;
    tmp0.reserve(100);
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
}

/// This implementation uses std::valarray.

void mete_valarray()
{
    va2 += va0 - 2.0 * va1;
}

void run_one_test(std::string const& s, void(*f)())
{
    double const max_seconds = 10.0;
    std::cout
        << "  Speed test: "
        << s
        << '\n'
        << aliquot_timer(f, max_seconds)
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

    mete_c();
    BOOST_TEST_EQUAL(cv2 [1], 0.001 + 0.100 - 2.0 * 0.010);

    mete_stl_naive();
    BOOST_TEST_EQUAL(sv2a[1], 0.001 + 0.100 - 2.0 * 0.010);

    mete_stl_smart();
    BOOST_TEST_EQUAL(sv2b[1], 0.001 + 0.100 - 2.0 * 0.010);

    mete_valarray();
    BOOST_TEST_EQUAL(va2 [1], 0.001 + 0.100 - 2.0 * 0.010);

    run_one_test("C"        , mete_c        );
    run_one_test("STL naive", mete_stl_naive);
    run_one_test("STL smart", mete_stl_smart);
    run_one_test("valarray" , mete_valarray );

    return 0;
}

