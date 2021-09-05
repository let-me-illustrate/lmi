// Regular expressions--unit test.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "contains.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#if defined LMI_HAS_PCRE
#include "pcre_regex.hpp"
#endif // defined LMI_HAS_PCRE

#include <regex>
#include <sstream>
#include <string>
#include <vector>

char const*const psalm37 = "\
Fret not thyself because of evil doers, neither be thou envious against the workers of iniquity.\n\
For they shall soon be cut down like the grass, and wither as the green herb.\n\
Trust in the LORD, and do good; [so] shalt thou dwell in the land, and verily thou shalt be fed.\n\
Delight thyself also in the LORD; and he shall give thee the desires of thine heart.\n\
Commit thy way unto the LORD; trust also in him; and he shall bring [it] to pass.\n\
And he shall bring forth thy righteousness as the light, and thy judgment as the noonday.\n\
Rest in the LORD, and wait patiently for him: fret not thyself because of him who prospereth in his way, because of the man who bringeth wicked devices to pass.\n\
Cease from anger, and forsake wrath: fret not thyself in any wise to do evil.\n\
For evil doers shall be cut off: but those that wait upon the LORD, they shall inherit the earth.\n\
For yet a little while, and the wicked [shall] not [be]: yea, thou shalt diligently consider his place, and it [shall] not [be].\n\
But the meek shall inherit the earth; and shall delight themselves in the abundance of peace.\n\
The wicked plotteth against the just, and gnasheth upon him with his teeth.\n\
The Lord shall laugh at him: for he seeth that his day is coming.\n\
The wicked have drawn out the sword, and have bent their bow, to cast down the poor and needy, [and] to slay such as be of upright conversation.\n\
Their sword shall enter into their own heart, and their bows shall be broken.\n\
A little that a righteous man hath [is] better than the riches of many wicked.\n\
For the arms of the wicked shall be broken: but the LORD upholdeth the righteous.\n\
The LORD knoweth the days of the upright: and their inheritance shall be for ever.\n\
They shall not be ashamed in the evil time: and in the days of famine they shall be satisfied.\n\
But the wicked shall perish, and the enemies of the LORD [shall be] as the fat of lambs: they shall consume; into smoke shall they consume away.\n\
The wicked borroweth, and payeth not again: but the righteous sheweth mercy, and giveth.\n\
For [such as be] blessed of him shall inherit the earth; and [they that be] cursed of him shall be cut off.\n\
The steps of a [good] man are ordered by the LORD: and he delighteth in his way.\n\
Though he fall, he shall not be utterly cast down: for the LORD upholdeth [him with] his hand.\n\
I have been young, and [now] am old; yet have I not seen the righteous forsaken, nor his seed begging bread.\n\
[He is] ever merciful, and lendeth; and his seed [is] blessed.\n\
Depart from evil, and do good; and dwell for evermore.\n\
For the LORD loveth judgment, and forsaketh not his saints; they are preserved for ever: but the seed of the wicked shall be cut off.\n\
The righteous shall inherit the land, and dwell therein for ever.\n\
The mouth of the righteous speaketh wisdom, and his tongue talketh of judgment.\n\
The law of his God [is] in his heart; none of his steps shall slide.\n\
The wicked watcheth the righteous, and seeketh to slay him.\n\
The LORD will not leave him in his hand, nor condemn him when he is judged.\n\
Wait on the LORD, and keep his way, and he shall exalt thee to inherit the land: when the wicked are cut off, thou shalt see [it].\n\
I have seen the wicked in great power, and spreading himself like a green bay tree.\n\
Yet he passed away, and, lo, he [was] not: yea, I sought him, but he could not be found.\n\
Mark the perfect [man], and behold the upright: for the end of [that] man [is] peace.\n\
But the transgressors shall be destroyed together: the end of the wicked shall be cut off.\n\
But the salvation of the righteous [is] of the LORD: [he is] their strength in the time of trouble.\n\
And the LORD shall help them, and deliver them: he shall deliver them from the wicked, and save them, because they trust in him.\n\
";

std::string const original(psalm37);
std::string text;
std::vector<std::string> lines;

