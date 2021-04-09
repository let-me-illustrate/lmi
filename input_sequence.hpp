// Input sequences (e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0...)
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
// the parser TU.
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

// Inchoate ideas for future consideration.
//
// Gather all ctor arguments in an easy-to-manage struct--see:
//   https://lists.nongnu.org/archive/html/lmi/2017-02/msg00037.html
//
// * Overlays
//
// Allow multiple layers of input sequences, like layers in a drawing.
// For example:
//
// This overlay means 0 past retirement:
//     1000 [0, 10); 0 [retirement, maturity)
//   ...@30: 10 yrs...truncate nothing, leaving 10
//   ...@60: 10 yrs...truncate, leaving 5
//
// This overlay means not less than ten years
//     1000 [0, retirement); 1000[0, 10)
//   ...@30: 35 yrs...extend nothing, leaving 35
//   ...@60:  5 yrs...extend to 10
//
// Of course, amounts might even differ, e.g.:
//     1000 [0, retirement); 200[0, 10)
//
// Algorithm:
//   create a years_to_maturity-element std::map<duration, ValueInterval>
//   add each layer in order (from left, probably)
//     crucially, deleting any endpoints underneath
//   traverse the map (from left) producing a vector<ValueInterval>
//     each value-change separates an interval: this is RLE, IOW
//     read mode from map...does right endpoint alone suffice?
//       yes, iff we alter obscured intervals' right endpoints in loop
//       instead...view them as transition-points
//         then each layer simply overwrites any older transition-point
//     ASCII-graphical example:
//       [.)  [.)[...) [..)
//       [........)[....)
//                ^^    ^ obscured: must delete deliberately
//
// This overlay method would answer certain open questions:
//  - Should further RLE be performed on input? E.g.:
//      123; 123; 123 [2,5); 123 maturity
//    At present, such RLE is not performed: if adjacent intervals have
//    the same value, that is taken as intentional. That's significant
//    at least while the GUI is open, because adding multiple intervals
//    reuses the default value until modified--but this would happen
//    after the GUI has closed.
//  - Should overlapping intervals be forbidden? Overlays require that.
// One could imagine a multidimensional GUI editor...

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

#include "input_sequence_interval.hpp"
#include "so_attributes.hpp"

#include <string>
#include <vector>

class LMI_SO InputSequence final
{
    template<typename T>
    friend std::string canonicalized_input_sequence(std::vector<T> const&);

    friend class input_sequence_test;

  public:
    InputSequence
        (std::string const&              input_expression
        ,int                             a_years_to_maturity
        ,int                             a_issue_age
        ,int                             a_retirement_age
        ,int                             a_inforce_duration
        ,int                             a_effective_year
        ,std::vector<std::string> const& a_allowed_keywords = {}
        ,bool                            a_keywords_only    = false
        ,std::string const&              a_default_keyword  = std::string()
        );

    ~InputSequence() = default;

    std::string canonical_form() const;

    std::vector<ValueInterval> const& intervals() const;
    std::vector<std::string>   const& seriatim_keywords() const;
    std::vector<double>        const& seriatim_numbers()  const;

  private:
    InputSequence(InputSequence const&) = delete;
    InputSequence& operator=(InputSequence const&) = delete;

    explicit InputSequence(std::vector<std::string> const&);
    explicit InputSequence(std::vector<double> const&);

    template<typename T>
    void initialize_from_vector(std::vector<T> const&);

    int years_to_maturity_;
    int issue_age_;

    std::vector<ValueInterval> intervals_;
    std::vector<std::string>   seriatim_keywords_;
    std::vector<double>        seriatim_numbers_;
};

template<typename T>
std::string canonicalized_input_sequence(std::vector<T> const& z)
{
    return InputSequence(z).canonical_form();
}

#endif // input_sequence_hpp
