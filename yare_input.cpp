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

// $Id: yare_input.cpp,v 1.4 2008-07-07 17:18:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "yare_input.hpp"

#include "input.hpp"
#include "inputs.hpp"

yare_input::yare_input(Input const& z)
{
(void)z;
/*
    int                               IssueAge                        ;
    int                               RetirementAge                   ;
    mcenum_gender                     Gender                          ;
    mcenum_smoking                    Smoking                         ;
    mcenum_class                      UnderwritingClass               ;
    bool                              WaiverOfPremiumBenefit          ;
    bool                              AccidentalDeathBenefit          ;
    bool                              TermRider                       ;
    double                            TermRiderAmount                 ;
    mcenum_table_rating               SubstandardTable                ;
    std::string                       ProductName                     ;
    double                            Dumpin                          ;
    double                            External1035ExchangeAmount      ;
    double                            External1035ExchangeBasis       ;
    bool                              External1035ExchangeFromMec     ;
    double                            Internal1035ExchangeAmount      ;
    double                            Internal1035ExchangeBasis       ;
    bool                              Internal1035ExchangeFromMec     ;
    mcenum_solve_type                 SolveType                       ;
    int                               SolveBeginYear                  ;
    int                               SolveEndYear                    ;
    mcenum_solve_target               SolveTarget                     ;
    double                            SolveTargetCashSurrenderValue   ;
    int                               SolveTargetYear                 ;
    mcenum_gen_basis                  SolveBasis                      ;
    mcenum_sep_basis                  SolveSeparateAccountBasis       ;
    mcenum_interest_rate_type         GeneralAccountRateType          ;
    mcenum_interest_rate_type         SeparateAccountRateType         ;
    double                            LoanRate                        ;
    mcenum_loan_rate_type             LoanRateType                    ;
    bool                              OverrideExperienceReserveRate   ;
    double                            ExperienceReserveRate           ;
    double                            ExperienceRatingInitialKFactor  ;
    double                            InforceNetExperienceReserve     ;
    double                            InforceYtdNetCoiCharge          ;
    bool                              WithdrawToBasisThenLoan         ;
    bool                              UseAverageOfAllFunds            ;
    bool                              OverrideFundManagementFee       ;
    double                            InputFundManagementFee          ;
    mcenum_run_order                  RunOrder                        ;
    int                               NumberOfIdenticalLives          ;
    bool                              UseExperienceRating             ;
    bool                              UsePartialMortality             ;
    mcenum_state                      State                           ;
    mcenum_state                      CorporationState                ;
    double                            InsuredPremiumTableFactor       ;
    double                            CorporationPremiumTableFactor   ;
    calendar_date                     EffectiveDate                   ;
    mcenum_defn_life_ins              DefinitionOfLifeInsurance       ;
    mcenum_defn_material_change       DefinitionOfMaterialChange      ;
    mcenum_mec_avoid_method           AvoidMecMethod                  ;
    bool                              RetireesCanEnroll               ;
    mcenum_uw_basis                   GroupUnderwritingType           ;
    calendar_date                     LastCoiReentryDate              ;
    bool                              BlendGender                     ;
    bool                              BlendSmoking                    ;
    double                            MaleProportion                  ;
    double                            NonsmokerProportion             ;
    mcenum_term_adj_method            TermAdjustmentMethod            ;
    bool                              IncludeInComposite              ;
    std::string                       Comments                        ;
    bool                              AmortizePremiumLoad             ;
    int                               InforceYear                     ;
    int                               InforceMonth                    ;
    double                            InforceGeneralAccountValue      ;
    double                            InforceSeparateAccountValue     ;
    double                            InforceRegularLoanValue         ;
    double                            InforcePreferredLoanValue       ;
    double                            InforceRegularLoanBalance       ;
    double                            InforcePreferredLoanBalance     ;
    double                            InforceCumulativeNoLapsePremium ;
    double                            InforceCumulativePayments       ;
    mcenum_survival_limit             SurviveToType                   ;
    int                               SurviveToYear                   ;
    int                               SurviveToAge                    ;
    double                            MaximumNaar                     ;
    bool                              ChildRider                      ;
    double                            ChildRiderAmount                ;
    bool                              SpouseRider                     ;
    double                            SpouseRiderAmount               ;
    int                               SpouseIssueAge                  ;
    double                            InforceTaxBasis                 ;
    bool                              InforceIsMec                    ;
    double                            InforceDcv                      ;
    double                            InforceAvBeforeLastMc           ;
    int                               InforceContractYear             ;
    int                               InforceContractMonth            ;
    double                            InforceLeastDeathBenefit        ;
    mcenum_state                      StateOfJurisdiction             ;
    double                            SalarySpecifiedAmountFactor     ;
    double                            SalarySpecifiedAmountCap        ;
    double                            SalarySpecifiedAmountOffset     ;
    bool                              HoneymoonEndorsement            ;
    double                            PostHoneymoonSpread             ;
    double                            InforceHoneymoonValue           ;
    std::vector<double>               ExtraMonthlyCustodialFee        ;
    std::vector<double>               ExtraCompensationOnAssets       ;
    std::vector<double>               ExtraCompensationOnPremium      ;
    std::vector<double>               PartialMortalityMultiplier      ;
    std::vector<double>               CurrentCoiMultiplier            ;
    std::vector<double>               ProjectedSalary                 ;
    std::vector<double>               SpecifiedAmount                 ;
    std::vector<mcenum_dbopt>         DeathBenefitOption              ;
    std::vector<double>               Payment                         ;
    std::vector<mcenum_mode>          PaymentMode                     ;
    std::vector<double>               CorporationPayment              ;
    std::vector<mcenum_mode>          CorporationPaymentMode          ;
    std::vector<double>               GeneralAccountRate              ;
    std::vector<double>               SeparateAccountRate             ;
    std::vector<double>               NewLoan                         ;
    std::vector<double>               Withdrawal                      ;
    std::vector<double>               FlatExtra                       ;
    std::vector<double>               HoneymoonValueSpread            ;
    std::vector<double>               PremiumHistory                  ;
    std::vector<double>               SpecamtHistory                  ;
    std::vector<double>               CashValueEnhancementRate        ;
    std::vector<mcenum_sa_strategy  > SpecifiedAmountStrategy         ;
    std::vector<mcenum_pmt_strategy > IndividualPaymentStrategy       ;
    std::vector<mcenum_pmt_strategy > CorporationPaymentStrategy      ;
    std::vector<mcenum_loan_strategy> NewLoanStrategy                 ;
    std::vector<mcenum_wd_strategy  > WithdrawalStrategy              ;
*/
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

yare_input::yare_input(InputParms const& z)
{
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
    CashValueEnhancementRate        = z.VectorCashValueEnhancementRate  ;
    SpecifiedAmountStrategy         = convert_vector_type<mcenum_sa_strategy  >(z.VectorSpecifiedAmountStrategy);
    IndividualPaymentStrategy       = convert_vector_type<mcenum_pmt_strategy >(z.VectorIndvPaymentStrategy    );
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

