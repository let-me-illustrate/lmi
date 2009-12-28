// Mortality rates.
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

// $Id: mortality_rates_fetch.cpp,v 1.3 2008-12-27 02:56:49 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "mortality_rates.hpp"

#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "rounding_rules.hpp"
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
    AllowAdb_          = basic_values.Database_->Query(DB_AllowADD         );
    AllowChild_        = basic_values.Database_->Query(DB_AllowChild       );
    AllowFlatExtras_   = basic_values.Database_->Query(DB_AllowFlatExtras  );
    AllowSpouse_       = basic_values.Database_->Query(DB_AllowSpouse      );
    AllowSubstdTable_  = basic_values.Database_->Query(DB_AllowSubstdTable );
    AllowTerm_         = basic_values.Database_->Query(DB_AllowTerm        );
    AllowWp_           = basic_values.Database_->Query(DB_AllowWP          );
    CCoiIsAnnual_      = basic_values.Database_->Query(DB_CCoiIsAnnual     );
    GCoiIsAnnual_      = basic_values.Database_->Query(DB_GCoiIsAnnual     );
    IsTgtPremTabular_ =
        oe_modal_table == basic_values.Database_->Query(DB_TgtPremType)
        ;

    MaxMonthlyCoiRate_ = basic_values.Database_->Query(DB_MaxMonthlyCoiRate);

    basic_values.Database_->Query(GCoiMultiplier_, DB_GCOIMultiplier);
    basic_values.Database_->Query(CCoiMultiplier_, DB_CCOIMultiplier);
    basic_values.Database_->Query(SubstdTblMult_ , DB_SubstdTblMult );

    CountryCoiMultiplier_ = basic_values.yare_input_.CountryCoiMultiplier;
    IsPolicyRated_        = is_policy_rated(basic_values.yare_input_);
    SubstandardTable_     = basic_values.yare_input_.SubstandardTable;

    CurrentCoiMultiplier_ = basic_values.yare_input_.CurrentCoiMultiplier;
    MonthlyFlatExtra_     = basic_values.yare_input_.FlatExtra;

    // TODO ?? Defectively, this data member is not yet used.
    PartialMortalityMultiplier_ = basic_values.yare_input_.PartialMortalityMultiplier;

    round_coi_rate_ = basic_values.GetRoundingRules().round_coi_rate();

// TODO ?? Rethink these "delicate" things. Should raw rates be stored
// temporarily in some other manner, e.g. using a handle-body idiom?

// TODO ?? These are delicate: they get modified downstream.
    MonthlyGuaranteedCoiRates_   = basic_values.GetGuarCOIRates();
    MonthlyCurrentCoiRatesBand0_ = basic_values.GetCurrCOIRates0();
    MonthlyCurrentCoiRatesBand1_ = basic_values.GetCurrCOIRates1();
    MonthlyCurrentCoiRatesBand2_ = basic_values.GetCurrCOIRates2();

// TODO ?? These are delicate: they are needed only conditionally.
    MonthlyGuaranteedTermCoiRates_ = basic_values.GetGuaranteedTermRates();
    MonthlyCurrentTermCoiRates_    = basic_values.GetCurrentTermRates();
    AdbRates_                   = basic_values.GetAdbRates();
    WpRates_                    = basic_values.GetWpRates();
    ChildRiderRates_            = basic_values.GetChildRiderRates();
    GuaranteedSpouseRiderRates_ = basic_values.GetGuaranteedSpouseRiderRates();
    CurrentSpouseRiderRates_    = basic_values.GetCurrentSpouseRiderRates();
    TargetPremiumRates_         = basic_values.GetTgtPremRates();

    // TODO ?? Why have these here, since they're already in the
    // basic-values class? Same question for 'TableYRates_' too.

    Irc7702Q_ = basic_values.GetIRC7702Rates();
    TableYRates_ = basic_values.GetTableYRates();
// TODO ?? 83 GAM and table Y should instead be something like
//   term cost table
//   partial mortality table
// should both permit a choice from among numerous and tables.
// We could offer the whole SOA database, but it lacks table Y,
// and some choices would be inappropriate (e.g. lapse factors).

// TODO ?? Is this correct or not?
/*
    for(int j = 0; j < Length_; j++)
        {
        TableYRates_[j] *= 12.0;  // stored as monthly
        }
*/

    PartialMortalityQ_   = basic_values.Get83GamRates();
    CvatCorridorFactors_ = basic_values.GetCvatCorridorFactors();
    SevenPayRates_       = basic_values.GetTAMRA7PayRates();

    std::transform
        (SubstdTblMult_.begin()
        ,SubstdTblMult_.end()
        ,basic_values.GetSubstdTblMultTable().begin()
        ,SubstdTblMult_.begin()
        ,std::multiplies<double>()
        );

// TODO ?? Move this up here?
//    CvatNspRates_;
}

