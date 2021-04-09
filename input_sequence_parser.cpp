// Input sequences (e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0...): parser
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "input_sequence_parser.hpp"

#include "assert_lmi.hpp"
#include "contains.hpp"
#include "miscellany.hpp"               // is_ok_for_cctype(), rtrim()

#include <algorithm>                    // copy()
#include <cctype>                       // isalnum(), isspace()
#include <iterator>                     // ostream_iterator

SequenceParser::SequenceParser
    (std::string const&              input_expression
    ,int                             a_years_to_maturity
    ,int                             a_issue_age
    ,int                             a_retirement_age
    ,int                             a_inforce_duration
    ,int                             a_effective_year
    ,std::vector<std::string> const& a_allowed_keywords
    ,bool                            a_keywords_only
    )
    :input_stream_                  {input_expression}
    ,years_to_maturity_             {a_years_to_maturity}
    ,issue_age_                     {a_issue_age}
    ,retirement_age_                {a_retirement_age}
    ,inforce_duration_              {a_inforce_duration}
    ,effective_year_                {a_effective_year}
    ,allowed_keywords_              {a_allowed_keywords}
    ,keywords_only_                 {a_keywords_only}
{
    // Suppress clang '-Wunused-private-field' warnings:
    stifle_warning_for_unused_value(inforce_duration_);
    stifle_warning_for_unused_value(effective_year_);
    sequence();
    diagnostic_messages_ = diagnostics_.str();
}

std::string SequenceParser::diagnostic_messages() const
{
    return diagnostic_messages_;
}

std::vector<ValueInterval> const& SequenceParser::intervals() const
{
    return intervals_;
}

std::string SequenceParser::token_type_name(SequenceParser::token_type t)
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
        }
    throw "Unreachable--silences a compiler diagnostic.";
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

void SequenceParser::duration_scalar()
{
    switch(current_token_type_)
        {
        case e_age_prefix:
            {
            current_duration_scalar_mode_ = e_attained_age;
            match(current_token_type_);
            }
            break;
        case e_cardinal_prefix:
            {
            current_duration_scalar_mode_ = e_number_of_years;
            match(current_token_type_);
            }
            break;
        case e_number:
            {
            current_duration_scalar_mode_ = e_duration;
            }
            break;
        case e_keyword:
            {
            if("retirement" == current_keyword_)
                {
                current_duration_scalar_mode_ = e_retirement;
                current_duration_scalar_ = retirement_age_ - issue_age_;
                match(e_keyword);
                return;
                }
            else if("maturity" == current_keyword_)
                {
                current_duration_scalar_mode_ = e_maturity;
                current_duration_scalar_ = years_to_maturity_;
                match(e_keyword);
                return;
                }
            else
                {
                diagnostics_
                    << "Expected keyword "
                    << "'retirement' "
                    << " or 'maturity'. "
                    ;
                mark_diagnostic_context();
                }
            }
            break;
        case e_eof:
        case e_major_separator:
        case e_minor_separator:
        case e_begin_incl:
        case e_begin_excl:
        case e_end_incl:
        case e_end_excl:
        case e_startup:
        default:
            {
            diagnostics_ << "Expected keyword, number, '@', or '#'. ";
            mark_diagnostic_context();
            }
        }

    current_duration_scalar_ = static_cast<int>(current_number_);
    match(e_number);

    switch(current_duration_scalar_mode_)
        {
        case e_duration:
            {
            ; // Do nothing.
            }
            break;
        case e_attained_age:
            {
            current_duration_scalar_ -= issue_age_;
            }
            break;
        case e_number_of_years:
            {
            current_duration_scalar_ += last_input_duration_;
            }
            break;
        case e_invalid_mode:
        case e_inception:
        case e_inforce:
        case e_retirement:
        case e_maturity:
        default:
            {
            diagnostics_ << "Expected number, '@', or '#'. ";
            mark_diagnostic_context();
            }
        }
}

