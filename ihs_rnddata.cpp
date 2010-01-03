// Rounding data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ihs_rnddata.cpp,v 1.9 2008-12-27 02:56:44 chicares Exp $

// Rounding rules for a policy form

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_rnddata.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "platform_dependent.hpp" // access()
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/static_assert.hpp>

#include <fstream>

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
    // for legacy file format, see ReadLegacy() below
    inline std::istream& operator>>(std::istream& is, round_to<double>& r)
        {
        int decimals;
        is >> decimals;
        int z;
        is >> z;
        rounding_style style = static_cast<rounding_style>(z);
        r = round_to<double>(decimals, style);
        return is;
        }
} // Unnamed namespace.

namespace xml_serialize
{
    template<>
    struct type_io< round_to<double> >
    {
        static void to_xml(xml::node& out, round_to<double> const& in)
        {
            add_property(out, "decimals", in.decimals());
            add_property(out, "style", in.style());
        }

        static void from_xml(round_to<double>& out, xml::node const& in)
        {
            int decimals;
            get_property(in, "decimals", decimals);
            rounding_style style;
            get_property(in, "style", style);
            out = round_to<double>(decimals, style);
        }
    };

    template<>
    struct type_io<rounding_style> : public enum_type_io<rounding_style> {};

    // TODO ?? Consider using mc_enum instead.
    template<>
    const enum_type_io_map<rounding_style>::MapEntry
    enum_type_io_map<rounding_style>::map[] =
        { {r_indeterminate, "indeterminate"}
        , {r_toward_zero,   "toward-zero"}
        , {r_to_nearest,    "to-nearest"}
        , {r_upward,        "upward"}
        , {r_downward,      "downward"}
        , {r_current,       "current"}
        , {r_not_at_all,    "not-at-all"}
        };
} // namespace xml_serialize

//============================================================================
void StreamableRoundingRules::ReadLegacy(std::string const& a_Filename)
{
    std::ifstream is(a_Filename.c_str());

    is >> round_specamt_;
    is >> round_death_benefit_;
    is >> round_naar_;
    is >> round_coi_rate_;
    is >> round_coi_charge_;
    is >> round_gross_premium_;
    is >> round_net_premium_;
    is >> round_interest_rate_;
    is >> round_interest_credit_;
    is >> round_withdrawal_;
    is >> round_loan_;
    is >> round_corridor_factor_;
    is >> round_surrender_charge_;
    is >> round_irr_;

    bool okay = is.good();
    if(!okay)
        {
        fatal_error()
            << "Unexpected end of rounding file '"
            << a_Filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
    std::string dummy;
    is >> dummy;
    okay = is.eof();
    if(!okay)
        {
        fatal_error()
            << "Data past expected end of rounding file '"
            << a_Filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void StreamableRoundingRules::Read(std::string const& a_Filename)
{
    if(access(a_Filename.c_str(), R_OK))
        {
        fatal_error()
            << "File '"
            << a_Filename
            << "' is required but could not be found. Try reinstalling."
            ;
        }

    // We temporarily support reading both XML and the old file formats.
    if(".rnd" == fs::extension(a_Filename))
        {
        ReadLegacy(a_Filename);
        return;
        }

    xml_lmi::dom_parser doc(a_Filename);
    xml::element const& root = doc.root_node("rounding");

    using namespace xml_serialize;
    get_property(root, "specamt",          round_specamt_);
    get_property(root, "death_benefit",    round_death_benefit_);
    get_property(root, "naar",             round_naar_);
    get_property(root, "coi_rate",         round_coi_rate_);
    get_property(root, "coi_charge",       round_coi_charge_);
    get_property(root, "gross_premium",    round_gross_premium_);
    get_property(root, "net_premium",      round_net_premium_);
    get_property(root, "interest_rate",    round_interest_rate_);
    get_property(root, "interest_credit",  round_interest_credit_);
    get_property(root, "withdrawal",       round_withdrawal_);
    get_property(root, "loan",             round_loan_);
    get_property(root, "corridor_factor",  round_corridor_factor_);
    get_property(root, "surrender_charge", round_surrender_charge_);
    get_property(root, "irr",              round_irr_);
}

//============================================================================
void StreamableRoundingRules::Write(std::string const& a_Filename)
{
    xml::document doc("rounding");
    xml::node& root = doc.get_root_node();

    using namespace xml_serialize;

    add_property(root, "specamt",          round_specamt_);
    add_property(root, "death_benefit",    round_death_benefit_);
    add_property(root, "naar",             round_naar_);
    add_property(root, "coi_rate",         round_coi_rate_);
    add_property(root, "coi_charge",       round_coi_charge_);
    add_property(root, "gross_premium",    round_gross_premium_);
    add_property(root, "net_premium",      round_net_premium_);
    add_property(root, "interest_rate",    round_interest_rate_);
    add_property(root, "interest_credit",  round_interest_credit_);
    add_property(root, "withdrawal",       round_withdrawal_);
    add_property(root, "loan",             round_loan_);
    add_property(root, "corridor_factor",  round_corridor_factor_);
    add_property(root, "surrender_charge", round_surrender_charge_);
    add_property(root, "irr",              round_irr_);

    if(!doc.save_to_file(a_Filename.c_str()))
        {
        fatal_error()
            << "Unable to write rounding file '"
            << a_Filename
            << "'."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void StreamableRoundingRules::WriteRndFiles()
{
    StreamableRoundingRules sample;
    sample.Write(AddDataDir("sample.xrnd"));
}

