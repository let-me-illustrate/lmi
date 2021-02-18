// Mortality rates.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "et_vector.hpp"
#include "math_functions.hpp"           // assign_midpoint()
#include "oecumenic_enumerations.hpp"

#include <algorithm>                    // min()

//============================================================================
MortalityRates::MortalityRates(BasicValues const& basic_values)
    :Length_ {basic_values.GetLength()}
{
    reserve_vectors();
    fetch_parameters(basic_values);
    initialize();
}

//============================================================================
void MortalityRates::reserve_vectors()
{
    MonthlyGuaranteedCoiRates_    .reserve(Length_);
    MonthlyCurrentCoiRatesBand0_  .reserve(Length_);
    MonthlyCurrentCoiRatesBand1_  .reserve(Length_);
    MonthlyCurrentCoiRatesBand2_  .reserve(Length_);
    Irc7702Q_                     .reserve(Length_);
    MonthlyMidpointCoiRatesBand0_ .reserve(Length_);
    MonthlyMidpointCoiRatesBand1_ .reserve(Length_);
    MonthlyMidpointCoiRatesBand2_ .reserve(Length_);
    MidpointSpouseRiderRates_     .reserve(Length_);
    MonthlyMidpointTermCoiRates_  .reserve(Length_);
    GroupProxyRates_              .reserve(Length_);
    PartialMortalityQ_            .reserve(Length_);
    CvatCorridorFactors_          .reserve(Length_);
    SevenPayRates_                .reserve(Length_);
    CvatNspRates_                 .reserve(1 + Length_);
}

//============================================================================
void MortalityRates::initialize()
{
    SetGuaranteedRates();
    SetNonguaranteedRates();
    SetOtherRates();

    if(AllowFlatExtras_ || AllowSubstdTable_)
        {
        MakeCoiRateSubstandard(MonthlyCurrentCoiRatesBand0_);
        MakeCoiRateSubstandard(MonthlyCurrentCoiRatesBand1_);
        MakeCoiRateSubstandard(MonthlyCurrentCoiRatesBand2_);
        MakeCoiRateSubstandard(MonthlyGuaranteedCoiRates_);
        }

    assign_midpoint(MonthlyMidpointCoiRatesBand0_, MonthlyGuaranteedCoiRates_, MonthlyCurrentCoiRatesBand0_);
    assign_midpoint(MonthlyMidpointCoiRatesBand1_, MonthlyGuaranteedCoiRates_, MonthlyCurrentCoiRatesBand1_);
    assign_midpoint(MonthlyMidpointCoiRatesBand2_, MonthlyGuaranteedCoiRates_, MonthlyCurrentCoiRatesBand2_);

/*
    input from database and input classes
    input
        unisex male proportion guar
        unisex male proportion curr
        ANB/ALB
        bool use NY COI limits
        uninsurable
        cCOI, gCOI stored as annual/monthly
    annual to monthly method for each
    rounding for each: decimals and bias
    max for each
*/
}

/// Multiply monthly tabular rates by multiplier; limit to maximum.
///
/// If tabular rates are annual, convert them to monthly after
/// multiplying by the multiplier, but before limiting them.
///
/// SOMEDAY !! Differentiate algorithm by DB_CoiUpper12Method.

void MortalityRates::SetCoiRates
    (std::vector<double>      & coi_rates
    ,std::vector<double> const& coi_multiplier
    ,std::vector<double> const& maximum
    ,bool                       table_is_annual
    )
{
    for(int j = 0; j < Length_; ++j)
        {
        double z = coi_rates[j] * coi_multiplier[j];
        if(table_is_annual)
            {
            z = coi_rate_from_q<double>()(z, maximum[j]);
            }
        else
            {
            z = std::min                 (z, maximum[j]);
            }
        coi_rates[j] = round_coi_rate_(z);
        }
}

//============================================================================
void MortalityRates::SetGuaranteedRates()
{
    SetCoiRates
        (MonthlyGuaranteedCoiRates_
        ,GCoiMultiplier_
        ,std::vector<double>(Length_, MaxMonthlyCoiRate_)
        ,GCoiIsAnnual_
        );
}

//============================================================================
void MortalityRates::SetNonguaranteedRates()
{
    // These multipliers affect only nonguaranteed COI rates.
    std::vector<double> z(Length_);
    assign(z, CCoiMultiplier_ * CountryCoiMultiplier_ * CurrentCoiMultiplier_);
    SetOneNonguaranteedRateBand(MonthlyCurrentCoiRatesBand0_, z);
    SetOneNonguaranteedRateBand(MonthlyCurrentCoiRatesBand1_, z);
    SetOneNonguaranteedRateBand(MonthlyCurrentCoiRatesBand2_, z);
}

//============================================================================
void MortalityRates::SetOneNonguaranteedRateBand
    (std::vector<double>      & coi_rates
    ,std::vector<double> const& curr_coi_multiplier
    )
{
    SetCoiRates
        (coi_rates
        ,curr_coi_multiplier
        ,MonthlyGuaranteedCoiRates_
        ,CCoiIsAnnual_
        );
}

