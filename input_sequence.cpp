// Input sequences e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0....
// Extract the grammar from lines matching the regexp _// GRAMMAR_ .
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

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "miscellany.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <cctype>
#include <ios>
#include <istream>
#include <iterator>                     // std::ostream_iterator
#include <ostream>
#include <sstream>

ValueInterval::ValueInterval()
    :value_number     (0.0)
    ,value_is_keyword (false)
    ,begin_duration   (0)
    ,begin_mode       (e_duration)
    ,end_duration     (0)
    ,end_mode         (e_duration)
    ,insane           (false)
{}

InputSequence::InputSequence
    (std::string const&              input_expression
    ,int                             a_years_to_maturity
    ,int                             a_issue_age
    ,int                             a_retirement_age
    ,int                             a_inforce_duration
    ,int                             a_effective_year
    ,std::vector<std::string> const& a_extra_keywords
    ,std::string const&              a_default_keyword
    ,bool                            a_keywords_only
    )
    :input_stream                  (input_expression.c_str())
    ,years_to_maturity             (a_years_to_maturity)
    ,issue_age                     (a_issue_age)
    ,retirement_age                (a_retirement_age)
    ,inforce_duration              (a_inforce_duration)
    ,effective_year                (a_effective_year)
    ,extra_keywords                (a_extra_keywords)
    ,default_keyword               (a_default_keyword)
    ,keywords_only                 (a_keywords_only)
    ,current_token_type            (e_startup)
    ,previous_duration_scalar_mode (e_inception)
    ,current_duration_scalar_mode  (e_inception)
    ,last_input_duration           (0)
{
    sequence();

    // Inception and maturity endpoints exist, so the interval they
    // define must exist. However, parsing an empty expression
    // constructs zero intervals, so a default one must be created
    // to make the physical reality meet the conceptual requirement.
    if(intervals.empty())
        {
        intervals.push_back(ValueInterval());
        }

    // Extend the last interval's endpoint to maturity, replicating
    // the last element. (This doesn't need to be done by the ctors
    // that take vector arguments, because those arguments specify
    // each value in [inception, maturity) and deduce the terminal
    // (maturity) duration from size().)

    // This invariant has not yet been established, whether or not the
    // sequence was empty.
    intervals.back().end_duration = years_to_maturity;
    // This invariant is established by realize_vector(), but it does
    // no harm to repeat it here, and it would be confusing not to do
    // so in conjunction with the line above.
    intervals.back().end_mode     = e_maturity;

    realize_vector();
}

// Constructors taking one or two vectors as their sole arguments are
// intended to convert flat vectors to input sequences, compacted with
// run-length encoding: 1 1 1 2 2 becomes 1[0,2), 2[2,4).
//
// The constructor that takes two vector arguments exists because some
// sequences may have both numeric and keyword values. Distinct vectors
// are supplied for these two value types; for each interval, the value
// type chosen is keyword if the keyword is not blank, else numeric.
//
// The control constructs may appear nonobvious. This design treats
// the push_back operation as fundamental: push_back is called exactly
// when we know that a new interval must be added. This avoids special
// handling
//   when the vectors are of length zero, and
//   for the last interval.
// As a consequence, we always push_back a dummy interval exactly when
// we know that it will be needed, and then write to intervals.back().
//
// An alternative design would work with a temporary interval and
// call push_back as needed. I tried that and concluded that this
// design is simpler.
//
// Strings in input vectors are not validated against a map of
// permissible strings: these constructors are designed for use only
// with vectors of strings generated by the program from known-valid
// input, and should not be used in any other situation.
// SOMEDAY !! Ideally, therefore, they should be protected from
// unintended use.

