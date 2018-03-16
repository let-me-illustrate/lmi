// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "miscellany.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "stl_extensions.hpp"           // nonstd::power()

#include <algorithm>                    // equal(), max()
#include <cmath>                        // floor(), log10()
#include <ctime>
#include <fstream>
#include <istream>
#include <iterator>                     // istreambuf_iterator

namespace
{
/// This function is a derived work adapted from usenet article
/// <1eo2sct.ggkc9z84ko0eN%cbarron3@ix.netcom.com>. GWC rewrote it in
/// 2005. Any defect should not reflect on Carl Barron's reputation.

bool streams_are_identical(std::istream& is0, std::istream& is1)
{
    LMI_ASSERT(! !is0);
    LMI_ASSERT(! !is1);

    std::istreambuf_iterator<char> i(is0);
    std::istreambuf_iterator<char> j(is1);
    std::istreambuf_iterator<char> end;

    for(; end != i && end != j; ++i, ++j)
        {
        if(*i != *j)
            {
            return false;
            }
        }
    return end == i && end == j;
}
} // Unnamed namespace.

/// Test whether two files are identical. Arguments are filenames.

bool files_are_identical(std::string const& file0, std::string const& file1)
{
    std::ifstream ifs0(file0.c_str(), ios_in_binary());
    std::ifstream ifs1(file1.c_str(), ios_in_binary());
    if(!ifs0) alarum() << "Unable to open '" << file0 << "'." << LMI_FLUSH;
    if(!ifs1) alarum() << "Unable to open '" << file1 << "'." << LMI_FLUSH;
    return streams_are_identical(ifs0, ifs1);
}

/// Triple-power-of-ten scaling to keep extremum < 10^max_power.
///
/// Returns a small nonnegative integer N. The values whose extrema
/// are passed as arguments will be divided by 10^N. N is a multiple
/// of three because it is common to write a caption like "(000)" or
/// "values in thousands", but "values in myriads" would not be seen
/// in the US. Similarly, "values in kibidollars" would not be seen
/// in finance.
///
/// After that scaling division, no value is wider when formatted
/// than 10^max_power - 1. Thus, if max_power is 6, scaled values
/// are in [-99,999, 999,999], with due regard to the minus sign.
/// Because the scaling power N is a multiple of three, it would
/// make no sense for max_power to be less than three. However,
/// max_power itself need not be an integral multiple of three:
/// a column might reasonably provide room for "99,999,999" only.
///
/// Asserted preconditions:
///   3 <= max_power
///   min_value <= max_value

int scale_power(int max_power, double min_value, double max_value)
{
    LMI_ASSERT(3 <= max_power);
    LMI_ASSERT(min_value <= max_value);

    // A negative value needs an extra '-' character: i.e., as many
    // total characters as ten times its absolute value requires.
    double widest = std::max(min_value * -10, max_value);

    if(0 == widest || widest < nonstd::power(10.0, max_power))
        {
        return 0;
        }

// PDF !! This seems not to be rigorously correct: $999,999,999.99 is
// less than one billion, but rounds to $1,000,000,000.
    double d = std::log10(widest);
    d = std::floor(d / 3.0);
    int k = 3 * static_cast<int>(d);
    k = k - 6;

    LMI_ASSERT(0 <= k);
    LMI_ASSERT(k <= 18);

    return k;
}

/// Return the number of newline characters in a string.

std::size_t count_newlines(std::string const& s)
{
    return std::count(s.begin(), s.end(), '\n');
}

/// Split an internally-newline-delimited string into lines.

std::vector<std::string> split_into_lines(std::string const& s)
{
    std::vector<std::string> lines;
    std::string line;
    for(std::string::const_iterator i = s.begin(); ; ++i)
        {
        if(i == s.end() || '\n' == *i)
            {
            lines.push_back(line);
            if(i == s.end())
                {
                break;
                }
            line.clear();
            }
        else
            {
            line += *i;
            }
        }
    // Assume that there is no newline at the end (or beginning) of
    // the string: i.e., that all newline delimiters are internal--
    // hence "1u + ".
    LMI_ASSERT(lines.size() == 1u + count_newlines(s));
    return lines;
}

