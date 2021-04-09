// Mortality rates.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "mortality_rates.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "basic_values.hpp"
#include "bourn_cast.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "math_functions.hpp"           // assign_midpoint()

#include <algorithm>                    // min()

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
        ,basic_values.database().query<int>(DB_CurrCoiTable)
        ,issue_age
        ,length
        );

    MonthlyGuaranteedCoiRates_ = actuarial_table_rates
        (GuaranteedTableFile()
        ,basic_values.database().query<int>(DB_GuarCoiTable)
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

    assign_midpoint(MonthlyMidpointCoiRatesBand0_, MonthlyGuaranteedCoiRates_, MonthlyCurrentCoiRatesBand0_);

    CvatCorridorFactors_ = actuarial_table_rates
        (CurrentTableFile()
        ,basic_values.database().query<int>(DB_CorridorTable)
        ,issue_age
        ,length
        );

    AdbRates_ = actuarial_table_rates
        (CurrentTableFile()
        ,basic_values.database().query<int>(DB_AdbTable)
        ,issue_age
        ,length
        );

    WpRates_ = actuarial_table_rates
        (CurrentTableFile()
        ,basic_values.database().query<int>(DB_WpTable)
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
        }
    throw "Unreachable--silences a compiler diagnostic.";
}
