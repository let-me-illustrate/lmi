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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "rounding_rules.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "platform_dependent.hpp" // access()
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

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
}

rounding_rules::rounding_rules(std::string const& filename)
{
    Read(filename);
    LMI_ASSERT(r_not_at_all == round_min_specamt_       .style() || r_upward   == round_min_specamt_       .style());
    LMI_ASSERT(r_not_at_all == round_max_specamt_       .style() || r_downward == round_max_specamt_       .style());
    LMI_ASSERT(r_not_at_all == round_min_premium_       .style() || r_upward   == round_min_premium_       .style());
    LMI_ASSERT(r_not_at_all == round_max_premium_       .style() || r_downward == round_max_premium_       .style());
    LMI_ASSERT(r_not_at_all == round_interest_rate_7702_.style() || r_upward   == round_interest_rate_7702_.style());
}

namespace
{
std::string xml_root_name()
{
    return "rounding";
}
} // Unnamed namespace.

void rounding_rules::Read(std::string const& filename)
{
    if(access(filename.c_str(), R_OK))
        {
        fatal_error()
            << "File '"
            << filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    xml_lmi::dom_parser parser(filename);
    xml::element const& root = parser.root_node(xml_root_name());

    xml_serialize::get_element(root, "RoundSpecAmt"    , round_specamt_           );
    xml_serialize::get_element(root, "RoundDeathBft"   , round_death_benefit_     );
    xml_serialize::get_element(root, "RoundNaar"       , round_naar_              );
    xml_serialize::get_element(root, "RoundCoiRate"    , round_coi_rate_          );
    xml_serialize::get_element(root, "RoundCoiCharge"  , round_coi_charge_        );
    xml_serialize::get_element(root, "RoundGrossPrem"  , round_gross_premium_     );
    xml_serialize::get_element(root, "RoundNetPrem"    , round_net_premium_       );
    xml_serialize::get_element(root, "RoundIntRate"    , round_interest_rate_     );
    xml_serialize::get_element(root, "RoundIntCredit"  , round_interest_credit_   );
    xml_serialize::get_element(root, "RoundWithdrawal" , round_withdrawal_        );
    xml_serialize::get_element(root, "RoundLoan"       , round_loan_              );
    xml_serialize::get_element(root, "RoundCorrFactor" , round_corridor_factor_   );
    xml_serialize::get_element(root, "RoundSurrCharge" , round_surrender_charge_  );
    xml_serialize::get_element(root, "RoundIrr"        , round_irr_               );
    xml_serialize::get_element(root, "RoundMinSpecamt" , round_min_specamt_       );
    xml_serialize::get_element(root, "RoundMaxSpecamt" , round_max_specamt_       );
    xml_serialize::get_element(root, "RoundMinPrem"    , round_min_premium_       );
    xml_serialize::get_element(root, "RoundMaxPrem"    , round_max_premium_       );
    xml_serialize::get_element(root, "RoundIntRate7702", round_interest_rate_7702_);
}

void rounding_rules::Write(std::string const& filename) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();

    xml_lmi::set_attr(root, "version", "0");

    xml_serialize::set_element(root, "RoundSpecAmt"    , round_specamt_           );
    xml_serialize::set_element(root, "RoundDeathBft"   , round_death_benefit_     );
    xml_serialize::set_element(root, "RoundNaar"       , round_naar_              );
    xml_serialize::set_element(root, "RoundCoiRate"    , round_coi_rate_          );
    xml_serialize::set_element(root, "RoundCoiCharge"  , round_coi_charge_        );
    xml_serialize::set_element(root, "RoundGrossPrem"  , round_gross_premium_     );
    xml_serialize::set_element(root, "RoundNetPrem"    , round_net_premium_       );
    xml_serialize::set_element(root, "RoundIntRate"    , round_interest_rate_     );
    xml_serialize::set_element(root, "RoundIntCredit"  , round_interest_credit_   );
    xml_serialize::set_element(root, "RoundWithdrawal" , round_withdrawal_        );
    xml_serialize::set_element(root, "RoundLoan"       , round_loan_              );
    xml_serialize::set_element(root, "RoundCorrFactor" , round_corridor_factor_   );
    xml_serialize::set_element(root, "RoundSurrCharge" , round_surrender_charge_  );
    xml_serialize::set_element(root, "RoundIrr"        , round_irr_               );
    xml_serialize::set_element(root, "RoundMinSpecamt" , round_min_specamt_       );
    xml_serialize::set_element(root, "RoundMaxSpecamt" , round_max_specamt_       );
    xml_serialize::set_element(root, "RoundMinPrem"    , round_min_premium_       );
    xml_serialize::set_element(root, "RoundMaxPrem"    , round_max_premium_       );
    xml_serialize::set_element(root, "RoundIntRate7702", round_interest_rate_7702_);

    // Instead of this:
//    document.save(filename);
    // for the nonce, explicitly change the extension, in order to
    // force external product-file code to use the new extension.
    fs::path path(filename, fs::native);
    path = fs::change_extension(path, ".rounding");
    document.save(path.string());
}

void rounding_rules::write_rounding_files()
{
    rounding_rules sample;
    sample.Write(AddDataDir("sample.rounding"));
}

