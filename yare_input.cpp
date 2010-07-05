// Illustration input ready for use in calculations.
//
// Copyright (C) 2008, 2009, 2010 Gregory W. Chicares.
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

#include "yare_input.hpp"

#include "input.hpp"
#include "miscellany.hpp" // each_equal()

#include <numeric>        // std::accumulate()

namespace
{
template<typename T>
std::vector<T> convert_vector_type
    (std::vector<mc_enum<T> > const& ve
    )
{
    std::vector<T> z;
    typename std::vector<mc_enum<T> >::const_iterator ve_i;
    for(ve_i = ve.begin(); ve_i != ve.end(); ++ve_i)
        {
        z.push_back(ve_i->value());
        }
    return z;
}

template<typename Number, typename Trammel>
std::vector<Number> convert_vector_type
    (std::vector<tn_range<Number,Trammel> > const& vr
    )
{
    std::vector<Number> z;
    typename std::vector<tn_range<Number,Trammel> >::const_iterator vr_i;
    for(vr_i = vr.begin(); vr_i != vr.end(); ++vr_i)
        {
        z.push_back(vr_i->value());
        }
    return z;
}
} // Unnamed namespace.

yare_input::yare_input(Input const& z)
{
// TODO ?? This temporarily works around a deeper defect. See:
//   http://lists.nongnu.org/archive/html/lmi/2009-02/msg00074.html
#if defined LMI_MSC
    const_cast<Input&>(z).RealizeAllSequenceInput();
#endif // defined LMI_MSC

    IssueAge                        = z.IssueAge                       .value();
    RetirementAge                   = z.RetirementAge                  .value();
    Gender                          = z.Gender                         .value();
    Smoking                         = z.Smoking                        .value();
    UnderwritingClass               = z.UnderwritingClass              .value();
    WaiverOfPremiumBenefit          = z.WaiverOfPremiumBenefit         .value();
    AccidentalDeathBenefit          = z.AccidentalDeathBenefit         .value();
    TermRider                       = z.TermRider                      .value();
    TermRiderAmount                 = z.TermRiderAmount                .value();
    SubstandardTable                = z.SubstandardTable               .value();
    ProductName                     = z.ProductName                    .value();
    Dumpin                          = z.Dumpin                         .value();
    External1035ExchangeAmount      = z.External1035ExchangeAmount     .value();
    External1035ExchangeBasis       = z.External1035ExchangeBasis      .value();
    External1035ExchangeFromMec     = z.External1035ExchangeFromMec    .value();
    Internal1035ExchangeAmount      = z.Internal1035ExchangeAmount     .value();
    Internal1035ExchangeBasis       = z.Internal1035ExchangeBasis      .value();
    Internal1035ExchangeFromMec     = z.Internal1035ExchangeFromMec    .value();
    SolveType                       = z.SolveType                      .value();
    SolveBeginYear                  = z.SolveBeginYear                 .value();
    SolveEndYear                    = z.SolveEndYear                   .value();
    SolveTarget                     = z.SolveTarget                    .value();
    SolveTargetCashSurrenderValue   = z.SolveTargetCashSurrenderValue  .value();
    SolveTargetYear                 = z.SolveTargetYear                .value();
    SolveBasis                      = z.SolveBasis                     .value();
    SolveSeparateAccountBasis       = z.SolveSeparateAccountBasis      .value();
    GeneralAccountRateType          = z.GeneralAccountRateType         .value();
    SeparateAccountRateType         = z.SeparateAccountRateType        .value();
    LoanRate                        = z.LoanRate                       .value();
    LoanRateType                    = z.LoanRateType                   .value();
    OverrideExperienceReserveRate   = z.OverrideExperienceReserveRate  .value();
    ExperienceReserveRate           = z.ExperienceReserveRate          .value();
    ExperienceRatingInitialKFactor  = z.ExperienceRatingInitialKFactor .value();
    InforceNetExperienceReserve     = z.InforceNetExperienceReserve    .value();
    InforceYtdNetCoiCharge          = z.InforceYtdNetCoiCharge         .value();
    WithdrawToBasisThenLoan         = z.WithdrawToBasisThenLoan        .value();
    UseAverageOfAllFunds            = z.UseAverageOfAllFunds           .value();
    OverrideFundManagementFee       = z.OverrideFundManagementFee      .value();
    InputFundManagementFee          = z.InputFundManagementFee         .value();
    RunOrder                        = z.RunOrder                       .value();
    NumberOfIdenticalLives          = z.NumberOfIdenticalLives         .value();
    UseExperienceRating             = z.UseExperienceRating            .value();
    UsePartialMortality             = z.UsePartialMortality            .value();
    State                           = z.State                          .value();
    CorporationState                = z.CorporationState               .value();
    InsuredPremiumTableFactor       = z.InsuredPremiumTableFactor      .value();
    CorporationPremiumTableFactor   = z.CorporationPremiumTableFactor  .value();
    EffectiveDate                   = z.EffectiveDate                  .value();
    DefinitionOfLifeInsurance       = z.DefinitionOfLifeInsurance      .value();
    DefinitionOfMaterialChange      = z.DefinitionOfMaterialChange     .value();
    AvoidMecMethod                  = z.AvoidMecMethod                 .value();
    RetireesCanEnroll               = z.RetireesCanEnroll              .value();
    GroupUnderwritingType           = z.GroupUnderwritingType          .value();
    LastCoiReentryDate              = z.LastCoiReentryDate             .value();
    BlendGender                     = z.BlendGender                    .value();
    BlendSmoking                    = z.BlendSmoking                   .value();
    MaleProportion                  = z.MaleProportion                 .value();
    NonsmokerProportion             = z.NonsmokerProportion            .value();
    TermAdjustmentMethod            = z.TermAdjustmentMethod           .value();
    IncludeInComposite              = z.IncludeInComposite             .value();
    Comments                        = z.Comments                       .value();
    AmortizePremiumLoad             = z.AmortizePremiumLoad            .value();
    InforceYear                     = z.InforceYear                    .value();
    InforceMonth                    = z.InforceMonth                   .value();
    InforceGeneralAccountValue      = z.InforceGeneralAccountValue     .value();
    InforceSeparateAccountValue     = z.InforceSeparateAccountValue    .value();
    InforceRegularLoanValue         = z.InforceRegularLoanValue        .value();
    InforcePreferredLoanValue       = z.InforcePreferredLoanValue      .value();
    InforceRegularLoanBalance       = z.InforceRegularLoanBalance      .value();
    InforcePreferredLoanBalance     = z.InforcePreferredLoanBalance    .value();
    InforceCumulativeNoLapsePremium = z.InforceCumulativeNoLapsePremium.value();
    InforceCumulativePayments       = z.InforceCumulativePayments      .value();
    CountryCoiMultiplier            = z.CountryCoiMultiplier           .value();
    SurviveToType                   = z.SurviveToType                  .value();
    SurviveToYear                   = z.SurviveToYear                  .value();
    SurviveToAge                    = z.SurviveToAge                   .value();
    MaximumNaar                     = z.MaximumNaar                    .value();
    ChildRider                      = z.ChildRider                     .value();
    ChildRiderAmount                = z.ChildRiderAmount               .value();
    SpouseRider                     = z.SpouseRider                    .value();
    SpouseRiderAmount               = z.SpouseRiderAmount              .value();
    SpouseIssueAge                  = z.SpouseIssueAge                 .value();
    InforceTaxBasis                 = z.InforceTaxBasis                .value();
    InforceIsMec                    = z.InforceIsMec                   .value();
    LastMaterialChangeDate          = z.LastMaterialChangeDate         .value();
    InforceDcv                      = z.InforceDcv                     .value();
    InforceAvBeforeLastMc           = z.InforceAvBeforeLastMc          .value();
    InforceContractYear             = z.InforceContractYear            .value();
    InforceContractMonth            = z.InforceContractMonth           .value();
    InforceLeastDeathBenefit        = z.InforceLeastDeathBenefit       .value();
    StateOfJurisdiction             = z.StateOfJurisdiction            .value();
    SalarySpecifiedAmountFactor     = z.SalarySpecifiedAmountFactor    .value();
    SalarySpecifiedAmountCap        = z.SalarySpecifiedAmountCap       .value();
    SalarySpecifiedAmountOffset     = z.SalarySpecifiedAmountOffset    .value();
    HoneymoonEndorsement            = z.HoneymoonEndorsement           .value();
    PostHoneymoonSpread             = z.PostHoneymoonSpread            .value();
    InforceHoneymoonValue           = z.InforceHoneymoonValue          .value();
    ExtraMonthlyCustodialFee        = convert_vector_type<double              >(z.ExtraMonthlyCustodialFeeRealized_       );
    ExtraCompensationOnAssets       = convert_vector_type<double              >(z.ExtraCompensationOnAssetsRealized_      );
    ExtraCompensationOnPremium      = convert_vector_type<double              >(z.ExtraCompensationOnPremiumRealized_     );
    PartialMortalityMultiplier      = convert_vector_type<double              >(z.PartialMortalityMultiplierRealized_     );
    CurrentCoiMultiplier            = convert_vector_type<double              >(z.CurrentCoiMultiplierRealized_           );
    CorporationTaxBracket           = convert_vector_type<double              >(z.CorporationTaxBracketRealized_          );
    TaxBracket                      = convert_vector_type<double              >(z.TaxBracketRealized_                     );
    ProjectedSalary                 = convert_vector_type<double              >(z.ProjectedSalaryRealized_                );
    SpecifiedAmount                 = convert_vector_type<double              >(z.SpecifiedAmountRealized_                );
    DeathBenefitOption              = convert_vector_type<mcenum_dbopt        >(z.DeathBenefitOptionRealized_             );
    Payment                         = convert_vector_type<double              >(z.PaymentRealized_                        );
    PaymentMode                     = convert_vector_type<mcenum_mode         >(z.PaymentModeRealized_                    );
    CorporationPayment              = convert_vector_type<double              >(z.CorporationPaymentRealized_             );
    CorporationPaymentMode          = convert_vector_type<mcenum_mode         >(z.CorporationPaymentModeRealized_         );
    GeneralAccountRate              = convert_vector_type<double              >(z.GeneralAccountRateRealized_             );
    SeparateAccountRate             = convert_vector_type<double              >(z.SeparateAccountRateRealized_            );
    NewLoan                         = convert_vector_type<double              >(z.NewLoanRealized_                        );
    Withdrawal                      = convert_vector_type<double              >(z.WithdrawalRealized_                     );
    FlatExtra                       = convert_vector_type<double              >(z.FlatExtraRealized_                      );
    HoneymoonValueSpread            = convert_vector_type<double              >(z.HoneymoonValueSpreadRealized_           );
    PremiumHistory                  = convert_vector_type<double              >(z.PremiumHistoryRealized_                 );
    SpecamtHistory                  = convert_vector_type<double              >(z.SpecamtHistoryRealized_                 );
// INPUT !! Not yet implemented:
    FundAllocations                 = convert_vector_type<double              >(z.FundAllocationsRealized_                );
    CashValueEnhancementRate        = convert_vector_type<double              >(z.CashValueEnhancementRateRealized_       );
    SpecifiedAmountStrategy         = convert_vector_type<mcenum_sa_strategy  >(z.SpecifiedAmountStrategyRealized_        );
    PaymentStrategy                 = convert_vector_type<mcenum_pmt_strategy >(z.PaymentStrategyRealized_      );
    CorporationPaymentStrategy      = convert_vector_type<mcenum_pmt_strategy >(z.CorporationPaymentStrategyRealized_     );
}