InputSequence::InputSequence(std::vector<double> const& v)
    :years_to_maturity(v.size())
{
    ValueInterval dummy;

    double prior_value =
        (0 != v.size())
        ? v.front()
        : 0.0
        ;
    double current_value = prior_value;

    intervals.push_back(dummy);
    intervals.back().value_number = current_value;

    for(auto const& vi : v)
        {
        current_value = vi;
        if(prior_value == current_value)
            {
            ++intervals.back().end_duration;
            }
        else
            {
            int value_change_duration = intervals.back().end_duration;
            intervals.push_back(dummy);
            intervals.back().value_number = current_value;
            intervals.back().begin_duration = value_change_duration;
            intervals.back().end_duration = ++value_change_duration;
            prior_value = current_value;
            }
        }

    realize_vector();
}

InputSequence::InputSequence(std::vector<std::string> const& v)
    :years_to_maturity(v.size())
{
    ValueInterval dummy;
    dummy.value_is_keyword = true;

    std::string prior_value =
        (0 != v.size())
        ? v.front()
        : std::string()
        ;
    std::string current_value = prior_value;

    intervals.push_back(dummy);
    intervals.back().value_keyword = current_value;

    for(auto const& vi : v)
        {
        current_value = vi;
        if(prior_value == current_value)
            {
            ++intervals.back().end_duration;
            }
        else
            {
            int value_change_duration = intervals.back().end_duration;
            intervals.push_back(dummy);
            intervals.back().value_keyword = current_value;
            intervals.back().begin_duration = value_change_duration;
            intervals.back().end_duration = ++value_change_duration;
            prior_value = current_value;
            }
        }

    realize_vector();
}

InputSequence::InputSequence
    (std::vector<double> const& n_v
    ,std::vector<std::string> const& s_v
    )
    :years_to_maturity(n_v.size())
{
    if(n_v.size() != s_v.size())
        {
        fatal_error()
            << "InputSequence: vector lengths differ."
            << LMI_FLUSH
            ;
        }

    ValueInterval dummy;

    double n_prior_value =
        (0 != n_v.size())
        ? n_v.front()
        : 0.0
        ;
    double n_current_value = n_prior_value;

    std::string s_prior_value =
        (0 != s_v.size())
        ? s_v.front()
        : std::string()
        ;
    std::string s_current_value = s_prior_value;

    intervals.push_back(dummy);
    intervals.back().value_number  = n_current_value;
    intervals.back().value_keyword = s_current_value;
    intervals.back().value_is_keyword = "" != s_current_value;

    std::vector<double>::const_iterator n_vi;
    std::vector<std::string>::const_iterator s_vi;
    for
        (n_vi = n_v.begin(), s_vi = s_v.begin()
        ;n_vi != n_v.end() // s_v has same length, as 'asserted' above.
        ;++n_vi, ++s_vi
        )
        {
        n_current_value = *n_vi;
        s_current_value = *s_vi;
        if(n_prior_value == n_current_value && s_prior_value == s_current_value)
            {
            ++intervals.back().end_duration;
            }
        else
            {
            int value_change_duration = intervals.back().end_duration;
            intervals.push_back(dummy);
            intervals.back().value_number = n_current_value;
            intervals.back().value_keyword = s_current_value;
            intervals.back().value_is_keyword = "" != s_current_value;
            intervals.back().begin_duration = value_change_duration;
            intervals.back().end_duration = ++value_change_duration;
            n_prior_value = n_current_value;
            s_prior_value = s_current_value;
            }
        }

    realize_vector();
}

InputSequence::~InputSequence() = default;