//============================================================================
void MortalityRates::SetOtherRates()
{
    if(AllowTerm_)
        {
        MakeCoiRateSubstandard(MonthlyCurrentTermCoiRates_);
        MakeCoiRateSubstandard(MonthlyGuaranteedTermCoiRates_);
        assign_midpoint(MonthlyMidpointTermCoiRates_, MonthlyGuaranteedTermCoiRates_, MonthlyCurrentTermCoiRates_);
        }
    else
        {
        MonthlyMidpointTermCoiRates_  .assign(Length_, 0.0);
        }

    if(AllowAdb_)
        {
// SOMEDAY !! Add substandard support for this rider (blocked upstream for now).
//        MakeCoiRateSubstandard(AdbRates_);
        }

    if(AllowWp_)
        {
// SOMEDAY !! Add substandard support for this rider (blocked upstream for now).
//        MakeCoiRateSubstandard(WpRates_);
        }

    if(AllowSpouse_)
        {
        // Spouse rider can't be substandard--spouse not underwritten.
        assign_midpoint(MidpointSpouseRiderRates_, GuaranteedSpouseRiderRates_, CurrentSpouseRiderRates_);
        }
    else
        {
        MidpointSpouseRiderRates_   .assign(Length_, 0.0);
        }

    if(AllowChild_)
        {
        // Child rider can't be substandard--child not underwritten.
        // Midpoint needn't be calculated--guaranteed equals current.
        }

    if(IsTgtPremTabular_)
        {
        // Assume target premium table is never changed for substandard.
        }

    // Use reciprocal of CVAT corridor factor as NSP, for both GPT and
    // CVAT.
    // TODO ?? TAXATION !! Do this only if DB_Irc7702NspWhence equals
    // oe_7702_nsp_reciprocal_cvat_corridor. DATABASE !! This probably
    // should have its own rounding rule.
    LMI_ASSERT(CvatNspRates_.empty());
    for(int j = 0; j < Length_; ++j)
        {
        LMI_ASSERT(0.0 < CvatCorridorFactors_[j]);
        CvatNspRates_.push_back(1.0 / CvatCorridorFactors_[j]);
        }
    CvatNspRates_.push_back(1.0);
}

/// Incorporate flat extras and table ratings into COI rates.
///
/// A popular selection {A,B,C,D,E,F,H,J,L,P} of industry-standard
/// table ratings is hard coded.
///
/// Flat extras are entered as annual rates per thousand, to conform
/// to standard industry usage. Some UL admin systems restrict flat
/// extras to integral number of cents per thousand per month, e.g.,
/// mapping five dollars per thousand annually to either forty-one or
/// forty-two cents per thousand monthly; lmi implicitly accommodates
/// that by accepting floating-point flat-extra (annual) values, e.g.,
/// 4.92 or 5.04 in the example given.

void MortalityRates::MakeCoiRateSubstandard(std::vector<double>& coi_rates)
{
    // Nothing to do if no rating.
    if(!IsPolicyRated_)
        {
        return;
        }

    if(!(AllowFlatExtras_ || AllowSubstdTable_))
        {
        alarum()
            << "Flat extras and table ratings not permitted."
            << LMI_FLUSH
            ;
        }

    static double const factors[11] =
        {0.0, 0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 2.00, 2.50, 3.00, 4.00,};
    assign
        (coi_rates
        ,apply_binary
            (lesser_of<double>()
            ,MaxMonthlyCoiRate_
            ,   AnnualFlatExtra_ / 12000.0
              + coi_rates * (1.0 + SubstdTblMult_ * factors[SubstandardTable_])
            )
        );
    std::transform(coi_rates.begin(), coi_rates.end(), coi_rates.begin(), round_coi_rate_);
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRatesBand0
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

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRatesBand1
    (mcenum_gen_basis b
    ) const
{
    switch(b)
        {
        case mce_gen_curr: return MonthlyCurrentCoiRatesBand1_;
        case mce_gen_mdpt: return MonthlyMidpointCoiRatesBand1_;
        case mce_gen_guar: return MonthlyGuaranteedCoiRates_;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRatesBand2
    (mcenum_gen_basis b
    ) const
{
    switch(b)
        {
        case mce_gen_curr: return MonthlyCurrentCoiRatesBand2_;
        case mce_gen_mdpt: return MonthlyMidpointCoiRatesBand2_;
        case mce_gen_guar: return MonthlyGuaranteedCoiRates_;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

//============================================================================
std::vector<double> const& MortalityRates::SpouseRiderRates
    (mcenum_gen_basis b
    ) const
{
    switch(b)
        {
        case mce_gen_curr: return CurrentSpouseRiderRates_;
        case mce_gen_mdpt: return MidpointSpouseRiderRates_;
        case mce_gen_guar: return GuaranteedSpouseRiderRates_;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyTermCoiRates
    (mcenum_gen_basis b
    ) const
{
    switch(b)
        {
        case mce_gen_curr: return MonthlyCurrentTermCoiRates_;
        case mce_gen_mdpt: return MonthlyMidpointTermCoiRates_;
        case mce_gen_guar: return MonthlyGuaranteedTermCoiRates_;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}