void SequenceParser::null_duration()
{
    int           trial_begin_duration =     last_input_duration_;
    duration_mode trial_begin_mode     = previous_duration_scalar_mode_;
    int           trial_end_duration   = 1 + last_input_duration_;
    duration_mode trial_end_mode       = e_duration;
    validate_duration
        (trial_begin_duration
        ,trial_begin_mode
        ,trial_end_duration
        ,trial_end_mode
        );
}

// GRAMMAR duration: duration-scalar

void SequenceParser::single_duration()
{
    duration_scalar();
    int           trial_begin_duration = last_input_duration_;
    duration_mode trial_begin_mode     = previous_duration_scalar_mode_;
    int           trial_end_duration   = current_duration_scalar_;
    duration_mode trial_end_mode       = current_duration_scalar_mode_;
    validate_duration
        (trial_begin_duration
        ,trial_begin_mode
        ,trial_end_duration
        ,trial_end_mode
        );
}

// GRAMMAR duration: interval-begin duration-scalar , duration-scalar interval-end

void SequenceParser::intervalic_duration()
{
    bool begin_excl = (e_begin_excl == current_token_type_);
    match(current_token_type_);
    duration_scalar();
    // Add one to the interval-beginning if it was expressed
    // as exclusive, because we store [begin, end).
    int trial_begin_duration         = current_duration_scalar_ + begin_excl;
    duration_mode trial_begin_mode   = current_duration_scalar_mode_;
    int           trial_end_duration = -1;
    duration_mode trial_end_mode     = e_invalid_mode;
    match(e_minor_separator);
    duration_scalar();
    if
        (  e_end_incl == current_token_type_
        || e_end_excl == current_token_type_
        )
        {
        bool end_incl = (e_end_incl == current_token_type_);
        match(current_token_type_);
        // Add one to the interval-end if it was expressed
        // as inclusive, because we store [begin, end).
        trial_end_duration = current_duration_scalar_ + end_incl;
        trial_end_mode     = current_duration_scalar_mode_;
        }
    else
        {
        diagnostics_ << "Expected ')' or ']'. ";
        mark_diagnostic_context();
        return;
        }
    validate_duration
        (trial_begin_duration
        ,trial_begin_mode
        ,trial_end_duration
        ,trial_end_mode
        );
}

