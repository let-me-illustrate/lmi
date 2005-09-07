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

// $Id: ihs_mortal.cpp,v 1.17 2005-09-07 03:04:54 chicares Exp $

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

// TODO ?? Rewrite, paying attention to the following issues.
//
// Mortality tables are read in class BasicValues, then used here.
// The vectors are used by value, not by reference, so they are
// copied needlessly. Tables should instead be read here.

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
}

//============================================================================
// TODO ?? Want alternative for unit testing.
void MortalityRates::fetch_parameters(BasicValues const& basic_values)
{
    // Some of these data members seem useless for now, but they will
    // become useful when mortality-table access is moved hither from
    // class BasicValues.
    AllowAdb_          = basic_values.Database_->Query(DB_AllowADD         );
    AllowChild_        = basic_values.Database_->Query(DB_AllowChild       );
    AllowExpRating_    = basic_values.Database_->Query(DB_AllowExpRating   );
    AllowFlatExtras_   = basic_values.Database_->Query(DB_AllowFlatExtras  );
    AllowSpouse_       = basic_values.Database_->Query(DB_AllowSpouse      );
    AllowSubstdTable_  = basic_values.Database_->Query(DB_AllowSubstdTable );
    AllowTerm_         = basic_values.Database_->Query(DB_AllowTerm        );
    AllowWp_           = basic_values.Database_->Query(DB_AllowWP          );
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
    UsePMQOnCurrCoiRate_= basic_values.Database_->Query(DB_UsePMQOnCurrCOI);

    basic_values.Database_->Query(GCoiMultiplier_, DB_GCOIMultiplier);
    basic_values.Database_->Query(CCoiMultiplier_, DB_CCOIMultiplier);
    basic_values.Database_->Query(SubstdTblMult_ , DB_SubstdTblMult );

    CountryCoiMultiplier_ = basic_values.Input_->CountryCOIMultiplier;
    IsPolicyRated_        = basic_values.Input_->Status[0].IsPolicyRated();
    SubstdTable_          = basic_values.Input_->Status[0].SubstdTable;
    UseExperienceRating_  = basic_values.Input_->UseExperienceRating;

    ShowGrading_          = std::string::npos != basic_values.Input_->Comments.find("idiosyncrasy_grading");

    CurrentCoiGrading_    = basic_values.Input_->VectorCurrentCoiGrading;
    CurrentCoiMultiplier_ = basic_values.Input_->VectorCurrentCoiMultiplier;
    MonthlyFlatExtra_     = basic_values.Input_->Status[0].VectorMonthlyFlatExtra;
    PartialMortalityMultiplier_ = basic_values.Input_->VectorPartialMortalityMultiplier;

    round_coi_rate_ = basic_values.GetRoundingRules().round_coi_rate();

// TODO ?? Rethink these "delicate" things. Should raw rates be stored
// temporarily in some other manner, e.g. using a handle-body idiom.

// TODO ?? These are delicate: they get modified downstream.
    MonthlyGuaranteedCoiRates_   = basic_values.GetGuarCOIRates();
    MonthlyCurrentCoiRatesBand0_ = basic_values.GetCurrCOIRates0();
    MonthlyCurrentCoiRatesBand1_ = basic_values.GetCurrCOIRates1();
    MonthlyCurrentCoiRatesBand2_ = basic_values.GetCurrCOIRates2();

// TODO ?? These are delicate: they are needed only conditionally.
    MonthlyGuaranteedTermCoiRates_ = basic_values.GetGuaranteedTermRates();
    MonthlyCurrentTermCoiRates_    = basic_values.GetCurrentTermRates();
    AdbRates_ = basic_values.GetAdbRates();
    WpRates_ = basic_values.GetWpRates();
    ChildRiderRates_ = basic_values.GetChildRiderRates();
    GuaranteedSpouseRiderRates_ = basic_values.GetGuaranteedSpouseRiderRates();
    CurrentSpouseRiderRates_    = basic_values.GetCurrentSpouseRiderRates();
    TargetPremiumRates_ = basic_values.GetTgtPremRates();
// Test this now, then eradicate calls through 'basic_values' below.

    // Alternative COI rate for experience rating.
    AlternativeMonthlyCoiRates_ = basic_values.GetSmokerBlendedGuarCOIRates();

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

    PartialMortalityQ_ = basic_values.Get83GamRates();

    CvatCorridorFactors_ = basic_values.GetCvatCorridorFactors();
    SevenPayRates_ = basic_values.GetTAMRA7PayRates();

// TODO ?? Move this up here?
//    CvatNspRates_;
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
        // TODO ?? If experience rating is ALLOWED, not necessarily USED?
        if(AllowExpRating_)
//      if(UseExperienceRating_)  // TODO ?? And this condition too?
            {
            MakeCoiRateSubstandard(AlternativeMonthlyCoiRates_);
            }
        MakeCoiRateSubstandard(MonthlyGuaranteedCoiRates_);
        }

    perform_grading();

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
void MortalityRates::perform_grading()
{
// TODO ?? Exit early instead of writing whole body in conditional.
    if
        (!each_equal
            (CurrentCoiGrading_.begin()
            ,CurrentCoiGrading_.end()
            ,0.0
            )
        )
        {
        std::vector<double> ungraded(MonthlyCurrentCoiRatesBand0_);

        // TODO ?? Ignores 'MaxSurvivalDur', which should be distinct
        // from partial mortality q, but isn't in the interface as of
        // 2004-03.
        //
        // ET !! Easier to write as
        //   std::vector<double> monthly_partial_mortality = coi_rate_from_q
        //     (max(0.0, min(1.0, PartialMortalityQ_ * PartialMortalityMultiplier_))
        //     ,MaxMonthlyCoiRate_
        //     );
        std::vector<double> monthly_partial_mortality(PartialMortalityQ_);
        std::transform
            (monthly_partial_mortality.begin()
            ,monthly_partial_mortality.end()
            ,PartialMortalityMultiplier_.begin()
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
            ,CurrentCoiGrading_
            );
        GradeCurrentCoiRatesFromPartialMortalityAssumption
            (MonthlyCurrentCoiRatesBand1_
            ,monthly_partial_mortality
            ,CurrentCoiGrading_
            );
        GradeCurrentCoiRatesFromPartialMortalityAssumption
            (MonthlyCurrentCoiRatesBand2_
            ,monthly_partial_mortality
            ,CurrentCoiGrading_
            );
        if(ShowGrading_)
            {
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
<< "        coi_rates[j] = round_coi_rate_(coi_rates[j]);\n"
<< "        }\n"

                << "\n'MaxMonthlyCoiRate_' is " << MaxMonthlyCoiRate_ << '\n'
                << "\nHere is the year-by-year calculation:\n\n"
                << "duration\tgrading_factor\tungraded_coi_rate\tpartial_mortality_rate\tresult\n"
                ;

            os << std::setiosflags(std::ios_base::scientific);

            int prec = 20;
            int width = 24;
            os << std::setprecision(prec) << std::setw(width);

            for(int j = 0; j < Length_; ++j)
                {
                os
                    << j
                    << '\t' << CurrentCoiGrading_[j]
                    << '\t' << ungraded[j]
                    << '\t' << monthly_partial_mortality[j]
                    << '\t' << MonthlyCurrentCoiRatesBand0_[j]
                    << '\n'
                    ;
                }
            }
        }
}