/// Turn a '\n'-delimited string into a vector of strings.
///
/// Other algorithms, e.g.
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/0e113b40360d8e5d
/// are faster, but not by much; and this one is clearer.

std::vector<std::string> vectorize(std::string const& s)
{
    std::vector<std::string> v;
    std::istringstream iss(s);
    std::string line;
    while(std::getline(iss, line))
        {
        v.push_back(line);
        }
    return v;
}

/// Match a regex degenerately--for performance comparison only.

bool contains_regex0(std::string const& regex)
{
    return contains(text, regex);
}

/// Match a regex line by line.
///
/// Perl 5 has 'm' and 's' modifiers that affect how
/// {caret, dollar, dot} match newlines:
///
///   modifiers    ^ and $ treat searched text as:   '.' matches '\n'?
///   ---------   --------------------------------   -----------------
///    [none]     one logical (= physical) line            no
///       s       one logical (= physical) line           yes
///       m       logical lines delimited by '\n'          no
///      ms       logical lines delimited by '\n'         yes
///
/// While C++17 has the equivalent of the 'm' metacharacter in the form of
/// std::regex::multiline flag, the behaviour of the '.' character is fixed
/// and can only be changed by switching to a non-default regex syntax.
/// The tests here compare performance of matching using different syntaxes.

bool contains_regex1(std::string const& regex)
{
    std::regex const r(regex, std::regex::basic | std::regex::optimize);
    for(auto const& i : lines)
        {
        if(std::regex_search(i, r))
            {
            return true;
            }
        }
    return false;
}

/// Match a regex as with Perl's '-s', i.e. dot does not match new line.

bool contains_regex2(std::string const& regex)
{
    // This is the behaviour of dot metacharacter in the default ECMAScript
    // syntax.
    return std::regex_search(text, std::regex(regex, std::regex::optimize));
}

/// Match a regex as with Perl's 's', i.e. dot matches new line.

bool contains_regex3(std::string const& regex)
{
    // To make dot match the new line character, one of POSIX regex syntaxes
    // must be used, arbitrarily choose the basic one as the exact syntax
    // doesn't matter for the regular expressions used in this test.
    return std::regex_search(text, std::regex(regex, std::regex::basic | std::regex::optimize));
}

#if defined LMI_HAS_PCRE

/// Match a regex using PCRE.

bool contains_regex4(std::string const& regex)
{
    // Note that pcre::regex currently always uses "DOT ALL" flag,
    // corresponding to Perl's '-s' behavior.
    return pcre::search(text, pcre::regex(regex)) ? true : false;
}

#endif // defined LMI_HAS_PCRE

void mete_vectorize()
{
    lines = vectorize(text);
}

template<int Function, char* Regex>
void mete()
{
    bool (*f)(std::string const&) =
         0 == Function ? contains_regex0
        :1 == Function ? contains_regex1
        :2 == Function ? contains_regex2
        :3 == Function ? contains_regex3
#if defined LMI_HAS_PCRE
        :4 == Function ? contains_regex4
#endif // defined LMI_HAS_PCRE
        :nullptr
        ;
    f(Regex);
}

char early[] = "he shall give thee the desires of thine heart";
char late [] = "he shall deliver them from the wicked";
char never[] = "lord";

/// Compare performance of various algorithms.
///
/// Searching C++ source files is the contemplated use case. The text
/// of Psalm 37 is artificially augmented to make it about the same
/// size as a typical source file. That augmentation properly shows
/// the high cost of line-by-line searching in a vector of strings.
/// The considerable cost of transforming a source file to a vector of
/// strings is excluded from the timing comparison because it can be
/// amortized over a large number of regex searches per file.