void SequenceParser::validate_duration
    (int           trial_begin_duration
    ,duration_mode trial_begin_mode
    ,int           trial_end_duration
    ,duration_mode trial_end_mode
    )
{
    if(trial_begin_duration < 0)
        {
        current_interval_.insane = true;
        diagnostics_
            << "Interval "
            << "[ " << trial_begin_duration
            << ", " << trial_end_duration << " )"
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
    else if(trial_end_duration <= trial_begin_duration)
        {
        current_interval_.insane = true;
        diagnostics_
            << "Interval "
            << "[ " << trial_begin_duration
            << ", " << trial_end_duration << " )"
            << " is improper: it "
            << "ends before it begins. "
            ;
        mark_diagnostic_context();
        return;
        }
    else if(years_to_maturity_ < trial_end_duration)
        {
        current_interval_.insane = true;
        diagnostics_
            << "Interval "
            << "[ " << trial_begin_duration
            << ", " << trial_end_duration << " )"
            << " is improper: it "
            << "ends after the last possible duration. "
            ;
        mark_diagnostic_context();
        return;
        }
    else if
        (  e_invalid_mode == trial_begin_mode
        || e_invalid_mode == trial_end_mode
        )
        {
        current_interval_.insane = true;
        diagnostics_
            << "Interval "
            << "[ " << trial_begin_duration
            << ", " << trial_end_duration << " )"
            << " has an invalid duration mode. "
            ;
        mark_diagnostic_context();
        return;
        }
    else
        {
        current_interval_.begin_duration = trial_begin_duration;
        current_interval_.end_duration   = trial_end_duration  ;
        current_interval_.begin_mode     = trial_begin_mode    ;
        current_interval_.end_mode       = trial_end_mode      ;
        last_input_duration_ = current_interval_.end_duration;
        }
}

void SequenceParser::duration()
{
    switch(current_token_type_)
        {
        case e_eof:
        case e_major_separator:
            {
            null_duration();
            }
            break;
        case e_age_prefix:
        case e_cardinal_prefix:
        case e_number:
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
        case e_minor_separator:
        case e_end_incl:
        case e_end_excl:
        case e_startup:
        default:
            {
            diagnostics_
                << "Expected number, '[', '(', 'retirement', or 'maturity'. "
                ;
            mark_diagnostic_context();
            }
        }
    previous_duration_scalar_mode_ = current_duration_scalar_mode_;
}

// GRAMMAR value: floating-point-number
// GRAMMAR value: value-constant

void SequenceParser::value()
{
    switch(current_token_type_)
        {
        case e_number:
            {
            current_interval_.value_is_keyword = false;
            if(keywords_only_)
                {
                diagnostics_ << "Expected keyword chosen from { ";
                std::copy
                    (allowed_keywords_.begin()
                    ,allowed_keywords_.end()
                    ,std::ostream_iterator<std::string>(diagnostics_, " ")
                    );
                diagnostics_ << "}. ";
                mark_diagnostic_context();
                break;
                }
            current_interval_.value_number = current_number_;
            match(current_token_type_);
            }
            break;
        case e_keyword:
            {
            current_interval_.value_is_keyword = true;
            if(allowed_keywords_.empty())
                {
                diagnostics_ << "Expected number. ";
                mark_diagnostic_context();
                break;
                }
            if(contains(allowed_keywords_, current_keyword_))
                {
                current_interval_.value_keyword = current_keyword_;
                match(current_token_type_);
                }
            else
                {
                diagnostics_ << "Expected keyword chosen from { ";
                std::copy
                    (allowed_keywords_.begin()
                    ,allowed_keywords_.end()
                    ,std::ostream_iterator<std::string>(diagnostics_, " ")
                    );
                diagnostics_ << "}. ";
                mark_diagnostic_context();
                }
            }
            break;
        case e_eof:
        case e_major_separator:
        case e_minor_separator:
        case e_begin_incl:
        case e_begin_excl:
        case e_end_incl:
        case e_end_excl:
        case e_age_prefix:
        case e_cardinal_prefix:
        case e_startup:
        default:
            {
            diagnostics_ << "Expected number or keyword. ";
            mark_diagnostic_context();
            }
        }
}

// GRAMMAR span: value
// GRAMMAR span: value , duration

void SequenceParser::span()
{
    // Assume interval is sane until shown otherwise.
    current_interval_.insane = false;
    value();
    switch(current_token_type_)
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
        case e_end_incl:
        case e_end_excl:
        case e_startup:
        default:
            {
            diagnostics_
                << "Expected ';', ',', '@', '#', '[', '(',"
                << " number, 'retirement', or 'maturity'. "
                ;
            mark_diagnostic_context();
            }
        }
    if(current_interval_.insane)
        {
        if(diagnostics_.str().empty())
            {
            diagnostics_ << "Internal parser error. ";
            mark_diagnostic_context();
            }
        }
    else
        {
        intervals_.push_back(current_interval_);
        }
}

// GRAMMAR sequence: span
// GRAMMAR sequence: span ; span
// GRAMMAR sequence: series ;

void SequenceParser::sequence()
{
    // All ValueInterval members should be set explicitly, so
    // initialize them now to recognizable, implausible values,
    // such that the interval is insane until modified.
    current_interval_.value_number     = -999999999999.999;
    current_interval_.value_keyword    = "daft";
    current_interval_.value_is_keyword = true;
    current_interval_.begin_duration   = -1;
    current_interval_.begin_mode       = e_invalid_mode;
    current_interval_.end_duration     = -2;
    current_interval_.end_mode         = e_invalid_mode;
    current_interval_.insane           = true;

    for(;;)
        {
        switch(current_token_type_)
            {
            case e_eof:
                {
                return;
                }
            case e_startup:
            case e_major_separator:
                {
                match(current_token_type_);
                if(e_eof == current_token_type_)
                    {
                    return;
                    }
                span();
                }
                break;
            case e_minor_separator:
            case e_begin_incl:
            case e_begin_excl:
            case e_end_incl:
            case e_end_excl:
            case e_age_prefix:
            case e_cardinal_prefix:
            case e_number:
            case e_keyword:
            default:
                {
                diagnostics_ << "Expected ';'. ";
                mark_diagnostic_context();
                return;
                }
            }
        }
}

