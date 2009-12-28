// Mortality rates.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: mortality_rates.cpp,v 1.16 2008-12-27 02:56:49 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
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

    MonthlyCurrentCoiRatesBand0_ = actuarial_table_rates
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_CurrCOITable))
        ,issue_age
        ,length
        );

    MonthlyGuaranteedCoiRates_ = actuarial_table_rates
        (GuaranteedTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_GuarCOITable))
        ,issue_age
        ,length
        );

    static double const one_twelfth = 1.0 / 12.0;
    for(int j = 0; j < length; ++j)
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

    CvatCorridorFactors_ = actuarial_table_rates
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_CorridorTable))
        ,issue_age
        ,length
        );

    AdbRates_ = actuarial_table_rates
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_ADDTable))
        ,issue_age
        ,length
        );

    WpRates_ = actuarial_table_rates
        (CurrentTableFile()
        ,static_cast<long int>(basic_values.Database_->Query(DB_WPTable))
        ,issue_age
        ,length
        );
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRates
    (mcenum_gen_basis b
    ) const
{
    switch(b)
        {
        case mce_gen_curr: return MonthlyCurrentCoiRatesBand0_;
        case mce_gen_mdpt: return MonthlyMidpointCoiRatesBand0_;
        case mce_gen_guar: return MonthlyGuaranteedCoiRates_;
        default: fatal_error() << "Case " << b << " not found." << LMI_FLUSH;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

