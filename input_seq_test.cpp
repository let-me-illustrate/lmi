// Input sequences e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0... : unit test.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "input_sequence.hpp"

#include "test_tools.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>                     // std::ostream_iterator

void analyze_errors
    (double const*                   d
    ,int                             n
    ,std::string const&              e
    ,std::vector<std::string> const& k = std::vector<std::string>(0)
    ,char const* const*              c = nullptr
    ,std::string const&              w = std::string()
    )
{
    InputSequence seq(e, n, 90, 95, 0, 2002, 0, k, w);

    std::vector<double> v(seq.linear_number_representation());
    // Assert that std::equal() has defined behavior.
    assert(v.size() == static_cast<unsigned int>(n));
    if(v != std::vector<double>(d, d + n))
        {
        std::cout << "\nExpression: '" << e << '\'';
        std::cout << "\n  expected: ";
        std::copy(d, d + n, std::ostream_iterator<double>(std::cout, " "));
        std::cout << "\n   but got: ";
        std::copy(v.begin(), v.end(), std::ostream_iterator<double>(std::cout, " "));
        std::cout << std::endl;
        }

    std::vector<std::string> s(seq.linear_keyword_representation());
    // Assert that std::equal() has defined behavior.
    assert(s.size() == static_cast<unsigned int>(n));
    if((nullptr != c) && !std::equal(c, c + n, s.begin()))
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

static char const* c_default[] = {""};

void check
    (char const*                     file
    ,int                             line
    ,double const*                   d
    ,int                             n
    ,std::string const&              e
    ,std::vector<std::string> const& k = std::vector<std::string>(0)
    ,char const* const*              c = c_default
    ,std::string const&              w = std::string()
    )
{
    InputSequence seq(e, n, 90, 95, 0, 2002, 0, k, w);
    std::vector<double> v(seq.linear_number_representation());
    bool const bv = v == std::vector<double>(d, d + n);
    std::vector<std::string> s(seq.linear_keyword_representation());
    // This "k.empty()" thing needs reconsideration.
    bool const bs = k.empty() || std::equal(c, c + n, s.begin());
    bool const b = bv && bs;
    if(!b) analyze_errors(d, n, e, k, c, w);
    INVOKE_BOOST_TEST(b, file, line);
}

int test_main(int, char*[])
{
    // Arguments to check():
    //   expected results
    //     c: keywords
    //     d: numeric values
    //   ctor arguments
    //     n: length
    //     e: expression
    //     k: extra keywords
    //     w: default keyword

    // An all-blank string is treated as zero.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string e(" ");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Make sure example in comment at top works.
    //   1 3; 7 5;0; --> 1 1 1 7 7 0...
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 7, 7, 0, 0, 0, 0};
    std::string e("1 3; 7 5;0");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Numbers separated by semicolons mean values; the last is
    // replicated to fill the vector.
    {
    int const n = 5;
    double const d[n] = {1, 2, 3, 3, 3};
    std::string e("1; 2; 3");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Number-pairs separated by semicolons mean {value, end-duration}.
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 3; 3 6; 5 9; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // {value, @ attained_age}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 @93; 3 @96; 5 @99; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // {value, # number_of_years_since_last_interval_endpoint}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 #3; 3 #3; 5 #3; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // {value [|( begin-duration, end-duration ]|) }

    // Test [x,y).
    {
    int const n = 9;
    double const d[n] = {1, 1, 3, 3, 3, 5, 7, 7, 7};
    std::string e("1 [0, 2); 3 [2, 5); 5 [5, 6); 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test (x,y].
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 7, 7};
    std::string e("1; 1 (0, 2]; 3 (2, 5]; 5 (5, 6]; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test a mixture of all five ways of specifying duration.
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 1, 2, 3, 4, 5, 5};
    std::string e("1 [0, 4); 2 5; 3 #1; 4 @97; 5");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test intervals of length one.
    {
    int const n = 5;
    double const d[n] = {1, 3, 5, 7, 7};
    std::string e("1 [0, 1); 3 [1, 2); 5 (1, 2]; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test empty intervals.
    {
    int const n = 5;
    double const d[n] = {1, 3, 5, 7, 7};
    std::string e("1 [0, 1); 3 [1, 1]; 5 (1, 2]; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test subtly improper intervals.
    {
    int const n = 5;
    double const d[n] = {0, 0, 0, 0, 0};
    std::string e("1 [0, 0); 3 (1, 2); 5 (2, 2]; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test grossly improper intervals.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string e("1; 9 (2, 0]; 3 [7, 3); 5 (5, 5); 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test intervals with 'holes'. Since the last element is replicated,
    // there can be no 'hole' at the end.
    {
    int const n = 9;
    double const d[n] = {0, 1, 0, 3, 0, 5, 7, 7, 7};
    std::string e("1 [1, 2); 3 [3, 3]; 5 (4, 5]; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test overlapping intervals.
    // TODO ?? Treat these as an error?
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 3, 3, 5, 5, 7, 7};
    std::string e("1; 1 (0, 8]; 3 (2, 7]; 5 (4, 6]; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test intervals with decreasing begin-points.
    // TODO ?? Should this case be allowed?
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string e("5 [5, 6); 3 [2, 5); 1 [0, 2); 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Durations with '@' prefix mean attained age.
    {
    int const n = 10;
    double const d[n] = {0, 12, 0, 27, 0, 1, 7, 7, 7, 7};
    std::string e("12 [1, @92); 27 [@93, @93]; 1 (@94, 5]; 7");
    check(__FILE__, __LINE__, d, n, e);
    }

    // Test floating-point values; we choose values that we know
    // must be exactly representable on a binary machine, so that a
    // simple test for equality suffices.
    {
    int const n = 10;
    double const d[n] = {0, 12.25, 0, 27.875, 0, 1.0625, 7.5, 7.5, 7.5, 7.5};
    std::string e("12.25 [1, @92); 27.875 [@93, @93]; 1.0625 (@94, 5]; 7.5");
    check(__FILE__, __LINE__, d, n, e);
    }

    // {value, @ age} means {value, to-attained-age}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string e("1 @93; 3 @96; 5 @99; 7");
    check(__FILE__, __LINE__, d, n, e);
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
    check(__FILE__, __LINE__, d, n, e, k, c);
    }

    // Test numbers mixed with (enumerative) extra keywords.
    {
    int const n = 9;
    char const* c[n] = {"", "", "keyword_00", "keyword_00", "", "", "", "", ""};
    double const d[n] = {1, 1, 0, 0, 5, 5, 7, 7, 7};
    std::string e("1 [0, 2); keyword_00 [2, 4); 5 [4, 6); 7");
    std::vector<std::string> k;
    k.push_back("keyword_00");
    check(__FILE__, __LINE__, d, n, e, k, c);
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
    check(__FILE__, __LINE__, d, n, e, k, c, w);
    }

// TODO ?? Also test keyword as scalar duration.

    // Duration keywords: {retirement, maturity}
    {
    int const n = 10;
    double const d[n] = {7, 7, 7, 7, 7, 4, 4, 4, 4, 4};
    std::string e("7, retirement; 4");
    check(__FILE__, __LINE__, d, n, e);
    InputSequence seq("7, retirement; 4", 10, 90, 95, 0, 2002, 0);
    std::vector<ValueInterval> const& i(seq.interval_representation());
    BOOST_TEST(e_inception  == i[0].begin_mode);
    BOOST_TEST(e_retirement == i[0].end_mode  );
    BOOST_TEST(e_retirement == i[1].begin_mode);
    BOOST_TEST(e_maturity   == i[1].end_mode  );
    }

// TODO ?? Also test default keyword.

// TODO ?? Also test keywords-only switch.

    return 0;
}