void test_psalm_37()
{
    lines = vectorize(original);
    for(auto const& i : lines)
        {
        for(int j = 0; j < 10; ++j)
            {
            text += i + '\n';
            }
        }
    lines = vectorize(text);

    std::cout << "  vectorize: " << TimeAnAliquot(mete_vectorize) << '\n';
    std::cout << '\n';

    std::cout << "Size of original text: " << original.size() << '\n';
    std::cout << "Size of searched text: " << text    .size() << '\n';
    std::cout << '\n';

    LMI_TEST( contains_regex0(early));
    LMI_TEST( contains_regex1(early));
    LMI_TEST( contains_regex2(early));
    LMI_TEST( contains_regex3(early));
#if defined LMI_HAS_PCRE
    LMI_TEST( contains_regex4(early));
#endif // defined LMI_HAS_PCRE

    LMI_TEST( contains_regex0(late ));
    LMI_TEST( contains_regex1(late ));
    LMI_TEST( contains_regex2(late ));
    LMI_TEST( contains_regex3(late ));
#if defined LMI_HAS_PCRE
    LMI_TEST( contains_regex4(late ));
#endif // defined LMI_HAS_PCRE

    LMI_TEST(!contains_regex0(never));
    LMI_TEST(!contains_regex1(never));
    LMI_TEST(!contains_regex2(never));
    LMI_TEST(!contains_regex3(never));
#if defined LMI_HAS_PCRE
    LMI_TEST(!contains_regex4(never));
#endif // defined LMI_HAS_PCRE

    std::cout << "  early 0:   " << TimeAnAliquot(mete<0,early>) << '\n';
    std::cout << "  early 1:   " << TimeAnAliquot(mete<1,early>) << '\n';
    std::cout << "  early 2:   " << TimeAnAliquot(mete<2,early>) << '\n';
    std::cout << "  early 3:   " << TimeAnAliquot(mete<3,early>) << '\n';
#if defined LMI_HAS_PCRE
    std::cout << "  early 4:   " << TimeAnAliquot(mete<4,early>) << '\n';
#endif // defined LMI_HAS_PCRE
    std::cout << '\n';

    std::cout << "  late  0:   " << TimeAnAliquot(mete<0,late >) << '\n';
    std::cout << "  late  1:   " << TimeAnAliquot(mete<1,late >) << '\n';
    std::cout << "  late  2:   " << TimeAnAliquot(mete<2,late >) << '\n';
    std::cout << "  late  3:   " << TimeAnAliquot(mete<3,late >) << '\n';
#if defined LMI_HAS_PCRE
    std::cout << "  late  4:   " << TimeAnAliquot(mete<4,late >) << '\n';
#endif // defined LMI_HAS_PCRE
    std::cout << '\n';

    std::cout << "  never 0:   " << TimeAnAliquot(mete<0,never>) << '\n';
    std::cout << "  never 1:   " << TimeAnAliquot(mete<1,never>) << '\n';
    std::cout << "  never 2:   " << TimeAnAliquot(mete<2,never>) << '\n';
    std::cout << "  never 3:   " << TimeAnAliquot(mete<3,never>) << '\n';
#if defined LMI_HAS_PCRE
    std::cout << "  never 4:   " << TimeAnAliquot(mete<4,never>) << '\n';
#endif // defined LMI_HAS_PCRE
    std::cout << '\n';
}

/// Test regexen for input-sequence validation.
///
/// Motivation: to validate data from external systems. To facilitate
/// maintenance of xml schemata, a regex is constructed and displayed
/// for every lmi sequence type.
///
/// These regular expressions are overly permissive by design. The
/// intention is to accept anything lmi's input-sequence parser does
/// and should, while rejecting all reasonably anticipated errors.