yare_input::~yare_input()
{
}

bool is_policy_rated(yare_input const& z)
{
    return
            mce_table_none != z.SubstandardTable
        ||  !each_equal(z.FlatExtra.begin(), z.FlatExtra.end(), 0.0)
        ;
}

/// Calculate proportion of premium allocated to separate account.
///
/// If "average of all funds" or "custom fund" is selected, then this
/// proportion is taken as unity.
///
/// Otherwise, it's the sum of allocations to all separate-account
/// funds. Those allocations are represented as percentages [0, 100];
/// because 100 * .01 does not exactly equal unity, treat a 100% sum
/// as a special case in order to avoid catastrophic cancellation when
/// calculating general-account proportion as the difference between
/// this quantity and unity.

double premium_allocation_to_sepacct(yare_input const& yi)
{
    double z = 0.0;
    double sum_of_fund_allocations = std::accumulate
        (yi.FundAllocations.begin()
        ,yi.FundAllocations.end()
        ,0.0
        );
    if(yi.UseAverageOfAllFunds || yi.OverrideFundManagementFee)
        {
        z = 1.0;
        }
    else if(100.0 == sum_of_fund_allocations)
        {
        z = 1.0;
        }
    else
        {
        z = .01 * sum_of_fund_allocations;
        }
    return z;
}

