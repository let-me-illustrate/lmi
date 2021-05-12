// Expression templates, investigation 0--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "pchfile.hpp"

#if 202002L <= __cplusplus
#   define USE_UBLAS
#endif // 202002L < __cplusplus

#if defined USE_UBLAS
// BOOST !! Startlingly enough, boost uBLAS depends on this standard
// macro. If it's not defined, then expression templates aren't used,
// which impairs performance significantly and removes an essential
// reason for using this library. However, if it is defined, then it
// must be defined consistently everywhere to avoid ODR problems; but
// that suppresses assertions in other libraries, which it might be
// desirable to leave in production code. It would have been easy to
// accommodate programmers who never want to turn off assertions by
// using a library-specific macro instead. Cf.:
//   http://lists.boost.org/Archives/boost/2003/10/55518.php
#   define NDEBUG 1
#endif // defined USE_UBLAS

#include "et_vector.hpp"
#include "materially_equal.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#if defined USE_UBLAS
#   if defined LMI_CLANG
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wdeprecated-copy"
#       pragma clang diagnostic ignored "-Wdeprecated-declarations"
#       pragma clang diagnostic ignored "-Wunused-parameter"
#   endif // defined LMI_CLANG
#   include <boost/numeric/ublas/vector.hpp>
#   if defined LMI_CLANG
#       pragma clang diagnostic pop
#   endif // defined LMI_CLANG
#endif // defined USE_UBLAS

#include <algorithm>
#include <functional>                   // bind() et al.
#include <iterator>                     // back_inserter()
#include <string>
#include <valarray>
#include <vector>

// A facility for concise expression and fast execution of simple
// operations on numeric arrays is wanted. This module calculates
// the expression
//   v2 += v0 - 2.1 * v1; // v0, v1, and v2 are arrays.
// using several methods:
//   - straightforward C
//   - STL only
//   - STL and a scalar-expression library
//   - std::valarray
//   - boost uBLAS
//   - PETE
// and serves mainly to demonstrate the verbosity and limitations of
// the STL approaches.

namespace
{
    // Global variables for timing tests. They could alternatively be
    // passed as arguments, e.g., by using std::bind, but that would
    // increase complexity in return for no real benefit.

    int g_length = 1;

    int const max_length = 10000;

    // Number of iterations for 'mete*' functions. This value is a
    // compromise: higher values make this unit test take too long,
    // while lower values may yield measurements that are less than
    // a one-microsecond timer tick.

    int const n_iter = 100;

    // cv*: C vectors.
    double cv0[max_length];
    double cv1[max_length];
    double cv2[max_length];

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

    // pv*: PETE standard vectors.
    std::vector<double> pv0;
    std::vector<double> pv1;
    std::vector<double> pv2;
} // Unnamed namespace.

// These 'mete*' functions perform the same set of operations using
// different implementations.

/// This implementation uses a straightforward technique typical of C.
/// Its shortcoming is that it is all too easy to code it incorrectly,
/// for instance
///    for(int j = 1; i <= g_length; ++j)
///        v2[j] += v0 - 2.1 * v1[0];
/// and that many such errors can be very difficult to find.

void mete_c()
{
    for(int i = 0; i < n_iter; ++i)
        {
        for(int j = 0; j < g_length; ++j)
            {
            cv2[j] += cv0[j] - 2.1 * cv1[j];
            }
        }
}

/// This implementation uses plain STL.

