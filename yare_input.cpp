// Illustration input ready for use in calculations.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "yare_input.hpp"

#include "input.hpp"
#include "input_sequence_aux.hpp"       // convert_vector_type()
#include "miscellany.hpp"               // each_equal()

#include <numeric>                      // accumulate()

yare_input::yare_input(Input const& z)
{
// TODO ?? This temporarily works around a deeper defect. See:
//   https://lists.nongnu.org/archive/html/lmi/2009-02/msg00074.html
#if defined LMI_MSC
    const_cast<Input&>(z).RealizeAllSequenceInput();
#endif // defined LMI_MSC

    IssueAge                         = z.IssueAge                        .value();
    RetirementAge                    = z.RetirementAge                   .value();
    Gender                           = z.Gender                          .value();
    Smoking                          = z.Smoking                         .value();
    UnderwritingClass                = z.UnderwritingClass               .value();
    WaiverOfPremiumBenefit           = z.WaiverOfPremiumBenefit          .value();
    AccidentalDeathBenefit           = z.AccidentalDeathBenefit          .value();
    TermRider                        = z.TermRider                       .value();
    TermRiderAmount                  = z.TermRiderAmount                 .value();
    DateOfBirth                      = z.DateOfBirth                     .value();
    SubstandardTable                 = z.SubstandardTable                .value();
    ProductName                      = z.ProductName                     .value();
    Dumpin                           = z.Dumpin                          .value();
    External1035ExchangeAmount       = z.External1035ExchangeAmount      .value();
    External1035ExchangeTaxBasis     = z.External1035ExchangeTaxBasis    .value();
    External1035ExchangeFromMec      = z.External1035ExchangeFromMec     .value();
    Internal1035ExchangeAmount       = z.Internal1035ExchangeAmount      .value();
    Internal1035ExchangeTaxBasis     = z.Internal1035ExchangeTaxBasis    .value();
    Internal1035ExchangeFromMec      = z.Internal1035ExchangeFromMec     .value();
    SolveType                        = z.SolveType                       .value();
    SolveBeginYear                   = z.SolveBeginYear                  .value();
    SolveEndYear                     = z.SolveEndYear                    .value();
    SolveTarget                      = z.SolveTarget                     .value();
    SolveTargetValue                 = z.SolveTargetValue                .value();
    SolveTargetYear                  = z.SolveTargetYear                 .value();
    SolveExpenseGeneralAccountBasis  = z.SolveExpenseGeneralAccountBasis .value();
    SolveSeparateAccountBasis        = z.SolveSeparateAccountBasis       .value();
    GeneralAccountRateType           = z.GeneralAccountRateType          .value();
    SeparateAccountRateType          = z.SeparateAccountRateType         .value();
    LoanRate                         = z.LoanRate                        .value();
    LoanRateType                     = z.LoanRateType                    .value();
    WithdrawToBasisThenLoan          = z.WithdrawToBasisThenLoan         .value();
    UseAverageOfAllFunds             = z.UseAverageOfAllFunds            .value();
    OverrideFundManagementFee        = z.OverrideFundManagementFee       .value();
    InputFundManagementFee           = z.InputFundManagementFee          .value();
    RunOrder                         = z.RunOrder                        .value();
    NumberOfIdenticalLives           = z.NumberOfIdenticalLives          .value();
    UsePartialMortality              = z.UsePartialMortality             .value();
    InsuredName                      = z.InsuredName                     .value();
    CorporationName                  = z.CorporationName                 .value();
    AgentName                        = z.AgentName                       .value();
    AgentAddress                     = z.AgentAddress                    .value();
    AgentCity                        = z.AgentCity                       .value();
    AgentState                       = z.AgentState                      .value();
    AgentZipCode                     = z.AgentZipCode                    .value();
    AgentPhone                       = z.AgentPhone                      .value();
    AgentId                          = z.AgentId                         .value();
    InsuredPremiumTableFactor        = z.InsuredPremiumTableFactor       .value();
    CorporationPremiumTableFactor    = z.CorporationPremiumTableFactor   .value();
    EffectiveDate                    = z.EffectiveDate                   .value();
    DefinitionOfLifeInsurance        = z.DefinitionOfLifeInsurance       .value();
    DefinitionOfMaterialChange       = z.DefinitionOfMaterialChange      .value();
    AvoidMecMethod                   = z.AvoidMecMethod                  .value();
    RetireesCanEnroll                = z.RetireesCanEnroll               .value();
    GroupUnderwritingType            = z.GroupUnderwritingType           .value();
    LastCoiReentryDate               = z.LastCoiReentryDate              .value();
    ListBillDate                     = z.ListBillDate                    .value();
    BlendGender                      = z.BlendGender                     .value();
    BlendSmoking                     = z.BlendSmoking                    .value();
    MaleProportion                   = z.MaleProportion                  .value();
    NonsmokerProportion              = z.NonsmokerProportion             .value();
    TermAdjustmentMethod             = z.TermAdjustmentMethod            .value();
    IncludeInComposite               = z.IncludeInComposite              .value();
    Comments                         = z.Comments                        .value();
    AmortizePremiumLoad              = z.AmortizePremiumLoad             .value();
    ContractNumber                   = z.ContractNumber                  .value();
    MasterContractNumber             = z.MasterContractNumber            .value();
    InforceAsOfDate                  = z.InforceAsOfDate                 .value();
    InforceYear                      = z.InforceYear                     .value();
    InforceMonth                     = z.InforceMonth                    .value();
    InforceAnnualTargetPremium       = z.InforceAnnualTargetPremium      .value();
    InforceYtdGrossPremium           = z.InforceYtdGrossPremium          .value();
    InforceGeneralAccountValue       = z.InforceGeneralAccountValue      .value();
    InforceSeparateAccountValue      = z.InforceSeparateAccountValue     .value();
    InforceRegularLoanValue          = z.InforceRegularLoanValue         .value();
    InforcePreferredLoanValue        = z.InforcePreferredLoanValue       .value();
    InforceRegularLoanBalance        = z.InforceRegularLoanBalance       .value();
    InforcePreferredLoanBalance      = z.InforcePreferredLoanBalance     .value();
    InforceNoLapseActive             = z.InforceNoLapseActive            .value();
    InforceMonthlyNoLapsePremium     = z.InforceMonthlyNoLapsePremium    .value();
    InforceCumulativeNoLapsePremium  = z.InforceCumulativeNoLapsePremium .value();
    InforceCumulativeNoLapsePayments = z.InforceCumulativeNoLapsePayments.value();
    InforceCumulativeRopPayments     = z.InforceCumulativeRopPayments    .value();
    InforceYtdTaxablePremium         = z.InforceYtdTaxablePremium        .value();
    InforceCumulativeSalesLoad       = z.InforceCumulativeSalesLoad      .value();
    InforceSpecAmtLoadBase           = z.InforceSpecAmtLoadBase          .value();
    InforceHoneymoonValue            = z.InforceHoneymoonValue           .value();
    InforceTaxBasis                  = z.InforceTaxBasis                 .value();
    InforceGlp                       = z.InforceGlp                      .value();
    InforceCumulativeGlp             = z.InforceCumulativeGlp            .value();
    InforceGsp                       = z.InforceGsp                      .value();
    InforceCumulativeGptPremiumsPaid = z.InforceCumulativeGptPremiumsPaid.value();
    InforceIsMec                     = z.InforceIsMec                    .value();
    InforceSevenPayPremium           = z.InforceSevenPayPremium          .value();
    LastMaterialChangeDate           = z.LastMaterialChangeDate          .value();
    InforceContractYear              = z.InforceContractYear             .value();
    InforceContractMonth             = z.InforceContractMonth            .value();
    InforceAvBeforeLastMc            = z.InforceAvBeforeLastMc           .value();
    InforceDcv                       = z.InforceDcv                      .value();
    InforceLeastDeathBenefit         = z.InforceLeastDeathBenefit        .value();
    Inforce7702AAmountsPaidHistory   = convert_vector_type<double              >(z.AmountsPaidHistoryRealized_         );
    Country                          = z.Country                         .value();
    CountryCoiMultiplier             = z.CountryCoiMultiplier            .value();
    SurviveToType                    = z.SurviveToType                   .value();
    SurviveToYear                    = z.SurviveToYear                   .value();
    SurviveToAge                     = z.SurviveToAge                    .value();
    MaximumNaar                      = z.MaximumNaar                     .value();
    ChildRider                       = z.ChildRider                      .value();
    ChildRiderAmount                 = z.ChildRiderAmount                .value();
    SpouseRider                      = z.SpouseRider                     .value();
    SpouseRiderAmount                = z.SpouseRiderAmount               .value();
    SpouseIssueAge                   = z.SpouseIssueAge                  .value();
    StateOfJurisdiction              = z.StateOfJurisdiction             .value();
    PremiumTaxState                  = z.PremiumTaxState                 .value();
    SalarySpecifiedAmountFactor      = z.SalarySpecifiedAmountFactor     .value();
    SalarySpecifiedAmountCap         = z.SalarySpecifiedAmountCap        .value();
    SalarySpecifiedAmountOffset      = z.SalarySpecifiedAmountOffset     .value();
    HoneymoonEndorsement             = z.HoneymoonEndorsement            .value();
    PostHoneymoonSpread              = z.PostHoneymoonSpread             .value();
    ExtraMonthlyCustodialFee         = convert_vector_type<double              >(z.ExtraMonthlyCustodialFeeRealized_   );
    ExtraCompensationOnAssets        = convert_vector_type<double              >(z.ExtraCompensationOnAssetsRealized_  );
    ExtraCompensationOnPremium       = convert_vector_type<double              >(z.ExtraCompensationOnPremiumRealized_ );
    PartialMortalityMultiplier       = convert_vector_type<double              >(z.PartialMortalityMultiplierRealized_ );
    CurrentCoiMultiplier             = convert_vector_type<double              >(z.CurrentCoiMultiplierRealized_       );
    CorporationTaxBracket            = convert_vector_type<double              >(z.CorporationTaxBracketRealized_      );
    TaxBracket                       = convert_vector_type<double              >(z.TaxBracketRealized_                 );
    ProjectedSalary                  = convert_vector_type<double              >(z.ProjectedSalaryRealized_            );
    SpecifiedAmount                  = convert_vector_type<double              >(z.SpecifiedAmountRealized_            );
    SupplementalAmount               = convert_vector_type<double              >(z.SupplementalAmountRealized_         );
    DeathBenefitOption               = convert_vector_type<mcenum_dbopt        >(z.DeathBenefitOptionRealized_         );
    Payment                          = convert_vector_type<double              >(z.PaymentRealized_                    );
    PaymentMode                      = convert_vector_type<mcenum_mode         >(z.PaymentModeRealized_                );
    CorporationPayment               = convert_vector_type<double              >(z.CorporationPaymentRealized_         );
    CorporationPaymentMode           = convert_vector_type<mcenum_mode         >(z.CorporationPaymentModeRealized_     );
    GeneralAccountRate               = convert_vector_type<double              >(z.GeneralAccountRateRealized_         );
    SeparateAccountRate              = convert_vector_type<double              >(z.SeparateAccountRateRealized_        );
    NewLoan                          = convert_vector_type<double              >(z.NewLoanRealized_                    );
    Withdrawal                       = convert_vector_type<double              >(z.WithdrawalRealized_                 );
    FlatExtra                        = convert_vector_type<double              >(z.FlatExtraRealized_                  );
    HoneymoonValueSpread             = convert_vector_type<double              >(z.HoneymoonValueSpreadRealized_       );
    FundAllocations                  = convert_vector_type<double              >(z.FundAllocationsRealized_            ); // INPUT !! Not yet implemented.
    CashValueEnhancementRate         = convert_vector_type<double              >(z.CashValueEnhancementRateRealized_   );
    CreateSupplementalReport         = z.CreateSupplementalReport        .value();
    SupplementalReportColumn00       = z.SupplementalReportColumn00      .value();
    SupplementalReportColumn01       = z.SupplementalReportColumn01      .value();
    SupplementalReportColumn02       = z.SupplementalReportColumn02      .value();
    SupplementalReportColumn03       = z.SupplementalReportColumn03      .value();
    SupplementalReportColumn04       = z.SupplementalReportColumn04      .value();
    SupplementalReportColumn05       = z.SupplementalReportColumn05      .value();
    SupplementalReportColumn06       = z.SupplementalReportColumn06      .value();
    SupplementalReportColumn07       = z.SupplementalReportColumn07      .value();
    SupplementalReportColumn08       = z.SupplementalReportColumn08      .value();
    SupplementalReportColumn09       = z.SupplementalReportColumn09      .value();
    SupplementalReportColumn10       = z.SupplementalReportColumn10      .value();
    SupplementalReportColumn11       = z.SupplementalReportColumn11      .value();
    SpecifiedAmountStrategy          = convert_vector_type<mcenum_sa_strategy  >(z.SpecifiedAmountStrategyRealized_    );
    SupplementalAmountStrategy       = convert_vector_type<mcenum_sa_strategy  >(z.SupplementalAmountStrategyRealized_ );
    PaymentStrategy                  = convert_vector_type<mcenum_pmt_strategy >(z.PaymentStrategyRealized_            );
    CorporationPaymentStrategy       = convert_vector_type<mcenum_pmt_strategy >(z.CorporationPaymentStrategyRealized_ );
}

bool is_policy_rated(yare_input const& z)
{
    return
            mce_table_none != z.SubstandardTable
        ||  !each_equal(z.FlatExtra, 0.0)
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