/// Escape text for html, e.g., "a < b" --> "a &lt; b".

std::string htmlize(std::string const& raw_text)
{
    std::string html;
    html.reserve(raw_text.size());

    for(auto const& c : raw_text)
        {
        switch(c)
            {
            case '&': {html += "&amp;";} break;
            case '<': {html += "&lt;" ;} break;
            case '>': {html += "&gt;" ;} break;
            default : {html += c      ;}
            }
        }

    return html;
}

/// Ascertain whether string begins with prefix.

bool begins_with(std::string const& s, std::string const& prefix)
{
    return
           (prefix.size() <= s.size())
        && std::equal(prefix.begin(), prefix.end(), s.begin());
}

/// Ascertain whether string ends with suffix.

bool ends_with(std::string const& s, std::string const& suffix)
{
    return
           (suffix.size() <= s.size())
        && std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

/// Remove superfluous characters from beginning of string.

void ltrim(std::string& s, char const* superfluous)
{
    std::string::size_type p = s.find_first_not_of(superfluous);
    if(std::string::npos != p)
        {
        s.erase(0, p);
        }
    else
        {
        s.clear();
        }
}

/// Remove superfluous characters from end of string.

void rtrim(std::string& s, char const* superfluous)
{
    std::string::size_type p = s.find_last_not_of(superfluous);
    if(std::string::npos != p)
        {
        s.erase(1 + p);
        }
    else
        {
        s.clear();
        }
}

/// Omitting colons yields a valid posix path.
///
/// http://groups.google.com/group/borland.public.cpp.borlandcpp/msg/638d1f25e66472d9
///   [2001-07-18T22:25:15Z from Greg Chicares]

std::string iso_8601_datestamp_verbose()
{
    std::size_t const len = sizeof "CCYY-MM-DDTHH:MM:SSZ";
    std::time_t const t0 = std::time(nullptr);
    std::tm const*const t1 = std::gmtime(&t0);
    LMI_ASSERT(nullptr != t1);
    char s[len];
    std::size_t rc = std::strftime(s, len, "%Y-%m-%dT%H:%M:%SZ", t1);
    LMI_ASSERT(0 != rc);
    return s;
}

/// Colons separate HH:MM:SS, so result is not a valid posix path.

std::string iso_8601_datestamp_terse()
{
    std::size_t const len = sizeof "CCYYMMDDTHHMMSSZ";
    std::time_t const t0 = std::time(nullptr);
    std::tm const*const t1 = std::gmtime(&t0);
    LMI_ASSERT(nullptr != t1);
    char s[len];
    std::size_t rc = std::strftime(s, len, "%Y%m%dT%H%M%SZ", t1);
    LMI_ASSERT(0 != rc);
    return s;
}

/// Compute the number of pages needed to display the given number of non-blank
/// rows in groups of the specified size separated by blank rows.
///
/// Preconditions: 0 < total_rows && 0 < rows_per_group <= rows_per_page

int page_count
    (int total_rows
    ,int rows_per_group
    ,int rows_per_page
    )
{
    LMI_ASSERT(0 < total_rows);
    LMI_ASSERT(0 < rows_per_group                 );
    LMI_ASSERT(    rows_per_group <= rows_per_page);

    // Each group actually takes rows_per_group+1 rows because of the
    // separator row between groups, hence the second +1, but there is no
    // need for the separator after the last group, hence the first +1.
    int const groups_per_page = (rows_per_page + 1) / (rows_per_group + 1);

    // But we are actually interested in the number of rows we can fit per page
    // and not the number of groups.
    int const used_per_page = groups_per_page * rows_per_group;

    // Finally determine how many pages are needed to show all the rows.
    int num_pages = (total_rows + used_per_page - 1) / used_per_page;

    // The last page may not be needed if all the rows on it can fit into the
    // remaining space, too small for a full group, but perhaps sufficient for
    // these rows, in the last by one page.
    if(1 < num_pages)
        {
        auto const rows_on_last_page = total_rows - (num_pages - 1) * used_per_page;
        auto const free_rows = rows_per_page - groups_per_page * (rows_per_group + 1);
        if(rows_on_last_page <= free_rows)
            {
            num_pages--;
            }
        }

    return num_pages;
}
