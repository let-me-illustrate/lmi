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
#include <iterator>                     // std::ostream_iterator

void check
    (char const*                     file
    ,int                             line
    ,int                             n
    ,double const*                   d
    ,std::string const&              e
    ,char const*                     m = ""
    ,std::vector<std::string> const& k = std::vector<std::string>()
    ,std::vector<std::string> const& c = std::vector<std::string>()
    ,bool                            o = false
    ,std::string const&              w = std::string()
    )
{
    try
        {
        InputSequence const seq(e, n, 90, 95, 0, 2002, k, o, w);

        std::vector<double> const& v(seq.linear_number_representation());
        bool const bv = v == std::vector<double>(d, d + n);
        if(!bv)
            {
            using osid = std::ostream_iterator<double>;
            std::cout << "\nExpression: '" << e << "'";
            std::cout << "\n      observed numbers: ";
            std::copy(v.begin(), v.end(), osid(std::cout, " "));
            std::cout << "\n  differ from expected: ";
            std::copy(d        , d + n  , osid(std::cout, " "));
            std::cout << std::endl;
            }

        std::vector<std::string> const& s(seq.linear_keyword_representation());
        std::vector<std::string> const t =
            ( std::vector<std::string>() == c)
            ? std::vector<std::string>(n)
            : c
            ;
        bool const bs = s == t;
        if(!bs)
            {
            using osis = std::ostream_iterator<std::string>;
            std::cout << "\nExpression: '" << e << "'";
            std::cout << "\n     observed keywords: ";
            std::copy(s.begin(), s.end(), osis(std::cout, "|"));
            std::cout << "\n  differ from expected: ";
            std::copy(t.begin(), t.end(), osis(std::cout, "|"));
            std::cout << std::endl;
            }

        INVOKE_BOOST_TEST(bv && bs, file, line);
        }
    catch(std::exception const& x)
        {
        std::string const f = x.what();
        std::string const g = f.substr(0, f.find("\n[file "));
        std::string const h = (nullptr == m) ? std::string() : std::string(m);
        bool const b = g == h;
        if(!b)
            {
            std::cout
                <<   "\nObserved exception:"
                << "\n\n'" << g << "'"
                << "\n\ndiffers from expected:"
                << "\n\n'" << h << "'"
                << std::endl
                ;
            }
        INVOKE_BOOST_TEST(b, file, line);
        }
    catch(...)
        {
        throw std::runtime_error("Unexpected exception");
        }
}

