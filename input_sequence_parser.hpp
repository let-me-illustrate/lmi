// Input sequences (e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0...): parser
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

// See documentation in main input-sequence header.

#ifndef input_sequence_parser_hpp
#define input_sequence_parser_hpp

#include "config.hpp"

#include "input_sequence_interval.hpp"

#include <sstream>
#include <string>
#include <vector>

class SequenceParser final
{
  public:
    SequenceParser
        (std::string const&              input_expression
        ,int                             a_years_to_maturity
        ,int                             a_issue_age
        ,int                             a_retirement_age
        ,int                             a_inforce_duration
        ,int                             a_effective_year
        ,std::vector<std::string> const& a_allowed_keywords
        ,bool                            a_keywords_only
        );

    // In case it is ever desired to implement these: they might
    // simply copy the "parser products" data members that have
    // public accessors.
    SequenceParser(SequenceParser const&) = delete;
    SequenceParser& operator=(SequenceParser const&) = delete;
    ~SequenceParser() = default;

    std::string diagnostic_messages() const;
    std::vector<ValueInterval> const& intervals() const;

  private:
    enum token_type
        {e_eof             = 0
        ,e_major_separator = ';'
        ,e_minor_separator = ','
        ,e_begin_incl      = '['
        ,e_begin_excl      = '('
        ,e_end_incl        = ']'
        ,e_end_excl        = ')'
        ,e_age_prefix      = '@'
        ,e_cardinal_prefix = '#'
        ,e_number
        ,e_keyword
        ,e_startup
        };
    std::string token_type_name(token_type);

    void duration_scalar();
    void null_duration();
    void single_duration();
    void intervalic_duration();
    void validate_duration
        (int           trial_begin_duration
        ,duration_mode trial_begin_mode
        ,int           trial_end_duration
        ,duration_mode trial_end_mode
        );
    void duration();
    void value();
    void span();
    void sequence();
    token_type get_token();
    void match(token_type);

    void mark_diagnostic_context();

    // Parser products.
    std::string diagnostic_messages_;
    std::vector<ValueInterval> intervals_;

    // Streams for parser input and diagnostic messages.
    std::istringstream input_stream_;
    std::ostringstream diagnostics_;

    // Copies of ctor args that are identical to class InputSequence's.
    int years_to_maturity_;
    int issue_age_;
    int retirement_age_;
    int inforce_duration_;
    int effective_year_;
    std::vector<std::string> allowed_keywords_;
    bool keywords_only_;

    // Parser internals.
    token_type current_token_type_               {e_startup};
    double current_number_                       {0.0};
    std::string current_keyword_                 {};
    int current_duration_scalar_                 {0};
    duration_mode previous_duration_scalar_mode_ {e_inception};
    duration_mode current_duration_scalar_mode_  {e_inception};
    ValueInterval current_interval_              {};
    int last_input_duration_                     {0};
};

#endif // input_sequence_parser_hpp
