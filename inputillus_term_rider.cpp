// Special handling for proportional term rider.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: inputillus_term_rider.cpp,v 1.4 2007-01-27 00:00:51 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputillus.hpp"
#include "value_cast.hpp"

#include <stdexcept>

//============================================================================
void IllusInputParms::make_term_rider_consistent(bool aggressively)
{
    // TODO ?? Rounding needs to be performed.

    if(e_no == Status[0].TermUseProportion)
        {
        double term_spec_amt   = Status[0].TermAmt;
        double base_spec_amt   = SpecAmt[0];
        double total_spec_amt  = term_spec_amt + base_spec_amt;
        double term_proportion = 0.0;
        if(0.0 != total_spec_amt)
            {
            term_proportion = term_spec_amt / total_spec_amt;
            }

        Status[0].TotalSpecAmt = total_spec_amt;
        Status[0].TermProportion = term_proportion;
        }
    else if(e_yes == Status[0].TermUseProportion)
        {
        double total_spec_amt  = Status[0].TotalSpecAmt;
        double term_proportion = Status[0].TermProportion;
        double term_spec_amt   = total_spec_amt * term_proportion;
        // TODO ?? Round 'term_spec_amt' appropriately here.
        Status[0].TermAmt = term_spec_amt;

        if(aggressively)
            {
            double base_spec_amt = total_spec_amt - term_spec_amt;
            SpecifiedAmount = value_cast<std::string>(base_spec_amt);
// TODO ?? Are the next two calls necessary? or does
//   realize_sequence_string_for_specified_amount();
// take care of everything?
            SpecAmt.assign(100, r_spec_amt(base_spec_amt));
            VectorSpecifiedAmountStrategy.assign
                (100
                ,e_sa_strategy(e_sainputscalar)
                );
            realize_sequence_string_for_specified_amount();
            }
        }
    else
        {
        throw std::logic_error("Term is neither proportional nor absolute.");
        }
}

