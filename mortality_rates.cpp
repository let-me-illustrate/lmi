// Mortality rates.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: mortality_rates.cpp,v 1.7 2005-10-18 16:57:58 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mortality_rates.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "math_functors.hpp" // mean()

#include <algorithm> // std::min()

/* TODO ?? Mortality-rate pointer-to-implementation might contain:
    double MonthlyRateLimit_;
    e_coi_rate_method AnnualToMonthlyMethod_;
*/

//============================================================================
MortalityRates::MortalityRates(BasicValues const& basic_values)
{
    Init(basic_values);
}

//============================================================================
void MortalityRates::Init(BasicValues const& basic_values)
{
    int const issue_age = basic_values.GetIssueAge();
    int const length = basic_values.GetLength();

    MonthlyCurrentCoiRatesBand0_ = actuarial_table
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_CurrCOITable))
        ,issue_age
        ,length
        );

    MonthlyGuaranteedCoiRates_ = actuarial_table
        (GuaranteedTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_GuarCOITable))
        ,issue_age
        ,length
        );

    static double const one_twelfth = 1.0 / 12.0;
    for(int j = 0; j < length; j++)
        {
        MonthlyGuaranteedCoiRates_[j] = std::min
            (one_twelfth
            , MonthlyGuaranteedCoiRates_[j]
            / (12.0 - MonthlyGuaranteedCoiRates_[j])
            );
        }

    MonthlyMidpointCoiRatesBand0_.resize(length);
    // Calculate midpoint as mean of current and guaranteed.
    // A different average might be used instead.
    // ET !! MonthlyMidpointCoiRatesBand0_ = mean(MonthlyCurrentCoiRatesBand0_, MonthlyGuaranteedCoiRates_);
    std::transform
        (MonthlyCurrentCoiRatesBand0_.begin()
        ,MonthlyCurrentCoiRatesBand0_.end()
        ,MonthlyGuaranteedCoiRates_.begin()
        ,MonthlyMidpointCoiRatesBand0_.begin()
        ,mean<double>()
        );

    CvatCorridorFactors_ = actuarial_table
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_CorridorTable))
        ,issue_age
        ,length
        );

    AdbRates_ = actuarial_table
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_ADDTable))
        ,issue_age
        ,length
        );

    WpRates_ = actuarial_table
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_WPTable))
        ,issue_age
        ,length
        );
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRates
    (enum_basis a_Basis
    ) const
{
    switch(a_Basis)
        {
        case e_currbasis: return MonthlyCurrentCoiRatesBand0_;
        case e_mdptbasis: return MonthlyMidpointCoiRatesBand0_;
        case e_guarbasis: return MonthlyGuaranteedCoiRates_;
        default:
            {
            fatal_error()
                << "Case '"
                << a_Basis
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    throw "Case not found.";
}

