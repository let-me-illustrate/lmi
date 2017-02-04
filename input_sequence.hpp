// Input sequences e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0....
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

// Motivation.
//
// Many illustration inputs must potentially vary by year. For instance,
// a premium might be paid only from issue to retirement, followed by
// loans from retirement through maturity:
//   premium: 10000 [0, retirement)
//   loan: 5000 [retirement, maturity)
// A really useful census manager must accept rules like that and apply
// them to lives with various retirement and maturity durations--so its
// meaning can be resolved only in the context of each life. It cannot
// be expressed as a simple vector of values:
//   premium: 10000, 10000, ...[how many?], 0, 0, ...[how many?]
// A long time ago, lmi offered separate pre- and post-retirement fields
// for such inputs, and a simple vector of values to accommodate varying
// values that don't fit the [issue, retirement), [retirement, maturity)
// paradigm, such as premiums following the exact pattern
//   premium: 50000 25000 25000 10000 10000...
// but these paradigms are disjoint, so it could not easily accommodate
//   premium: 50000; 25000 [1, 3); 10000 [3, retirement); 0
// Applying such a rule across a census of 1000 lives required manually
// editing each life, which was unacceptably tedious.

// Extract the grammar from lines matching the regexp _// GRAMMAR_ in
// the implementation file.
//
// Index origin is always zero. This is actually natural for end users,
// who think in terms of endpoints--inception implicitly being duration
// zero, and each postinitial interval implicitly beginning at the prior
// interval's endpoint, inclusive. Thus, a single-premium policy allows
// payments in "year one" only, i.e., duration [0,1): from issue up to
// but not including the first anniversary.
//
// Possible enhancements:
//
// Alternatives to
// 1[0,3);2[3,5);0 --> 1 1 1 2 2 0 0...0
//   Keywords could be used instead, e.g.
//     1 from 1 to 3; 2 from 3 through 4; 0
//
// Additional keywords for durations
//   inforce
//   life expectancy [perhaps]
// [Note: Life expectancy would be nice to offer, but it's tricky because
// it's not readily available to input modules. A variety of mortality
// tables would need to be offered, so expectancy can't be stored in a
// table. This means that it is not evident at input time whether an
// interval such as [@75, expectancy) is improper, yet one of our key
// principles is that the input modules should never accept invalid input.
//
// That problem could be overcome by making expectancy available to the
// input modules. But then changing the mortality table could change the
// validity of all intervals involving expectancy. And rerunning an
// illustration several years after issue would logically produce a
// different recalculated expectancy; that would need to be explained
// to naive users. Furthermore, appropriate footnotes would need to be
// written, and consumers might still be confused: it's not evident to
// most that they really should live longer than their expectancy
// calculated under the statutory valuation table, for example. And at
// any rate many changes can occur only on anniversary due to the nature
// of illustrations, so we'd have to round expectancy and disclose that.
// The problems are not insuperable, but it's not obvious that this is
// worth the trip. --end note]
//
// Alternative ways to specify duration
//   YYYY, possibly with a prefix such as 'CY'
//
// Subexpressions for durations
//   duration-keyword +|- integer
// [Note: One might want level loans for fifteen years after retirement
//   [retirement, retirement+15]
// but it is not useful to support [20, 20+15) because that can easily be
// entered as [20, 35). --end note]
//
// Other conceivable enhancements:
//
// Subexpressions for amounts, e.g.
//   1.5 * salary
//   100000 increasing 5% annually

// GUI considerations.
//
// Hybrid types: A really useful facility for entering premiums must
// permit simple strategies as well as amounts: for instance, target
// premium for twenty years, then 5000 per year to retirement, then zero.
//
// We might provide parallel controls for amounts and strategies.
//   premium amount:          0 [0, 20); 5000 [20, retirement); 0
//   premium strategy:   target [0, 20);    0 [20, retirement); 0
// That seems confusing: e.g., the initial premium is given as zero, but
// it is not zero. We might instead introduce a keyword 'strategy' and say
//   premium amount:   strategy [0, 20); 5000 [20, retirement); 0
//   premium strategy:   target [0, 20);    0 [20, retirement); 0
// but it would seem much clearer to have a single control and specify
//   premium:            target [0, 20); 5000 [20, retirement); 0
// Therefore the meaning of the premium control must be overridden and
// linked to both premium amount and premium strategy.
//
// Representation in input file.
//
// Strings are stored in the least surprising way: exactly as entered.
//
// Alternatively, we can choose a canonical representation for input file
// storage. Explicit interval notation is probably best because it
// has the greatest flexibility; half-open intervals of the form [x, y)
// are preferable. Someday we might instead define a 'simplest form' that
// might e.g. reduce singletons to simple scalars.

