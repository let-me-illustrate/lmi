// Input-sequence class for wx data-transfer framework.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "datum_sequence.hpp"

#include "assert_lmi.hpp"

/// Construct from string.

sequence_base::sequence_base(std::string const& s)
    :datum_string_base{s}
{
}

sequence_base& sequence_base::operator=(std::string const& s)
{
    datum_string_base::operator=(s);
    return *this;
}

/// Declare whether numeric values are allowable.

bool sequence_base::numeric_values_are_allowable() const
{
    return true;
}

/// Declare whether keyword values are allowable.

bool sequence_base::keyword_values_are_allowable() const
{
    return false;
}

/// Return default keyword in current context.
///
/// Return empty string if there's no default keyword.

std::string const sequence_base::default_keyword() const
{
    return std::string();
}

/// Ascertain which keywords are allowed in the current context.
///
/// The return value is a map: keyword --> mc_enum string. Rationale:
/// keywords are part of the user interface, and should be easy for
/// users to write and remember; whereas mc_enum strings are internal,
/// and should be easy for domain experts to read--thus, evocative and
/// unambiguous more than terse. For example:
///   - "glp" --> "PmtGLP" // for payments
///   - "glp" --> "SAGLP"  // for specified amount
/// Here, "PmtGLP" can mean only a guideline-level-premium payment
/// strategy, but users wouldn't want to type it; "glp" is easy to
/// type, but its meaning depends upon context. Furthermore, using a
/// map makes it possible to change internal names without affecting
/// the user interface.

std::map<std::string,std::string> const sequence_base::allowed_keywords() const
{
    return std::map<std::string,std::string>();
}

/// Determine whether two objects are equivalent.
///
/// Rationale: to support free function operator==(). See:
///   https://lists.nongnu.org/archive/html/lmi/2010-07/msg00002.html
/// It would be simpler to provide a trivial accessor for the member
/// datum, but maintaining strong encapsulation reduces the temptation
/// for one component of MVC to inspect another's internals.

bool sequence_base::equals(sequence_base const& z) const
{
    return z.value() == value();
}

/// Ensure that input is possible; throw otherwise.
///
/// Input is possible iff either
///   - keyword values are allowable, and at least one is allowed, or
///   - numeric values are allowable.

void sequence_base::assert_sanity() const
{
    LMI_ASSERT
        (  keyword_values_are_allowable() && !allowed_keywords().empty()
        || numeric_values_are_allowable()
        );
}

bool operator==(sequence_base const& lhs, sequence_base const& rhs)
{
    return lhs.equals(rhs);
}

// Numeric MVC input sequence.

numeric_sequence::numeric_sequence()
{
    assert_sanity();
}

numeric_sequence::numeric_sequence(std::string const& s)
    :sequence_base{s}
{
    assert_sanity();
}

numeric_sequence& numeric_sequence::operator=(std::string const& s)
{
    sequence_base::operator=(s);
    return *this;
}

std::map<std::string,std::string> const numeric_sequence::allowed_keywords() const
{
    return sequence_base::allowed_keywords();
}

bool operator==(numeric_sequence const& lhs, numeric_sequence const& rhs)
{
    return lhs.equals(rhs);
}

// MVC input sequence for payments.

payment_sequence::payment_sequence()
{
    assert_sanity();
}

payment_sequence::payment_sequence(std::string const& s)
    :sequence_base{s}
{
    assert_sanity();
}

payment_sequence& payment_sequence::operator=(std::string const& s)
{
    sequence_base::operator=(s);
    return *this;
}

std::map<std::string,std::string> const payment_sequence::allowed_keywords() const
{
    static std::map<std::string,std::string> all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["minimum" ] = "PmtMinimum" ;
        all_keywords["target"  ] = "PmtTarget"  ;
        all_keywords["sevenpay"] = "Pmt7PP"     ;
        all_keywords["glp"     ] = "PmtGLP"     ;
        all_keywords["gsp"     ] = "PmtGSP"     ;
        all_keywords["corridor"] = "PmtCorridor";
        all_keywords["table"   ] = "PmtTable"   ;
        }
    std::map<std::string,std::string> permissible_keywords = all_keywords;

    return permissible_keywords;
}

bool operator==(payment_sequence const& lhs, payment_sequence const& rhs)
{
    return lhs.equals(rhs);
}

// MVC input sequence for payment mode.

mode_sequence::mode_sequence()
{
    assert_sanity();
}

mode_sequence::mode_sequence(std::string const& s)
    :sequence_base{s}
{
    assert_sanity();
}

mode_sequence& mode_sequence::operator=(std::string const& s)
{
    sequence_base::operator=(s);
    return *this;
}

std::string const mode_sequence::default_keyword() const
{
    return "annual";
}

std::map<std::string,std::string> const mode_sequence::allowed_keywords() const
{
    static std::map<std::string,std::string> all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["annual"    ] = "Annual"    ;
        all_keywords["semiannual"] = "Semiannual";
        all_keywords["quarterly" ] = "Quarterly" ;
        all_keywords["monthly"   ] = "Monthly"   ;
        }
    std::map<std::string,std::string> permissible_keywords = all_keywords;
    return permissible_keywords;
}

bool operator==(mode_sequence const& lhs, mode_sequence const& rhs)
{
    return lhs.equals(rhs);
}

// MVC input sequence for specified amount.

specamt_sequence::specamt_sequence()
{
    assert_sanity();
}

specamt_sequence::specamt_sequence(std::string const& s)
    :sequence_base{s}
{
    assert_sanity();
}

specamt_sequence& specamt_sequence::operator=(std::string const& s)
{
    sequence_base::operator=(s);
    return *this;
}

// Cf. Input::permissible_specified_amount_strategy_keywords(), which
// is marked as obsolete but may turn out to be necessary. The crucial
// issue is how to restrict keywords in context. If they are somehow
// restricted here, then perhaps the map shouldn't be static.

std::map<std::string,std::string> const specamt_sequence::allowed_keywords() const
{
    static std::map<std::string,std::string> all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["maximum" ] = "SAMaximum" ;
        all_keywords["target"  ] = "SATarget"  ;
        all_keywords["sevenpay"] = "SA7PP"     ;
        all_keywords["glp"     ] = "SAGLP"     ;
        all_keywords["gsp"     ] = "SAGSP"     ;
        all_keywords["corridor"] = "SACorridor";
        all_keywords["salary"  ] = "SASalary"  ;
        }
    std::map<std::string,std::string> permissible_keywords = all_keywords;

    return permissible_keywords;
}

bool operator==(specamt_sequence const& lhs, specamt_sequence const& rhs)
{
    return lhs.equals(rhs);
}

// MVC input sequence for death benefit option.

dbo_sequence::dbo_sequence()
{
    assert_sanity();
}

dbo_sequence::dbo_sequence(std::string const& s)
    :sequence_base{s}
{
    assert_sanity();
}

dbo_sequence& dbo_sequence::operator=(std::string const& s)
{
    sequence_base::operator=(s);
    return *this;
}

std::string const dbo_sequence::default_keyword() const
{
    return "a";
}

std::map<std::string,std::string> const dbo_sequence::allowed_keywords() const
{
    static std::map<std::string,std::string> all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["a"  ] = "A"  ;
        all_keywords["b"  ] = "B"  ;
        all_keywords["rop"] = "ROP";
        all_keywords["mdb"] = "MDB";
        }
    std::map<std::string,std::string> permissible_keywords = all_keywords;
    return permissible_keywords;
}

bool operator==(dbo_sequence const& lhs, dbo_sequence const& rhs)
{
    return lhs.equals(rhs);
}
