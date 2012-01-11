// Regular expressions--unit test.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "contains.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/regex.hpp>

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
/// Perl's 's' is the default for boost regex; boost offers
///   http://boost.org/libs/regex/doc/syntax_option_type.html
/// "no_mod_s" and
///   http://boost.org/libs/regex/doc/match_flag_type.html
/// "match_not_dot_newline" to prevent '.' from matching '\n', as well
/// as recognizing "(?-s)" in the regex. However, TR1
///   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1836.pdf
/// does not:
///
/// [7.5.2] omits boost's "match_not_dot_newline"; and
///
/// [7.5.1] offers "awk", "grep", and "egrep" syntax options, but
/// defines them in terms of IEEE 1003.1-2001
///   http://www.opengroup.org/onlinepubs/000095399/basedefs/xbd_chap09.html
/// which requires '.' to match '\n'.
///
/// TR1 implements only POSIX and ECMAScript behavior; the latter
///   http://www.ecma-international.org/publications/files/ecma-st/ECMA-262.pdf
/// [15.10.4.1] describes a "multiline property", but excludes 's'.
/// However, TR1 offers no way to manipulate even that "property",
/// much less to set or unset the Perl 's' modifier.
///
/// Furthermore, in dinkumware's TR1 implementation, a '.' wildcard
/// matches anything except '\n':
///   http://www.dinkumware.com/manuals/?manual=compleat&page=lib_regex.html#wildcard%20character
///   "A wildcard character matches any character in the target
///   expression except a newline"
/// whereas ECMA-262 section 15.10.2.8 says '.' matches
///   the set of all characters except the four line terminator
///   characters <LF>, <CR>, <LS>, or <PS>
/// yet '.' matches anything in the boost implementation.
///
/// Conclusion: TR1 offers no control over 's' or 'm' behavior, and
/// implementors seem not to agree even on what '.' matches. Where
/// such questions matter in code that's desired to be compatible with
/// TR1, there seem to be only two viable options:
///  - use a vectorized approach such as this; or
///  - write regexen with
///    - "[^\\n]" instead of '.', and
///    - "\n" instead of '^' and '$' anchors
///    and prepend a '\n' sentry to the string to be searched (and
///    assert that all files end in '\n').
/// The second option is far more attractive for performance reasons,
/// as this unit test shows.

bool contains_regex1(std::string const& regex)
{
    boost::regex const r(regex, boost::regex::sed);
    typedef std::vector<std::string>::const_iterator vsi;
    for(vsi i = lines.begin(); i != lines.end(); ++i)
        {
        if(boost::regex_search(*i, r))
            {
            return true;
            }
        }
    return false;
}

/// Match a regex as with Perl's '-s'.

bool contains_regex2(std::string const& regex)
{
    return boost::regex_search(text, boost::regex("(?-s)" + regex));
}

/// Match a regex as with Perl's 's'.

bool contains_regex3(std::string const& regex)
{
    return boost::regex_search(text, boost::regex(regex));
}

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
        :0
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

int test_main(int, char*[])
{
    lines = vectorize(original);
    typedef std::vector<std::string>::iterator vsi;
    for(vsi i = lines.begin(); i != lines.end(); ++i)
        {
        for(int j = 0; j < 10; ++j)
            {
            text += *i + '\n';
            }
        }
    lines = vectorize(text);

    std::cout << "  vectorize: " << TimeAnAliquot(mete_vectorize) << '\n';
    std::cout << '\n';

    std::cout << "Size of original text: " << original.size() << '\n';
    std::cout << "Size of searched text: " << text    .size() << '\n';
    std::cout << '\n';

    BOOST_TEST( contains_regex0(early));
    BOOST_TEST( contains_regex1(early));
    BOOST_TEST( contains_regex2(early));
    BOOST_TEST( contains_regex3(early));

    BOOST_TEST( contains_regex0(late ));
    BOOST_TEST( contains_regex1(late ));
    BOOST_TEST( contains_regex2(late ));
    BOOST_TEST( contains_regex3(late ));

    BOOST_TEST(!contains_regex0(never));
    BOOST_TEST(!contains_regex1(never));
    BOOST_TEST(!contains_regex2(never));
    BOOST_TEST(!contains_regex3(never));

    std::cout << "  early 0:   " << TimeAnAliquot(mete<0, early>) << '\n';
    std::cout << "  early 1:   " << TimeAnAliquot(mete<1, early>) << '\n';
    std::cout << "  early 2:   " << TimeAnAliquot(mete<2, early>) << '\n';
    std::cout << "  early 3:   " << TimeAnAliquot(mete<3, early>) << '\n';
    std::cout << '\n';

    std::cout << "  late  0:   " << TimeAnAliquot(mete<0, late >) << '\n';
    std::cout << "  late  1:   " << TimeAnAliquot(mete<1, late >) << '\n';
    std::cout << "  late  2:   " << TimeAnAliquot(mete<2, late >) << '\n';
    std::cout << "  late  3:   " << TimeAnAliquot(mete<3, late >) << '\n';
    std::cout << '\n';

    std::cout << "  never 0:   " << TimeAnAliquot(mete<0, never>) << '\n';
    std::cout << "  never 1:   " << TimeAnAliquot(mete<1, never>) << '\n';
    std::cout << "  never 2:   " << TimeAnAliquot(mete<2, never>) << '\n';
    std::cout << "  never 3:   " << TimeAnAliquot(mete<3, never>) << '\n';
    std::cout << '\n';

    return 0;
}

