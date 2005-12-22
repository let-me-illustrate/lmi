// MVC Model for life-insurance illustrations.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: input.hpp,v 1.10 2005-12-22 13:59:49 chicares Exp $

#ifndef input_hpp
#define input_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "ce_product_name.hpp"
#include "database.hpp"
#include "datum_boolean.hpp"
#include "datum_string.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "tn_range.hpp"
#include "tn_range_types.hpp"

class IllusInputParms;
class InputSequence;

// TODO ?? Forward declaration doesn't work with std::auto_ptr?
// Use one of boost's smart pointers instead. And lose the declspec,
// which gcc-3.4.2 doesn't like to see on forward-declared classes.
//class LMI_SO TDatabase;

#include <boost/operators.hpp>

#include <memory>
#include <vector>

/// Design notes for class input.
///
/// This class is the Model of the MVC framework for life-insurance
/// input.
///
/// The MVC framework uses two distinct data structures. One is simply
/// a std::map<std::string>,std::string> > member of the Controller;
/// it captures user input exactly. The other, this class, holds data
/// of various types that a real program might capture from GUI input
/// and use downstream. These two data structures are distinct because
/// conversion between them may not perfectly preserve value.
///
/// For example, "1.07" in a text control may be translated to
///   (double)(1.07)
/// but the latter, converted to a string, with the maximum precision
/// the machine is capable of, would differ from the original "1.07".
/// A user who reloads saved input from a file would likely protest
/// "but I didn't say 1.0700000000001". Truncating to a 'reasonable'
/// precision merely engenders complaints from other users who may
/// enter pi to machine precision and expect more than "3.1416": there
/// is no universally reasonable way to truncate numbers.
///
/// [Note: that example impedes interconvertibility. Adding floating-
/// point text controls later will force us to grapple with that.]
///
/// Data members are UDTs that help express certain relationships
/// among the controls that represent them. For example:
///  - discrete-valued controls like wxControlWithItems and wxRadioBox
///    are mapped to an enumerative type that constrains assignment to
///    values that are permissible within the overall context of the
///    input object;
///  - a radiobox might offer three choices, but allow only the first
///    two if the input object is in a particular state determined by
///    the contents of other controls;
///  - a text control that represents a number might have a maximum
///    and a minimum value.
/// These UDTs bear values in a natural, more primitive type, provide
/// for conversion to and from strings, and hold enablement state and
/// other information for use by controls.
///
/// Harmonize() enforces rules governing relationships among data and
/// their associated controls. Perhaps it should be factored into a
/// const function that handles only enablement (which would entail
/// making ancillary, non-value-bearing data members of the UDTs
/// mutable) and a non-const function that forces changes in values.
///
/// reset_database(): Reset database if necessary, i.e., if the
/// product or any database axis changed. Conditionally update
/// general-account rate (see implementation for details).
///
/// TODO ?? Add functions to convert to and from a std::map<std::string> >?