int test_main(int, char*[])
{
    using strvec = std::vector<std::string>;

    // Arguments to check():
    //   expected results
    //     c: keywords
    //     d: numeric values
    //     m: diagnostics
    //   InputSequence ctor arguments
    //     n: length
    //     e: expression
    //     k: allowed keywords
    //     o: keywords only
    //     w: default keyword
    // Tests instantiate local variables with those names as needed,
    // preferably in this order:
    //   n c d e m k o w
    // which is the same as the order in check()'s declaration except
    // that c and d are juxtaposed to facilitate visual comparison.

    // An empty string is treated as zero.
    {
    int const n = 5;
    double const d[n] = {0, 0, 0, 0, 0};
    std::string const e("");
    check(__FILE__, __LINE__, n, d, e);
    }

    // An all-blank string is treated as zero.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string const e(" ");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Make sure example in comment at top works.
    //   1 3; 7 5;0; --> 1 1 1 7 7 0...
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 7, 7, 0, 0, 0, 0};
    std::string const e("1 3; 7 5;0");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Numbers separated by semicolons mean values; the last is
    // replicated to fill the vector.
    {
    int const n = 5;
    double const d[n] = {1, 2, 3, 3, 3};
    std::string const e("1; 2; 3");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Number-pairs separated by semicolons mean {value, end-duration}.
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string const e("1 3; 3 6; 5 9; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // {value, @ attained_age}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string const e("1 @93; 3 @96; 5 @99; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // {value, # number_of_years_since_last_interval_endpoint}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string const e("1 #3; 3 #3; 5 #3; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // {value [|( begin-duration, end-duration ]|) }

    // Test [a,b).
    {
    int const n = 9;
    double const d[n] = {1, 1, 3, 3, 3, 5, 7, 7, 7};
    std::string const e("1 [0, 2); 3 [2, 5); 5 [5, 6); 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test (a,b].
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 7, 7};
    std::string const e("1; 1 (0, 2]; 3 (2, 5]; 5 (5, 6]; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test a mixture of all five ways of specifying duration.
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 1, 2, 3, 4, 5, 5};
    std::string const e("1 [0, 4); 2 5; 3 #1; 4 @97; 5");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test intervals of length one.
    {
    int const n = 5;
    double const d[n] = {1, 3, 5, 7, 7};
    std::string const e("1 [0, 1); 3 [1, 2); 5 (1, 2]; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test empty intervals.
    {
    int const n = 5;
    double const d[n] = {1, 3, 5, 7, 7};
    std::string const e("1 [0, 1); 3 [1, 1]; 5 (1, 2]; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test subtly improper intervals.
    {
    int const n = 5;
    double const d[n] = {0, 0, 0, 0, 0};
    std::string const e("1 [0, 0); 3 (1, 2); 5 (2, 2]; 7");
    char const* m =
        "Interval [ 0, 0 ) is improper: it ends before it begins."
        " Current token ';' at position 9.\n"
        "Interval [ 2, 2 ) is improper: it ends before it begins."
        " Current token ';' at position 19.\n"
        "Interval [ 3, 3 ) is improper: it ends before it begins."
        " Current token ';' at position 29.\n"
        ;
    check(__FILE__, __LINE__, n, d, e, m);
    BOOST_TEST_EQUAL
        ("Interval [ 0, 0 ) is improper: it ends before it begins."
        ,abridge_diagnostics(m)
        );
    }

    // Test grossly improper intervals.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string const e("1; 9 (2, 0]; 3 [7, 3); 5 (5, 5); 7");
    char const* m =
        "Interval [ 3, 1 ) is improper: it ends before it begins."
        " Current token ';' at position 12.\n"
        "Interval [ 7, 3 ) is improper: it ends before it begins."
        " Current token ';' at position 22.\n"
        "Interval [ 6, 5 ) is improper: it ends before it begins."
        " Current token ';' at position 32.\n"
        ;
    check(__FILE__, __LINE__, n, d, e, m);
    BOOST_TEST_EQUAL
        ("Interval [ 3, 1 ) is improper: it ends before it begins."
        ,abridge_diagnostics(m)
        );
    }

    // Test an expression with gaps between intervals. Because the
    // last element is replicated, there can be no gap at the end.
    {
    int const n = 9;
    double const d[n] = {0, 1, 0, 3, 0, 5, 7, 7, 7};
    std::string const e("1 [1, 2); 3 [3, 3]; 5 (4, 5]; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test overlapping intervals.
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 3, 3, 5, 5, 7, 7};
    std::string const e("1; 1 (0, 8]; 3 (2, 7]; 5 (4, 6]; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test intervals with decreasing begin-points.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string const e("5 [5, 6); 3 [2, 5); 1 [0, 2); 7");
    char const* m =
        "Previous interval began at duration 5;"
        " current interval [ 2, 5 ) would begin before that."
        ;
    check(__FILE__, __LINE__, n, d, e, m);
    BOOST_TEST_EQUAL
        (std::string(m)
        ,abridge_diagnostics(m)
        );
    }

    // Durations with '@' prefix mean attained age.
    {
    int const n = 10;
    double const d[n] = {0, 12, 0, 27, 0, 1, 7, 7, 7, 7};
    std::string const e("12 [1, @92); 27 [@93, @93]; 1 (@94, 5]; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

    // Test floating-point values; we choose values that we know
    // must be exactly representable on a binary machine, so that a
    // simple test for equality suffices.
    {
    int const n = 10;
    double const d[n] = {0, 12.25, 0, 27.875, 0, 1.0625, 7.5, 7.5, 7.5, 7.5};
    std::string const e("12.25 [1,@92); 27.875 [@93,@93]; 1.0625 (@94,5]; 7.5");
    check(__FILE__, __LINE__, n, d, e);
    }

    // {value, @ age} means {value, to-attained-age}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string const e("1 @93; 3 @96; 5 @99; 7");
    check(__FILE__, __LINE__, n, d, e);
    }

// SOMEDAY !! Also support and test:
//   additive expressions e.g. retirement-10 ?

    // Test construction from vector.
    // TODO ?? Test against canonical representation once we define that.
    {
    int const n = 5;
    double const d[n] = {1, 1, 1, 2, 2};
    std::vector<double> const v(d, d + n);
    BOOST_TEST(v == InputSequence(v).linear_number_representation());
    BOOST_TEST_EQUAL
        ("1 [0, 3); 2 [3, maturity)"
        ,InputSequence(v).mathematical_representation()
        );
    }

    // Test construction from one-element vector.
    {
    int const n = 1;
    double const d[n] = {3};
    std::vector<double> const v(d, d + n);
    BOOST_TEST(v == InputSequence(v).linear_number_representation());
    BOOST_TEST_EQUAL("3", InputSequence(v).mathematical_representation());
    }

    // Test construction from empty vector.
    {
    std::vector<double> const v;
    BOOST_TEST(v == InputSequence(v).linear_number_representation());
    BOOST_TEST_EQUAL("0", InputSequence(v).mathematical_representation());
    }

    // Test (enumerative) allowed keywords, and keywords-only switch
    // (with input it allows).
    {
    int const n = 9;
    strvec const c      {"p", "p", "rrr", "rrr", "q", "q", "q", "q", "q"};
    double const d[n] = { 0 ,  0 ,   0  ,   0  ,  0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("p[0, 2); rrr [2, 4);q[4, 6);");
    strvec const k{"not_used", "p", "q", "r", "rr", "rrr"};
    check(__FILE__, __LINE__, n, d, e, "", k, c);
    // Toggle keywords-only switch on: same result.
    bool const o = true;
    check(__FILE__, __LINE__, n, d, e, "", k, c, o);
    // Toggle keywords-only switch explicitly off: same result.
    check(__FILE__, __LINE__, n, d, e, "", k, c, false);
    }

    // Test numbers mixed with (enumerative) allowed keywords.
    {
    int const n = 9;
    strvec const c     {"", "", "keyword_00", "keyword_00", "", "", "", "", ""};
    double const d[n] ={ 1,  1,       0     ,       0     ,  5,  5,  7,  7,  7};
    std::string const e("1 [0, 2); keyword_00 [2, 4); 5 [4, 6); 7");
    strvec const k{"keyword_00"};
    check(__FILE__, __LINE__, n, d, e, "", k, c);
    }

    // Test numbers mixed with (enumerative) allowed keywords, with
    // a default keyword. Because numbers are allowed, gaps are
    // filled with a default number (such as zero), so a default
    // keyword cannot be necessary and is therefore forbidden.
    {
    int const n = 10;
    strvec const c      {"q", "q", "z", "p", "z", "z", "p", "z", "z", "z"};
    double const d[n] = { 0 ,  0 ,  0 ,  0 ,  5 ,  5 ,  0 ,  7 ,  7 ,  7 };
    std::string const e("q [0, 2); p [3, 4); 5 [4, 6); p; 7");
    char const* m =
        "Assertion 'a_default_keyword.empty() ||"
        " a_keywords_only && contains(a_allowed_keywords, a_default_keyword)'"
        " failed."
        ;
    strvec const k{"p", "q", "z"};
    std::string w("z");
    check(__FILE__, __LINE__, n, d, e, m, k, c, false, w);
    BOOST_TEST_EQUAL
        (std::string(m)
        ,abridge_diagnostics(m)
        );
    }

    // Test keywords-only switch with input it forbids.
    {
    int const n = 10;
    strvec const c      {"z", "z", "z", "z", "z", "z", "z", "z", "z", "z"};
    double const d[n] = { 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("q [0, 2); p [3, 4); 5 [4, 6); p; 7");
    char const* m =
        "Expected keyword chosen from { p q z }."
        " Current token 'number' at position 21.\n"
        "Expected ';'."
        " Current token '[' at position 23.\n"
        ;
    strvec const k{"p", "q", "z"};
    bool const o = true;
    std::string w("z");
    check(__FILE__, __LINE__, n, d, e, m, k, c, o, w);
    BOOST_TEST_EQUAL
        ("Expected keyword chosen from { p q z }."
        ,abridge_diagnostics(m)
        );
    }

    // Test an expression with a gap between intervals, with the
    // keywords-only switch and a default keyword to fill the gap.
    {
    int const n = 5;
    strvec const c      {"q", "q", "z", "z", "p"};
    double const d[n] = { 0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("q [0, 2); p [4, maturity)");
    strvec const k{"p", "q", "z"};
    bool const o = true;
    std::string w("z");
    check(__FILE__, __LINE__, n, d, e, "", k, c, o, w);
    }

    // Test a default keyword that is not an element of the set of
    // allowed keywords. Even if this were not forbidden, the values
    // in 'c' below could not be realized from an expression 'e' that
    // specifies a value for each year: "q;q;u;u;p" would be rejected
    // because 'u' is not an element of {p, q, z}.
    {
    int const n = 5;
    strvec const c      {"q", "q", "u", "u", "p"};
    double const d[n] = { 0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("q [0, 2); p [4, maturity)");
    char const* m =
        "Assertion 'a_default_keyword.empty() ||"
        " a_keywords_only && contains(a_allowed_keywords, a_default_keyword)'"
        " failed."
        ;
    strvec const k{"p", "q", "z"};
    bool const o = true;
    std::string w("u");
    check(__FILE__, __LINE__, n, d, e, m, k, c, o, w);
    BOOST_TEST_EQUAL
        (std::string(m)
        ,abridge_diagnostics(m)
        );
    }

    // Duration keywords: {retirement, maturity}
    {
    int const n = 10;
    double const d[n] = {7, 7, 7, 7, 7, 4, 4, 4, 4, 4};
    std::string const e("7, retirement; 4, maturity");
    check(__FILE__, __LINE__, n, d, e);
    InputSequence const seq(e, 10, 90, 95, 0, 2002);
    std::vector<ValueInterval> const& i(seq.interval_representation());
    BOOST_TEST_EQUAL(e_inception , i[0].begin_mode);
    BOOST_TEST_EQUAL(e_retirement, i[0].end_mode  );
    BOOST_TEST_EQUAL(e_retirement, i[1].begin_mode);
    BOOST_TEST_EQUAL(e_maturity  , i[1].end_mode  );
    }

    // Test a simple parser error.
    {
    int const n = 2;
    double const d[n] = {0, 0};
    std::string const e("[0, 1)");
    char const* m =
        "Expected number or keyword."
        " Current token '[' at position 1.\n"
        ;
    check(__FILE__, __LINE__, n, d, e, m);
    BOOST_TEST_EQUAL
        ("Expected number or keyword."
        ,abridge_diagnostics(m)
        );
    }

    return 0;
}

