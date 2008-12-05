// Mortality rates.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ihs_mortal.cpp,v 1.35 2008-12-05 10:59:33 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mortality_rates.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "math_functors.hpp"

#include <algorithm>
#include <functional>

//============================================================================
MortalityRates::MortalityRates(BasicValues const& basic_values)
    :Length_(basic_values.GetLength())
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
    TableYRates_                  .reserve(Length_);
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

    LMI_ASSERT(0 == MonthlyMidpointCoiRatesBand0_.size());
    LMI_ASSERT(0 == MonthlyMidpointCoiRatesBand1_.size());
    LMI_ASSERT(0 == MonthlyMidpointCoiRatesBand2_.size());
    for(int j = 0; j < Length_; j++)
        {
        // Here we take midpoint as average of monthly curr and guar.
        // Other approaches are possible.
        // TODO ?? Use mean() instead.
        MonthlyMidpointCoiRatesBand0_.push_back
            (  0.5
            * (MonthlyCurrentCoiRatesBand0_[j] + MonthlyGuaranteedCoiRates_[j])
            );
        MonthlyMidpointCoiRatesBand1_.push_back
            (  0.5
            * (MonthlyCurrentCoiRatesBand1_[j] + MonthlyGuaranteedCoiRates_[j])
            );
        MonthlyMidpointCoiRatesBand2_.push_back
            (  0.5
            * (MonthlyCurrentCoiRatesBand2_[j] + MonthlyGuaranteedCoiRates_[j])
            );
        }

/*
    input from database and input classes
    input
        unisex male proportion guar
        unisex male proportion curr
        ANB/ALB
        bool use NY COI limits
        TODO ?? bool ignore ratings for 7702
        flat extras
        substd table
        uninsurable
        cCOI, gCOI stored as annual/monthly
    annual to monthly method for each
    rounding for each: decimals and bias
    max for each
*/
}

//============================================================================
void MortalityRates::SetGuaranteedRates()
{
    if(GCoiIsAnnual_) // TODO ?? Assume this means experience rated?
        {
// TODO ?? Merge these pointlessly-distinguished alternatives.
        // If experience rating is ALLOWED, not necessarily USED.
        if(AllowExpRating_)
            {
            for(int j = 0; j < Length_; j++)
                {
                double qstart = MonthlyGuaranteedCoiRates_[j];
                double q = std::min(1.0, qstart);
                q = coi_rate_from_q<double>()(q, MaxMonthlyCoiRate_);
                MonthlyGuaranteedCoiRates_[j] = round_coi_rate_(q);
                }
            }
        // This is just one way it might be done...
        else
            {
            for(int j = 0; j < Length_; j++)
                {
                // TODO ?? Should respect DB_CoiUpper12Method.
                MonthlyGuaranteedCoiRates_[j] *= GCoiMultiplier_[j];
                MonthlyGuaranteedCoiRates_[j] = coi_rate_from_q<double>()
                    (MonthlyGuaranteedCoiRates_[j]
                    ,MaxMonthlyCoiRate_
                    );
                MonthlyGuaranteedCoiRates_[j] = round_coi_rate_
                        (MonthlyGuaranteedCoiRates_[j]
                        );
                }
            }
        }
    else
        {
        for(int j = 0; j < Length_; j++)
            {
            double q = MonthlyGuaranteedCoiRates_[j];
            // TODO ?? COI multiple is applied to the monthly COI.
            q = std::min(GCoiMultiplier_[j] * q, MaxMonthlyCoiRate_);
            MonthlyGuaranteedCoiRates_[j] = round_coi_rate_(q);
            }
        }
}

//============================================================================
void MortalityRates::SetNonguaranteedRates()
{
    // ET !! Easier to write as
    //   std::vector<double> curr_coi_multiplier =
    //     CCoiMultiplier_ * CountryCoiMultiplier_ * CurrentCoiMultiplier_;
    std::vector<double> curr_coi_multiplier(CCoiMultiplier_);

    // Multiplier for country affects only nonguaranteed COI rates.
    std::transform
        (curr_coi_multiplier.begin()
        ,curr_coi_multiplier.end()
        ,curr_coi_multiplier.begin()
        ,std::bind1st
            (std::multiplies<double>()
            ,CountryCoiMultiplier_
            )
        );
    // Input vector current-COI multiplier affects only nonguaranteed COI rates.
    std::transform
        (curr_coi_multiplier.begin()
        ,curr_coi_multiplier.end()
        ,CurrentCoiMultiplier_.begin()
        ,curr_coi_multiplier.begin()
        ,std::multiplies<double>()
        );

    SetOneNonguaranteedRateBand
        (MonthlyCurrentCoiRatesBand0_
        ,curr_coi_multiplier
        );
    SetOneNonguaranteedRateBand
        (MonthlyCurrentCoiRatesBand1_
        ,curr_coi_multiplier
        );
    SetOneNonguaranteedRateBand
        (MonthlyCurrentCoiRatesBand2_
        ,curr_coi_multiplier
        );
}

