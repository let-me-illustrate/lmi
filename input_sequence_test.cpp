// Input sequences (e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0...): unit test
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "input_sequence.hpp"

#include "test_tools.hpp"
#include "unwind.hpp"                   // scoped_unwind_toggler

#include <algorithm>
#include <iterator>                     // ostream_iterator

class input_sequence_test
{
  public:
    static void test();

  private:
    static void check
        (char const*                     file
        ,int                             line
        ,int                             n
        ,double const*                   d
        ,std::string const&              e
        ,std::string const&              g
        ,char const*                     m
        ,std::vector<std::string> const& k
        ,std::vector<std::string> const& c
        ,bool                            o
        ,std::string const&              w
        );
};

void input_sequence_test::check
    (char const*                     file
    ,int                             line
    ,int                             n
    ,double const*                   d
    ,std::string const&              e
    ,std::string const&              g
    ,char const*                     m = ""
    ,std::vector<std::string> const& k = std::vector<std::string>()
    ,std::vector<std::string> const& c = std::vector<std::string>()
    ,bool                            o = false
    ,std::string const&              w = std::string()
    )
{
    try
        {
        scoped_unwind_toggler meaningless_name;
        InputSequence const seq(e, n, 90, 95, 0, 2002, k, o, w);

        std::vector<double> const& v(seq.seriatim_numbers());
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

        std::vector<std::string> const& s(seq.seriatim_keywords());
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

        std::string const& f = seq.canonical_form();
        bool const bf = f == g;
        if(!bf)
            {
            std::cout << "\nExpression:                '" << e << "'";
            std::cout << "\n  observed representation: '" << f << "'";
            std::cout << "\n  differs from expected:   '" << g << "'";
            std::cout << std::endl;
            }

        InputSequence const idempotence_test(f, n, 90, 95, 0, 2002, k, o, w);
        std::string const& h = idempotence_test.canonical_form();
        bool const bh = h == f;
        if(!bh)
            {
            std::cout << "\nExpression:          '" << e << "'";
            std::cout << "\n  c14n(c14n):        '" << h << "'";
            std::cout << "\n  differs from c14n: '" << f << "'";
            std::cout << std::endl;
            }

        INVOKE_LMI_TEST(bv && bs && bf && bh, file, line);
        }
    catch(std::exception const& x)
        {
        std::string const y = x.what();
        std::string const i = y.substr(0, y.find("\n["));
        std::string const j = (nullptr == m) ? std::string() : std::string(m);
        bool const b = i == j;
        if(!b)
            {
            std::cout
                <<   "\nObserved exception:"
                << "\n\n'" << i << "'"
                << "\n\ndiffers from expected:"
                << "\n\n'" << j << "'"
                << std::endl
                ;
            }
        INVOKE_LMI_TEST(b, file, line);
        }
    catch(...)
        {
        throw std::runtime_error("Unexpected exception");
        }
}