void InputSequence::realize_vector()
{
    // Post-construction invariants.
    // Every ctor must already have established this...
    LMI_ASSERT(!intervals.empty());
    // ...and this:
    LMI_ASSERT(years_to_maturity == intervals.back().end_duration);
    // It cannot be assumed that all ctors have yet established this...
    intervals.back().end_mode = e_maturity;
    // ...though now of course it has been established:
    LMI_ASSERT(e_maturity        == intervals.back().end_mode    );

    std::vector<double> default_numeric_vector(years_to_maturity);
    std::vector<std::string> default_string_vector(years_to_maturity, default_keyword);
    std::vector<double> r(default_numeric_vector);
    number_result = r;
    std::vector<std::string> s(default_string_vector);
    keyword_result = s;

    // Vectors have default values if the input expression could not be parsed.
    if(formatted_diagnostics().size())
        {
        return;
        }

    int prior_begin_duration = 0;
    for(auto const& interval_i : intervals)
        {
        if(interval_i.insane)
            {
            fatal_error()
                << "InputSequence: untrapped parser error."
                << LMI_FLUSH
                ;
            }
        if(e_invalid_mode == interval_i.begin_mode)
            {
            fatal_error()
                << "Interval "
                << "[ " << interval_i.begin_duration << ", "
                << interval_i.end_duration << " )"
                << " has invalid begin_mode."
                << LMI_FLUSH
                ;
            }
        if(e_invalid_mode == interval_i.end_mode)
            {
            fatal_error()
                << "Interval "
                << "[ " << interval_i.begin_duration << ", "
                << interval_i.end_duration << " )"
                << " has invalid end_mode."
                << LMI_FLUSH
                ;
            }
        if(interval_i.value_is_keyword && "insane" == interval_i.value_keyword)
            {
            fatal_error()
                << "Interval "
                << "[ " << interval_i.begin_duration << ", "
                << interval_i.end_duration << " )"
                << " has invalid value_keyword."
                << LMI_FLUSH
                ;
            }
        if(interval_i.begin_duration < prior_begin_duration)
            {
            diagnostics
                << "Previous interval began at duration "
                << prior_begin_duration
                << "; current interval "
                << "[ " << interval_i.begin_duration << ", "
                << interval_i.end_duration << " )"
                << " would begin before that. "
                ;
                mark_diagnostic_context();
            return;
            }
        prior_begin_duration = interval_i.begin_duration;
        bool interval_is_ok =
               0                         <= interval_i.begin_duration
            && interval_i.begin_duration <= interval_i.end_duration
            && interval_i.end_duration   <= years_to_maturity
            ;
        if(!interval_is_ok)
            {
            fatal_error()
                << "Interval "
                << "[ " << interval_i.begin_duration << ", "
                << interval_i.end_duration << " )"
                << " not valid."
                << LMI_FLUSH
                ;
            }
        if(interval_i.value_is_keyword)
            {
            std::fill
                (s.begin() + interval_i.begin_duration
                ,s.begin() + interval_i.end_duration
                ,interval_i.value_keyword
                );
            }
        else
            {
            std::fill
                (r.begin() + interval_i.begin_duration
                ,r.begin() + interval_i.end_duration
                ,interval_i.value_number
                );
            }
        }

    number_result = r;
    keyword_result = s;
}

// GRAMMAR interval-begin: one of [ (
// GRAMMAR interval-end: one of ] )

// GRAMMAR duration-constant: one of inforce retirement maturity
// SOMEDAY !! 'inforce' not yet implemented

// GRAMMAR duration-scalar: integer
// GRAMMAR duration-scalar: @ integer
// GRAMMAR duration-scalar: # integer
// GRAMMAR duration-scalar: duration-constant
// SOMEDAY !! calendar year not yet implemented