SequenceParser::token_type SequenceParser::get_token()
{
    char c = '\0';
    do
        {
        if(!input_stream_.get(c))
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
            return current_token_type_ = e_eof;
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
            return current_token_type_ = token_type(c);
            }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '.': case '-':
            {
            // Lookahead is limited to a single character, not because
            // this is an LL(1) grammar (where "1" means one token,
            // not one character), but rather because a std::istream
            // is used for convenience, and calling putback() multiple
            // times may fail. If e.g. '.' or '-' were used elsewhere
            // as well as in numeric tokens, then that convenience
            // might be unaffordable.
            input_stream_.putback(c);
            // Zero-initialize so that parsing can continue with a
            // non-random value in case extraction fails.
            current_number_ = 0.0;
            input_stream_ >> current_number_;
            if(input_stream_.fail())
                {
                diagnostics_ << "Invalid number starting with '" << c << "'. ";
                mark_diagnostic_context();
                }
            return current_token_type_ = e_number;
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
            current_keyword_ = c;
            while
                (  input_stream_.get(c)
                && ((is_ok_for_cctype(c) && std::isalnum(c)) || '_' == c)
                )
                {
                // COMPILER !! 21.3/6 gives push_back() as a member of the
                // string class, and Stroustrup uses it in 6.1.3 of TC++PL3,
                // whence this and other ideas are borrowed, but gcc-2.95.x
                // seems not to support it; operator+=() is likely to be
                // implemented the same way anyway.
                current_keyword_ += c;
                }
            input_stream_.putback(c);
            return current_token_type_ = e_keyword;
            }
        default:
            {
            diagnostics_ << "Unknown token '" << c << "'. ";
            mark_diagnostic_context();
            return current_token_type_ = token_type(0);
            }
        }
}

void SequenceParser::match(SequenceParser::token_type t)
{
    if(current_token_type_ == t)
        {
        current_token_type_ = get_token();
        }
    else
        {
        diagnostics_ << "Expected '" << token_type_name(t) << "' . ";
        mark_diagnostic_context();
        }
}

void SequenceParser::mark_diagnostic_context()
{
    diagnostics_
        << "Current token '"
        << token_type_name(current_token_type_)
        << "' at position " << input_stream_.tellg()
        << ".\n"
        ;
}

/// Extract first substring from a '\n'-delimited exception::what().
///
/// SequenceParser::diagnostic_messages() returns a '\n'-delimited
/// string describing all anomalies diagnosed while parsing an input
/// sequence. When that string is not empty, it is reasonable to throw
/// an exception constructed from it--most generally, in its entirety.
///
/// In the important special case where diagnostics are to be shown to
/// end users, to whom the full multiline set may be overwhelming, use
/// this function where the exception is caught. It extracts only the
/// first line, which is presumably the most helpful element, removing
/// that line's terminal '\n'.
///
/// It then strips anything SequenceParser::mark_diagnostic_context()
/// may have added at the end (and any blanks preceding it), because
/// end users are unlikely to know what a "token" is, or to care about
/// the (origin-zero) offset of the error.
///
/// Precondition: the argument is not empty; throws otherwise.
/// Postcondition: the return value is not empty; throws otherwise.

std::string abridge_diagnostics(char const* what)
{
    std::string s(what);
    LMI_ASSERT(!s.empty());

    std::string::size_type z0(s.find('\n'));
    if(std::string::npos != z0)
        {
        s.erase(z0);
        }

    std::string::size_type z1(s.find("Current token"));
    if(std::string::npos != z1)
        {
        s.erase(z1);
        }
    rtrim(s, " ");

    LMI_ASSERT(!s.empty());
    return s;
}
