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

// $Id: yare_input.hpp,v 1.1 2008-06-29 00:13:18 chicares Exp $

#ifndef yare_input_hpp
#define yare_input_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

class Input;
class InputParms;

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
/// This is a work in progress. The plan is to uncomment data members
/// as they become necessary, gradually replacing each use of the raw-
/// input class in calculations.
///
/// It is not yet known whether member functions such as
///    yare_input();
///    yare_input& operator=(Input const&);
/// will become useful.

class yare_input
    :private boost::noncopyable
    ,virtual private obstruct_slicing<yare_input>
{
  public:
    explicit yare_input(Input const&);
    explicit yare_input(InputParms const&);
    ~yare_input();

  private:
//    tnr_issue_age            IssueAge                        ;
//    tnr_attained_age         RetirementAge                   ;
//    mce_gender               Gender                          ;
//    mce_smoking              Smoking                         ;
//    mce_class                UnderwritingClass               ;
//    mce_yes_or_no            WaiverOfPremiumBenefit          ;
//    mce_yes_or_no            AccidentalDeathBenefit          ;
//    mce_yes_or_no            TermRider                       ;
//    tnr_nonnegative_double   TermRiderAmount                 ;
//    tnr_nonnegative_double   TotalSpecifiedAmount            ;
//    tnr_proportion           TermRiderProportion             ;
//    mce_yes_or_no            TermRiderUseProportion          ;
//    tnr_date                 DateOfBirth                     ;
//    tnr_date                 DateOfRetirement                ;
//    mce_table_rating         SubstandardTable                ;
//    ce_product_name          ProductName                     ;
//    tnr_nonnegative_double   Dumpin                          ;
//    tnr_nonnegative_double   External1035ExchangeAmount      ;
//    tnr_nonnegative_double   External1035ExchangeBasis       ;
//    mce_yes_or_no            External1035ExchangeFromMec     ;
//    tnr_nonnegative_double   Internal1035ExchangeAmount      ;
//    tnr_nonnegative_double   Internal1035ExchangeBasis       ;
//    mce_yes_or_no            Internal1035ExchangeFromMec     ;
//    tnr_duration             SolveTargetTime                 ;
//    tnr_duration             SolveBeginTime                  ;
//    tnr_duration             SolveEndTime                    ;
//    mce_solve_type           SolveType                       ;
//    tnr_duration             SolveBeginYear                  ;
//    tnr_duration             SolveEndYear                    ;
//    mce_solve_target         SolveTarget                     ;
//    tnr_nonnegative_double   SolveTargetCashSurrenderValue   ;
//    tnr_duration             SolveTargetYear                 ;
//    mce_basis                SolveBasis                      ;
//    mce_sep_acct_basis       SolveSeparateAccountBasis       ;
//    mce_yes_or_no            UseCurrentDeclaredRate          ;
//    mce_interest_rate_type   GeneralAccountRateType          ;
//    mce_interest_rate_type   SeparateAccountRateType         ;
//    tnr_proportion           LoanRate                        ;
//    mce_loan_rate_type       LoanRateType                    ;
//    mce_yes_or_no            OverrideExperienceReserveRate   ;
//    tnr_proportion           ExperienceReserveRate           ;
//    tnr_nonnegative_double   ExperienceRatingInitialKFactor  ;
//    tnr_unrestricted_double  InforceNetExperienceReserve     ;
//    tnr_nonnegative_double   InforceYtdNetCoiCharge          ;
//    mce_yes_or_no            WithdrawToBasisThenLoan         ;
//    mce_yes_or_no            UseAverageOfAllFunds            ;
//    mce_yes_or_no            OverrideFundManagementFee       ;
//    mce_fund_input_method    FundChoiceType                  ;
//    tnr_nonnegative_double   InputFundManagementFee          ;
//    mce_run_order            RunOrder                        ;
//    tnr_nonnegative_integer  NumberOfIdenticalLives          ;
//    mce_yes_or_no            UseExperienceRating             ;
//    mce_yes_or_no            UsePartialMortality             ;
//    mce_part_mort_table      PartialMortalityTable           ;
//    datum_string             InsuredName                     ;
//    datum_string             Address                         ;
//    datum_string             City                            ;
//    mce_state                State                           ;
//    datum_string             ZipCode                         ;
//    datum_string             EmployeeClass                   ;
//    datum_string             CorporationName                 ;
//    datum_string             CorporationAddress              ;
//    datum_string             CorporationCity                 ;
//    mce_state                CorporationState                ;
//    datum_string             CorporationZipCode              ;
//    datum_string             AgentName                       ;
//    datum_string             AgentAddress                    ;
//    datum_string             AgentCity                       ;
//    mce_state                AgentState                      ;
//    datum_string             AgentZipCode                    ;
//    datum_string             AgentPhone                      ;
//    datum_string             AgentId                         ;
//    mce_premium_table        InsuredPremiumTableNumber       ;
//    tnr_nonnegative_double   InsuredPremiumTableFactor       ;
//    mce_premium_table        CorporationPremiumTableNumber   ;
//    tnr_nonnegative_double   CorporationPremiumTableFactor   ;
//    tnr_date                 EffectiveDate                   ;
//    mce_defn_life_ins        DefinitionOfLifeInsurance       ;
//    mce_defn_material_change DefinitionOfMaterialChange      ;
//    mce_mec_avoid_method     AvoidMecMethod                  ;
//    mce_yes_or_no            RetireesCanEnroll               ;
//    mce_uw_basis             GroupUnderwritingType           ;
//    tnr_date                 LastCoiReentryDate              ;
//    mce_yes_or_no            BlendGender                     ;
//    mce_yes_or_no            BlendSmoking                    ;
//    tnr_proportion           MaleProportion                  ;
//    tnr_proportion           NonsmokerProportion             ;
//    tnr_proportion           TermProportion                  ;
//    mce_term_adj_method      TermAdjustmentMethod            ;
//    mce_yes_or_no            IncludeInComposite              ;
//    datum_string             Comments                        ;
//    mce_yes_or_no            AmortizePremiumLoad             ;
//    tnr_duration             InforceYear                     ;
//    tnr_month                InforceMonth                    ;
//    tnr_nonnegative_double   InforceGeneralAccountValue      ;
//    tnr_nonnegative_double   InforceSeparateAccountValue     ;
//    tnr_nonnegative_double   InforceRegularLoanValue         ;
//    tnr_nonnegative_double   InforcePreferredLoanValue       ;
//    tnr_nonnegative_double   InforceRegularLoanBalance       ;
//    tnr_nonnegative_double   InforcePreferredLoanBalance     ;
//    tnr_nonnegative_double   InforceCumulativeNoLapsePremium ;
//    tnr_nonnegative_double   InforceCumulativePayments       ;
//    mce_country              Country                         ;
//    mce_yes_or_no            OverrideCoiMultiplier           ;
//    tnr_nonnegative_double   CountryCoiMultiplier            ;
//    mce_survival_limit       SurviveToType                   ;
//    tnr_duration             SurviveToYear                   ;
//    tnr_attained_age         SurviveToAge                    ;
//    tnr_nonnegative_double   MaximumNaar                     ;
//    tnr_corridor_factor      UseOffshoreCorridorFactor       ;
//    mce_yes_or_no            ChildRider                      ;
//    tnr_nonnegative_double   ChildRiderAmount                ;
//    mce_yes_or_no            SpouseRider                     ;
//    tnr_nonnegative_double   SpouseRiderAmount               ;
//    tnr_issue_age            SpouseIssueAge                  ;
//    datum_string             Franchise                       ;
//    datum_string             PolicyNumber                    ;
//    tnr_date                 PolicyDate                      ;
//    tnr_nonnegative_double   InforceTaxBasis                 ;
//    tnr_nonnegative_double   InforceCumulativeGlp            ;
//    tnr_nonnegative_double   InforceGlp                      ;
//    tnr_nonnegative_double   InforceGsp                      ;
//    tnr_nonnegative_double   InforceSevenPayPremium          ;
//    mce_yes_or_no            InforceIsMec                    ;
//    tnr_date                 LastMaterialChangeDate          ;
//    tnr_nonnegative_double   InforceDcv                      ;
//    tnr_nonnegative_double   InforceAvBeforeLastMc           ;
//    tnr_duration             InforceContractYear             ;
//    tnr_month                InforceContractMonth            ;
//    tnr_nonnegative_double   InforceLeastDeathBenefit        ;
//    mce_state                StateOfJurisdiction             ;
//    tnr_nonnegative_double   SalarySpecifiedAmountFactor     ;
//    tnr_nonnegative_double   SalarySpecifiedAmountCap        ;
//    tnr_nonnegative_double   SalarySpecifiedAmountOffset     ;
//    mce_yes_or_no            HoneymoonEndorsement            ;
//    tnr_proportion           PostHoneymoonSpread             ;
//    tnr_nonnegative_double   InforceHoneymoonValue           ;
//    datum_sequence           ExtraMonthlyCustodialFee        ;
//    datum_sequence           ExtraCompensationOnAssets       ;
//    datum_sequence           ExtraCompensationOnPremium      ;
//    datum_sequence           OffshoreCorridorFactor          ;
//    datum_sequence           PartialMortalityMultiplier      ;
//    datum_sequence           CurrentCoiMultiplier            ;
//    datum_sequence           CurrentCoiGrading               ;
//    datum_sequence           CorporationTaxBracket           ;
//    datum_sequence           TaxBracket                      ;
//    datum_sequence           ProjectedSalary                 ;
//    datum_sequence           SpecifiedAmount                 ;
//    datum_sequence           DeathBenefitOption              ;
//    datum_sequence           Payment                         ;
//    datum_sequence           PaymentMode                     ;
//    datum_sequence           CorporationPayment              ;
//    datum_sequence           CorporationPaymentMode          ;
//    datum_sequence           GeneralAccountRate              ;
//    datum_sequence           SeparateAccountRate             ;
//    datum_sequence           NewLoan                         ;
//    datum_sequence           Withdrawal                      ;
//    datum_sequence           FlatExtra                       ;
//    datum_sequence           PolicyLevelFlatExtra            ;
//    datum_sequence           HoneymoonValueSpread            ;
//    datum_sequence           PremiumHistory                  ;
//    datum_sequence           SpecamtHistory                  ;
//    datum_sequence           FundAllocations                 ;
//    datum_sequence           CashValueEnhancementRate        ;
//
//    mce_yes_or_no            CreateSupplementalReport        ;
//    mce_report_column        SupplementalReportColumn00      ;
//    mce_report_column        SupplementalReportColumn01      ;
//    mce_report_column        SupplementalReportColumn02      ;
//    mce_report_column        SupplementalReportColumn03      ;
//    mce_report_column        SupplementalReportColumn04      ;
//    mce_report_column        SupplementalReportColumn05      ;
//    mce_report_column        SupplementalReportColumn06      ;
//    mce_report_column        SupplementalReportColumn07      ;
//    mce_report_column        SupplementalReportColumn08      ;
//    mce_report_column        SupplementalReportColumn09      ;
//    mce_report_column        SupplementalReportColumn10      ;
//    mce_report_column        SupplementalReportColumn11      ;
//
//    mce_solve_tgt_at         DeprecatedSolveTgtAtWhich       ;
//    mce_solve_from           DeprecatedSolveFromWhich        ;
//    mce_solve_to             DeprecatedSolveToWhich          ;
//    mce_yes_or_no            DeprecatedUseDOB                ;
//    mce_yes_or_no            DeprecatedUseDOR                ;
//    mce_yes_or_no            EffectiveDateToday              ;
//    mce_dbopt                DeathBenefitOptionFromRetirement;
//    mce_dbopt                DeathBenefitOptionFromIssue     ;
//    tnr_nonnegative_double   SpecifiedAmountFromRetirement   ;
//    tnr_nonnegative_double   SpecifiedAmountFromIssue        ;
//    mce_sa_strategy          SpecifiedAmountStrategyFromIssue;
//    mce_mode                 IndividualPaymentMode           ;
//    mce_to_point             IndividualPaymentToAlternative  ;
//    tnr_attained_age         IndividualPaymentToAge          ;
//    tnr_duration             IndividualPaymentToDuration     ;
//    tnr_nonnegative_double   IndividualPaymentAmount         ;
//    mce_pmt_strategy         IndividualPaymentStrategy       ;
//    tnr_nonnegative_double   LoanAmount                      ;
//    tnr_attained_age         LoanFromAge                     ;
//    mce_from_point           LoanFromAlternative             ;
//    tnr_duration             LoanFromDuration                ;
//    tnr_attained_age         LoanToAge                       ;
//    mce_to_point             LoanToAlternative               ;
//    tnr_duration             LoanToDuration                  ;
//    tnr_nonnegative_double   WithdrawalAmount                ;
//    tnr_attained_age         WithdrawalFromAge               ;
//    mce_from_point           WithdrawalFromAlternative       ;
//    tnr_duration             WithdrawalFromDuration          ;
//    tnr_attained_age         WithdrawalToAge                 ;
//    mce_to_point             WithdrawalToAlternative         ;
//    tnr_duration             WithdrawalToDuration            ;
};

#endif // yare_input_hpp