void InputSequence::duration_scalar()
{
    switch(current_token_type)
        {
        case e_age_prefix:
            {
            current_duration_scalar_mode = e_attained_age;
            match(current_token_type);
            }
            break;
        case e_cardinal_prefix:
            {
            current_duration_scalar_mode = e_number_of_years;
            match(current_token_type);
            }
            break;
        case e_number:
            {
            current_duration_scalar_mode = e_duration;
            }
            break;
        case e_keyword:
            {
            if("retirement" == current_keyword)
                {
                current_duration_scalar_mode = e_retirement;
                current_duration_scalar = retirement_age - issue_age;
                match(e_keyword);
                return;
                }
            else if("maturity" == current_keyword)
                {
                current_duration_scalar_mode = e_maturity;
                current_duration_scalar = years_to_maturity;
                match(e_keyword);
                return;
                }
            else
                {
                diagnostics
                    << "Expected keyword "
                    << "'retirement' "
                    << " or 'maturity'. "
                    ;
                mark_diagnostic_context();
                }
            }
            break;
        default:
            {
            diagnostics << "Expected keyword, number, '@', or '#'. ";
            mark_diagnostic_context();
            }
        }

    current_duration_scalar = static_cast<int>(current_number);
    match(e_number);

    switch(current_duration_scalar_mode)
        {
        case e_duration:
            {
            ; // Do nothing.
            }
            break;
        case e_attained_age:
            {
            current_duration_scalar -= issue_age;
            }
            break;
        case e_number_of_years:
            {
            current_duration_scalar += last_input_duration;
            }
            break;
        default:
            {
            diagnostics << "Expected number, '@', or '#'. ";
            mark_diagnostic_context();
            }
        }
}

void InputSequence::null_duration()
{
    int tentative_begin_duration                = last_input_duration++;
    duration_mode tentative_begin_duration_mode = previous_duration_scalar_mode;
    int tentative_end_duration                  = last_input_duration;
    duration_mode tentative_end_duration_mode   = e_duration;
    validate_duration
        (tentative_begin_duration
        ,tentative_begin_duration_mode
        ,tentative_end_duration
        ,tentative_end_duration_mode
        );
}

// GRAMMAR duration: duration-scalar

void InputSequence::single_duration()
{
    duration_scalar();
    int tentative_begin_duration                = last_input_duration;
//    last_input_duration += static_cast<int>(current_duration_scalar);
//    last_input_duration = static_cast<int>(current_duration_scalar);
//    duration_mode tentative_begin_duration_mode = e_duration;
    duration_mode tentative_begin_duration_mode = previous_duration_scalar_mode;
    int tentative_end_duration                  = current_duration_scalar;
//    int tentative_end_duration   = last_input_duration + static_cast<int>(current_duration_scalar);
    duration_mode tentative_end_duration_mode   = current_duration_scalar_mode;
    validate_duration
        (tentative_begin_duration
        ,tentative_begin_duration_mode
        ,tentative_end_duration
        ,tentative_end_duration_mode
        );
}

// GRAMMAR duration: interval-begin duration-scalar , duration-scalar interval-end

void InputSequence::intervalic_duration()
{
    bool begin_excl = (e_begin_excl == current_token_type);
    match(current_token_type);
    duration_scalar();
    // Add one to the interval-beginning if it was expressed
    // as exclusive, because we store [begin, end).
    int tentative_begin_duration =
          static_cast<int>(current_duration_scalar)
        + begin_excl
        ;
    duration_mode tentative_begin_duration_mode = current_duration_scalar_mode;
    int tentative_end_duration                  = -1;
    duration_mode tentative_end_duration_mode   = e_invalid_mode;
    match(e_minor_separator); // TODO ?? Require this?
    duration_scalar();
    if
        (  e_end_incl == current_token_type
        || e_end_excl == current_token_type
        )
        {
        bool end_incl = (e_end_incl == current_token_type);
        match(current_token_type);
        // Add one to the interval-end if it was expressed
        // as inclusive, because we store [begin, end).
        tentative_end_duration      = current_duration_scalar + end_incl;
        tentative_end_duration_mode = current_duration_scalar_mode;
        }
    else
        {
        diagnostics << "Expected ')' or ']'. ";
        mark_diagnostic_context();
        return;
        }
    validate_duration
        (tentative_begin_duration
        ,tentative_begin_duration_mode
        ,tentative_end_duration
        ,tentative_end_duration_mode
        );
}