//============================================================================
void MortalityRates::SetGuaranteedRates()
{
    if(GCoiIsAnnual_) // TODO ?? Assume this means experience rated?
        {
        // If experience rating is ALLOWED, not necessarily USED.
        if(AllowExpRating_)
//      if(UseExperienceRating_) // TODO ?? And this condition too?
            {
            for(int j = 0; j < Length_; j++)
                {
                double qstart = MonthlyGuaranteedCoiRates_[j];
                double q = std::min(1.0, ExpRatCoiMultGuar_ * qstart);
                q = coi_rate_from_q<double>()(q, MaxMonthlyCoiRate_);
                MonthlyGuaranteedCoiRates_[j] = round_coi_rate_(q);

                q = std::min(1.0, ExpRatCoiMultAlt_ * qstart);
                q = coi_rate_from_q<double>()(q, MonthlyGuaranteedCoiRates_[j]);
                q = std::min(q, MonthlyGuaranteedCoiRates_[j]);
                AlternativeMonthlyCoiRates_[j] = round_coi_rate_(q);
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
//              MonthlyGuaranteedCoiRates_[j] *= GCoiMultiplier_[j];
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
    else  // gCoi is not annual
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
        // If experience rating is ALLOWED, not necessarily USED
        if(AllowExpRating_)
//      if(UseExperienceRating_)  // TODO ?? And this condition too?
            {
            bool apply_partial_mortality_multiplier = false;

            double experience_rating_coi_multiplier;
            if(UseExperienceRating_)
                {
                experience_rating_coi_multiplier = // if experience rated
                    ExpRatCoiMultCurr1_;
                if(UsePMQOnCurrCoiRate_)
                    {
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
                    multiplier *= PartialMortalityMultiplier_[j];
                    }
                double q = multiplier * curr_coi_multiplier[j] * coi_rates[j];
                q = coi_rate_from_q<double>()(q, MonthlyGuaranteedCoiRates_[j]);
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
            // TODO ?? COI Multiple is applied to the monthly COI.
            q = std::min(curr_coi_multiplier[j] * q, MaxMonthlyCoiRate_);
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
    double table_multiple = factors[SubstdTable_];
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
void MortalityRates::GradeCurrentCoiRatesFromPartialMortalityAssumption
    (std::vector<double>& coi_rates
    ,std::vector<double> const& monthly_partial_mortality
    ,std::vector<double> const& grading_factors
    )
{
    // It seems necessary to limit the result to the maximum monthly
    // COI rate, because one would never use a higher rate. No floor
    // need be imposed because both inputs are constrained to be
    // nonnegative.
    for(unsigned int j = 0; j < coi_rates.size(); ++j)
        {
        coi_rates[j] = std::min
            (MaxMonthlyCoiRate_
            ,       coi_rates[j]                 * (1.0 - grading_factors[j])
                +   monthly_partial_mortality[j] *        grading_factors[j]
            );
        coi_rates[j] = round_coi_rate_(coi_rates[j]);
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

