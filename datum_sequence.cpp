// Input-sequence class for wx data-transfer framework.
//
// Copyright (C) 2010 Gregory W. Chicares.
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

#include "datum_sequence.hpp"

#include "assert_lmi.hpp"

/// Default constructor.
///
/// Throws if insane.

datum_sequence::datum_sequence()
    :keyword_values_are_blocked_(false)
{
    assert_sanity();
}

/// Construct from string.
///
/// Throws if insane.

datum_sequence::datum_sequence(std::string const& s)
    :datum_string(s)
    ,keyword_values_are_blocked_(false)
{
    assert_sanity();
}

datum_sequence::~datum_sequence()
{
}

datum_sequence& datum_sequence::operator=(std::string const& s)
{
    datum_string::operator=(s);
    return *this;
}

/// Block or unblock keyword values.
///
/// This has no practical effect if keyword values are not allowable.
///
/// Throws if insane.

void datum_sequence::block_keyword_values(bool z)
{
    keyword_values_are_blocked_ = z;
    assert_sanity();
}

/// Declare whether numeric values are allowable.

bool datum_sequence::numeric_values_are_allowable() const
{
    return true;
}

/// Declare whether keyword values are allowable.
///
/// Even if they are allowable, they may be blocked.

bool datum_sequence::keyword_values_are_allowable() const
{
    return false;
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

std::map<std::string,std::string> const datum_sequence::allowed_keywords() const
{
    return std::map<std::string,std::string>();
}

/// Determine whether two objects are equivalent.
///
/// Rationale: to support free function operator==(). See:
///   http://lists.nongnu.org/archive/html/lmi/2010-07/msg00002.html
/// It would be simpler to provide a trivial accessor for the member
/// datum, but maintaining strong encapsulation reduces the temptation
/// for one component of MVC to inspect another's internals.

bool datum_sequence::equals(datum_sequence const& z) const
{
    return
           z.value()                     == value()
        && z.keyword_values_are_blocked_ == keyword_values_are_blocked_
        ;
}

/// Determine whether keywords are blocked.
///
/// Rationale: to support allowed_keywords() in derived classes.
///
/// It would be simple to provide a public accessor for the member
/// datum, but maintaining strong encapsulation reduces the temptation
/// for one component of MVC to inspect another's internals.

bool datum_sequence::keyword_values_are_blocked() const
{
    return keyword_values_are_blocked_;
}

/// Ensure that input is possible; throw otherwise.
///
/// Input is possible iff either
///   - keyword values are allowable and not blocked; or
///   - numeric values are allowable.
/// For the nonce at least, the first condition doesn't require
/// allowed_keywords() to return a non-empty map; that can be
/// considered as experience emerges with derived classes.

void datum_sequence::assert_sanity() const
{
    LMI_ASSERT
        (  (keyword_values_are_allowable() && !keyword_values_are_blocked_)
        ||  numeric_values_are_allowable()
        );
}

bool operator==(datum_sequence const& lhs, datum_sequence const& rhs)
{
    return lhs.equals(rhs);
}

// Payments.

payment_sequence& payment_sequence::operator=(std::string const& s)
{
    datum_sequence::operator=(s);
    return *this;
}

std::map<std::string,std::string> const payment_sequence::allowed_keywords() const
{
    if(keyword_values_are_blocked())
        {
        return std::map<std::string,std::string>();
        }

    static std::map<std::string,std::string> all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["minimum" ] = "PmtMinimum"      ;
        all_keywords["target"  ] = "PmtTarget"       ;
        all_keywords["sevenpay"] = "PmtMEP"          ;
        all_keywords["glp"     ] = "PmtGLP"          ;
        all_keywords["gsp"     ] = "PmtGSP"          ;
        all_keywords["corridor"] = "PmtCorridor"     ;
        all_keywords["table"   ] = "PmtTable"        ;
        all_keywords["none"    ] = "PmtInputScalar"  ;
        }
    std::map<std::string,std::string> permissible_keywords = all_keywords;
    permissible_keywords.erase("none");

    return permissible_keywords;
}

bool operator==(payment_sequence const& lhs, payment_sequence const& rhs)
{
    return lhs.equals(rhs);
}

// Payment modes.

mode_sequence& mode_sequence::operator=(std::string const& s)
{
    datum_sequence::operator=(s);
    return *this;
}

std::map<std::string,std::string> const mode_sequence::allowed_keywords() const
{
    LMI_ASSERT(!keyword_values_are_blocked());
    static std::map<std::string,std::string> all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["annual"    ] = "Annual";
        all_keywords["semiannual"] = "Semiannual";
        all_keywords["quarterly" ] = "Quarterly";
        all_keywords["monthly"   ] = "Monthly";
        }
    std::map<std::string,std::string> permissible_keywords = all_keywords;
    return permissible_keywords;
}

bool operator==(mode_sequence const& lhs, mode_sequence const& rhs)
{
    return lhs.equals(rhs);
}