void InputSequence::validate_duration
    (int           tentative_begin_duration
    ,duration_mode tentative_begin_duration_mode
    ,int           tentative_end_duration
    ,duration_mode tentative_end_duration_mode
    )
{
    if(tentative_begin_duration < 0)
        {
        current_interval.insane = true;
        diagnostics
            << "Interval "
            << "[ " << tentative_begin_duration << ", "
            << tentative_end_duration << " )"
            << " is improper: it "
            << "begins before the first possible duration. "
            ;
        mark_diagnostic_context();
        return;
        }
    // Remember that our paradigm is [begin, end), so that
    // [X, X) truly is improper: it
    //   starts at X, and includes X, and
    //   ends   at X, and excludes X
    // so it both includes and excludes X. Thus, an interval
    // [B, E) is improper if B == E.
    else if(tentative_end_duration <= tentative_begin_duration)
        {
        current_interval.insane = true;
        diagnostics
            << "Interval "
            << "[ " << tentative_begin_duration << ", "
            << tentative_end_duration << " )"
            << " is improper: it "
            << "ends before it begins. "
            ;
        mark_diagnostic_context();
        return;
        }
    else if(years_to_maturity < tentative_end_duration)
        {
        current_interval.insane = true;
        diagnostics
            << "Interval "
            << "[ " << tentative_begin_duration << ", "
            << tentative_end_duration << " )"
            << " is improper: it "
            << "ends after the last possible duration. "
            ;
        mark_diagnostic_context();
        return;
        }
    else if
        (  e_invalid_mode == tentative_begin_duration_mode
        || e_invalid_mode == tentative_end_duration_mode
        )
        {
        current_interval.insane = true;
        diagnostics
            << "Interval "
            << "[ " << tentative_begin_duration << ", "
            << tentative_end_duration << " )"
            << " has an invalid duration mode. "
            ;
        mark_diagnostic_context();
        return;
        }
    else
        {
        current_interval.begin_duration = tentative_begin_duration     ;
        current_interval.end_duration   = tentative_end_duration       ;
        current_interval.begin_mode     = tentative_begin_duration_mode;
        current_interval.end_mode       = tentative_end_duration_mode  ;
        last_input_duration             = current_interval.end_duration;
        }
}

void InputSequence::duration()
{
    switch(current_token_type)
        {
        case e_eof:
        case e_major_separator:
            {
            null_duration();
            }
            break;
        case e_number:
        case e_age_prefix:
        case e_cardinal_prefix:
        case e_keyword:
            {
            single_duration();
            }
            break;
        case e_begin_incl:
        case e_begin_excl:
            {
            intervalic_duration();
            }
            break;
        default:
            {
            diagnostics
                << "Expected number, '[', '(', 'retirement', or 'maturity'. "
                ;
            mark_diagnostic_context();
            }
        }
    previous_duration_scalar_mode = current_duration_scalar_mode;
}

// GRAMMAR value: floating-point-number
// GRAMMAR value: value-constant

void InputSequence::value()
{
    switch(current_token_type)
        {
        case e_number:
            {
            current_interval.value_is_keyword = false;
            if(keywords_only)
                {
                diagnostics
                    << "Expected keyword chosen from { "
                    ;
                std::copy
                    (extra_keywords.begin()
                    ,extra_keywords.end()
                    ,std::ostream_iterator<std::string>(diagnostics, " ")
                    );
                diagnostics << "}. ";
                mark_diagnostic_context();
                break;
                }
            current_interval.value_number = current_number;
            match(current_token_type);
            }
            break;
        case e_keyword:
            {
            current_interval.value_is_keyword = true;
            if(extra_keywords.empty())
                {
                diagnostics << "Expected number. ";
                mark_diagnostic_context();
                break;
                }
            if(contains(extra_keywords, current_keyword))
                {
                current_interval.value_keyword = current_keyword;
                match(current_token_type);
                }
            else
                {
                diagnostics
                    << "Expected keyword chosen from { "
                    ;
                std::copy
                    (extra_keywords.begin()
                    ,extra_keywords.end()
                    ,std::ostream_iterator<std::string>(diagnostics, " ")
                    );
                diagnostics << "}. ";
                mark_diagnostic_context();
                }
            }
            break;
        default:
            {
            diagnostics << "Expected number or keyword. ";
            mark_diagnostic_context();
            }
        }
}

