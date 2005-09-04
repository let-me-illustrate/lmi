// Expression templates, investigation 0--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: expression_template_0_test.cpp,v 1.1 2005-09-04 17:05:33 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#define BOOST_INCLUDE_MAIN
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

// These 'mete*' functions perform the same set of operations using
// different implementations.

/// This implementation uses a straightforward technique typical of C.
/// Its shortcoming is that it is all too easy to code it incorrectly,
/// for instance
///    for(int j = 1; i <= length; ++j)
///        v2[j] += v0 - 2.0 * v1[0];
/// and that many such errors can be very difficult to find.

void mete_c
    (int length
    ,double* v0
    ,double* v1
    ,double* v2
    )
{
    for(int j = 0; j < length; ++j)
        {
        v2[j] += v0[j] - 2.0 * v1[j];
        }
}

/// This implementation uses STL in a naive way.

void mete_stl_naive
    (int
    ,std::vector<double>& v0
    ,std::vector<double>& v1
    ,std::vector<double>& v2
    )
{
    std::vector<double> tmp0;
    // Omitting the call to reserve() greatly impairs performance.
    tmp0.reserve(100);
    std::transform
        (v1.begin()
        ,v1.end()
        ,std::back_inserter(tmp0)
        ,std::bind1st(std::multiplies<double>(), 2.0)
        );
    std::transform
        (v0.begin()
        ,v0.end()
        ,tmp0.begin()
        ,tmp0.begin()
        ,std::minus<double>()
        );
    std::transform
        (v2.begin()
        ,v2.end()
        ,tmp0.begin()
        ,v2.begin()
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

void mete_stl_smart
    (int
    ,std::vector<double>& v0
    ,std::vector<double>& v1
    ,std::vector<double>& v2
    )
{
    // Writing 'static' here is an optimization, though of course it
    // is not consonant with thread safety.
    static std::vector<double> tmp0;
    tmp0.reserve(100);
    std::transform
        (v0.begin()
        ,v0.end()
        ,v1.begin()
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
        (v2.begin()
        ,v2.end()
        ,tmp0.begin()
        ,v2.begin()
        ,std::plus<double>()
        );
}

/// This implementation uses std::valarray.

void mete_valarray
    (int
    ,std::valarray<double>& v0
    ,std::valarray<double>& v1
    ,std::valarray<double>& v2
    )
{
    v2 += v0 - 2.0 * v1;
}

int test_main(int, char*[])
{
    int const length = 100;

    // cv*: C vectors.
    double cv0[length];
    double cv1[length];
    double cv2[length];

    for(int j = 0; j < length; ++j)
        {
        cv0[j] = 0.100 * j;
        cv1[j] = 0.010 * j;
        cv2[j] = 0.001 * j;
        }

    // sv*: standard vectors
    std::vector<double> sv0(cv0, cv0 + length);
    std::vector<double> sv1(cv1, cv1 + length);
    std::vector<double> sv2(cv2, cv2 + length);

    // va*: valarrays
    std::valarray<double> va0(cv0, length);
    std::valarray<double> va1(cv1, length);
    std::valarray<double> va2(cv2, length);

    double original = cv2[1];

    mete_c        (length, cv0, cv1, cv2);
    BOOST_TEST_EQUAL(cv2[1], 0.001 + 0.100 - 2.0 * 0.010);

    mete_stl_naive(length, sv0, sv1, sv2);
    BOOST_TEST_EQUAL(sv2[1], 0.001 + 0.100 - 2.0 * 0.010);

    sv2[1] = original; // Restore overwritten initial value.
    mete_stl_smart(length, sv0, sv1, sv2);
    BOOST_TEST_EQUAL(sv2[1], 0.001 + 0.100 - 2.0 * 0.010);

    mete_valarray (length, va0, va1, va2);
    BOOST_TEST_EQUAL(va2[1], 0.001 + 0.100 - 2.0 * 0.010);

    double const max_seconds = 10.0;
    std::cout << "  Speed test: C\n"         << aliquot_timer(boost::bind(mete_c        , length, cv0, cv1, cv2), max_seconds) << '\n';
    std::cout << "  Speed test: STL naive\n" << aliquot_timer(boost::bind(mete_stl_naive, length, sv0, sv1, sv2), max_seconds) << '\n';
    std::cout << "  Speed test: STL smart\n" << aliquot_timer(boost::bind(mete_stl_smart, length, sv0, sv1, sv2), max_seconds) << '\n';
    std::cout << "  Speed test: valarray\n"  << aliquot_timer(boost::bind(mete_valarray , length, va0, va1, va2), max_seconds) << '\n';

    return 0;
}

