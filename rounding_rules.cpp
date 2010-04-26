// Rounding rules.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "rounding_rules.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp" // AddDataDir()
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

template class xml_serializable<rounding_rules>;

namespace xml_serialize
{
template<> struct xml_io<round_to<double> >
{
    static void to_xml(xml::element& e, round_to<double> const& t)
    {
        set_element(e, "decimals",                    t.decimals() );
        set_element(e, "style"   , mce_rounding_style(t.style   ()));
    }

    static void from_xml(xml::element const& e, round_to<double>& t)
    {
        int                decimals;
        mce_rounding_style style   ;
        get_element(e, "decimals", decimals);
        get_element(e, "style"   , style   );
        t = round_to<double>(decimals, style.value());
    }
};
} // namespace xml_serialize

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::str()
/// which is not useful here.

template<> std::string value_cast<std::string>(round_to<double> const&)
{
    fatal_error() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::operator=(std::string const&)
/// which is not useful here.

template<> round_to<double> value_cast<round_to<double> >(std::string const&)
{
    fatal_error() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Private default ctor.

rounding_rules::rounding_rules()
    :round_specamt_           (0, r_upward    )
    ,round_death_benefit_     (2, r_to_nearest)
    ,round_naar_              (2, r_to_nearest)
    ,round_coi_rate_          (8, r_downward  )
    ,round_coi_charge_        (2, r_to_nearest)
    ,round_gross_premium_     (2, r_to_nearest)
    ,round_net_premium_       (2, r_to_nearest)
    ,round_interest_rate_     (0, r_not_at_all)
    ,round_interest_credit_   (2, r_to_nearest)
    ,round_withdrawal_        (2, r_to_nearest)
    ,round_loan_              (2, r_to_nearest)
    ,round_corridor_factor_   (2, r_to_nearest)
    ,round_surrender_charge_  (2, r_to_nearest)
    ,round_irr_               (5, r_downward  )
    ,round_min_specamt_       (0, r_upward    )
    ,round_max_specamt_       (0, r_downward  )
    ,round_min_premium_       (2, r_upward    )
    ,round_max_premium_       (2, r_downward  )
    ,round_interest_rate_7702_(0, r_not_at_all)
{
    ascribe_members();
}

/// Construct from filename.
///
/// Postcondition: rounding direction is appropriate for every rule
/// used in 7702 and 7702A calculations.
///
/// RoundingDocument::WriteDocument() may (defectively) write files
/// that violate this precondition, but this ctor prevents them from
/// being used to produce illustrations.

rounding_rules::rounding_rules(std::string const& filename)
{
    ascribe_members();

    load(filename);

    LMI_ASSERT(r_not_at_all == round_min_specamt_       .style() || r_upward   == round_min_specamt_       .style());
    LMI_ASSERT(r_not_at_all == round_max_specamt_       .style() || r_downward == round_max_specamt_       .style());
    LMI_ASSERT(r_not_at_all == round_min_premium_       .style() || r_upward   == round_min_premium_       .style());
    LMI_ASSERT(r_not_at_all == round_max_premium_       .style() || r_downward == round_max_premium_       .style());
    LMI_ASSERT(r_not_at_all == round_interest_rate_7702_.style() || r_upward   == round_interest_rate_7702_.style());
}

rounding_rules::~rounding_rules()
{
}

/// Member datum nominated by the given name.

round_to<double> const& rounding_rules::datum(std::string const& name)
{
    return *member_cast<round_to<double> >(operator[](name));
}

/// Enregister certain data members for access via any_member<>[].

void rounding_rules::ascribe_members()
{
    ascribe("RoundSpecAmt"    , &rounding_rules::round_specamt_           );
    ascribe("RoundDeathBft"   , &rounding_rules::round_death_benefit_     );
    ascribe("RoundNaar"       , &rounding_rules::round_naar_              );
    ascribe("RoundCoiRate"    , &rounding_rules::round_coi_rate_          );
    ascribe("RoundCoiCharge"  , &rounding_rules::round_coi_charge_        );
    ascribe("RoundGrossPrem"  , &rounding_rules::round_gross_premium_     );
    ascribe("RoundNetPrem"    , &rounding_rules::round_net_premium_       );
    ascribe("RoundIntRate"    , &rounding_rules::round_interest_rate_     );
    ascribe("RoundIntCredit"  , &rounding_rules::round_interest_credit_   );
    ascribe("RoundWithdrawal" , &rounding_rules::round_withdrawal_        );
    ascribe("RoundLoan"       , &rounding_rules::round_loan_              );
    ascribe("RoundCorrFactor" , &rounding_rules::round_corridor_factor_   );
    ascribe("RoundSurrCharge" , &rounding_rules::round_surrender_charge_  );
    ascribe("RoundIrr"        , &rounding_rules::round_irr_               );
    ascribe("RoundMinSpecamt" , &rounding_rules::round_min_specamt_       );
    ascribe("RoundMaxSpecamt" , &rounding_rules::round_max_specamt_       );
    ascribe("RoundMinPrem"    , &rounding_rules::round_min_premium_       );
    ascribe("RoundMaxPrem"    , &rounding_rules::round_max_premium_       );
    ascribe("RoundIntRate7702", &rounding_rules::round_interest_rate_7702_);
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20100407T1144Z

int rounding_rules::class_version() const
{
    return 0;
}

std::string rounding_rules::xml_root_name() const
{
    return "rounding";
}

/// This override doesn't call redintegrate_ex_ante(); that wouldn't
/// make sense, at least not for now.

void rounding_rules::read_element
    (xml::element const& e
    ,std::string const&  name
    ,int                 // file_version
    )
{
    round_to<double>& r = *member_cast<round_to<double> >(operator[](name));
    xml_serialize::from_xml(e, r);
}

void rounding_rules::write_element
    (xml::element&       parent
    ,std::string const&  name
    ) const
{
    round_to<double> const& r = *member_cast<round_to<double> >(operator[](name));
    xml_serialize::set_element(parent, name, r);
}

void rounding_rules::write_rounding_files()
{
    rounding_rules sample;
    sample.save(AddDataDir("sample.rounding"));
}

