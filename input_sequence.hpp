// Input sequences e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0... : unit test.
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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
// At present, LMI offers separate pre- and post-retirement fields for
// such inputs, and a simple vector of values to accommodate varying
// values that don't fit the [issue, retirement), [retirement, maturity)
// paradigm, such as premiums following the exact pattern
//   premium: 50000 25000 25000 10000 10000...
// but these pardigms are disjoint, so it cannot easily accomodate
//   premium: 50000; 25000 [1, 3); 10000 [3, retirement); 0
// Applying such a rule across a census of 1000 lives requires manually
// editing each life, which is unacceptably tedious.

// Extract the grammar from lines matching the regexp _// GRAMMAR_ in
// the implementation file.
//
// Possible enhancements:
//
// Alternatives to
// 1[0,3);2[3,5);0 --> 1 1 1 2 2 0 0...0
//   Index origin 1 could be a configuration option
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
// illustration several year after issue would logically produce a
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

// Steps to incorporate this into LMI/IHS.
//
// Input fields that need to vary by year:
//
//   2     add-on custodial fee
// 1 2     add-on comp on assets
// 1 2     add-on comp on premium
//   2     non-US corridor
//   2     partial mortality mult
//   2     case assumed assets?
//   2     client tax bracket
//         salary
// 1 2     flat extras
//   2     corp tax bracket
// 1     4 spec amt
// 1   3   dbo
// 1     4 ee pmt
//     3   ee mode
// 1     4 er pmt
//     3   er mode
//         GA int rate
//         SA int rate
// 1       loan
// 1       WD
//   ?     term rider amount [not yet]
//
// 1 = remove associated duration control
// 2 = input class must be changed to vary by year
// 3 = enumerated type
// 4 = hybrid type: floating point + enumerated
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
// This is certainly a string. Distinctions between e.g. age, date, and
// policy year must be preserved, because their interpretation depends
// on context.
//
// Strings might simply be saved exactly as entered. That seems to be
// the least surprising way. However, if we make index origin a
// configurable option, then this method becomes ambiguous at best, and
// at worst
//   1 [0, 1)
// becomes an error in origin one. This could be resolved by storing
// the index origin in the input file, but then it's not really a
// configuration option, and files saved with one convention cannot
// easily and reliably be shared with a different user who prefers a
// different convention. Furthermore, this approach means we can never
// change the grammar without preserving backward compatibility.
//
// Alternatively, we can choose a canonical representation for input file
// storage. Explicit interval notation is probably best because it
// has the greatest flexibility; half-open intervals of the form [x, y)
// are preferable. Someday we might instead define a 'simplest form' that
// might e.g. reduce singletons to simple scalars.
//
// Backward compatibility with old input files will require some effort.
//
// Items that are already vectors in the input class must be
// redone to remove things like
//   VaryingSpecAmt()
//   SetSpecAmt()
//   SpecAmtVaries
//   sSpecAmt
//   MakeSpecAmt
// Old cases including regression test decks must be updated.
// Presumably census cut and paste doesn't need varying amounts.
//
// Validators must be applied to vector input.
//
// All edit fields must be widened or given a wide popup.
// The pre/post-retirement distinction must be removed.
// The 'Varying' tab must be removed.
// Other tabs might be combined: e.g. the 'Face' tab becomes tiny.
//
// New output page(s) for yearly varying input:
// # columns
// 2  tax brackets
// 1  salary
// 1  flat extras
// 1  dbo
// 4  ee and er pmt and mode
// 2  GA and SA interest rates
// 2  loans and WDs
// 1  term rider amount

#ifndef input_sequence_hpp
#define input_sequence_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#include <iosfwd>
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

class LMI_SO InputSequence
    :        private lmi::uncopyable <InputSequence>
    ,virtual private obstruct_slicing<InputSequence>
{
  public:
    InputSequence
        (std::string const& input_expression
        ,int a_last_possible_duration // TODO ?? Prefer maturity age?
        ,int a_issue_age
        ,int a_retirement_age
        ,int a_inforce_duration
        ,int a_effective_year
        ,int a_index_origin
        ,std::vector<std::string> const& a_extra_keywords
            = std::vector<std::string>(0)
        ,std::string const& a_default_keyword = ""
        ,bool a_keywords_only = false
        );

    InputSequence
        (std::vector<double> const&
        );
    InputSequence
        (std::vector<std::string> const&
        );
    InputSequence
        (std::vector<double> const&
        ,std::vector<std::string> const&
        );
    InputSequence(double value_to_retirement, double value_from_retirement);
    ~InputSequence();

    void realize_vector();

    // TODO ?? This prolly ought to return a reference; first, let's see
    // how we'll use it, though.
    std::vector<double> linear_number_representation() const;
    std::vector<std::string> linear_keyword_representation() const;
    std::string element_by_element_representation() const;

    // TODO ?? The two following 'regularized representation' functions
    // are probably not useful. The representation we care about is a
    // std::vector<double>.
    std::string mathematical_representation() const;
    std::string natural_language_representation() const;

    std::vector<ValueInterval> const& interval_representation() const;

    std::string formatted_diagnostics
        (bool show_first_message_only = false
        ) const;

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
    void match(token_type t);

    void mark_diagnostic_context();

    // TODO ?? Something like this may be part of a solution
    // if we decide to permit specification of intervals in random order.
    void sort_intervals();

    std::istringstream input_stream;
    // Maturity (last possible) duration in context of this particular
    // life's issue age.
    int last_possible_duration;
    int issue_age;
    int retirement_age;
    int inforce_duration;
    int effective_year;
    int index_origin;
    std::vector<std::string> extra_keywords;
    std::string default_keyword;
    bool keywords_only;

    token_type current_token_type;
    double current_number;
    std::string current_keyword;
    int current_duration_scalar;
    duration_mode previous_duration_scalar_mode;
    duration_mode current_duration_scalar_mode;

    int last_input_duration;

    std::ostringstream diagnostics;

    ValueInterval current_interval;
    std::vector<ValueInterval> intervals;
    std::vector<double> number_result;
    std::vector<std::string> keyword_result;
};

#endif // input_sequence_hpp