void test_input_sequence_regex()
{
    // A crude regex for a floating-point number, which defectively
    // accepts '.'.
    std::string const N(R"(\-?[0-9.]+)");
    // A set of keywords. These happen to be the ones permitted for
    // 'specamt_sequence'.
    std::string       K("maximum|target|sevenpay|glp|gsp|corridor|salary");
    // A datum: allow both numbers and keywords for these tests,
    // because that's the most general case.
    // Compactly: "(N|K)"
    std::string       X("(" + N + "|" + K + ")");
    // An optional subexpression indicating the durations for which a
    // datum is used. It begins with an obligatory ' ' or ',' (with
    // optional extra spaces) that separates it from the preceding
    // datum. Following that prefix, exactly one of these occurs:
    //  - an integer, optionally prefixed with '@' or '#'; or
    //  - a keyword: 'retirement' and 'maturity' are allowed, but for
    //    simplicity any string of lowercase alphabetic characters is
    //    accepted (designated '[a-z]' for brevity, because xml schema
    //    languages don't support POSIX [:lower:]); or
    //  - an interval expression beginning with '[' or '(' and ending
    //    with ']' or ')', with anything but a semicolon in between
    //    (the actual grammar is of course more restrictive, but need
    //    not be described here).
    std::string const Y(R"((( +| *, *)([@#]? *[0-9]+|[a-z]+|[\[\(][^;]+[\]\)])))");
    // The regex to be tested. It can consist solely of zero or more
    // spaces. Otherwise, it consists of one or more data-duration
    // pairs ('X' and an optional 'Y' as above), with an obligatory
    // semicolon between successive pairs. Leading and trailing blanks
    // are permitted, as is an optional semicolon after the last pair.
    // Compactly: " *| *XY? *(; *XY? *)*;? *"
    std::string       R(" *| *" + X + Y + "? *(; *" + X + Y + "? *)*;? *");

    // This is intended to be useful with xml schema languages, which
    // implicitly anchor the entire regex, so '^' and '$' aren't used.
    std::regex const r(R);

    // Tests that are designed to succeed.

    // Simple scalars.
    LMI_TEST( std::regex_match("1234"                                                       , r));
    LMI_TEST( std::regex_match("glp"                                                        , r));
    // Semicolon-delimited values, as expected in inforce extracts.
    LMI_TEST( std::regex_match("123;456;0"                                                  , r));
    // Same, with whitespace.
    LMI_TEST( std::regex_match("123; 456; 0"                                                , r));
    LMI_TEST( std::regex_match("123 ;456 ;0"                                                , r));
    LMI_TEST( std::regex_match("123;  456;  0"                                              , r));
    LMI_TEST( std::regex_match("123  ;456  ;0"                                              , r));
    LMI_TEST( std::regex_match(" 123  ;  456  ;  0 "                                        , r));
    LMI_TEST( std::regex_match("  123  ;  456  ;  0  "                                      , r));
    // Same, with optional terminal semicolon.
    LMI_TEST( std::regex_match("  123  ;  456  ;  0  ;"                                     , r));
    LMI_TEST( std::regex_match("  123  ;  456  ;  0  ;  "                                   , r));
    // Single scalar with terminal semicolon and various whitespace.
    LMI_TEST( std::regex_match("123;"                                                       , r));
    LMI_TEST( std::regex_match("123 ;"                                                      , r));
    LMI_TEST( std::regex_match("123; "                                                      , r));
    LMI_TEST( std::regex_match(" 123 ; "                                                    , r));
    // Negatives (e.g., "negative" loans representing repayments).
    LMI_TEST( std::regex_match("-987; -654"                                                 , r));
    // Decimals.
    LMI_TEST( std::regex_match("0.;.0;0.0;1234.5678"                                        , r));
    // Decimals, along with '#' and '@'.
    LMI_TEST( std::regex_match("0.,2;.0,#3;0.0,@75;1234.5678"                               , r));
    // Same, with whitespace.
    LMI_TEST( std::regex_match(" 0. , 2 ; .0 , # 3 ; 0.0 , @ 75 ; 1234.5678 "               , r));
    // No numbers--only keywords.
    LMI_TEST( std::regex_match("salary,retirement;corridor,maturity"                        , r));
    // Same, with whitespace.
    LMI_TEST( std::regex_match("  salary  ,  retirement;  corridor  ,  maturity"            , r));
    LMI_TEST( std::regex_match("  salary  ,  retirement;  corridor  ,  maturity  "          , r));
    LMI_TEST( std::regex_match("  salary  ,  retirement  ;  corridor  ,  maturity"          , r));
    LMI_TEST( std::regex_match("  salary  ,  retirement  ;  corridor  ,  maturity  "        , r));
    // Empty except for zero or more blanks.
    LMI_TEST( std::regex_match(""                                                           , r));
    LMI_TEST( std::regex_match(" "                                                          , r));
    LMI_TEST( std::regex_match("  "                                                         , r));
    // Interval notation.
    LMI_TEST( std::regex_match("1 [2,3);4 (5,6]"                                            , r));
    // User-manual examples. See: https://www.nongnu.org/lmi/sequence_input.html
    LMI_TEST( std::regex_match("sevenpay 7; 250000 retirement; 100000 #10; 75000 @95; 50000", r));
    LMI_TEST( std::regex_match("100000; 110000; 120000; 130000; 140000; 150000"             , r));
    LMI_TEST( std::regex_match("target; maximum"                                            , r)); // [Modified example.]
    LMI_TEST( std::regex_match("10000 20; 0"                                                , r));
    LMI_TEST( std::regex_match("10000 10; 5000 15; 0"                                       , r));
    LMI_TEST( std::regex_match("10000 @70; 0"                                               , r));
    LMI_TEST( std::regex_match("10000 retirement; 0"                                        , r));
    LMI_TEST( std::regex_match("0 retirement; 5000"                                         , r));
    LMI_TEST( std::regex_match("0 retirement; 5000 maturity"                                , r));
    LMI_TEST( std::regex_match("0 retirement; 5000 #10; 0"                                  , r));
    LMI_TEST( std::regex_match("0,[0,retirement);10000,[retirement,#10);0"                  , r));

    // Tests that are designed to fail.

    // Naked semicolon.
    LMI_TEST(!std::regex_match(";"                                                          , r));
    LMI_TEST(!std::regex_match(" ; "                                                        , r));
    // Missing required semicolon.
    LMI_TEST(!std::regex_match("7 24 25"                                                    , r));
    LMI_TEST(!std::regex_match("7,24,25"                                                    , r));
    LMI_TEST(!std::regex_match("7, 24, 25"                                                  , r));
    LMI_TEST(!std::regex_match("7 , 24 , 25"                                                , r));
    // Extraneous commas.
    LMI_TEST(!std::regex_match(",1"                                                         , r));
    LMI_TEST(!std::regex_match("1,"                                                         , r));
    LMI_TEST(!std::regex_match("1,2,"                                                       , r));
    LMI_TEST(!std::regex_match("1,,2"                                                       , r));
    // Impermissible character.
    LMI_TEST(!std::regex_match("%"                                                          , r));
    // Uppercase in keywords.
    LMI_TEST(!std::regex_match("Glp"                                                        , r));
    LMI_TEST(!std::regex_match("GLP"                                                        , r));
    // Misppellings.
    LMI_TEST(!std::regex_match("gdp"                                                        , r));
    LMI_TEST(!std::regex_match("glpp"                                                       , r));
    LMI_TEST(!std::regex_match("gglp"                                                       , r));

    X = "(\\-?[0-9.]+)";
    R = " *| *" + X + Y + "? *(; *" + X + Y + "? *)*;? *";
    std::cout << "numeric_sequence = xsd:string {pattern = \"" << R << "\"}" << std::endl;
    X = "(\\-?[0-9.]+|minimum|target|sevenpay|glp|gsp|corridor|table)";
    R = " *| *" + X + Y + "? *(; *" + X + Y + "? *)*;? *";
    std::cout << "payment_sequence = xsd:string {pattern = \"" << R << "\"}" << std::endl;
    X = "(annual|semiannual|quarterly|monthly)";
    R = " *| *" + X + Y + "? *(; *" + X + Y + "? *)*;? *";
    std::cout << "mode_sequence    = xsd:string {pattern = \"" << R << "\"}" << std::endl;
    X = "(\\-?[0-9.]+|maximum|target|sevenpay|glp|gsp|corridor|salary)";
    R = " *| *" + X + Y + "? *(; *" + X + Y + "? *)*;? *";
    std::cout << "specamt_sequence = xsd:string {pattern = \"" << R << "\"}" << std::endl;
    X = "(a|b|rop|mdb)";
    R = " *| *" + X + Y + "? *(; *" + X + Y + "? *)*;? *";
    std::cout << "dbo_sequence     = xsd:string {pattern = \"" << R << "\"}" << std::endl;
}

int test_main(int, char*[])
{
    test_psalm_37();
    test_input_sequence_regex();

    return 0;
}