// GRAMMAR span: value
// GRAMMAR span: value , duration

void InputSequence::span()
{
    // Assume interval is sane until shown otherwise.
    current_interval.insane = false;
    value();
    switch(current_token_type)
        {
        case e_minor_separator:
            {
            match(e_minor_separator);
            duration();
            }
            break;
        case e_eof:
        case e_major_separator:
        case e_begin_incl:
        case e_begin_excl:
        case e_age_prefix:
        case e_cardinal_prefix:
        case e_number:
        case e_keyword:
            {
            duration();
            }
            break;
        default:
            {
            diagnostics
                << "Expected ';', ',', '@', '#', '[', '(',"
                << " number, 'retirement', or 'maturity'. "
                ;
            mark_diagnostic_context();
            }
        }
    if(!current_interval.insane)
        {
        intervals.push_back(current_interval);
        }
}

// GRAMMAR sequence: span
// GRAMMAR sequence: span ; span
// GRAMMAR sequence: series ;

void InputSequence::sequence()
{
    // Numbers should always be set explicitly, so we initialize
    // them to recognizable, implausible values. Set the 'insane'
    // flag: assume the sequence is insane until known otherwise.
    current_interval.value_number     = -999999999999.999;
    current_interval.value_keyword    = "insane";
    current_interval.value_is_keyword = true;
    current_interval.begin_duration   = -1;
    current_interval.begin_mode       = e_invalid_mode;
    current_interval.end_duration     = -2;
    current_interval.end_mode         = e_invalid_mode;
    current_interval.insane           = true;

    for(;;)
        {
        switch(current_token_type)
            {
            case e_eof:
                {
                return;
                }
            case e_major_separator:
                {
                match(current_token_type);
                if(e_eof == current_token_type)
                    {
                    return;
                    }
                span();
                }
                break;
// TODO ?? Superfluous. Same comment elsewhere. Fall through instead.
            case e_startup:
                {
                match(current_token_type);
                if(e_eof == current_token_type)
                    {
                    return;
                    }
                span();
                }
                break;
            default:
                {
                diagnostics << "Expected ';'. ";
                mark_diagnostic_context();
                return;
                }
            }
        }
}

InputSequence::token_type InputSequence::get_token()
{
    char c = '\0';
    do
        {
        if(!input_stream.get(c))
            {
            // COMPILER !! bc++5.02 puts garbage into 'c': reset to '\0'.
            // I believe [27.6.1.3] doesn't allow the garbage.
            c = '\0';
            break;
            }
        }
    while(is_ok_for_cctype(c) && std::isspace(c));

    switch(c)
        {
        case 0:
            {
            return current_token_type = e_eof;
            }
        case e_major_separator:
        case e_minor_separator:
        case e_begin_incl:
        case e_begin_excl:
        case e_end_incl:
        case e_end_excl:
        case e_age_prefix:
        case e_cardinal_prefix:
            {
            return current_token_type = token_type(c);
            }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '.': case '-':
            {
            input_stream.putback(c);
            // TODO ?? Assigning 0.0 here at least gives a predictable
            // value upon failure. We can't read both '.' and the
            // following character and then reliably put both back.
            current_number = 0.0;
            input_stream >> current_number;
            if(input_stream.fail())
                {
                diagnostics << "Invalid number '" << c << "'. ";
                mark_diagnostic_context();
                }
            return current_token_type = e_number;
            }
        // An arbitrary rule must be selected for keyword names;
        // we choose the regexp '[a-z][a-z0-9_]*'.
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z':
            {
            current_keyword = c;
            while
                (  input_stream.get(c)
                && ((is_ok_for_cctype(c) && std::isalnum(c)) || '_' == c)
                )
                {
                // COMPILER !! 21.3/6 gives push_back() as a member of the
                // string class, and Stroustrup uses it in 6.1.3 of TC++PL3,
                // whence this and other ideas are borrowed, but gcc-2.95.x
                // seems not to support it; operator+=() is likely to be
                // implemented the same way anyway.
                current_keyword += c;
                }
            input_stream.putback(c);
            return current_token_type = e_keyword;
            }
        default:
            {
            diagnostics << "Unknown token '" << c << "'. ";
            mark_diagnostic_context();
            return current_token_type = token_type(0);
            }
        }
}

