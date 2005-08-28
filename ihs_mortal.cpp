// Mortality rates.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
// Portions marked JLM Copyright (C) 2001 Gregory W. Chicares and Joseph L. Murdzek.
// Author is GWC except where specifically noted otherwise.
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

// $Id: ihs_mortal.cpp,v 1.10 2005-08-28 14:09:03 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mortality_rates.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "math_functors.hpp"
#include "miscellany.hpp" // each_equal
#include "rounding_rules.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>

//============================================================================
MortalityRates::MortalityRates(BasicValues const& basic_values)
    :Length_(basic_values.GetLength())
{
    MonthlyGuaranteedCoiRates_    .reserve(Length_);
    MonthlyCurrentCoiRatesBand0_  .reserve(Length_);
    MonthlyCurrentCoiRatesBand1_  .reserve(Length_);
    MonthlyCurrentCoiRatesBand2_  .reserve(Length_);
    AlternativeMonthlyCoiRates_   .reserve(Length_);
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
    Init(basic_values);
}

//============================================================================
void MortalityRates::Init(BasicValues const& basic_values)
{
    AllowADD_          = basic_values.Database_->Query(DB_AllowADD         );
    AllowChild_        = basic_values.Database_->Query(DB_AllowChild       );
    AllowExpRating_    = basic_values.Database_->Query(DB_AllowExpRating   );
    AllowFlatExtras_   = basic_values.Database_->Query(DB_AllowFlatExtras  );
    AllowSpouse_       = basic_values.Database_->Query(DB_AllowSpouse      );
    AllowSubstdTable_  = basic_values.Database_->Query(DB_AllowSubstdTable );
    AllowTerm_         = basic_values.Database_->Query(DB_AllowTerm        );
    AllowWP_           = basic_values.Database_->Query(DB_AllowWP          );
    CCoiIsAnnual_      = basic_values.Database_->Query(DB_CCoiIsAnnual     );
    GCoiIsAnnual_      = basic_values.Database_->Query(DB_GCoiIsAnnual     );

    IsTgtPremTabular_ =
        e_modal_table == basic_values.Database_->Query(DB_TgtPremType)
        ;

    MaxMonthlyCoiRate_ = basic_values.Database_->Query(DB_MaxMonthlyCoiRate);

    // TODO ?? Dubious.
    ExpRatCoiMultAlt_   = basic_values.Database_->Query(DB_ExpRatCoiMultAlt);
    ExpRatCoiMultCurr0_ = basic_values.Database_->Query(DB_ExpRatCoiMultCurr0);
    ExpRatCoiMultCurr1_ = basic_values.Database_->Query(DB_ExpRatCoiMultCurr1);
    ExpRatCoiMultGuar_  = basic_values.Database_->Query(DB_ExpRatCoiMultGuar);
    UsePMQOnCurrCOI_    = basic_values.Database_->Query(DB_UsePMQOnCurrCOI);

    basic_values.Database_->Query(GCOIMultiplier_, DB_GCOIMultiplier);
    basic_values.Database_->Query(CCOIMultiplier_, DB_CCOIMultiplier);
    basic_values.Database_->Query(SubstdTblMult_ , DB_SubstdTblMult );

    // This is the additive part of COI retention,
    // expressed as an addition to q.
    // It is a constant retrieved from the database.
    AdditiveCoiRetention_ = basic_values.Database_->Query
        (DB_ExpRatCOIRetention
        );

    // This is the multiplicative part of COI retention,
    // expressed as 1 + constant: e.g. 1.05 for 5% retention.
    //
    // TODO ?? Clean this up.
    //
    // Compare notes in AccountValue::GetNetCOI() that suggest
    // serious problems with COI retention calculations.
    //
// Formerly, this was:
// It was once tiered by initial "assumed" number of lives, but
// that quantity is no longer used; '1.0' is just a placeholder.
//
//    MultiplicativeCoiRetention_ = basic_values.TieredCharges_->coi_retention
//        (1.0
//        );
// but support for the tiered 'coi_retention' has been withdrawn.
    MultiplicativeCoiRetention_ = 1.0;

    // Alternative COI rate for experience rating.
    AlternativeMonthlyCoiRates_ = basic_values.GetSmokerBlendedGuarCOIRates();

    SetGuaranteedRates(basic_values);
    SetNonguaranteedRates(basic_values);
    SetOtherRates(basic_values);

    if(AllowFlatExtras_ || AllowSubstdTable_)
        {
        MakeCoiRateSubstandard(MonthlyCurrentCoiRatesBand0_, basic_values);
        MakeCoiRateSubstandard(MonthlyCurrentCoiRatesBand1_, basic_values);
        MakeCoiRateSubstandard(MonthlyCurrentCoiRatesBand2_, basic_values);
        // TODO ?? If experience rating is ALLOWED, not necessarily USED?
        if(AllowExpRating_)
//      if(basic_values.Input_->UseExperienceRating)  // TODO ?? And this condition too?
            {
            MakeCoiRateSubstandard(AlternativeMonthlyCoiRates_, basic_values);
            }
        MakeCoiRateSubstandard(MonthlyGuaranteedCoiRates_, basic_values);
        }

    if
        (!each_equal
            (basic_values.Input_->VectorCurrentCoiGrading.begin()
            ,basic_values.Input_->VectorCurrentCoiGrading.end()
            ,0.0
            )
        )
        {
        // TODO ?? Ignores 'MaxSurvivalDur', which should be distinct
        // from partial mortality q, but isn't in the interface as of
        // 2004-03.
        std::vector<double> monthly_partial_mortality(PartialMortalityQ_);
        std::transform
            (monthly_partial_mortality.begin()
            ,monthly_partial_mortality.end()
            ,basic_values.Input_->VectorPartialMortalityMultiplier.begin()
            ,monthly_partial_mortality.begin()
            ,std::multiplies<double>()
            );
        std::transform
            (monthly_partial_mortality.begin()
            ,monthly_partial_mortality.end()
            ,monthly_partial_mortality.begin()
            ,std::bind1st(greater_of<double>(), 0.0)
            );
        std::transform
            (monthly_partial_mortality.begin()
            ,monthly_partial_mortality.end()
            ,monthly_partial_mortality.begin()
            ,std::bind1st(lesser_of<double>(), 1.0)
            );
        std::transform
            (monthly_partial_mortality.begin()
            ,monthly_partial_mortality.end()
            ,monthly_partial_mortality.begin()
            ,std::bind2nd
                (coi_rate_from_q<double>()
                ,MaxMonthlyCoiRate_
                )
            );
        GradeCurrentCoiRatesFromPartialMortalityAssumption
            (MonthlyCurrentCoiRatesBand0_
            ,monthly_partial_mortality
            ,basic_values.Input_->VectorCurrentCoiGrading
            ,basic_values
            );
        GradeCurrentCoiRatesFromPartialMortalityAssumption
            (MonthlyCurrentCoiRatesBand1_
            ,monthly_partial_mortality
            ,basic_values.Input_->VectorCurrentCoiGrading
            ,basic_values
            );
        GradeCurrentCoiRatesFromPartialMortalityAssumption
            (MonthlyCurrentCoiRatesBand2_
            ,monthly_partial_mortality
            ,basic_values.Input_->VectorCurrentCoiGrading
            ,basic_values
            );
        if(std::string::npos != basic_values.Input_->Comments.find("idiosyncrasy_grading"))
            {
            std::vector<double>original = basic_values.GetCurrCOIRates0();
            MakeCoiRateSubstandard(original, basic_values);
            std::ofstream os
                ("coi_grading"
                ,std::ios_base::out | std::ios_base::trunc
//                ,std::ios_base::out | std::ios_base::ate | std::ios_base::app
                );
            os
                << "This file shows COI grading detail. These are monthly\n"
                << "current rates. Only the first COI band is shown,\n"
                << "because that's probably all you want to see.\n\n"
                << "The table of values is tab delimited, so you can just\n"
                << "drop it into a spreadsheet program. The columns are\n"
                << "arranged so that the ones you probably want to graph\n"
                << "are contiguous.\n\n"

                << "Each time you run a life, its grading detail replaces\n"
                << "whatever was in the former 'coi_grading' file: probably\n"
                << "you would find it annoying if it appended instead.\n"

                << "\nThe code looks like this:\n"

<< "    for(unsigned int j = 0; j < coi_rates.size(); ++j)\n"
<< "        {\n"
<< "        coi_rates[j] = std::min\n"
<< "            (MaxMonthlyCoiRate_\n"
<< "            ,       coi_rates[j]                 * (1.0 - grading_factors[j])\n"
<< "                +   monthly_partial_mortality[j] *        grading_factors[j]\n"
<< "            );\n"
<< "        coi_rates[j] = round_coi_rate(coi_rates[j]);\n"
<< "        }\n"

                << "\n'MaxMonthlyCoiRate_' is " << MaxMonthlyCoiRate_ << '\n'
                << "\nHere is the year-by-year calculation:\n\n"
                << "duration\tgrading_factor\toriginal_coi_rate\tpartial_mortality_rate\tresult\n"
                ;

            os << std::setiosflags(std::ios_base::scientific);

            int prec = 20;
            int width = 24;
            os << std::setprecision(prec) << std::setw(width);

            for(int j = 0; j < Length_; ++j)
                {
                os
                    << j
                    << '\t' << basic_values.Input_->VectorCurrentCoiGrading[j]
                    << '\t' << original[j]
                    << '\t' << monthly_partial_mortality[j]
                    << '\t' << MonthlyCurrentCoiRatesBand0_[j]
                    << '\n'
                    ;
                }
            }
        }

    LMI_ASSERT(0 == MonthlyMidpointCoiRatesBand0_.size());
    LMI_ASSERT(0 == MonthlyMidpointCoiRatesBand1_.size());
    LMI_ASSERT(0 == MonthlyMidpointCoiRatesBand2_.size());
    for(int j = 0; j < Length_; j++)
        {
        // Here we take midpoint as average of monthly curr and guar.
        // Other approaches are possible.
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
    input from Database, InputParms
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
void MortalityRates::SetGuaranteedRates(BasicValues const& basic_values)
{
    round_to<double> const& round_coi_rate
        (basic_values.GetRoundingRules().round_coi_rate()
        );
    MonthlyGuaranteedCoiRates_ = basic_values.GetGuarCOIRates();

    if(GCoiIsAnnual_) // TODO ?? Assume this means experience rated?
        {
        // If experience rating is ALLOWED, not necessarily USED.
        if(AllowExpRating_)
//      if(basic_values.Input_->UseExperienceRating) // TODO ?? And this condition too?
            {
            for(int j = 0; j < Length_; j++)
                {
                double qstart = MonthlyGuaranteedCoiRates_[j];
                double q = std::min(1.0, ExpRatCoiMultGuar_ * qstart);
                q = coi_rate_from_q<double>()(q, MaxMonthlyCoiRate_);
                // No COI retention for guaranteed
                MonthlyGuaranteedCoiRates_[j] = round_coi_rate(q);

                q = std::min(1.0, ExpRatCoiMultAlt_ * qstart);
                q = coi_rate_from_q<double>()(q, MonthlyGuaranteedCoiRates_[j]);
                q *= MultiplicativeCoiRetention_;
                q += AdditiveCoiRetention_;
                q = std::min(q, MonthlyGuaranteedCoiRates_[j]);
                AlternativeMonthlyCoiRates_[j] = round_coi_rate(q);
                }
            }
        // This is just one way it might be done...
        else
            {
            for(int j = 0; j < Length_; j++)
                {
// Author of this commented block: JLM.
// [Joe said:]
// I could not find the database item for choosing different methods, so I am
// hard coding the exponential method for now.
// [It's DB_CoiUpper12Method: 0=exponential, 1=linear]
//  linear COI method:
//              MonthlyGuaranteedCoiRates_[j] *= GCOIMultiplier_[j];
//              MonthlyGuaranteedCoiRates_[j] = std::min
//                  (1.0 / 12.0
//                  , MonthlyGuaranteedCoiRates_[j]
//                  / (12.0 - MonthlyGuaranteedCoiRates_[j])
//                  );

// Author of this commented block: GWC.
// TODO ?? Determine whether any of the comments above indicate
// defects here.

//  exponential COI method
// TODO ?? should respect DB_CoiUpper12Method
                // No COI retention for guaranteed
                MonthlyGuaranteedCoiRates_[j] *= GCOIMultiplier_[j];
                MonthlyGuaranteedCoiRates_[j] = coi_rate_from_q<double>()
                    (MonthlyGuaranteedCoiRates_[j]
                    ,MaxMonthlyCoiRate_
                    );
                MonthlyGuaranteedCoiRates_[j] = round_coi_rate
                        (MonthlyGuaranteedCoiRates_[j]
                        );
                }
            }
        }
    else  // gCoi is not annual
        {
        for(int j = 0; j < Length_; j++)
            {
            double q = MonthlyGuaranteedCoiRates_[j];
            // TODO ?? COI multiple is applied to the monthly COI.
            q = std::min(GCOIMultiplier_[j] * q, MaxMonthlyCoiRate_);
            MonthlyGuaranteedCoiRates_[j] = round_coi_rate(q);
            }
        }
}

//============================================================================
void MortalityRates::SetNonguaranteedRates(BasicValues const& basic_values)
{
    std::vector<double> curr_coi_multiplier(CCOIMultiplier_);

    // Multiplier for country affects only nonguaranteed COI rates.
    std::transform
        (curr_coi_multiplier.begin()
        ,curr_coi_multiplier.end()
        ,curr_coi_multiplier.begin()
        ,std::bind1st
            (std::multiplies<double>()
            ,basic_values.Input_->CountryCOIMultiplier
            )
        );
    // Input vector current-COI multiplier affects only nonguaranteed COI rates.
    std::transform
        (curr_coi_multiplier.begin()
        ,curr_coi_multiplier.end()
        ,basic_values.Input_->VectorCurrentCoiMultiplier.begin()
        ,curr_coi_multiplier.begin()
        ,std::multiplies<double>()
        );

    MonthlyCurrentCoiRatesBand0_ = basic_values.GetCurrCOIRates0();
    MonthlyCurrentCoiRatesBand1_ = basic_values.GetCurrCOIRates1();
    MonthlyCurrentCoiRatesBand2_ = basic_values.GetCurrCOIRates2();

    SetOneNonguaranteedRateBand
        (MonthlyCurrentCoiRatesBand0_
        ,curr_coi_multiplier
        ,basic_values
        );
    SetOneNonguaranteedRateBand
        (MonthlyCurrentCoiRatesBand1_
        ,curr_coi_multiplier
        ,basic_values
        );
    SetOneNonguaranteedRateBand
        (MonthlyCurrentCoiRatesBand2_
        ,curr_coi_multiplier
        ,basic_values
        );
}

//============================================================================
void MortalityRates::SetOneNonguaranteedRateBand
    (std::vector<double>      & coi_rates
    ,std::vector<double> const& curr_coi_multiplier
    ,BasicValues         const& basic_values
    )
{
    round_to<double> const& round_coi_rate
        (basic_values.GetRoundingRules().round_coi_rate()
        );
    if(CCoiIsAnnual_)
        {
        // If experience rating is ALLOWED, not necessarily USED
        if(AllowExpRating_)
//      if(basic_values.Input_->UseExperienceRating)  // TODO ?? And this condition too?
            {
            bool apply_partial_mortality_multiplier = false;

            double experience_rating_coi_multiplier;
            if(basic_values.Input_->UseExperienceRating)
                {
                experience_rating_coi_multiplier = // if experience rated
                    ExpRatCoiMultCurr1_;
                if(UsePMQOnCurrCOI_)
                    {
//                    experience_rating_coi_multiplier *= basic_values.Input_->PartialMortTableMult; // TODO ?? expunge
                    apply_partial_mortality_multiplier = true;
                    }
                }
            else
                {
                experience_rating_coi_multiplier = // if not experience rated
                    ExpRatCoiMultCurr0_;
                }

            for(int j = 0; j < Length_; j++)
                {
                double multiplier = experience_rating_coi_multiplier;
                if(apply_partial_mortality_multiplier)
                    {
                    multiplier *= basic_values.Input_->VectorPartialMortalityMultiplier[j];
                    }
                double q = multiplier * curr_coi_multiplier[j] * coi_rates[j];
                q = coi_rate_from_q<double>()(q, MonthlyGuaranteedCoiRates_[j]);
                q *= MultiplicativeCoiRetention_;
                q += AdditiveCoiRetention_;
                q = std::min(q, MonthlyGuaranteedCoiRates_[j]);
                coi_rates[j] = round_coi_rate(q);
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
                coi_rates[j] = round_coi_rate(coi_rates[j]);
                }
            }
        }
    else
        {
        for(int j = 0; j < Length_; j++)
            {
            double q = coi_rates[j];
            // TODO ?? COI Multiple is applied to the monthly COI.
            q = std::min(curr_coi_multiplier[j] * q, MaxMonthlyCoiRate_);
            coi_rates[j] = round_coi_rate(q);
            }
        }
}

//============================================================================
void MortalityRates::SetOtherRates(BasicValues const& basic_values)
{
    Irc7702Q_ = basic_values.GetIRC7702Rates();

    if(AllowTerm_)
        {
        MonthlyCurrentTermCoiRates_    = basic_values.GetCurrentTermRates();
        MonthlyGuaranteedTermCoiRates_ = basic_values.GetGuaranteedTermRates();

        MakeCoiRateSubstandard(MonthlyCurrentTermCoiRates_, basic_values);
        MakeCoiRateSubstandard(MonthlyGuaranteedTermCoiRates_, basic_values);

        LMI_ASSERT(0 == MonthlyMidpointTermCoiRates_.size());
        for(int j = 0; j < Length_; j++)
            {
            // Here we take midpoint as average of monthly curr and guar.
            // Other approaches are possible.
            MonthlyMidpointTermCoiRates_.push_back
                (  0.5
                * (MonthlyCurrentTermCoiRates_[j] + MonthlyGuaranteedTermCoiRates_[j])
                );
            }
        }
    else
        {
        MonthlyCurrentTermCoiRates_   .assign(Length_, 0.0);
        MonthlyMidpointTermCoiRates_  .assign(Length_, 0.0);
        MonthlyGuaranteedTermCoiRates_.assign(Length_, 0.0);
        }

    if(AllowADD_)
        {
        ADDRates_ = basic_values.GetADDRates();
// TODO ?? No substandard support yet for this rider.
//        MakeCoiRateSubstandard(ADDRates_, basic_values);
        }
    else
        {
        ADDRates_.assign(Length_, 0.0);
        }

    if(AllowWP_)
        {
        WPRates_ = basic_values.GetWPRates();
// TODO ?? No substandard support yet for this rider.
//        MakeCoiRateSubstandard(WPRates_, basic_values);
        }
    else
        {
        WPRates_.assign(Length_, 0.0);
        }

    if(AllowSpouse_)
        {
        // Don't rate spouse rider--assume spouse not underwritten.
        CurrentSpouseRiderRates_    = basic_values.GetCurrentSpouseRiderRates();
        GuaranteedSpouseRiderRates_ = basic_values.GetGuaranteedSpouseRiderRates();
        LMI_ASSERT(0 == MidpointSpouseRiderRates_.size());
        for(int j = 0; j < Length_; j++)
            {
            // Here we take midpoint as average of monthly curr and guar.
            // Other approaches are possible.
            MidpointSpouseRiderRates_.push_back
                (  0.5
                * (CurrentSpouseRiderRates_[j] + GuaranteedSpouseRiderRates_[j])
                );
            }
        }
    else
        {
        CurrentSpouseRiderRates_    .assign(Length_, 0.0);
        GuaranteedSpouseRiderRates_ .assign(Length_, 0.0);
        MidpointSpouseRiderRates_   .assign(Length_, 0.0);
        }

    if(AllowChild_)
        {
        // Don't rate child rider--assume child not underwritten.
        ChildRiderRates_ = basic_values.GetChildRiderRates();
        }
    else
        {
        ChildRiderRates_.assign(Length_, 0.0);
        }

    if(IsTgtPremTabular_)
        {
        TargetPremiumRates_ = basic_values.GetTgtPremRates();
        }
    else
        {
        TargetPremiumRates_.assign(Length_, 0.0);
        }

// TODO ?? These things should be global to all products,
// and we should offer others to choose from too.
    TableYRates_ = basic_values.GetTableYRates();
/*
    for(int j = 0; j < Length_; j++)
        {
        TableYRates_[j] *= 12.0;  // stored as monthly
        }
*/

    // TODO ?? Not correct--just a placeholder for now.
    PartialMortalityQ_ = basic_values.Get83GamRates();

    // TODO ?? Why have these here, since they're already in the
    // basic-values class? Same question for 'TableYRates_' too.
    CvatCorridorFactors_ = basic_values.GetCvatCorridorFactors();
    SevenPayRates_ = basic_values.GetTAMRA7PayRates();

    // TODO ?? Temporary stuff to support NSP for 7702A
    // TODO ?? Incorrect if GPT
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
    ,BasicValues         const& basic_values
    )
{
    round_to<double> const& round_coi_rate
        (basic_values.GetRoundingRules().round_coi_rate()
        );
    InputStatus const& status = basic_values.Input_->Status[0];
    // Nothing to do if no rating.

    if(!status.IsPolicyRated())
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
    double TableMultiple = factors[status.SubstdTable];
    for(int j = 0; j < Length_; ++j)
        {
        // Flat extra: input as annual per K, want monthly per $.
        double flat_extra = status.VectorMonthlyFlatExtra[j] / 12000.0;

// TODO ?? Results here really should be rounded. Instead, for the
// nonce, we round only the maximum, in order to match old
// regression tests.
        double z = round_coi_rate(MaxMonthlyCoiRate_);
        coi_rates[j] = std::min
            (z
            ,flat_extra + coi_rates[j] * (1.0 + SubstdTblMult_[j] * TableMultiple)
            );

// TODO ?? Some UL admin systems convert flat extras to an integral
// number of cents per thousand per month. It would be nice to offer
// such a behavior here.
        }
}

//============================================================================
void MortalityRates::GradeCurrentCoiRatesFromPartialMortalityAssumption
    (std::vector<double>& coi_rates
    ,std::vector<double> const& monthly_partial_mortality
    ,std::vector<double> const& grading_factors
    ,BasicValues         const& basic_values
    )
{
    // It seems necessary to limit the result to the maximum monthly
    // COI rate, because one would never use a higher rate. No floor
    // need be imposed because both inputs are constrained to be
    // nonnegative.
    round_to<double> const& round_coi_rate
        (basic_values.GetRoundingRules().round_coi_rate()
        );
    for(unsigned int j = 0; j < coi_rates.size(); ++j)
        {
        coi_rates[j] = std::min
            (MaxMonthlyCoiRate_
            ,       coi_rates[j]                 * (1.0 - grading_factors[j])
                +   monthly_partial_mortality[j] *        grading_factors[j]
            );
        coi_rates[j] = round_coi_rate(coi_rates[j]);
        }
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRatesBand0
    (e_basis const& Basis
    ) const
{
    switch(Basis)
        {
        case e_currbasis: return MonthlyCurrentCoiRatesBand0_;
        case e_mdptbasis: return MonthlyMidpointCoiRatesBand0_;
        case e_guarbasis: return MonthlyGuaranteedCoiRates_;
        default:
            fatal_error()
                << "Case '"
                << Basis
                << "' not found."
                << LMI_FLUSH
                ;
        }
    throw("Logic error");
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRatesBand1
    (e_basis const& Basis
    ) const
{
    switch(Basis)
        {
        case e_currbasis: return MonthlyCurrentCoiRatesBand1_;
        case e_mdptbasis: return MonthlyMidpointCoiRatesBand1_;
        case e_guarbasis: return MonthlyGuaranteedCoiRates_;
        default:
            fatal_error()
                << "Case '"
                << Basis
                << "' not found."
                << LMI_FLUSH
                ;
        }
    throw("Logic error");
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyCoiRatesBand2
    (e_basis const& Basis
    ) const
{
    switch(Basis)
        {
        case e_currbasis: return MonthlyCurrentCoiRatesBand2_;
        case e_mdptbasis: return MonthlyMidpointCoiRatesBand2_;
        case e_guarbasis: return MonthlyGuaranteedCoiRates_;
        default:
            fatal_error()
                << "Case '"
                << Basis
                << "' not found."
                << LMI_FLUSH
                ;
        }
    throw("Logic error");
}

//============================================================================
std::vector<double> const& MortalityRates::SpouseRiderRates
    (e_basis const& Basis
    ) const
{
    switch(Basis)
        {
        case e_currbasis: return CurrentSpouseRiderRates_;
        case e_mdptbasis: return MidpointSpouseRiderRates_;
        case e_guarbasis: return GuaranteedSpouseRiderRates_;
        default:
            fatal_error()
                << "Case '"
                << Basis
                << "' not found."
                << LMI_FLUSH
                ;
        }
    throw("Logic error");
}

//============================================================================
std::vector<double> const& MortalityRates::MonthlyTermCoiRates
    (e_basis const& Basis
    ) const
{
    switch(Basis)
        {
        case e_currbasis: return MonthlyCurrentTermCoiRates_;
        case e_mdptbasis: return MonthlyMidpointTermCoiRates_;
        case e_guarbasis: return MonthlyGuaranteedTermCoiRates_;
        default:
            fatal_error()
                << "Case '"
                << Basis
                << "' not found."
                << LMI_FLUSH
                ;
        }
    throw("Logic error");
}

//============================================================================
std::vector<double> const& MortalityRates::TargetPremiumRates() const
{
    LMI_ASSERT(0 != TargetPremiumRates_.size());
    return TargetPremiumRates_;
}

