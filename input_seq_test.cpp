// Input sequences e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0... : unit test.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: input_seq_test.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "input_sequence.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <cassert>
#include <iterator>

// Macros don't offer default arguments, so we need one macro
// for each number of arguments.

#ifdef CHECK0
#   error CHECK0 already defined.
#else //  CHECK0
#   define CHECK0(D, N, E) \
        { \
        InputSequence seq((E), (N), 90, 95, 0, 2002, 0); \
        std::vector<double> v(seq.linear_number_representation()); \
        bool b = std::equal((D), (D) + (N), v.begin()); \
        (b) \
            ? static_cast<void>(0) \
            : check((D), (N), (E)); BOOST_TEST(b); \
        }
#endif // CHECK0

#ifdef CHECK1
#   error CHECK1 already defined.
#else //  CHECK1
#   define CHECK1(D, N, E, K, C) \
        { \
        InputSequence seq((E), (N), 90, 95, 0, 2002, 0, (K)); \
        std::vector<double> v(seq.linear_number_representation()); \
        std::vector<std::string> s(seq.linear_keyword_representation()); \
        bool b = \
               std::equal((D), (D) + (N), v.begin()) \
            && std::equal((C), (C) + (N), s.begin()) \
            ; \
        (b) \
            ? static_cast<void>(0) \
            : check((D), (N), (E), (K), (C)); BOOST_TEST(b); \
        }
#endif // CHECK1

#ifdef CHECK2
#   error CHECK2 already defined.
#else //  CHECK2
#   define CHECK2(D, N, E, K, C, W) \
        { \
        InputSequence seq((E), (N), 90, 95, 0, 2002, 0, (K), (W)); \
        std::vector<double> v(seq.linear_number_representation()); \
        std::vector<std::string> s(seq.linear_keyword_representation()); \
        bool b = \
               std::equal((D), (D) + (N), v.begin()) \
            && std::equal((C), (C) + (N), s.begin()) \
            ; \
        (b) \
            ? static_cast<void>(0) \
            : check((D), (N), (E), (K), (C), (W)); BOOST_TEST(b); \
        }
#endif // CHECK2

template<typename T>
void check
    (T const* d
    ,int n
    ,std::string const& e
    ,std::vector<std::string> const& k = std::vector<std::string>(0)
    ,char const* const* c = 0
    ,std::string const& w = std::string("")
    )
{
    InputSequence seq(e, n, 90, 95, 0, 2002, 0, k, w);

    std::vector<T> v(seq.linear_number_representation());
    // Assert that std::equal() has defined behavior.
    assert(v.size() == static_cast<unsigned int>(n));
    if(!std::equal(d, d + n, v.begin()))
        {
        std::cout << "\nExpression: '" << e << '\'';
        std::cout << "\n  expected: ";
        std::copy(d, d + n, std::ostream_iterator<T>(std::cout, " "));
        std::cout << "\n   but got: ";
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
        std::cout << std::endl;
        }

    std::vector<std::string> s(seq.linear_keyword_representation());
    // Assert that std::equal() has defined behavior.
    assert(s.size() == static_cast<unsigned int>(n));
    if((0 != c) && !std::equal(c, c + n, s.begin()))
        {
        std::cout << "\nExpression: '" << e << '\'';
        std::cout << "\n  expected: ";
        std::copy(c, c + n, std::ostream_iterator<char const*>(std::cout, "|"));
        std::cout << "\n   but got: ";
        std::copy(s.begin(), s.end(), std::ostream_iterator<std::string>(std::cout, "|"));
        std::cout << std::endl;
        }

    if(seq.formatted_diagnostics().size())
        {
        std::cout << '\n' << seq.formatted_diagnostics() << '\n';
        std::cout << std::endl;
        }
}

