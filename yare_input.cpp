// Illustration input ready for use in calculations.
//
// Copyright (C) 2008 Gregory W. Chicares.
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

// $Id: yare_input.cpp,v 1.13 2008-08-10 01:12:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "yare_input.hpp"

#include "input.hpp"
#include "inputillus.hpp"
#include "miscellany.hpp" // each_equal()

namespace
{
template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::vector<T> convert_vector_type
    (std::vector<mc_enum<T,n,e,c> > const& ve
    )
{
    std::vector<T> z;
    typename std::vector<mc_enum<T,n,e,c> >::const_iterator ve_i;
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
// INPUT !! Not yet implemented:
//    NewLoanStrategy                 = convert_vector_type<mcenum_loan_strategy>(z.NewLoanStrategyRealized_                );
//    WithdrawalStrategy              = convert_vector_type<mcenum_wd_strategy  >(z.WithdrawalStrategyRealized_             );
}

namespace
{
template<typename NewType, typename XEnumType, int N>
std::vector<NewType> convert_vector_type
    (std::vector<xenum<XEnumType,N> > const& ve
    )
{
    std::vector<NewType> z;
    typename std::vector<xenum<XEnumType,N> >::const_iterator ve_i;
    for(ve_i = ve.begin(); ve_i != ve.end(); ++ve_i)
        {
        z.push_back(static_cast<NewType>(ve_i->value()));
        }
    return z;
}

template<typename NewType, typename Essence, typename Substance>
std::vector<NewType> convert_vector_type
    (std::vector<xrange<Essence,Substance> > const& vr
    )
{
    std::vector<NewType> z;
    typename std::vector<xrange<Essence,Substance> >::const_iterator vr_i;
    for(vr_i = vr.begin(); vr_i != vr.end(); ++vr_i)
        {
        z.push_back(static_cast<NewType>(vr_i->value()));
        }
    return z;
}
} // Unnamed namespace.

yare_input::yare_input(IllusInputParms const& z)
{
    // DEPRECATED This works around an ancient problem that eventually
    // must be fixed at its source.
    z.SetSolveDurations();

    IssueAge                        = z.Status[0].IssueAge              ;
    RetirementAge                   = z.Status[0].RetAge                ;
    Gender                          = static_cast<mcenum_gender              >(z.Status[0].Gender          .value());
    Smoking                         = static_cast<mcenum_smoking             >(z.Status[0].Smoking         .value());
    UnderwritingClass               = static_cast<mcenum_class               >(z.Status[0].Class           .value());
    WaiverOfPremiumBenefit          = z.Status[0].HasWP                 ;
    AccidentalDeathBenefit          = z.Status[0].HasADD                ;
    TermRider                       = z.Status[0].HasTerm               ;
    TermRiderAmount                 = z.Status[0].TermAmt               ;
    SubstandardTable                = static_cast<mcenum_table_rating        >(z.Status[0].SubstdTable     .value());
    ProductName                     = z.ProductName                     ;
    Dumpin                          = z.Dumpin                          ;
    External1035ExchangeAmount      = z.External1035ExchangeAmount      ;
    External1035ExchangeBasis       = z.External1035ExchangeBasis       ;
    External1035ExchangeFromMec     = z.External1035ExchangeFromMec     ;
    Internal1035ExchangeAmount      = z.Internal1035ExchangeAmount      ;
    Internal1035ExchangeBasis       = z.Internal1035ExchangeBasis       ;
    Internal1035ExchangeFromMec     = z.Internal1035ExchangeFromMec     ;
    SolveType                       = static_cast<mcenum_solve_type          >(z.SolveType                 .value());
    SolveBeginYear                  = z.SolveBegYear                    ;
    SolveEndYear                    = z.SolveEndYear                    ;
    SolveTarget                     = static_cast<mcenum_solve_target        >(z.SolveTarget               .value());
    SolveTargetCashSurrenderValue   = z.SolveTgtCSV                     ;
    SolveTargetYear                 = z.SolveTgtYear                    ;
    SolveBasis                      = static_cast<mcenum_gen_basis           >(z.SolveBasis                .value());
    SolveSeparateAccountBasis       = static_cast<mcenum_sep_basis           >(z.SolveSABasis              .value());
    GeneralAccountRateType          = static_cast<mcenum_interest_rate_type  >(z.IntRateTypeGA             .value());
    SeparateAccountRateType         = static_cast<mcenum_interest_rate_type  >(z.IntRateTypeSA             .value());
    LoanRate                        = z.LoanIntRate                     ;
    LoanRateType                    = static_cast<mcenum_loan_rate_type      >(z.LoanRateType              .value());
    OverrideExperienceReserveRate   = z.OverrideExperienceReserveRate   ;
    ExperienceReserveRate           = z.ExperienceReserveRate           ;
    ExperienceRatingInitialKFactor  = z.ExperienceRatingInitialKFactor  ;
    InforceNetExperienceReserve     = z.InforceNetExperienceReserve     ;
    InforceYtdNetCoiCharge          = z.InforceYtdNetCoiCharge          ;
    WithdrawToBasisThenLoan         = z.WDToBasisThenLoan               ;
    UseAverageOfAllFunds            = z.AvgFund                         ;
    OverrideFundManagementFee       = z.OverrideFundMgmtFee             ;
    InputFundManagementFee          = z.InputFundMgmtFee                ;
    RunOrder                        = static_cast<mcenum_run_order           >(z.RunOrder                  .value());
    NumberOfIdenticalLives          = z.NumIdenticalLives               ;
    UseExperienceRating             = z.UseExperienceRating             ;
    UsePartialMortality             = z.UsePartialMort                  ;
    State                           = static_cast<mcenum_state               >(z.InsdState                 .value());
    CorporationState                = static_cast<mcenum_state               >(z.SponsorState              .value());
    InsuredPremiumTableFactor       = z.EePremTableMult                 ;
    CorporationPremiumTableFactor   = z.ErPremTableMult                 ;
    EffectiveDate                   = z.EffDate                         ;
    DefinitionOfLifeInsurance       = static_cast<mcenum_defn_life_ins       >(z.DefnLifeIns               .value());
    DefinitionOfMaterialChange      = static_cast<mcenum_defn_material_change>(z.DefnMaterialChange        .value());
    AvoidMecMethod                  = static_cast<mcenum_mec_avoid_method    >(z.AvoidMec                  .value());
    RetireesCanEnroll               = z.RetireesCanEnroll               ;
    GroupUnderwritingType           = static_cast<mcenum_uw_basis            >(z.GroupUWType               .value());
    LastCoiReentryDate              = z.LastCoiReentryDate              ;
    BlendGender                     = z.BlendMortGender                 ;
    BlendSmoking                    = z.BlendMortSmoking                ;
    MaleProportion                  = z.MaleProportion                  ;
    NonsmokerProportion             = z.NonsmokerProportion             ;
    TermAdjustmentMethod            = static_cast<mcenum_term_adj_method     >(z.TermAdj                   .value());
    IncludeInComposite              = z.IncludeInComposite              ;
    Comments                        = z.Comments                        ;
    AmortizePremiumLoad             = z.AmortizePremLoad                ;
    InforceYear                     = z.InforceYear                     ;
    InforceMonth                    = z.InforceMonth                    ;
    InforceGeneralAccountValue      = z.InforceAVGenAcct                ;
    InforceSeparateAccountValue     = z.InforceAVSepAcct                ;
    InforceRegularLoanValue         = z.InforceAVRegLn                  ;
    InforcePreferredLoanValue       = z.InforceAVPrfLn                  ;
    InforceRegularLoanBalance       = z.InforceRegLnBal                 ;
    InforcePreferredLoanBalance     = z.InforcePrfLnBal                 ;
    InforceCumulativeNoLapsePremium = z.InforceCumNoLapsePrem           ;
    InforceCumulativePayments       = z.InforceCumPmts                  ;
    CountryCoiMultiplier            = z.CountryCOIMultiplier            ;
    SurviveToType                   = static_cast<mcenum_survival_limit      >(z.SurviveToType             .value());
    SurviveToYear                   = z.SurviveToYear                   ;
    SurviveToAge                    = z.SurviveToAge                    ;
    MaximumNaar                     = z.MaxNAAR                         ;
    ChildRider                      = z.HasChildRider                   ;
    ChildRiderAmount                = z.ChildRiderAmount                ;
    SpouseRider                     = z.HasSpouseRider                  ;
    SpouseRiderAmount               = z.SpouseRiderAmount               ;
    SpouseIssueAge                  = z.SpouseIssueAge                  ;
    InforceTaxBasis                 = z.InforceTaxBasis                 ;
    InforceIsMec                    = z.InforceIsMec                    ;
    InforceDcv                      = z.InforceDcv                      ;
    InforceAvBeforeLastMc           = z.InforceAvBeforeLastMc           ;
    InforceContractYear             = z.InforceContractYear             ;
    InforceContractMonth            = z.InforceContractMonth            ;
    InforceLeastDeathBenefit        = z.InforceLeastDeathBenefit        ;
    StateOfJurisdiction             = static_cast<mcenum_state               >(z.StateOfJurisdiction       .value());
    SalarySpecifiedAmountFactor     = z.SalarySAPct                     ;
    SalarySpecifiedAmountCap        = z.SalarySACap                     ;
    SalarySpecifiedAmountOffset     = z.SalarySAOffset                  ;
    HoneymoonEndorsement            = z.HasHoneymoon                    ;
    PostHoneymoonSpread             = z.PostHoneymoonSpread             ;
    InforceHoneymoonValue           = z.InforceHoneymoonValue           ;
    ExtraMonthlyCustodialFee        = z.VectorAddonMonthlyCustodialFee  ;
    ExtraCompensationOnAssets       = z.VectorAddonCompOnAssets         ;
    ExtraCompensationOnPremium      = z.VectorAddonCompOnPremium        ;
    PartialMortalityMultiplier      = z.VectorPartialMortalityMultiplier;
    CurrentCoiMultiplier            = z.VectorCurrentCoiMultiplier      ;
    CorporationTaxBracket           = z.VectorCorpTaxBracket            ;
    TaxBracket                      = z.VectorIndvTaxBracket            ;
    ProjectedSalary                 = z.Salary                          ;
    SpecifiedAmount                 = convert_vector_type<double              >(z.SpecAmt                      );
    DeathBenefitOption              = convert_vector_type<mcenum_dbopt        >(z.DBOpt                        );
    Payment                         = convert_vector_type<double              >(z.EePremium                    );
    PaymentMode                     = convert_vector_type<mcenum_mode         >(z.EeMode                       );
    CorporationPayment              = convert_vector_type<double              >(z.ErPremium                    );
    CorporationPaymentMode          = convert_vector_type<mcenum_mode         >(z.ErMode                       );
    GeneralAccountRate              = convert_vector_type<double              >(z.GenAcctRate                  );
    SeparateAccountRate             = convert_vector_type<double              >(z.SepAcctRate                  );
    NewLoan                         = convert_vector_type<double              >(z.Loan                         );
    Withdrawal                      = convert_vector_type<double              >(z.WD                           );
    FlatExtra                       = z.Status[0].VectorMonthlyFlatExtra;
    HoneymoonValueSpread            = z.VectorHoneymoonValueSpread      ;
    PremiumHistory                  = convert_vector_type<double              >(z.VectorPremiumHistory         );
    SpecamtHistory                  = convert_vector_type<double              >(z.VectorSpecamtHistory         );
    FundAllocations                 = convert_vector_type<double              >(z.FundAllocs                   );
    CashValueEnhancementRate        = z.VectorCashValueEnhancementRate  ;
    SpecifiedAmountStrategy         = convert_vector_type<mcenum_sa_strategy  >(z.VectorSpecifiedAmountStrategy);
    PaymentStrategy                 = convert_vector_type<mcenum_pmt_strategy >(z.VectorIndvPaymentStrategy    );
    CorporationPaymentStrategy      = convert_vector_type<mcenum_pmt_strategy >(z.VectorCorpPaymentStrategy    );
    NewLoanStrategy                 = convert_vector_type<mcenum_loan_strategy>(z.VectorNewLoanStrategy        );
    WithdrawalStrategy              = convert_vector_type<mcenum_wd_strategy  >(z.VectorWithdrawalStrategy     );

    // TODO ?? Resolve this ancient backward-compatibility issue.
    if(mce_cred_rate == GeneralAccountRateType)
        {
        GeneralAccountRateType = mce_net_rate;
        }
    if(mce_cred_rate == SeparateAccountRateType)
        {
        SeparateAccountRateType = mce_net_rate;
        }
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

