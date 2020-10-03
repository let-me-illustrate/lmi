// Rounding rules.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "rounding_rules.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "my_proem.hpp"                 // ::write_proem()
#include "path.hpp"
#include "xml_serialize.hpp"

template class xml_serializable<rounding_rules>;

namespace xml_serialize
{
template<> struct xml_io<rounding_parameters>
{
    static void to_xml(xml::element& e, rounding_parameters const& t)
    {
        set_element(e, "decimals", t.decimals());
        set_element(e, "style"   , t.style   ());
        set_element(e, "gloss"   , t.gloss   ());
    }

    static void from_xml(xml::element const& e, rounding_parameters& t)
    {
        int                decimals;
        mce_rounding_style style   ;
        std::string        gloss   ;
        get_element(e, "decimals", decimals);
        get_element(e, "style"   , style   );
        get_element(e, "gloss"   , gloss   );
        t = rounding_parameters(decimals, style.value(), gloss);
    }
};
} // namespace xml_serialize

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::str()
/// which is not useful here.

template<>
std::string value_cast<std::string>(rounding_parameters const&)
{
    alarum() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::operator=(std::string const&)
/// which is not useful here.

template<>
rounding_parameters value_cast<rounding_parameters>(std::string const&)
{
    alarum() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Construct from components.
///
/// The type of the 'style' argument is 'rounding_style', rather than
/// 'mce_rounding_style'; the former is easier to work with, though
/// the latter might have been expected here because it is the type of
/// the corresponding member datum.

rounding_parameters::rounding_parameters
    (int                decimals
    ,rounding_style     style
    ,std::string const& gloss
    )
    :decimals_ {decimals}
    ,style_    {style   }
    ,gloss_    {gloss   }
{
}

bool rounding_parameters::operator==(rounding_parameters const& z) const
{
    return z.decimals_ == decimals_ && z.style_ == style_ && z.gloss_ == gloss_;
}

int rounding_parameters::decimals() const
{
    return decimals_;
}

mce_rounding_style const& rounding_parameters::style() const
{
    return style_;
}

std::string const& rounding_parameters::gloss() const
{
    return gloss_;
}

rounding_style rounding_parameters::raw_style() const
{
    return style_.value();
}

/// Private default ctor for friend class and write_rounding_files().

rounding_rules::rounding_rules()
    :round_specamt_           (0, r_upward    , "")
    ,round_death_benefit_     (2, r_to_nearest, "")
    ,round_naar_              (2, r_to_nearest, "")
    ,round_coi_rate_          (8, r_downward  , "")
    ,round_coi_charge_        (2, r_to_nearest, "")
    ,round_rider_charges_     (2, r_to_nearest, "")
    ,round_gross_premium_     (2, r_to_nearest, "")
    ,round_net_premium_       (2, r_to_nearest, "")
    ,round_interest_rate_     (0, r_not_at_all, "")
    ,round_interest_credit_   (2, r_to_nearest, "")
    ,round_withdrawal_        (2, r_to_nearest, "")
    ,round_loan_              (2, r_to_nearest, "")
    ,round_interest_rate_7702_(0, r_not_at_all, "")
    ,round_corridor_factor_   (2, r_to_nearest, "")
    ,round_nsp_rate_7702_     (6, r_downward  , "")
    ,round_seven_pay_rate_    (6, r_downward  , "")
    ,round_surrender_charge_  (2, r_to_nearest, "")
    ,round_irr_               (4, r_downward  , "")
    ,round_min_specamt_       (0, r_upward    , "")
    ,round_max_specamt_       (0, r_downward  , "")
    ,round_min_premium_       (2, r_upward    , "")
    ,round_max_premium_       (2, r_downward  , "")
    ,round_minutiae_          (2, r_to_nearest, "")
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

    // Settings such as these (called in all ctors):
    //   round_interest_rate_7702_.style_.allow(0, false); // r_indeterminate
    //   round_interest_rate_7702_.style_.allow(1, false); // r_toward_zero
    //   round_interest_rate_7702_.style_.allow(2, false); // r_to_nearest
    //   round_interest_rate_7702_.style_.allow(3, true ); // r_upward
    //   round_interest_rate_7702_.style_.allow(4, false); // r_downward
    //   round_interest_rate_7702_.style_.allow(5, false); // r_current
    //   round_interest_rate_7702_.style_.allow(6, true ); // r_not_at_all
    // wouldn't affect class RoundingDocument or class RoundingButtons,
    // which don't use lmi's mvc framework. The assertions written after
    // load() is called provide adequate though inconvenient safety.

    load(filename);

    LMI_ASSERT(r_not_at_all == round_interest_rate_7702_.style() || r_upward   == round_interest_rate_7702_.style());
    LMI_ASSERT(r_not_at_all == round_min_specamt_       .style() || r_upward   == round_min_specamt_       .style());
    LMI_ASSERT(r_not_at_all == round_max_specamt_       .style() || r_downward == round_max_specamt_       .style());
    LMI_ASSERT(r_not_at_all == round_min_premium_       .style() || r_upward   == round_min_premium_       .style());
    LMI_ASSERT(r_not_at_all == round_max_premium_       .style() || r_downward == round_max_premium_       .style());
}

/// Member datum nominated by the given name.

rounding_parameters const& rounding_rules::datum(std::string const& name) const
{
    return *member_cast<rounding_parameters>(operator[](name));
}

/// Enregister certain data members for access via any_member<>[].

void rounding_rules::ascribe_members()
{
    ascribe("RoundSpecAmt"     , &rounding_rules::round_specamt_           );
    ascribe("RoundDeathBft"    , &rounding_rules::round_death_benefit_     );
    ascribe("RoundNaar"        , &rounding_rules::round_naar_              );
    ascribe("RoundCoiRate"     , &rounding_rules::round_coi_rate_          );
    ascribe("RoundCoiCharge"   , &rounding_rules::round_coi_charge_        );
    ascribe("RoundRiderCharges", &rounding_rules::round_rider_charges_     );
    ascribe("RoundGrossPrem"   , &rounding_rules::round_gross_premium_     );
    ascribe("RoundNetPrem"     , &rounding_rules::round_net_premium_       );
    ascribe("RoundIntRate"     , &rounding_rules::round_interest_rate_     );
    ascribe("RoundIntCredit"   , &rounding_rules::round_interest_credit_   );
    ascribe("RoundWithdrawal"  , &rounding_rules::round_withdrawal_        );
    ascribe("RoundLoan"        , &rounding_rules::round_loan_              );
    ascribe("RoundIntRate7702" , &rounding_rules::round_interest_rate_7702_);
    ascribe("RoundCorrFactor"  , &rounding_rules::round_corridor_factor_   );
    ascribe("RoundNspRate7702" , &rounding_rules::round_nsp_rate_7702_     );
    ascribe("RoundSevenPayRate", &rounding_rules::round_seven_pay_rate_    );
    ascribe("RoundSurrCharge"  , &rounding_rules::round_surrender_charge_  );
    ascribe("RoundIrr"         , &rounding_rules::round_irr_               );
    ascribe("RoundMinSpecamt"  , &rounding_rules::round_min_specamt_       );
    ascribe("RoundMaxSpecamt"  , &rounding_rules::round_max_specamt_       );
    ascribe("RoundMinPrem"     , &rounding_rules::round_min_premium_       );
    ascribe("RoundMaxPrem"     , &rounding_rules::round_max_premium_       );
    ascribe("RoundMinutiae"    , &rounding_rules::round_minutiae_          );
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20100407T1144Z

int rounding_rules::class_version() const
{
    return 0;
}

std::string const& rounding_rules::xml_root_name() const
{
    static std::string const s("rounding");
    return s;
}

/// This override doesn't call redintegrate_ex_ante(); that wouldn't
/// make sense, at least not for now.

void rounding_rules::read_element
    (xml::element const& e
    ,std::string const&  name
    ,int                 // file_version
    )
{
    rounding_parameters& r = *member_cast<rounding_parameters>(operator[](name));
    xml_serialize::from_xml(e, r);
}

void rounding_rules::write_element
    (xml::element&       parent
    ,std::string const&  name
    ) const
{
    rounding_parameters const& r = *member_cast<rounding_parameters>(operator[](name));
    xml_serialize::set_element(parent, name, r);
}

void rounding_rules::write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     file_leaf_name
    ) const
{
    ::write_proem(document, file_leaf_name);
}

void rounding_rules::write_rounding_files()
{
    rounding_rules sample;
    sample.save(AddDataDir("sample.rounding"));
}

/// Load from file. This free function can be invoked across dll
/// boundaries, even though xml_serializable<> is instantiated only
/// in the present TU.

void load(rounding_rules& z, fs::path const& path)
{
    z.xml_serializable<rounding_rules>::load(path);
}

/// Save to file. This free function can be invoked across dll
/// boundaries, even though xml_serializable<> is instantiated only
/// in the present TU.

void save(rounding_rules const& z, fs::path const& path)
{
    z.xml_serializable<rounding_rules>::save(path);
}