#ifndef input_sequence_hpp
#define input_sequence_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#include <string>
#include <sstream>
#include <vector>

enum duration_mode
    {e_invalid_mode
    ,e_duration
    ,e_attained_age
    ,e_number_of_years
    ,e_inception
    ,e_inforce
    ,e_retirement
    ,e_maturity
    };

// value applies throughout the interval [begin_duration, end_duration).
//   where value means value_keyword if value_is_keyword, else value_number
// 'insane' flags instances that are syntactically valid but semantically
// invalid, such as improper intervals e.g. [5, 3).
struct ValueInterval
{
    ValueInterval();

    double        value_number;
    std::string   value_keyword;
    bool          value_is_keyword;
    int           begin_duration;
    duration_mode begin_mode;
    int           end_duration;
    duration_mode end_mode;
    bool          insane;
};

class SequenceParser
    :        private lmi::uncopyable <SequenceParser>
    ,virtual private obstruct_slicing<SequenceParser>
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

    ~SequenceParser();

    std::string diagnostics() const;
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
        (int           tentative_begin_duration
        ,duration_mode tentative_begin_duration_mode
        ,int           tentative_end_duration
        ,duration_mode tentative_end_duration_mode
        );
    void duration();
    void value();
    void span();
    void sequence();
    token_type get_token();
    void match(token_type);

    void mark_diagnostic_context();

    std::istringstream input_stream_;

    // Copies of ctor args that are identical to class InputSequence's.
    int years_to_maturity_;
    int issue_age_;
    int retirement_age_;
    int inforce_duration_;
    int effective_year_;
    std::vector<std::string> allowed_keywords_;
    bool keywords_only_;

    token_type current_token_type_;
    double current_number_;
    std::string current_keyword_;
    int current_duration_scalar_;
    duration_mode previous_duration_scalar_mode_;
    duration_mode current_duration_scalar_mode_;
    ValueInterval current_interval_;
    int last_input_duration_;

    std::ostringstream diagnostics_;

    std::vector<ValueInterval> intervals_;
};

class LMI_SO InputSequence
    :        private lmi::uncopyable <InputSequence>
    ,virtual private obstruct_slicing<InputSequence>
{
  public:
    InputSequence
        (std::string const&              input_expression
        ,int                             a_years_to_maturity
        ,int                             a_issue_age
        ,int                             a_retirement_age
        ,int                             a_inforce_duration
        ,int                             a_effective_year
        ,std::vector<std::string> const& a_allowed_keywords
            = std::vector<std::string>()
        ,bool                            a_keywords_only = false
        ,std::string const&              a_default_keyword = ""
        );

    InputSequence(std::vector<double> const&);
    InputSequence(std::vector<std::string> const&);
    InputSequence(std::vector<double> const&, std::vector<std::string> const&);

    ~InputSequence();

    std::vector<double>      const& linear_number_representation()  const;
    std::vector<std::string> const& linear_keyword_representation() const;

    std::string mathematical_representation() const;

    std::vector<ValueInterval> const& interval_representation() const;

    std::string formatted_diagnostics
        (bool show_first_message_only = false
        ) const;

  private:
    void realize_vector();

    // Copies of ctor args that are identical to class SequenceParser's.
    // Most of these copies are unused as this is written in 2017-01;
    // they're retained in case a use is someday found for them.
    int years_to_maturity_;
    int issue_age_;
    int retirement_age_;
    int inforce_duration_;
    int effective_year_;
    std::vector<std::string> allowed_keywords_;
    bool keywords_only_;
    // Copy of a ctor arg that is unique to this class.
    std::string default_keyword_;

    std::string parser_diagnostics_;

    std::vector<ValueInterval> intervals_;
    std::vector<double> number_result_;
    std::vector<std::string> keyword_result_;
};

#endif // input_sequence_hpp

