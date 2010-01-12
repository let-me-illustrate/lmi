// Rounding data.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_rnddata.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "platform_dependent.hpp" // access()

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
    inline std::ostream& operator<<(std::ostream& os, round_to<double> const& r)
        {
        os << r.decimals() << '\n';
        os << r.style() << '\n';
        return os;
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
            ;
        }
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
void StreamableRoundingRules::Write(std::string const& a_Filename)
{
    std::ofstream os(a_Filename.c_str());

    os << round_specamt_;
    os << round_death_benefit_;
    os << round_naar_;
    os << round_coi_rate_;
    os << round_coi_charge_;
    os << round_gross_premium_;
    os << round_net_premium_;
    os << round_interest_rate_;
    os << round_interest_credit_;
    os << round_withdrawal_;
    os << round_loan_;
    os << round_corridor_factor_;
    os << round_surrender_charge_;
    os << round_irr_;

    if(!os.good())
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
    sample.Write(AddDataDir("sample.rnd"));
}