void input_sequence_test::test()
{
    std::string census = "\nCorporationPayment\tCorporationPaymentMode\tComments\n";

    using strvec = std::vector<std::string>;

    // Arguments to check():
    //   expected results
    //     c: keywords
    //     d: numeric values
    //     g: representation
    //     m: diagnostics
    //   InputSequence ctor arguments
    //     n: length
    //     e: expression
    //     k: allowed keywords
    //     o: keywords only
    //     w: default keyword
    // Tests instantiate local variables with those names as needed,
    // preferably in this order:
    //   n c d e g m k o w
    // which is the same as the order in check()'s declaration except
    // that c and d are juxtaposed to facilitate visual comparison.
    //
    // Each assignment to 'census' is followed by a comment showing
    // how the census manager canonicalized the sequence as of
    // 20170224T1200Z, determined by editing the sequence using
    // InputSequenceEntry and copying and pasting the result.

    // An empty string is treated as zero.
    {
    int const n = 5;
    double const d[n] = {0, 0, 0, 0, 0};
    std::string const e("");
    census += e + "\t\tcorp pmt empty\t\n";
    std::string const g("0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // An all-blank string is treated as zero.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string const e(" ");
    census += e + "\t\tcorp pmt blank\t\n";
    std::string const g("0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Make sure example in comment at top works.
    //   1 3; 7 5;0; --> 1 1 1 7 7 0...
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 7, 7, 0, 0, 0, 0};
    std::string const e("1 3; 7 5;0");
    census += e + "\t\t\t\n";
    std::string const g("1 3; 7 5; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Numbers separated by semicolons mean values; the last is
    // replicated to fill the vector.
    {
    int const n = 5;
    double const d[n] = {1, 2, 3, 3, 3};
    std::string const e("1; 2; 3");
    census += e + "\t\t\t\n";
    std::string const g("1; 2; 3");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Number-pairs separated by semicolons mean {value, end-duration}.
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string const e("1 3; 3 6; 5 9; 7");
    census += e + "\t\t\t\n";
    std::string const g("1 3; 3 6; 5 9; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // {value, @ to-attained-age}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string const e("1 @93; 3 @96; 5 @99; 7");
    census += e + "\t\t\t\n";
    std::string const g("1 @93; 3 @96; 5 @99; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // {value, # number_of_years_since_last_interval_endpoint}
    {
    int const n = 10;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 7};
    std::string const e("1 #3; 3 #3; 5 #3; 7");
    census += e + "\t\t\t\n";
    std::string const g("1 #3; 3 #3; 5 #3; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // {value [|( begin-duration, end-duration ]|) }

    // Test [a,b).
    {
    int const n = 9;
    double const d[n] = {1, 1, 3, 3, 3, 5, 7, 7, 7};
    std::string const e("1 [0, 2); 3 [2, 5); 5 [5, 6); 7");
    census += e + "\t\t\t\n";
    std::string const g("1 2; 3 5; 5; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Test (a,b].
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 3, 3, 3, 5, 7, 7};
    std::string const e("1; 1 (0, 2]; 3 (2, 5]; 5 (5, 6]; 7");
    census += e + "\t\t\t\n";
    // Should the first two intervals be combined?
    std::string const g("1; 1 3; 3 6; 5; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Test a mixture of all five ways of specifying duration.
    {
    int const n = 9;
    double const d[n] = {1, 1, 1, 1, 2, 3, 4, 5, 5};
    std::string const e("1 [0, 4); 2 5; 3 #1; 4 @97; 5");
    census += e + "\t\t\t\n";
    std::string const g("1 4; 2; 3 #1; 4 @97; 5");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Test intervals of length one.
    {
    int const n = 5;
    double const d[n] = {1, 3, 5, 7, 7};
    std::string const e("1 [0, 1); 3 [1, 2); 5 (1, 2]; 7");
    census += e + "\t\t\t\n";
    std::string const g("1; 3; 5; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Test subtly improper (empty) intervals.
    //
    // Mathematical convention might regard these intervals:
    //   (x,x); (x,x]; [x,x); and [y,x], x<y
    // as proper but empty, yet that's merely a convention.
    {
    int const n = 5;
    double const d[n] = {0, 0, 0, 0, 0};
    std::string const e("1 [0, 0); 3 (1, 2); 5 (2, 2]; 7");
    // census: invalid expression cannot be pasted into GUI
    std::string const g(""); // Expression is invalid.
    char const* m =
        "Interval [ 0, 0 ) is improper: it ends before it begins."
        " Current token ';' at position 9.\n"
        "Interval [ 2, 2 ) is improper: it ends before it begins."
        " Current token ';' at position 19.\n"
        "Interval [ 3, 3 ) is improper: it ends before it begins."
        " Current token ';' at position 29.\n"
        ;
    check(__FILE__, __LINE__, n, d, e, g, m);
    LMI_TEST_EQUAL
        ("Interval [ 0, 0 ) is improper: it ends before it begins."
        ,abridge_diagnostics(m)
        );
    }

    // Test grossly improper intervals.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string const e("1; 9 (2, 0]; 3 [7, 3); 5 (5, 5); 7");
    // census: invalid expression cannot be pasted into GUI
    std::string const g(""); // Expression is invalid.
    char const* m =
        "Interval [ 3, 1 ) is improper: it ends before it begins."
        " Current token ';' at position 12.\n"
        "Interval [ 7, 3 ) is improper: it ends before it begins."
        " Current token ';' at position 22.\n"
        "Interval [ 6, 5 ) is improper: it ends before it begins."
        " Current token ';' at position 32.\n"
        ;
    check(__FILE__, __LINE__, n, d, e, g, m);
    LMI_TEST_EQUAL
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
    census += e + "\t\t\t\n";
    std::string const g("0; 1; 0; 3; 0; 5; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Test overlapping intervals.
    {
    int const n = 9;
    double const d[n] = {0, 1, 1, 3, 3, 5, 5, 7, 7};
    std::string const e("0; 1 (0, 8]; 3 (2, 7]; 5 (4, 6]; 7");
    // census: invalid expression cannot be pasted into GUI
    std::string const g("0; 1 9; 3 8; 5 7; 7");
    char const* m =
        "Interval [ 9, 3 ) is improper: it ends before it begins."
        ;
    check(__FILE__, __LINE__, n, d, e, g, m);
    }

    // Test intervals with decreasing begin-points.
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string const e("5 [5, 6); 3 [2, 5); 1 [0, 2); 7");
    // census: invalid expression cannot be pasted into GUI
    std::string const g(""); // Expression is invalid.
    char const* m =
        "Previous interval began at duration 5;"
        " current interval [ 2, 5 ) would begin before that."
        ;
    check(__FILE__, __LINE__, n, d, e, g, m);
    LMI_TEST_EQUAL
        (std::string(m)
        ,abridge_diagnostics(m)
        );
    }

    // Durations with '@' prefix mean attained age.
    {
    int const n = 10;
    double const d[n] = {0, 12, 0, 27, 0, 1, 7, 7, 7, 7};
    std::string const e("12 [1, @92); 27 [@93, @93]; 1 (@94, #1]; 7");
    census += e + "\t\t\t\n";
    std::string const g("0; 12 @92; 0 @93; 27 @94; 0 @95; 1 #1; 7");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Test floating-point values; we choose values that we know
    // must be exactly representable on a binary machine, so that a
    // simple test for equality suffices.
    {
    int const n = 10;
    double const d[n] = {0, 12.25, 0, 27.875, 0, 1.0625, 7.5, 7.5, 7.5, 7.5};
    std::string const e("12.25 [1,@92); 27.875 [@93,@93]; 1.0625(@94,#1]; 7.5");
    census += e + "\t\t\t\n";
    std::string const g("0; 12.25 @92; 0 @93; 27.875 @94; 0 @95; 1.0625 #1; 7.5");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // Test construction from numeric vector.
    {
    std::vector<double> const v{1, 1, 1, 2, 2};
    InputSequence const seq(v);
    LMI_TEST(v == seq.seriatim_numbers());
    LMI_TEST_EQUAL("1 3; 2", canonicalized_input_sequence(v));
    }

    // Test construction from string (keyword) vector.
    {
    std::vector<std::string> const v{"alpha", "beta", "beta", "gamma", "eta"};
    InputSequence const seq(v);
    LMI_TEST(v == seq.seriatim_keywords());
    LMI_TEST_EQUAL
        ("alpha; beta 3; gamma; eta"
        ,canonicalized_input_sequence(v)
        );
    }

    // Test construction from one-element vector.
    {
    std::vector<double> const v{3};
    InputSequence const seq(v);
    LMI_TEST(v == seq.seriatim_numbers());
    LMI_TEST_EQUAL("3", canonicalized_input_sequence(v));
    }

    // Test construction from empty vector.
    {
    std::vector<double> const v;
    InputSequence const seq(v);
    LMI_TEST(v == seq.seriatim_numbers());
    LMI_TEST_EQUAL("0", canonicalized_input_sequence(v));
    }

    // Test (enumerative) allowed keywords, and keywords-only switch
    // (with input it allows).
    {
    int const n = 9;
    strvec const c      {"p", "p", "rrr", "rrr", "q", "q", "q", "q", "q"};
    double const d[n] = { 0 ,  0 ,   0  ,   0  ,  0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("p[0, 2); rrr [2, 4);q[4, 6);");
    census += "glp[0, 2); target [2, 4);gsp[4, 6);\t\t\t\n";
    std::string const g("p 2; rrr 4; q");
    strvec const k{"not_used", "p", "q", "r", "rr", "rrr"};
    check(__FILE__, __LINE__, n, d, e, g, "", k, c);
    // Toggle keywords-only switch on: same result.
    census += "\tannual[0, 2); quarterly [2, 4);monthly[4, 6);\t\t\n";
    bool const o = true;
    check(__FILE__, __LINE__, n, d, e, g, "", k, c, o);
    // Toggle keywords-only switch explicitly off: same result.
    check(__FILE__, __LINE__, n, d, e, g, "", k, c, false);
    }

    // Test numbers mixed with (enumerative) allowed keywords.
    {
    int const n = 9;
    strvec const c     {"", "", "keyword_00", "keyword_00", "", "", "", "", ""};
    double const d[n] ={ 1,  1,       0     ,       0     ,  5,  5,  7,  7,  7};
    std::string const e("1 [0, 2); keyword_00 [2, 4); 5 [4, 6); 7");
    census += "1 [0, 2); corridor [2, 4); 5 [4, 6); 7\t\t\t\n";
    std::string const g("1 2; keyword_00 4; 5 6; 7");
    strvec const k{"keyword_00"};
    check(__FILE__, __LINE__, n, d, e, g, "", k, c);
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
    // census: invalid expression cannot be pasted into GUI
    std::string const g(""); // Expression is invalid.
    char const* m =
        "Assertion 'a_default_keyword.empty() ||"
        " a_keywords_only && contains(a_allowed_keywords, a_default_keyword)'"
        " failed."
        ;
    strvec const k{"p", "q", "z"};
    std::string w("z");
    check(__FILE__, __LINE__, n, d, e, g, m, k, c, false, w);
    LMI_TEST_EQUAL
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
    // census: invalid expression cannot be pasted into GUI
    std::string const g(""); // Expression is invalid.
    char const* m =
        "Expected keyword chosen from { p q z }."
        " Current token 'number' at position 21.\n"
        "Expected ';'."
        " Current token '[' at position 23.\n"
        ;
    strvec const k{"p", "q", "z"};
    bool const o = true;
    std::string w("z");
    check(__FILE__, __LINE__, n, d, e, g, m, k, c, o, w);
    LMI_TEST_EQUAL
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
    census += "\tquarterly [0, 2); monthly [4, maturity)\t\t\n";
    std::string const g("q 2; z 4; p");
    strvec const k{"p", "q", "z"};
    bool const o = true;
    std::string w("z");
    check(__FILE__, __LINE__, n, d, e, g, "", k, c, o, w);
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
    // census: invalid expression cannot be pasted into GUI
    std::string const g(""); // Expression is invalid.
    char const* m =
        "Assertion 'a_default_keyword.empty() ||"
        " a_keywords_only && contains(a_allowed_keywords, a_default_keyword)'"
        " failed."
        ;
    strvec const k{"p", "q", "z"};
    bool const o = true;
    std::string w("u");
    check(__FILE__, __LINE__, n, d, e, g, m, k, c, o, w);
    LMI_TEST_EQUAL
        (std::string(m)
        ,abridge_diagnostics(m)
        );
    }

    // Test an expression with a gap before the first interval,
    // with the keywords-only switch (and a default keyword).
    {
    int const n = 5;
    strvec const c      {"z", "q", "q", "p", "p"};
    double const d[n] = { 0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("q [1, 3); p [3, maturity)");
    census += "\tquarterly [1, 3); monthly [3, maturity)\t\t\n";
    std::string const g("z; q 3; p");
    strvec const k{"p", "q", "z"};
    bool const o = true;
    std::string w("z");
    check(__FILE__, __LINE__, n, d, e, g, "", k, c, o, w);
    }

    // Test an expression with a gap before the first interval,
    // without the keywords-only switch. This is otherwise the same
    // as the preceding case; results differ in that the default value
    // is numeric and the keyword gap is filled with an empty string.
    // (Should it be empty?)
    {
    int const n = 5;
    strvec const c      {"", "q", "q", "p", "p"};
    double const d[n] = { 0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("q [1, 3); p [3, maturity)");
    census += "sevenpay [1, 3); glp [3, maturity)\t\t\t\n";
    std::string const g("0; q 3; p");
    strvec const k{"p", "q", "z"};
    check(__FILE__, __LINE__, n, d, e, g, "", k, c);
    }

    // Duration keywords: {retirement, maturity}
    {
    int const n = 10;
    double const d[n] = {7, 7, 7, 7, 7, 4, 4, 4, 4, 4};
    std::string const e("7, retirement; 4, maturity");
    census += e + "\t\t\t\n";
    std::string const g("7 retirement; 4");
    check(__FILE__, __LINE__, n, d, e, g);
    InputSequence const seq(e, 10, 90, 95, 0, 2002);
    std::vector<ValueInterval> const& i(seq.intervals());
    LMI_TEST_EQUAL(e_inception , i[0].begin_mode);
    LMI_TEST_EQUAL(e_retirement, i[0].end_mode  );
    LMI_TEST_EQUAL(e_retirement, i[1].begin_mode);
    LMI_TEST_EQUAL(e_maturity  , i[1].end_mode  );
    }

    // Duration keywords, with a one-time-only event at retirement.
    {
    int const n = 10;
    double const d[n] = {0, 0, 0, 0, 0, 1000, 0, 0, 0, 0};
    std::string const e("0 retirement; 1000; 0 maturity");
    census += e + "\t\t\t\n";
    std::string const g("0 retirement; 1000; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    InputSequence const seq(e, 10, 90, 95, 0, 2002);
    std::vector<ValueInterval> const& i(seq.intervals());
    LMI_TEST_EQUAL(e_inception , i[0].begin_mode);
    LMI_TEST_EQUAL(e_retirement, i[0].end_mode  );
    LMI_TEST_EQUAL(e_retirement, i[1].begin_mode);
    LMI_TEST_EQUAL(e_duration  , i[1].end_mode  );
    LMI_TEST_EQUAL(e_retirement, i[2].begin_mode);
    LMI_TEST_EQUAL(e_maturity  , i[2].end_mode  );
    }

    // Duration keywords, with a one-year-long event at retirement.
    // This differs from the immediately preceding example. If it
    // were canonicalized the same way, the difference between the
    // two would be nullified.
    //
    // One could argue that an interval of implicit length one ought
    // to be considered as
    //   begin-point, #1
    // instead of
    //   begin-point, 1+begin-point
    // but that's six of one vs. half a dozen of the other.
    {
    int const n = 10;
    double const d[n] = {0, 0, 0, 0, 0, 1000, 0, 0, 0, 0};
    std::string const e("0 retirement; 1000 #1; 0 maturity");
    census += e + "\t\t\t\n";
    std::string const g("0 retirement; 1000 #1; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    InputSequence const seq(e, 10, 90, 95, 0, 2002);
    std::vector<ValueInterval> const& i(seq.intervals());
    LMI_TEST_EQUAL(e_inception      , i[0].begin_mode);
    LMI_TEST_EQUAL(e_retirement     , i[0].end_mode  );
    LMI_TEST_EQUAL(e_retirement     , i[1].begin_mode);
    LMI_TEST_EQUAL(e_number_of_years, i[1].end_mode  );
    LMI_TEST_EQUAL(e_number_of_years, i[2].begin_mode);
    LMI_TEST_EQUAL(e_maturity       , i[2].end_mode  );
    }

    // Test a simple parser error.
    {
    int const n = 2;
    double const d[n] = {0, 0};
    std::string const e("[0, 1)");
    // census: invalid expression cannot be pasted into GUI
    std::string const g(""); // Expression is invalid.
    char const* m =
        "Expected number or keyword."
        " Current token '[' at position 1.\n"
        ;
    check(__FILE__, __LINE__, n, d, e, g, m);
    LMI_TEST_EQUAL
        ("Expected number or keyword."
        ,abridge_diagnostics(m)
        );
    }

    // Test all examples in the user manual:
    //   https://www.nongnu.org/lmi/sequence_input.html
    // Each example is quoted unmodified as a comment before its test.
    // Some examples have been superficially changed to work with
    // check(), e.g. because check() uses issue age 90. Others scale
    // numbers or use shorter keywords for brevity.

    // sevenpay 7; 250000 retirement; 100000 #10; 75000 @95; 50000
    {
    int const n = 9;
    strvec const c     {"s", "s", "s",  "",  "",  "",  "",  "",  ""};
    double const d[n] ={ 0 ,  0 ,  0 , 250, 250, 100,  75,  75,  50};
    std::string const e("s 3; 250 retirement; 100 #1; 75 @98; 50");
    census += "sevenpay 3; 250 retirement; 100 #1; 75 @98; 50\t\t\t\n";
    std::string const g("s 3; 250 retirement; 100 #1; 75 @98; 50");
    strvec const k{"s", "_"};
    check(__FILE__, __LINE__, n, d, e, g, "", k, c);
    }

    // 100000; 110000; 120000; 130000; 140000; 150000
    {
    int const n = 9;
    double const d[n] = {10, 11, 12, 13, 14, 15, 15, 15, 15};
    std::string const e("10; 11; 12; 13; 14; 15");
    census += e + "\t\t\t\n";
    std::string const g("10; 11; 12; 13; 14; 15");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // annual; monthly
    {
    int const n = 5;
    strvec const c      {"a", "m", "m", "m", "m"};
    double const d[n] = { 0 ,  0 ,  0 ,  0 ,  0 };
    std::string const e("a; m");
    census += "\tannual; monthly\t\t\n";
    std::string const g("a; m");
    strvec const k{"a", "m"};
    bool const o = true;
    std::string w("a");
    check(__FILE__, __LINE__, n, d, e, g, "", k, c, o, w);
    }

    // 10000 20; 0
    {
    int const n = 9;
    double const d[n] = {10000, 10000, 10000, 10000, 0, 0, 0, 0, 0};
    std::string const e("10000 4; 0");
    census += e + "\t\t\t\n";
    std::string const g("10000 4; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // 10000 10; 5000 15; 0
    {
    int const n = 9;
    double const d[n] = {10000, 10000, 10000, 5000, 5000, 0, 0, 0, 0};
    std::string const e("10000 3; 5000 5; 0");
    census += e + "\t\t\t\n";
    std::string const g("10000 3; 5000 5; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // 10000 @70; 0
    {
    int const n = 9;
    double const d[n] = {10000, 10000, 10000, 0, 0, 0, 0, 0, 0};
    std::string const e("10000 @93; 0");
    census += e + "\t\t\t\n";
    std::string const g("10000 @93; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // 10000 retirement; 0
    {
    int const n = 9;
    double const d[n] = {10000, 10000, 10000, 10000, 10000, 0, 0, 0, 0};
    std::string const e("10000 retirement; 0");
    census += e + "\t\t\t\n";
    std::string const g("10000 retirement; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // 0 retirement; 5000
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 5000, 5000, 5000, 5000};
    std::string const e("0 retirement; 5000");
    census += e + "\t\t\t\n";
    std::string const g("0 retirement; 5000");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // 0 retirement; 5000 maturity
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 5000, 5000, 5000, 5000};
    std::string const e("0 retirement; 5000 maturity");
    census += e + "\t\t\t\n";
    std::string const g("0 retirement; 5000");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // 0 retirement; 5000 #10; 0
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 5000, 5000, 0, 0};
    std::string const e("0 retirement; 5000 #2; 0");
    census += e + "\t\t\t\n";
    std::string const g("0 retirement; 5000 #2; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

    // 0,[0,retirement);10000,[retirement,#10);0
    {
    int const n = 9;
    double const d[n] = {0, 0, 0, 0, 0, 10, 10, 10, 0};
    std::string const e("0,[0,retirement);10,[retirement,#3);0");
    census += e + "\t\t\t\n";
    std::string const g("0 retirement; 10 #3; 0");
    check(__FILE__, __LINE__, n, d, e, g);
    }

#if defined SHOW_CENSUS_PASTE_TEST_CASES
    std::cout
        << "\nPaste into a census to test similar expressions in the GUI:\n\n"
        << "---------8<--------8<--------8<--------"
        << "8<--------8<--------8<--------8<-------"
        << census
        << "--------->8-------->8-------->8--------"
        << ">8-------->8-------->8-------->8-------"
        << std::endl
        ;
#endif // defined SHOW_CENSUS_PASTE_TEST_CASES
}

int test_main(int, char*[])
{
    input_sequence_test::test();

    return EXIT_SUCCESS;
}