//============================================================================
void MortalityRates::SetOneNonguaranteedRateBand
    (std::vector<double>      & coi_rates
    ,std::vector<double> const& curr_coi_multiplier
    )
{
    if(CCoiIsAnnual_)
        {
// TODO ?? Merge these pointlessly-distinguished alternatives.
        // If experience rating is ALLOWED, not necessarily USED
        if(AllowExpRating_)
            {
            for(int j = 0; j < Length_; j++)
                {
                double q = curr_coi_multiplier[j] * coi_rates[j];
                q = coi_rate_from_q<double>()(q, MonthlyGuaranteedCoiRates_[j]);
                // TODO ?? Is this not handled already by coi_rate_from_q()?
                q = std::min(q, MonthlyGuaranteedCoiRates_[j]);
                coi_rates[j] = round_coi_rate_(q);
                }
            }
        // This is just one way it might be done...
        else
            {
            for(int j = 0; j < Length_; j++)
                {
// TODO ?? should respect DB_CoiUpper12Method
                coi_rates[j] *= curr_coi_multiplier[j];
                coi_rates[j] = coi_rate_from_q<double>()
                    (coi_rates[j]
                    ,MonthlyGuaranteedCoiRates_[j]
                    );
                coi_rates[j] = round_coi_rate_(coi_rates[j]);
                }
            }
        }
    else
        {
        for(int j = 0; j < Length_; j++)
            {
            double q = coi_rates[j];
            // USER !! Multiplier is applied to the monthly COI rate
            // if only a monthly rate is given.
            q = std::min(curr_coi_multiplier[j] * q, MonthlyGuaranteedCoiRates_[j]);
            coi_rates[j] = round_coi_rate_(q);
            }
        }
}

//============================================================================
void MortalityRates::SetOtherRates()
{
    if(AllowTerm_)
        {
        MakeCoiRateSubstandard(MonthlyCurrentTermCoiRates_);
        MakeCoiRateSubstandard(MonthlyGuaranteedTermCoiRates_);

        LMI_ASSERT(0 == MonthlyMidpointTermCoiRates_.size());
        for(int j = 0; j < Length_; j++)
            {
            // Here we take midpoint as average of monthly curr and guar.
            // Other approaches are possible.
            // TODO ?? Use mean() instead.
            MonthlyMidpointTermCoiRates_.push_back
                (   0.5
                *   (   MonthlyCurrentTermCoiRates_[j]
                    +   MonthlyGuaranteedTermCoiRates_[j]
                    )
                );
            }
        }
    else
        {
        MonthlyMidpointTermCoiRates_  .assign(Length_, 0.0);
        }

    if(AllowAdb_)
        {
// TODO ?? No substandard support yet for this rider.
//        MakeCoiRateSubstandard(AdbRates_);
        }

    if(AllowWp_)
        {
// TODO ?? No substandard support yet for this rider.
//        MakeCoiRateSubstandard(WpRates_);
        }

    if(AllowSpouse_)
        {
        // Spouse rider can't be substandard--spouse not underwritten.
        LMI_ASSERT(0 == MidpointSpouseRiderRates_.size());
        for(int j = 0; j < Length_; j++)
            {
            // Here we take midpoint as average of monthly curr and guar.
            // Other approaches are possible.
            // TODO ?? Use mean() instead.
            MidpointSpouseRiderRates_.push_back
                (   0.5
                *   (   CurrentSpouseRiderRates_[j]
                    +   GuaranteedSpouseRiderRates_[j]
                    )
                );
            }
        }
    else
        {
        MidpointSpouseRiderRates_   .assign(Length_, 0.0);
        }

    if(AllowChild_)
        {
        // Child rider can't be substandard--child not underwritten.
        }

    if(IsTgtPremTabular_)
        {
        // Assume target premium table is never changed for substandard.
        }

    // TODO ?? Temporary stuff to support NSP for 7702A
    // TODO ?? Incorrect if GPT
    LMI_ASSERT(0 == CvatNspRates_.size());
    for(int j = 0; j < Length_; j++)
        {
        LMI_ASSERT(0.0 < CvatCorridorFactors_[j]);
        CvatNspRates_.push_back(1.0 / CvatCorridorFactors_[j]);
        }
    CvatNspRates_.push_back(1.0);
}

//============================================================================
void MortalityRates::MakeCoiRateSubstandard
    (std::vector<double>      & coi_rates
    )
{
    // Nothing to do if no rating.
    if(!IsPolicyRated_)
        {
        return;
        }

    if(!(AllowFlatExtras_ || AllowSubstdTable_))
        {
        fatal_error()
            << "Substandard not available for this policy form."
            << LMI_FLUSH
            ;
        }

    static double const factors[11] =
        {0.0, 0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 2.00, 2.50, 3.00, 4.00,};
    double table_multiple = factors[SubstandardTable_];
    for(int j = 0; j < Length_; ++j)
        {
        // Flat extra: input as annual per K, want monthly per $.
        double flat_extra = MonthlyFlatExtra_[j] / 12000.0;

// TODO ?? Results here really should be rounded. Instead, for the
// nonce, we round only the maximum, in order to match old
// regression tests.
        double z = round_coi_rate_(MaxMonthlyCoiRate_);
        coi_rates[j] = std::min
            (z
            ,   flat_extra
            +   coi_rates[j] * (1.0 + SubstdTblMult_[j] * table_multiple)
            );

// TODO ?? Some UL admin systems convert flat extras to an integral
// number of cents per thousand per month. It would be nice to offer
// such a behavior here.
        }
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
        default: fatal_error() << "Case " << b << " not found." << LMI_FLUSH;
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
        default: fatal_error() << "Case " << b << " not found." << LMI_FLUSH;
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
        default: fatal_error() << "Case " << b << " not found." << LMI_FLUSH;
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
        default: fatal_error() << "Case " << b << " not found." << LMI_FLUSH;
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
        default: fatal_error() << "Case " << b << " not found." << LMI_FLUSH;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