void mete_stl_plain()
{
    for(int i = 0; i < n_iter; ++i)
        {
        std::vector<double> tmp0;
        tmp0.reserve(g_length);
        std::transform
            (sv1a.begin()
            ,sv1a.end()
            ,std::back_inserter(tmp0)
            ,[](double x) { return 2.1 * x; }
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
///    v2 += v0 - 2.1 * v1;
/// with only one call to std::transform, which at best writes the
/// result of a binary operation to an OutputIterator--and an
/// OutputIterator can't access its own prior value.
///
/// Of course, n-ary analogs of std::transform could be written,
/// but what's really wanted is a much more concise notation.

void mete_stl_fancy()
{
    for(int i = 0; i < n_iter; ++i)
        {
        static std::vector<double> tmp0(max_length);
        std::transform
            (sv0b.begin()
            ,sv0b.end()
            ,sv1b.begin()
            ,tmp0.begin()
            ,std::bind
                (std::minus<double>()
                ,std::placeholders::_1
                ,std::bind
                    (std::multiplies<double>()
                    ,std::placeholders::_2
                    ,2.1
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
}

/// This implementation uses std::valarray.

void mete_valarray()
{
    for(int i = 0; i < n_iter; ++i)
        {
        va2 += va0 - 2.1 * va1;
        }
}

/// This implementation uses boost::numeric::ublas::vector.

#if defined USE_UBLAS
void mete_ublas()
{
    for(int i = 0; i < n_iter; ++i)
        {
        ub2 += ub0 - 2.1 * ub1;
        }
}
#endif // defined USE_UBLAS

void mete_pete()
{
    for(int i = 0; i < n_iter; ++i)
        {
        pv2 += pv0 - 2.1 * pv1;
        }
}

void run_one_test(std::string const& s, void(*f)())
{
    double const max_seconds = 1.0;
    std::cout
        << "  "
        << s
        << ": "
        << TimeAnAliquot(f, max_seconds)
        << std::endl
        ;
}

// Review of 'ET !! ' markers shows that lmi's needs are fairly
// represented by these examples:
//
// new_v = v0 - v1;
// new_v = s0 - v1;
//
// v0 += v1;
// v0 = max(c0, v1);
// v0 = mean(v1, v2);
// v0 = (1 - v0) * v1;

void mete_valarray_typical()
{
    for(int i = 0; i < n_iter; ++i)
        {
        std::valarray<double> va8 = va0 - va1;
        std::valarray<double> va9 = 3.14 - va0;
        va8 += va0;
        va8 += va0 * va1;
// This doesn't compile, and std::valarray's only comparable facility
// is its apply() member function, which applies only unary functions.
//    va0 = std::max(2.7, va8);
//    va0 = std::max(va8, va9);
        va9 = (1.0 - va8) * va9;
        }
}

#if defined USE_UBLAS
// This is never actually called. It is incomplete: it is not known
// whether the operations measured by other /mete.*typical/ functions
// can even be expressed with this library; and that doesn't matter,
// because other tests show it to be slower than its competitors.
//
// This library seems to provide most of what we need as named
// functions like prod(), not as the overloaded operators that
// we'd prefer for clarity.
void mete_ublas_typical()
{
    for(int i = 0; i < n_iter; ++i)
        {
        boost::numeric::ublas::vector<double> ub8 = ub0 - ub1;
        boost::numeric::ublas::vector<double> ub9;
        // "no match for 'operator-'":
//        ub9 = 3.14 - ub0;
        ub8 += ub0;
        }

        // "ambiguous overload for 'operator*' in 'ub0 * ub1'":
//        ub8 += ub0 * ub1;

//     This doesn't compile:
//        ub0 = std::max(2.7, ub8);
//        ub0 = std::max(ub8, ub9);

        // "error: no match for 'operator-' in '1.0e+0 - ub8'":
//        ub9 = (1.0 - ub8) * ub9;
}
#endif // defined USE_UBLAS

void mete_pete_typical()
{
    for(int i = 0; i < n_iter; ++i)
        {
        std::vector<double> pv8(pv0.size()); assign(pv8, pv0 - pv1);
        std::vector<double> pv9(pv0.size()); assign(pv9, 3.14 - pv0);
        pv8 += pv0;
        pv8 += pv0 * pv1;

// This doesn't compile:
//    pv0 = std::max(2.7, pv8);

// This compiles, but doesn't apply the functor to each element pair:
//    pv0 = std::max(pv8, pv9);

// This works. It's commented out only for comparability to other
// approaches.
//    assign(pv0, Max(pv8, pv9));

        assign(pv9, (1.0 - pv8) * pv9);
        }
}

void time_one_array_length(int length)
{
    g_length = length;
    std::cout << "  Speed tests: array length " << g_length << std::endl;

    for(int j = 0; j < g_length; ++j)
        {
        cv0[j] = 0.100 * j;
        cv1[j] = 0.010 * j;
        cv2[j] = 0.001 * j;
        }

    sv0a = std::vector<double>(cv0, cv0 + g_length);
    sv1a = std::vector<double>(cv1, cv1 + g_length);
    sv2a = std::vector<double>(cv2, cv2 + g_length);

    sv0b = std::vector<double>(cv0, cv0 + g_length);
    sv1b = std::vector<double>(cv1, cv1 + g_length);
    sv2b = std::vector<double>(cv2, cv2 + g_length);

    // Don't try to assign to a default-constructed valarray without
    // resizing it first [26.3.2.2/1].
    va0.resize(g_length);
    va1.resize(g_length);
    va2.resize(g_length);

    va0 = std::valarray<double>(cv0, g_length);
    va1 = std::valarray<double>(cv1, g_length);
    va2 = std::valarray<double>(cv2, g_length);

#if defined USE_UBLAS
    ub0.resize(g_length);
    ub1.resize(g_length);
    ub2.resize(g_length);

    std::copy(cv0, cv0 + g_length, ub0.begin());
    std::copy(cv1, cv1 + g_length, ub1.begin());
    std::copy(cv2, cv2 + g_length, ub2.begin());
#endif // defined USE_UBLAS

    pv0 = std::vector<double>(cv0, cv0 + g_length);
    pv1 = std::vector<double>(cv1, cv1 + g_length);
    pv2 = std::vector<double>(cv2, cv2 + g_length);

    int const alpha = 1 < g_length ? 1 : 0;
    int const omega = g_length - 1;
    double const value_alpha = alpha * (0.001 + n_iter * (0.100 - 2.1 * 0.010));
    double const value_omega = omega * value_alpha;

    mete_c();
    LMI_TEST(materially_equal(cv2 [omega], value_omega));

    mete_stl_plain();
    LMI_TEST(materially_equal(sv2a[omega], value_omega));

    mete_stl_fancy();
    LMI_TEST(materially_equal(sv2b[omega], value_omega));

    mete_valarray();
    LMI_TEST(materially_equal(va2 [omega], value_omega));

#if defined USE_UBLAS
    mete_ublas();
    LMI_TEST(materially_equal(ub2 [omega], value_omega));
#endif // defined USE_UBLAS

    mete_pete();
    LMI_TEST(materially_equal(pv2[omega], value_omega));

    run_one_test("C               ", mete_c        );
    run_one_test("STL plain       ", mete_stl_plain);
    run_one_test("STL fancy       ", mete_stl_fancy);
    run_one_test("valarray        ", mete_valarray );
#if defined USE_UBLAS
    run_one_test("uBLAS           ", mete_ublas    );
#endif // defined USE_UBLAS
    run_one_test("PETE            ", mete_pete     );

    std::cout << std::endl;

    run_one_test("valarray typical", mete_valarray_typical);
    run_one_test("PETE typical    ", mete_pete_typical    );

    std::cout << std::endl;
}

/// Assigning PETE expressions to a std::vector
///
/// std::vector<>::operator= is necessarily a member function, and
/// cannot be overloaded. This function shows some alternatives,
/// none of which seems ideal:
///
///   assign(v, expression);
///   v = std::vector(intended_size); v << expression;
///   v = std::vector(intended_size); v += expression;

void test_pete_assignment()
{
    std::vector<double> const v0 = {1.1, 2.2, 3.3, 4.4, 5.5};
    std::vector<double> const v1 = {0.1, 0.2, 0.3, 0.4, 0.5};
    std::vector<double> const v2 = {1.0, 2.0, 3.0, 4.0, 5.0};
// With lmi's overload of operator<<=(), this assign() call:
    std::vector<double> v7a(v0.size());
    assign(v7a, v0 - v1);
    LMI_TEST(v2 == v7a);
// can be written as an operator instead:
    std::vector<double> v7b(v0.size());
    v7b <<= v0 - v1;
    LMI_TEST(v2 == v7b);
// though these still wouldn't compile:
//  std::vector<double> v7c <<= v0 - v1;
//  std::vector<double> v7d(v0 - v1);
// and, even though this default-constructed vector is of length zero:
    std::vector<double> v7e;
// this just works (the result has the intended size):
    v7e <<= v0 - v1;
    LMI_TEST(v0.size() == v7e.size());

// On the other hand, this syntax is almost natural, even though it's
// silly to add zero to everything.
    std::vector<double> v7f(v0.size());
    v7f += v0 - v1;
    LMI_TEST(v2 == v7f);
// But that may be the best that can easily be done with PETE: where
//  std::vector<double> v7f += v0 - v1;
// is wanted, instead write
//  std::vector<double> v7f(intended_size);
//  v7f += v0 - v1;
}

int test_main(int, char*[])
{
    time_one_array_length(1);
    time_one_array_length(10);
    time_one_array_length(100);
    time_one_array_length(1000);
    time_one_array_length(10000);

    test_pete_assignment();

    return 0;
}
