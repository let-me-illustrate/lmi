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

#ifndef yare_input_hpp
#define yare_input_hpp

#include "config.hpp"

#include "calendar_date.hpp"
#include "mc_enum_type_enums.hpp"

#include <string>
#include <vector>

class Input;

/// Illustration input ready for use in calculations.
///
/// This class has a data member for each useful input field, of a
/// type suitable for calculations. For example, a field containing a
/// sequence of numbers would be represented in the raw-input class as
/// a string for GUI editing and persistent storage, but here as a
/// vector.
///
/// It is a behaviorless aggregate, so its data members are public.
///
/// Commented-out data members seem to be unused, and may eventually
/// be expunged both here and in the raw-input class.
///
/// This is a work in progress. The goal is to expose input data to
/// calculation classes only through a const instance of this class,
/// in particular dispensing with the considerable overhead of UDTs
/// (mc_enum and tn_range classes) designed for interactive input.

class yare_input final
{
  public:
    explicit yare_input(Input const&);
    ~yare_input() = default;

    int                               IssueAge                        ;
    int                               RetirementAge                   ;
    mcenum_gender                     Gender                          ;
    mcenum_smoking                    Smoking                         ;
    mcenum_class                      UnderwritingClass               ;
    bool                              WaiverOfPremiumBenefit          ;
    bool                              AccidentalDeathBenefit          ;
    bool                              TermRider                       ;
    double                            TermRiderAmount                 ;
//    double                            TotalSpecifiedAmount            ;
//    double                            TermRiderProportion             ;
//    bool                              TermRiderUseProportion          ;
    calendar_date                     DateOfBirth                     ;
    mcenum_table_rating               SubstandardTable                ;
    std::string                       ProductName                     ;
    double                            Dumpin                          ;
    double                            External1035ExchangeAmount      ;
    double                            External1035ExchangeTaxBasis    ;
    bool                              External1035ExchangeFromMec     ;
    double                            Internal1035ExchangeAmount      ;
    double                            Internal1035ExchangeTaxBasis    ;
    bool                              Internal1035ExchangeFromMec     ;
//    int                               SolveTargetAge                  ;
//    int                               SolveBeginAge                   ;
//    int                               SolveEndAge                     ;
    mcenum_solve_type                 SolveType                       ;
    int                               SolveBeginYear                  ;
    int                               SolveEndYear                    ;
    mcenum_solve_target               SolveTarget                     ;
    double                            SolveTargetValue                ;
    int                               SolveTargetYear                 ;
    mcenum_gen_basis                  SolveExpenseGeneralAccountBasis ;
    mcenum_sep_basis                  SolveSeparateAccountBasis       ;
//    bool                              UseCurrentDeclaredRate          ;
    mcenum_gen_acct_rate_type         GeneralAccountRateType          ;
    mcenum_sep_acct_rate_type         SeparateAccountRateType         ;
    double                            LoanRate                        ;
    mcenum_loan_rate_type             LoanRateType                    ;
    bool                              WithdrawToBasisThenLoan         ;
    bool                              UseAverageOfAllFunds            ;
    bool                              OverrideFundManagementFee       ;
//    mcenum_fund_input_method          FundChoiceType                  ;
    double                            InputFundManagementFee          ;
    mcenum_run_order                  RunOrder                        ;
    int                               NumberOfIdenticalLives          ;
    bool                              UsePartialMortality             ;
//    mcenum_part_mort_table            PartialMortalityTable           ;
    std::string                       InsuredName                     ;
//    std::string                       Address                         ;
//    std::string                       City                            ;
//    mcenum_state                      State                           ;
//    std::string                       ZipCode                         ;
//    std::string                       EmployeeClass                   ;
    std::string                       CorporationName                 ;
//    std::string                       CorporationAddress              ;
//    std::string                       CorporationCity                 ;
//    mcenum_state                      CorporationState                ;
//    std::string                       CorporationZipCode              ;
    std::string                       AgentName                       ;
    std::string                       AgentAddress                    ;
    std::string                       AgentCity                       ;
    mcenum_state                      AgentState                      ;
    std::string                       AgentZipCode                    ;
    std::string                       AgentPhone                      ;
    std::string                       AgentId                         ;
//    mcenum_premium_table              InsuredPremiumTableNumber       ;
    double                            InsuredPremiumTableFactor       ;
//    mcenum_premium_table              CorporationPremiumTableNumber   ;
    double                            CorporationPremiumTableFactor   ;
    calendar_date                     EffectiveDate                   ;
    mcenum_defn_life_ins              DefinitionOfLifeInsurance       ;
    mcenum_defn_material_change       DefinitionOfMaterialChange      ;
    mcenum_mec_avoid_method           AvoidMecMethod                  ;
    bool                              RetireesCanEnroll               ;
    mcenum_uw_basis                   GroupUnderwritingType           ;
    calendar_date                     LastCoiReentryDate              ;
    calendar_date                     ListBillDate                    ;
    bool                              BlendGender                     ;
    bool                              BlendSmoking                    ;
    double                            MaleProportion                  ;
    double                            NonsmokerProportion             ;
    mcenum_term_adj_method            TermAdjustmentMethod            ;
    bool                              IncludeInComposite              ;
    std::string                       Comments                        ;
    bool                              AmortizePremiumLoad             ;
    std::string                       ContractNumber                  ;
    std::string                       MasterContractNumber            ;
    calendar_date                     InforceAsOfDate                 ;
    int                               InforceYear                     ;
    int                               InforceMonth                    ;
    double                            InforceAnnualTargetPremium      ;
    double                            InforceYtdGrossPremium          ;
    double                            InforceGeneralAccountValue      ;
    double                            InforceSeparateAccountValue     ;
    double                            InforceRegularLoanValue         ;
    double                            InforcePreferredLoanValue       ;
    double                            InforceRegularLoanBalance       ;
    double                            InforcePreferredLoanBalance     ;
    bool                              InforceNoLapseActive            ;
    double                            InforceMonthlyNoLapsePremium    ;
    double                            InforceCumulativeNoLapsePremium ;
    double                            InforceCumulativeNoLapsePayments;
    double                            InforceCumulativeRopPayments    ;
    double                            InforceYtdTaxablePremium        ;
    double                            InforceCumulativeSalesLoad      ;
    double                            InforceSpecAmtLoadBase          ;
    double                            InforceHoneymoonValue           ;
    double                            InforceTaxBasis                 ;
    double                            InforceGlp                      ;
    double                            InforceCumulativeGlp            ;
    double                            InforceGsp                      ;
    double                            InforceCumulativeGptPremiumsPaid;
    bool                              InforceIsMec                    ;
    double                            InforceSevenPayPremium          ;
    calendar_date                     LastMaterialChangeDate          ;
    int                               InforceContractYear             ;
    int                               InforceContractMonth            ;
    double                            InforceAvBeforeLastMc           ;
    double                            InforceDcv                      ;
    double                            InforceLeastDeathBenefit        ;
    std::vector<double>               Inforce7702AAmountsPaidHistory  ;
    mcenum_country                    Country                         ;
//    bool                              OverrideCoiMultiplier           ;
    double                            CountryCoiMultiplier            ;
    mcenum_survival_limit             SurviveToType                   ;
    int                               SurviveToYear                   ;
    int                               SurviveToAge                    ;
    double                            MaximumNaar                     ;
    bool                              ChildRider                      ;
    double                            ChildRiderAmount                ;
    bool                              SpouseRider                     ;
    double                            SpouseRiderAmount               ;
    int                               SpouseIssueAge                  ;
    mcenum_state                      StateOfJurisdiction             ;
    mcenum_state                      PremiumTaxState                 ;
    double                            SalarySpecifiedAmountFactor     ;
    double                            SalarySpecifiedAmountCap        ;
    double                            SalarySpecifiedAmountOffset     ;
    bool                              HoneymoonEndorsement            ;
    double                            PostHoneymoonSpread             ;
    std::vector<double>               ExtraMonthlyCustodialFee        ;
    std::vector<double>               ExtraCompensationOnAssets       ;
    std::vector<double>               ExtraCompensationOnPremium      ;
    std::vector<double>               PartialMortalityMultiplier      ;
    std::vector<double>               CurrentCoiMultiplier            ;
    std::vector<double>               CorporationTaxBracket           ;
    std::vector<double>               TaxBracket                      ;
    std::vector<double>               ProjectedSalary                 ;
    std::vector<double>               SpecifiedAmount                 ;
    std::vector<double>               SupplementalAmount              ;
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
//    std::vector<double>               PolicyLevelFlatExtra            ;
    std::vector<double>               HoneymoonValueSpread            ;
    std::vector<double>               FundAllocations                 ;
    std::vector<double>               CashValueEnhancementRate        ;
//
    bool                              CreateSupplementalReport        ;
    mcenum_report_column              SupplementalReportColumn00      ;
    mcenum_report_column              SupplementalReportColumn01      ;
    mcenum_report_column              SupplementalReportColumn02      ;
    mcenum_report_column              SupplementalReportColumn03      ;
    mcenum_report_column              SupplementalReportColumn04      ;
    mcenum_report_column              SupplementalReportColumn05      ;
    mcenum_report_column              SupplementalReportColumn06      ;
    mcenum_report_column              SupplementalReportColumn07      ;
    mcenum_report_column              SupplementalReportColumn08      ;
    mcenum_report_column              SupplementalReportColumn09      ;
    mcenum_report_column              SupplementalReportColumn10      ;
    mcenum_report_column              SupplementalReportColumn11      ;
//
//    mcenum_to_point                   SolveTgtAtWhich                 ;
//    mcenum_from_point                 SolveFromWhich                  ;
//    mcenum_to_point                   SolveToWhich                    ;
//    bool                              UseDOB                          ;
//    bool                              EffectiveDateToday              ;
//    mcenum_sa_strategy                SpecifiedAmountStrategyFromIssue;
//    mcenum_pmt_strategy               IndividualPaymentStrategy       ;

    std::vector<mcenum_sa_strategy  > SpecifiedAmountStrategy         ;
    std::vector<mcenum_sa_strategy  > SupplementalAmountStrategy      ;
    std::vector<mcenum_pmt_strategy > PaymentStrategy                 ;
    std::vector<mcenum_pmt_strategy > CorporationPaymentStrategy      ;
};

bool is_policy_rated(yare_input const&);

double premium_allocation_to_sepacct(yare_input const&);

#endif // yare_input_hpp