class LMI_SO Input
    :public MemberSymbolTable<Input>
    ,private boost::equality_comparable<Input>
    ,virtual private obstruct_slicing<Input>
{
  public:
    Input();
    Input(Input const&);
    ~Input();

    Input& operator=(Input const&);
    bool operator==(Input const&) const;

    std::string differing_fields(Input const&) const;

    void Harmonize();

  private:
    // TODO ?? Is there no way around this? Maybe a virtual that's called
    // in the base?
    void ascribe_members();

    // TODO ?? Dubious stuff to support scalar alternative controls.
    void WithdrawalChanged();
    void EnableTransferToSequenceControls(bool enable);
    void TransferWithdrawalSimpleControlsToInputSequence();
    void TransferWithdrawalInputSequenceToSimpleControls
        (InputSequence const& s
        );

    void reset_database();

    std::auto_ptr<TDatabase> database                        ;

    // TODO ?? Temporary.
    typedef datum_string datum_sequence;

    tnr_issue_age            IssueAge                        ;
    tnr_attained_age         RetirementAge                   ;
    mce_gender               Gender                          ;
    mce_smoking              Smoking                         ;
    mce_class                UnderwritingClass               ;
    mce_yes_or_no            WaiverOfPremiumBenefit          ;
    mce_yes_or_no            AccidentalDeathBenefit          ;
    mce_yes_or_no            TermRider                       ;
    tnr_nonnegative_double   TermRiderAmount                 ;
    tnr_nonnegative_double   TotalSpecifiedAmount            ;
    tnr_proportion           TermRiderProportion             ; // TODO ?? Not actually used?
    mce_yes_or_no            TermRiderUseProportion          ;
    tnr_date                 DateOfBirth                     ;
    tnr_date                 DateOfRetirement                ;
    mce_table_rating         SubstandardTable                ;
    ce_product_name          ProductName                     ;
    tnr_nonnegative_double   Dumpin                          ;
    tnr_nonnegative_double   External1035ExchangeAmount      ;
    tnr_nonnegative_double   External1035ExchangeBasis       ;
    mce_yes_or_no            External1035ExchangeFromMec     ;
    tnr_nonnegative_double   Internal1035ExchangeAmount      ;
    tnr_nonnegative_double   Internal1035ExchangeBasis       ;
    mce_yes_or_no            Internal1035ExchangeFromMec     ;
    tnr_duration             SolveTargetTime                 ;
    tnr_duration             SolveBeginTime                  ;
    tnr_duration             SolveEndTime                    ;
    mce_solve_type           SolveType                       ;
    tnr_duration             SolveBeginYear                  ;
    tnr_duration             SolveEndYear                    ;
    mce_solve_target         SolveTarget                     ; // TODO ?? {csv@dur, endt]: could be bool.
    tnr_nonnegative_double   SolveTargetCashSurrenderValue   ;
    tnr_duration             SolveTargetYear                 ;
    mce_basis                SolveBasis                      ; // TODO ?? Poor name, but enumerators are correct.
    mce_sep_acct_basis       SolveSeparateAccountBasis       ;
    mce_interest_rate_type   GeneralAccountRateType          ;
    mce_interest_rate_type   SeparateAccountRateType         ;
    tnr_proportion           LoanRate                        ;
    mce_loan_rate_type       LoanRateType                    ;
    tnr_nonnegative_double   ExperienceRatingInitialKFactor  ;
    tnr_proportion           ExperienceReserveRate           ;
    tnr_unrestricted_double  InforceExperienceReserve        ;
    mce_yes_or_no            OverrideExperienceReserveRate   ;
    mce_yes_or_no            WithdrawToBasisThenLoan         ;
    mce_yes_or_no            UseAverageOfAllFunds            ;
    mce_yes_or_no            OverrideFundManagementFee       ;
    mce_fund_input_method    FundChoiceType                  ;
    tnr_nonnegative_double   InputFundManagementFee          ;
    mce_run_order            RunOrder                        ;
    tnr_nonnegative_integer  NumberOfIdenticalLives          ;
    mce_yes_or_no            UseExperienceRating             ;
    mce_yes_or_no            UsePartialMortality             ;
    mce_part_mort_table      PartialMortalityTable           ;
    datum_string             InsuredName                     ;
    datum_string             Address                         ;
    datum_string             City                            ;
    mce_state                State                           ;
    datum_string             ZipCode                         ;
    datum_string             EmployeeClass                   ;
    datum_string             CorporationName                 ;
    datum_string             CorporationAddress              ;
    datum_string             CorporationCity                 ;
    mce_state                CorporationState                ;
    datum_string             CorporationZipCode              ;
    datum_string             AgentName                       ;
    datum_string             AgentAddress                    ;
    datum_string             AgentCity                       ;
    mce_state                AgentState                      ;
    datum_string             AgentZipCode                    ;
    datum_string             AgentPhone                      ;
    datum_string             AgentId                         ;
    mce_premium_table        InsuredPremiumTableNumber       ;
    tnr_nonnegative_double   InsuredPremiumTableFactor       ;
    mce_premium_table        CorporationPremiumTableNumber   ;
    tnr_nonnegative_double   CorporationPremiumTableFactor   ;
    tnr_date                 EffectiveDate                   ; // TODO ?? Should date types be *essentially* different?
    mce_defn_life_ins        DefinitionOfLifeInsurance       ;
    mce_defn_material_change DefinitionOfMaterialChange      ;
    mce_mec_avoid_method     AvoidMecMethod                  ;
    mce_yes_or_no            RetireesCanEnroll               ;
    mce_uw_basis             GroupUnderwritingType           ;
    mce_yes_or_no            BlendGender                     ;
    mce_yes_or_no            BlendSmoking                    ;
    tnr_proportion           MaleProportion                  ;
    tnr_proportion           NonsmokerProportion             ;
    tnr_proportion           TermProportion                  ; // TODO ?? *Initial* term proportion?
    mce_term_adj_method      TermAdjustmentMethod            ;
    mce_yes_or_no            IncludeInComposite              ;
    datum_string             Comments                        ;
    mce_yes_or_no            AmortizePremiumLoad             ;
    tnr_duration             InforceYear                     ;
    tnr_month                InforceMonth                    ;
    tnr_nonnegative_double   InforceGeneralAccountValue      ;
    tnr_nonnegative_double   InforceSeparateAccountValue     ;
    tnr_nonnegative_double   InforceRegularLoanValue         ;
    tnr_nonnegative_double   InforcePreferredLoanValue       ;
    tnr_nonnegative_double   InforceRegularLoanBalance       ;
    tnr_nonnegative_double   InforcePreferredLoanBalance     ;
    tnr_nonnegative_double   InforceCumulativeNoLapsePremium ;
    tnr_nonnegative_double   InforceCumulativePayments       ;
    mce_country              Country                         ;
    mce_yes_or_no            OverrideCoiMultiplier           ;
    tnr_nonnegative_double   CountryCoiMultiplier            ;
    mce_survival_limit       SurviveToType                   ;
    tnr_duration             SurviveToYear                   ;
    tnr_attained_age         SurviveToAge                    ;
    tnr_nonnegative_double   MaximumNaar                     ;
    tnr_corridor_factor      UseOffshoreCorridorFactor       ; // TODO ?? Awful name: suggests bool.
    mce_yes_or_no            ChildRider                      ;
    tnr_nonnegative_double   ChildRiderAmount                ; // TODO ?? Specialize?
    mce_yes_or_no            SpouseRider                     ;
    tnr_nonnegative_double   SpouseRiderAmount               ; // TODO ?? Specialize?
    tnr_issue_age            SpouseIssueAge                  ; // TODO ?? Specialize?
    datum_string             Franchise                       ;
    datum_string             PolicyNumber                    ;
    tnr_date                 PolicyDate                      ;
    tnr_nonnegative_double   InforceTaxBasis                 ;
    tnr_nonnegative_double   InforceCumulativeGlp            ;
    tnr_nonnegative_double   InforceGlp                      ;
    tnr_nonnegative_double   InforceGsp                      ;
    tnr_nonnegative_double   InforceSevenPayPremium          ;
    mce_yes_or_no            InforceIsMec                    ;
    tnr_date                 LastMaterialChangeDate          ;
    tnr_nonnegative_double   InforceDcv                      ;
    tnr_nonnegative_double   InforceAvBeforeLastMc           ;
    tnr_duration             InforceContractYear             ;
    tnr_month                InforceContractMonth            ;
    tnr_nonnegative_double   InforceLeastDeathBenefit        ;
    mce_state                StateOfJurisdiction             ;
    tnr_nonnegative_double   SalarySpecifiedAmountFactor     ;
    tnr_nonnegative_double   SalarySpecifiedAmountCap        ;
    tnr_nonnegative_double   SalarySpecifiedAmountOffset     ;
    mce_yes_or_no            HoneymoonEndorsement            ;
    tnr_proportion           PostHoneymoonSpread             ;
    tnr_nonnegative_double   InforceHoneymoonValue           ;
    datum_sequence           ExtraMonthlyCustodialFee        ;
    datum_sequence           ExtraCompensationOnAssets       ;
    datum_sequence           ExtraCompensationOnPremium      ;
    datum_sequence           OffshoreCorridorFactor          ;
    datum_sequence           PartialMortalityMultiplier      ;
    datum_sequence           CurrentCoiMultiplier            ;
    datum_sequence           CurrentCoiGrading               ; // TODO ?? Obsolete: expunge.
    datum_sequence           CorporationTaxBracket           ;
    datum_sequence           TaxBracket                      ;
    datum_sequence           ProjectedSalary                 ;
    datum_sequence           SpecifiedAmount                 ;
    datum_sequence           DeathBenefitOption              ;
    datum_sequence           Payment                         ;
    datum_sequence           PaymentMode                     ;
    datum_sequence           CorporationPayment              ;
    datum_sequence           CorporationPaymentMode          ;
    datum_sequence           GeneralAccountRate              ;
    datum_sequence           SeparateAccountRate             ;
    datum_sequence           NewLoan                         ;
    datum_sequence           Withdrawal                      ;
    datum_sequence           FlatExtra                       ;
    datum_sequence           PolicyLevelFlatExtra            ;
    datum_sequence           HoneymoonValueSpread            ;
    datum_sequence           PremiumHistory                  ;
    datum_sequence           SpecamtHistory                  ;
    datum_sequence           FundAllocations                 ; // TODO ?? Needs work.
    datum_sequence           CashValueEnhancementRate        ;
    datum_sequence           NetMortalityChargeHistory       ;

    mce_yes_or_no            CreateSupplementalReport        ;
    mce_report_column        SupplementalReportColumn00      ;
    mce_report_column        SupplementalReportColumn01      ;
    mce_report_column        SupplementalReportColumn02      ;
    mce_report_column        SupplementalReportColumn03      ;
    mce_report_column        SupplementalReportColumn04      ;
    mce_report_column        SupplementalReportColumn05      ;
    mce_report_column        SupplementalReportColumn06      ;
    mce_report_column        SupplementalReportColumn07      ;
    mce_report_column        SupplementalReportColumn08      ;
    mce_report_column        SupplementalReportColumn09      ;
    mce_report_column        SupplementalReportColumn10      ;
    mce_report_column        SupplementalReportColumn11      ;

    mce_solve_tgt_at         DeprecatedSolveTgtAtWhich       ;
    mce_solve_from           DeprecatedSolveFromWhich        ;
    mce_solve_to             DeprecatedSolveToWhich          ;
    mce_yes_or_no            DeprecatedUseDOB                ;
    mce_yes_or_no            DeprecatedUseDOR                ;
    mce_yes_or_no            EffectiveDateToday              ;
    mce_dbopt                DeathBenefitOptionFromRetirement;
    mce_dbopt                DeathBenefitOptionFromIssue     ;
    tnr_nonnegative_double   SpecifiedAmountFromRetirement   ;
    tnr_nonnegative_double   SpecifiedAmountFromIssue        ;
//    mce_                     SpecifiedAmountStrategyFromRetirement; // TODO ?? Rethink this.
    mce_sa_strategy          SpecifiedAmountStrategyFromIssue;
    mce_mode                 IndividualPaymentMode           ;
    mce_to_point             IndividualPaymentToAlternative  ;
    tnr_attained_age         IndividualPaymentToAge          ;
    tnr_duration             IndividualPaymentToDuration     ;
    tnr_nonnegative_double   IndividualPaymentAmount         ;
    mce_pmt_strategy         IndividualPaymentStrategy       ;
    tnr_nonnegative_double   LoanAmount                      ;
    tnr_attained_age         LoanFromAge                     ;
    mce_from_point           LoanFromAlternative             ;
    tnr_duration             LoanFromDuration                ;
    tnr_attained_age         LoanToAge                       ;
    mce_to_point             LoanToAlternative               ;
    tnr_duration             LoanToDuration                  ;
    tnr_nonnegative_double   WithdrawalAmount                ;
    tnr_attained_age         WithdrawalFromAge               ;
    mce_from_point           WithdrawalFromAlternative       ;
    tnr_duration             WithdrawalFromDuration          ;
    tnr_attained_age         WithdrawalToAge                 ;
    mce_to_point             WithdrawalToAlternative         ;
    tnr_duration             WithdrawalToDuration            ;

    ce_product_name          CachedProductName               ;
    mce_gender               CachedGender                    ;
    mce_class                CachedUnderwritingClass         ;
    mce_smoking              CachedSmoking                   ;
    tnr_issue_age            CachedIssueAge                  ;
    mce_uw_basis             CachedGroupUnderwritingType     ;
    mce_state                CachedStateOfJurisdiction       ;
};

void LMI_SO convert_to_ihs(IllusInputParms&, Input const&);
void LMI_SO convert_to_ihs
    (std::vector<IllusInputParms>&
    ,std::vector<Input> const&
    );

void LMI_SO convert_from_ihs(IllusInputParms const&, Input&);
void LMI_SO convert_from_ihs
    (std::vector<IllusInputParms> const&
    ,std::vector<Input>&
    );

#endif // input_hpp