std::string InputSequence::token_type_name(InputSequence::token_type t)
{
    switch(t)
        {
        case 0:
            {
            return "end of input";
            }
        case e_major_separator:
        case e_minor_separator:
        case e_begin_incl:
        case e_begin_excl:
        case e_end_incl:
        case e_end_excl:
        case e_age_prefix:
        case e_cardinal_prefix:
            {
            std::string s;
            s += static_cast<char>(t);
            return s;
            }
        case e_startup:
            {
            return "beginning of input";
            }
        case e_number:
            {
            return "number";
            }
        case e_keyword:
            {
            return "keyword";
            }
        default:
            {
            return std::string("unknown: ") + static_cast<char>(t);
            }
        }
}

void InputSequence::match(InputSequence::token_type t)
{
    if(current_token_type == t)
        {
        current_token_type = get_token();
        }
    else
        {
        diagnostics
            << "Expected '"
            << token_type_name(t)
            << "' . "
            ;
        mark_diagnostic_context();
        }
}

void InputSequence::mark_diagnostic_context()
{
    diagnostics
        << "Current token '"
        << token_type_name(current_token_type)
        << "' at position " << input_stream.tellg()
        << ".\n"
        ;
}

/// Rationale for option to show only first diagnostic:
/// downstream errors can confuse users.

std::string InputSequence::formatted_diagnostics
    (bool show_first_message_only
    ) const
{
    std::string s(diagnostics.str());
    if(show_first_message_only)
        {
        std::string::size_type z(s.find('\n'));
        if(std::string::npos != z)
            {
            s.erase(z);
            }
        }
    return s;
}

std::vector<double> const& InputSequence::linear_number_representation() const
{
    return number_result;
}

std::vector<std::string> const& InputSequence::linear_keyword_representation() const
{
    return keyword_result;
}

/// Regularized representation in [x,y) interval notation.
///
/// If there's only one interval, it must span all years, so depict it
/// as the simple scalar that it is, specifying no interval.
///
/// Use keyword 'maturity' for the last duration. This avoids
/// gratuitous differences between lives, e.g.
///   '10000 [20,55); 0' for a 45-year-old
/// and
///   '10000 [20,65); 0' for a 35-year-old
/// which the census GUI would treat as varying across cells, whereas
///   '10000 [20,65); maturity'
/// expresses the same sequence uniformly.
///
/// TODO ?? For the same reason, this representation should preserve
/// duration keywords such as 'retirement'.

std::string InputSequence::mathematical_representation() const
{
    std::ostringstream oss;
    for(auto const& interval_i : intervals)
        {
        if(interval_i.value_is_keyword)
            {
            oss << interval_i.value_keyword;
            }
        else
            {
            oss << value_cast<std::string>(interval_i.value_number);
            }

        if(1 == intervals.size())
            {
            break;
            }

        if(interval_i.end_duration != years_to_maturity)
            {
            oss
                << " ["
                << interval_i.begin_duration
                << ", "
                << interval_i.end_duration
                << "); "
                ;
            }
        else
            {
            oss
                << " ["
                << interval_i.begin_duration
                << ", "
                << "maturity"
                << ")"
                ;
            }
        }
    return oss.str();
}

std::vector<ValueInterval> const& InputSequence::interval_representation() const
{
    return intervals;
}