int test_main(int, char*[])
{
    // Make sure example in comment at top works.
    //   1 3; 7 5;0; --> 1 1 1 7 7 0...
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 7, 7, 0, 0, 0, 0};
    std::string e("1 3; 7 5;0");
    CHECK0(d, n, e);
    }

    // Numbers separated by semicolons mean values; the last is
    // replicated to fill the vector.
    {
    int const n = 5;
    double const d[n] = {1, 2, 3, 3, 3};
    std::string e("1; 2; 3");
    CHECK0(d, n, e);
    }

    // Number-pairs separated by semicolons mean {value, end-duration}.
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 3; 3 6; 5 9; 7");
    CHECK0(d, n, e);
    }

    // {value, @ attained_age}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 @93; 3 @96; 5 @99; 7");
    CHECK0(d, n, e);
    }

    // {value, # number_of_years_since_last_interval_endpoint}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 #3; 3 #3; 5 #3; 7");
    CHECK0(d, n, e);
    }

    // {value [|( begin-duration, end-duration ]|) }

    // Test [x,y).
    {
    int const n = 9;
    double const d[n] = {1, 1, 3, 3, 3, 5, 7, 7, 7};
    std::string e("1 [0, 2); 3 [2, 5); 5 [5, 6); 7");
    CHECK0(d, n, e);
    }

    // Test (x,y].
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 7, 7};
    std::string e("1; 1 (0, 2]; 3 (2, 5]; 5 (5, 6]; 7");
    CHECK0(d, n, e);
    }

    // Test a mixture of all five ways of specifying duration.
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 1, 2, 3, 4, 5, 5};
    std::string e("1 [0, 4); 2 5; 3 #1; 4 @97; 5");
    CHECK0(d, n, e);
    }

    // Test intervals of length one.
    {
    int const n = 5;
    double const d[n] = {1, 3, 5, 7, 7};
    std::string e("1 [0, 1); 3 [1, 2); 5 (1, 2]; 7");
    CHECK0(d, n, e);
    }

    // Test empty intervals.
    {
    int const n = 5;
    double const d[n] = {1, 3, 5, 7, 7};
    std::string e("1 [0, 1); 3 [1, 1]; 5 (1, 2]; 7");
    CHECK0(d, n, e);
    }

    // Test subtly improper intervals.
    {
    int const n = 5;
    double const d[n] = {0, 0, 0, 0, 0};
    std::string e("1 [0, 0); 3 (1, 2); 5 (2, 2]; 7");
    CHECK0(d, n, e);
    }

    // Test grossly improper intervals.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string e("1; 9 (2, 0]; 3 [7, 3); 5 (5, 5); 7");
    CHECK0(d, n, e);
    }

    // Test intervals with 'holes'. Since the last element is replicated,
    // there can be no 'hole' at the end.
    {
    int const n = 9;
    double const d[n] = {0, 1, 0, 3, 0, 5, 7, 7, 7};
    std::string e("1 [1, 2); 3 [3, 3]; 5 (4, 5]; 7");
    CHECK0(d, n, e);
    }

    // Test overlapping intervals.
    // TODO ?? Treat these as an error?
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 3, 3, 5, 5, 7, 7};
    std::string e("1; 1 (0, 8]; 3 (2, 7]; 5 (4, 6]; 7");
    CHECK0(d, n, e);
    }

    // Test intervals with decreasing begin-points.
    // TODO ?? Should this case be allowed?
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string e("5 [5, 6); 3 [2, 5); 1 [0, 2); 7");
    CHECK0(d, n, e);
    }

    // Durations with '@' prefix mean attained age.
    {
    int const n = 10;
    double const d[n] = {0, 12, 0, 27, 0, 1, 7, 7, 7, 7};
    std::string e("12 [1, @92); 27 [@93, @93]; 1 (@94, 5]; 7");
    CHECK0(d, n, e);
    }

    // Test floating-point values; we choose values that we know
    // must be exactly representable on a binary machine, so that a
    // simple test for equality suffices.
    {
    int const n = 10;
    double const d[n] = {0, 12.25, 0, 27.875, 0, 1.0625, 7.5, 7.5, 7.5, 7.5};
    std::string e("12.25 [1, @92); 27.875 [@93, @93]; 1.0625 (@94, 5]; 7.5");
    CHECK0(d, n, e);
    }

    // {value, @ age} means {value, to-attained-age}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 @93; 3 @96; 5 @99; 7");
    CHECK0(d, n, e);
    }

// TODO ?? Also support and test:
//   additive expressions e.g. retirement-10 ?

    // Test construction from vector.
    //   1 1 1 2 2 --> 1 [0,3);2 [3,5)
    // TODO ?? Test against canonical representation once we define that.
    {
    int const n = 5;
    double const d[n] = {1, 1, 1, 2, 2};
    std::vector<double> v(d, d + n);
    BOOST_TEST(v == InputSequence(v).linear_number_representation());
    }

    // Test (enumerative) extra keywords.
    {
    int const n = 9;
    char const* c[n] = {"a", "a", "ccc", "ccc", "b", "b", "b", "b", "b"};
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string e("a[0, 2); ccc [2, 4);b[4, 6);");
    std::vector<std::string> k;
    k.push_back("not_used");
    k.push_back("a");
    k.push_back("b");
    k.push_back("c");
    k.push_back("cc");
    k.push_back("ccc");
    CHECK1(d, n, e, k, c);
    }

    // Test numbers mixed with (enumerative) extra keywords.
    {
    int const n = 9;
    char const* c[n] = {"", "", "keyword_00", "keyword_00", "", "", "", "", ""};
    double const d[n] = {1, 1, 0, 0, 5, 5, 7, 7, 7};
    std::string e("1 [0, 2); keyword_00 [2, 4); 5 [4, 6); 7");
    std::vector<std::string> k;
    k.push_back("keyword_00");
    CHECK1(d, n, e, k, c);
    }

    // Test numbers mixed with (enumerative) extra keywords, with
    // a default keyword.
    {
    int const n = 10;
    char const* c[n] =  {"b", "b", "x", "a", "x", "x", "a", "x", "x", "x"};
    double const d[n] = {  0,   0,   0,   0,   5,   5,   0,   7,   7,   7};
    std::string e("b [0, 2); a [3, 4); 5 [4, 6); a; 7");
    std::vector<std::string> k;
    k.push_back("a");
    k.push_back("b");
    k.push_back("x");
    std::string w("x");
    CHECK2(d, n, e, k, c, w);
    }

// TODO ?? Also test keyword as scalar duration.

    // Duration keywords: {retirement, maturity}
    {
    int const n = 10;
    double const d[n] = {7, 7, 7, 7, 7, 4, 4, 4, 4, 4};
    std::string e("7, retirement; 4");
    CHECK0(d, n, e);
    InputSequence seq("7, retirement; 4", 10, 90, 95, 0, 2002, 0);
    std::vector<ValueInterval> const& i(seq.interval_representation());
    BOOST_TEST(e_inception  == i[0].begin_mode);
    BOOST_TEST(e_retirement == i[0].end_mode  );
    BOOST_TEST(e_retirement == i[1].begin_mode);
    BOOST_TEST(e_maturity   == i[1].end_mode  );
std::cout << i[0].begin_mode << ' ' << i[0].end_mode << '\n';
std::cout << i[1].begin_mode << ' ' << i[1].end_mode << '\n';
    }


// TODO ?? Also test default keyword.

// TODO ?? Also test keywords-only switch.


    return 0;
}

#undef CHECK0
#undef CHECK1
#undef CHECK2

