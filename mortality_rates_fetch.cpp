// Mortality rates.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#include "mortality_rates.hpp"

#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "yare_input.hpp"

#include <algorithm>
#include <functional>

// TODO ?? Rewrite, paying attention to the following issues.
//
// Mortality tables are read in class BasicValues, then used here.
// The vectors are used by value, not by reference, so they are
// copied needlessly. Tables should instead be read here.

//============================================================================
void MortalityRates::fetch_parameters(BasicValues const& basic_values)
{
    // Some of these data members seem useless for now, but they will
    // become useful when mortality-table access is moved hither from
    // class BasicValues.
    AllowAdb_          = basic_values.Database_->Query(DB_AllowAdb         );
    AllowChild_        = basic_values.Database_->Query(DB_AllowChildRider  );
    AllowFlatExtras_   = basic_values.Database_->Query(DB_AllowFlatExtras  );
    AllowSpouse_       = basic_values.Database_->Query(DB_AllowSpouseRider );
    AllowSubstdTable_  = basic_values.Database_->Query(DB_AllowSubstdTable );
    AllowTerm_         = basic_values.Database_->Query(DB_AllowTerm        );
    AllowWp_           = basic_values.Database_->Query(DB_AllowWp          );
    CCoiIsAnnual_      = basic_values.Database_->Query(DB_CurrCoiIsAnnual  );
    GCoiIsAnnual_      = basic_values.Database_->Query(DB_GuarCoiIsAnnual  );
    IsTgtPremTabular_ =
        oe_modal_table == basic_values.Database_->Query(DB_TgtPremType)
        ;

    double max_coi_rate = basic_values.Database_->Query(DB_MaxMonthlyCoiRate);
    LMI_ASSERT(0.0 != max_coi_rate);
    max_coi_rate = 1.0 / max_coi_rate;
    MaxMonthlyCoiRate_ = max_coi_rate;

    basic_values.Database_->Query(GCoiMultiplier_, DB_GuarCoiMultiplier);
    basic_values.Database_->Query(CCoiMultiplier_, DB_CurrCoiMultiplier);
    basic_values.Database_->Query(SubstdTblMult_ , DB_SubstdTableMult  );

    CountryCoiMultiplier_ = basic_values.yare_input_.CountryCoiMultiplier;
    IsPolicyRated_        = is_policy_rated(basic_values.yare_input_);
    SubstandardTable_     = basic_values.yare_input_.SubstandardTable;

    CurrentCoiMultiplier_ = basic_values.yare_input_.CurrentCoiMultiplier;
    MonthlyFlatExtra_     = basic_values.yare_input_.FlatExtra;

    // TODO ?? Defectively, this data member is not yet used.
    PartialMortalityMultiplier_ = basic_values.yare_input_.PartialMortalityMultiplier;

    round_coi_rate_ = basic_values.round_coi_rate();

// TODO ?? Rethink these "delicate" things. Should raw rates be stored
// temporarily in some other manner, e.g. using a handle-body idiom?

// TODO ?? These are delicate: they get modified downstream.
    MonthlyGuaranteedCoiRates_     = basic_values.GetGuarCOIRates();
    MonthlyCurrentCoiRatesBand0_   = basic_values.GetCurrCOIRates0();
    MonthlyCurrentCoiRatesBand1_   = basic_values.GetCurrCOIRates1();
    MonthlyCurrentCoiRatesBand2_   = basic_values.GetCurrCOIRates2();

// TODO ?? These are delicate: they are needed only conditionally.
    MonthlyGuaranteedTermCoiRates_ = basic_values.GetGuaranteedTermRates();
    MonthlyCurrentTermCoiRates_    = basic_values.GetCurrentTermRates();
    AdbRates_                      = basic_values.GetAdbRates();
    WpRates_                       = basic_values.GetWpRates();
    ChildRiderRates_               = basic_values.GetChildRiderRates();
    GuaranteedSpouseRiderRates_    = basic_values.GetGuaranteedSpouseRiderRates();
    CurrentSpouseRiderRates_       = basic_values.GetCurrentSpouseRiderRates();
    TargetPremiumRates_            = basic_values.GetTgtPremRates();

    Irc7702Q_                      = basic_values.GetIrc7702QRates();
    GroupProxyRates_               = basic_values.GetGroupProxyRates();
    PartialMortalityQ_             = basic_values.GetPartialMortalityRates();
    CvatCorridorFactors_           = basic_values.GetCvatCorridorFactors();
    SevenPayRates_                 = basic_values.GetSevenPayRates();

    std::transform
        (SubstdTblMult_.begin()
        ,SubstdTblMult_.end()
        ,basic_values.GetSubstdTblMultTable().begin()
        ,SubstdTblMult_.begin()
        ,std::multiplies<double>()
        );
}

