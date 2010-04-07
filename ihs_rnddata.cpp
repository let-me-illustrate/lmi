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

// Rounding rules for a policy form

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_rnddata.hpp"

#include "alert.hpp"
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

//============================================================================
StreamableRoundingRules::StreamableRoundingRules()
{
}

//============================================================================
StreamableRoundingRules::StreamableRoundingRules(std::string const& a_Filename)
{
    Init(a_Filename);
}

//============================================================================
void StreamableRoundingRules::Init(std::string const& a_Filename)
{
    Read(a_Filename);
    round_min_specamt_        = round_to<double>(round_specamt_      .decimals(), r_upward);
    round_max_specamt_        = round_to<double>(round_specamt_      .decimals(), r_downward);
    round_min_premium_        = round_to<double>(round_gross_premium_.decimals(), r_upward);
    round_max_premium_        = round_to<double>(round_gross_premium_.decimals(), r_downward);
    // One might alternatively not round this interest rate at all.
    round_interest_rate_7702_ = round_to<double>(round_interest_rate_.decimals(), r_upward);
}

//============================================================================
rounding_rules const& StreamableRoundingRules::get_rounding_rules()
{
    return dynamic_cast<rounding_rules&>(*this);
}

namespace
{
std::string xml_root_name()
{
    return "rounding";
}
} // Unnamed namespace.

//============================================================================
void StreamableRoundingRules::Read(std::string const& a_Filename)
{
    if(access(a_Filename.c_str(), R_OK))
        {
        fatal_error()
            << "File '"
            << a_Filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    xml_lmi::dom_parser parser(a_Filename);
    xml::element const& root = parser.root_node(xml_root_name());

#   define GET_ELEMENT(name) xml_serialize::get_element(root, #name, round_##name##_)

    GET_ELEMENT(specamt         );
    GET_ELEMENT(death_benefit   );
    GET_ELEMENT(naar            );
    GET_ELEMENT(coi_rate        );
    GET_ELEMENT(coi_charge      );
    GET_ELEMENT(gross_premium   );
    GET_ELEMENT(net_premium     );
    GET_ELEMENT(interest_rate   );
    GET_ELEMENT(interest_credit );
    GET_ELEMENT(withdrawal      );
    GET_ELEMENT(loan            );
    GET_ELEMENT(corridor_factor );
    GET_ELEMENT(surrender_charge);
    GET_ELEMENT(irr             );

#   undef GET_ELEMENT
}

//============================================================================
void StreamableRoundingRules::Write(std::string const& a_Filename) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();

    xml_lmi::set_attr(root, "version", "0");

#   define SET_ELEMENT(name) xml_serialize::set_element(root, #name, round_##name##_)

    SET_ELEMENT(specamt         );
    SET_ELEMENT(death_benefit   );
    SET_ELEMENT(naar            );
    SET_ELEMENT(coi_rate        );
    SET_ELEMENT(coi_charge      );
    SET_ELEMENT(gross_premium   );
    SET_ELEMENT(net_premium     );
    SET_ELEMENT(interest_rate   );
    SET_ELEMENT(interest_credit );
    SET_ELEMENT(withdrawal      );
    SET_ELEMENT(loan            );
    SET_ELEMENT(corridor_factor );
    SET_ELEMENT(surrender_charge);
    SET_ELEMENT(irr             );

#   undef SET_ELEMENT

    // Instead of this:
//    document.save(a_Filename);
    // for the nonce, explicitly change the extension, in order to
    // force external product-file code to use the new extension.
    fs::path path(a_Filename, fs::native);
    path = fs::change_extension(path, ".rounding");
    document.save(path.string());
}

//============================================================================
void StreamableRoundingRules::WriteRndFiles()
{
    StreamableRoundingRules sample;
    sample.Write(AddDataDir("sample.rounding"));
}

