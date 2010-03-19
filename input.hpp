// MVC Model for life-insurance illustrations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef input_hpp
#define input_hpp

#include "config.hpp"

#include "mvc_model.hpp"

#include "any_member.hpp"
#include "ce_product_name.hpp"
#include "datum_boolean.hpp"
#include "datum_string.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "tn_range.hpp"
#include "tn_range_types.hpp"
#include "xml_serializable.hpp"

class InputSequence;
class TDatabase;

#include <boost/operators.hpp>
#include <boost/scoped_ptr.hpp>

#include <list>
#include <map>
#include <string>
#include <vector>

/// Eventually it may become important to distinguish strings that
/// represent input sequences, for interactive validation.

typedef datum_string datum_sequence;

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
/// this class's UDTs generally store arithmetic quantities as native
/// arithmetic types, whereas it's simplest for the Controller to hold
/// all data uniformly as strings in its map; and because conversion
/// between them may not perfectly preserve value, notably in the case
/// of floating-point quantities.
///
/// For example, "1.07" in a text control may be translated to
///   (double)(1.07) [an implementation-defined value: C++98 2.13.3/1]
/// but the latter, converted to a string, with the maximum precision
/// the machine is capable of, could differ from the original "1.07".
/// A user who reloads saved input from a file would likely protest
/// "but I didn't say 1.0700000000001". Truncating to less precision
/// than is available merely engenders complaints from other users who
/// may enter pi to machine precision and expect more than "3.1416".
/// This framework uses function template numeric_io_cast() (which
/// avoids this problem as well as is possible for decimal fractions)
/// to convert what the user types to a native arithmetic type. Iff
/// that changes the value, then user input is altered accordingly.
/// For instance, on the author's machine, a number entered in an edit
/// control as
///   1.07000000000000081
/// is transformed to
///   1.070000000000001
/// as soon as the edit control loses focus, because numeric_io_cast()
/// converts them to different strings; but the input string
///   1.07000000000000011
/// is not altered. Formally, let
///   N be numeric_io_cast<std::string>(double), and
///   S be numeric_io_cast<double>(std::string);
/// then
///   S( 1.07000000000000081 ) == S( 1.070000000000001 )
/// where both convert to "1.070000000000001", but
///   N("1.07000000000000081") <> N("1.070000000000001")
/// the relative error being epsilon. The general rule is that user
/// (string) input X is altered to S(X) iff N(X) <> N(S(X)).
///
/// Data members are UDTs that help express certain relationships
/// among the controls that represent them. For example:
///  - discrete-valued controls like wxItemContainer and wxRadioBox
///    (all of which share wxItemContainerImmutable as a base class)
///    are mapped to an enumerative type that constrains assignment to
///    values that are permissible within the overall context of the
///    input object;
///  - a radiobox might offer three choices, but allow only the first
///    two if the input object is in a particular state determined by
///    the contents of other controls;
///  - a text control that represents a number might have a minimum
///    and a maximum value.
/// These UDTs bear values in a natural, more primitive type, provide
/// for conversion to and from strings, and hold enablement state and
/// other information for use by controls.
///
/// Usually, data member names ending in a single '_' are preferred.
/// Some of this class's data members' names nominate Model entities
/// that are referenced externally (in xml as well as C++ files) as
/// strings (through base class template MemberSymbolTable), though;
/// for them, readability overcomes the usual convention.

class LMI_SO Input
    :virtual private obstruct_slicing<Input>
    ,virtual public xml_serializable
    ,public MvcModel
    ,public MemberSymbolTable<Input>
    ,private boost::equality_comparable<Input>
{
    friend class input_test;
    friend class yare_input;

  public:
    Input();
    Input(Input const&);
    virtual ~Input();

    Input& operator=(Input const&);
    bool operator==(Input const&) const;

    std::vector<std::string> RealizeAllSequenceInput(bool report_errors = true);

    std::string differing_fields(Input const&) const;

    mcenum_ledger_type ledger_type () const {return GleanedLedgerType_;}
    int                maturity_age() const {return GleanedMaturityAge_;}

    int years_to_retirement() const {return retirement_age() - issue_age();}
    int years_to_maturity  () const {return maturity_age() - issue_age();}
    int issue_age          () const {return IssueAge     .value();}
    int retirement_age     () const {return RetirementAge.value();}
    int inforce_year       () const {return InforceYear  .value();}
    int effective_year     () const {return EffectiveDate.value().year();}

    static Input magically_rectify(Input const&);

  private:
    void AscribeMembers();

    // TODO ?? Dubious stuff to support scalar alternative controls.
    void WithdrawalChanged();
    void EnableTransferToSequenceControls(bool enable);
    void TransferWithdrawalSimpleControlsToInputSequence();
    void TransferWithdrawalInputSequenceToSimpleControls
        (InputSequence const& s
        );

    // Class 'xml_serializable' required implementation.
    virtual void read (xml::element const&);
    virtual void write(xml::element&) const;
    virtual int class_version() const;
    virtual std::string xml_root_name() const;

    // Backward compatibility.
    std::string RedintegrateExAnte
        (int                file_version
        ,std::string const& name
        ,std::string const& value
        ) const;
    void        RedintegrateExPost
        (int                                file_version
        ,std::map<std::string, std::string> detritus_map
        ,std::list<std::string>             residuary_names
        );

    // MvcModel required implementation.
    virtual void DoAdaptExternalities();
    virtual datum_base const* DoBaseDatumPointer(std::string const&) const;
    virtual any_entity      & DoEntity(std::string const&)      ;
    virtual any_entity const& DoEntity(std::string const&) const;
    virtual NamesType const& DoNames() const;
    virtual StateType        DoState() const;
    virtual void DoCustomizeInitialValues();
    virtual void DoEnforceCircumscription(std::string const&);
    virtual void DoEnforceProscription   (std::string const&);
    virtual void DoHarmonize();
    virtual void DoTransmogrify();

    void SetSolveDurations();

    std::map<std::string,std::string> const permissible_specified_amount_strategy_keywords();
    std::map<std::string,std::string> const permissible_death_benefit_option_keywords();
    std::map<std::string,std::string> const permissible_payment_strategy_keywords();
    std::map<std::string,std::string> const permissible_payment_mode_keywords();

    std::string RealizeExtraMonthlyCustodialFee   ();
    std::string RealizeExtraCompensationOnAssets  ();
    std::string RealizeExtraCompensationOnPremium ();
    std::string RealizePartialMortalityMultiplier ();
    std::string RealizeCurrentCoiMultiplier       ();
    std::string RealizeCashValueEnhancementRate   ();
    std::string RealizeCorporationTaxBracket      ();
    std::string RealizeTaxBracket                 ();
    std::string RealizeProjectedSalary            ();
    std::string RealizeSpecifiedAmount            ();
    std::string RealizeDeathBenefitOption         ();
    std::string RealizePayment                    ();
    std::string RealizePaymentMode                ();
    std::string RealizeCorporationPayment         ();
    std::string RealizeCorporationPaymentMode     ();
    std::string RealizeGeneralAccountRate         ();
    std::string RealizeSeparateAccountRate        ();
    std::string RealizeNewLoan                    ();
    std::string RealizeWithdrawal                 ();
    std::string RealizeFlatExtra                  ();
    std::string RealizeHoneymoonValueSpread       ();
    std::string RealizePremiumHistory             ();
    std::string RealizeSpecamtHistory             ();

    void make_term_rider_consistent(bool aggressively = true);

    boost::scoped_ptr<TDatabase> database_;

    // Database axes are independent variables; they're "cached" along
    // with the database, which is reset when any of them changes.
    // Dependent variables, stored only as an optimization, are
    // "gleaned" whenever the database is reset.
    std::string              CachedProductName_          ;
    mcenum_gender            CachedGender_               ;
    mcenum_class             CachedUnderwritingClass_    ;
    mcenum_smoking           CachedSmoking_              ;
    int                      CachedIssueAge_             ;
    mcenum_uw_basis          CachedGroupUnderwritingType_;
    mcenum_state             CachedStateOfJurisdiction_  ;
    int                      GleanedMaturityAge_         ;
    mcenum_ledger_type       GleanedLedgerType_          ;

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
    tnr_proportion           TermRiderProportion             ;
    mce_yes_or_no            TermRiderUseProportion          ;
    tnr_date                 DateOfBirth                     ;
    tnr_date                 DateOfRetirement                ; // TODO ?? Expunge this everywhere.
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
    mce_solve_target         SolveTarget                     ;
    tnr_nonnegative_double   SolveTargetCashSurrenderValue   ;
    tnr_duration             SolveTargetYear                 ;
    mce_gen_basis            SolveBasis                      ; // TODO ?? Poor name, but enumerators are correct.
    mce_sep_basis            SolveSeparateAccountBasis       ;
    mce_yes_or_no            UseCurrentDeclaredRate          ;
    mce_gen_acct_rate_type   GeneralAccountRateType          ;
    mce_sep_acct_rate_type   SeparateAccountRateType         ;
    tnr_proportion           LoanRate                        ;
    mce_loan_rate_type       LoanRateType                    ;
    mce_yes_or_no            OverrideExperienceReserveRate   ;
    tnr_proportion           ExperienceReserveRate           ;
    tnr_nonnegative_double   ExperienceRatingInitialKFactor  ;
    tnr_unrestricted_double  InforceNetExperienceReserve     ;
    tnr_nonnegative_double   InforceYtdNetCoiCharge          ;
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
    tnr_date                 LastCoiReentryDate              ;
    mce_yes_or_no            BlendGender                     ;
    mce_yes_or_no            BlendSmoking                    ;
    tnr_proportion           MaleProportion                  ;
    tnr_proportion           NonsmokerProportion             ;
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
    tnr_corridor_factor      UseOffshoreCorridorFactor       ; // INPUT !! Obsolete, and apparently wrong type.
    mce_yes_or_no            ChildRider                      ;
    tnr_nonnegative_double   ChildRiderAmount                ; // TODO ?? Specialize?
    mce_yes_or_no            SpouseRider                     ;
    tnr_nonnegative_double   SpouseRiderAmount               ; // TODO ?? Specialize?
    tnr_issue_age            SpouseIssueAge                  ; // TODO ?? Specialize?
    datum_string             Franchise                       ;
    datum_string             PolicyNumber                    ;
    tnr_date                 InforceAsOfDate                 ;
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
    datum_sequence           OffshoreCorridorFactor          ; // INPUT !! Obsolete.
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
    mce_to_point             DeprecatedSolveTgtAtWhich       ;
    mce_from_point           DeprecatedSolveFromWhich        ;
    mce_to_point             DeprecatedSolveToWhich          ;
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

    // TODO ?? For the nonce, only an unrestricted tn_range type is
    // used, because tn_range doesn't permit initialization or
    // assignment from any out-of-range value, but the Realize*()
    // functions expect to test raw values themselves.
    //
    // Likely types are suggested in comments for later consideration.

    std::vector<tnr_unrestricted_double> FundAllocationsRealized_;

    std::vector<tnr_unrestricted_double> ExtraMonthlyCustodialFeeRealized_  ; // [permit negative?]
    std::vector<tnr_unrestricted_double> ExtraCompensationOnAssetsRealized_ ; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> ExtraCompensationOnPremiumRealized_; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> PartialMortalityMultiplierRealized_; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> CurrentCoiMultiplierRealized_      ; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> CashValueEnhancementRateRealized_  ; // [to be expunged]
    std::vector<tnr_unrestricted_double> CorporationTaxBracketRealized_     ; // tnr_proportion
    std::vector<tnr_unrestricted_double> TaxBracketRealized_                ; // tnr_proportion
    std::vector<tnr_unrestricted_double> ProjectedSalaryRealized_           ; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> SpecifiedAmountRealized_           ; // [permit negative for term adjustment?]
    std::vector<mce_sa_strategy>         SpecifiedAmountStrategyRealized_   ;
    std::vector<mce_dbopt>               DeathBenefitOptionRealized_        ;
    std::vector<tnr_unrestricted_double> PaymentRealized_                   ; // tnr_nonnegative_double
    std::vector<mce_pmt_strategy>        PaymentStrategyRealized_           ;
    std::vector<mce_mode>                PaymentModeRealized_               ;
    std::vector<tnr_unrestricted_double> CorporationPaymentRealized_        ; // tnr_nonnegative_double
    std::vector<mce_pmt_strategy>        CorporationPaymentStrategyRealized_;
    std::vector<mce_mode>                CorporationPaymentModeRealized_    ;
    std::vector<tnr_unrestricted_double> GeneralAccountRateRealized_        ; // tnr_interest_rate (new)
    std::vector<tnr_unrestricted_double> SeparateAccountRateRealized_       ; // tnr_interest_rate (new)
    std::vector<tnr_unrestricted_double> NewLoanRealized_                   ; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> WithdrawalRealized_                ; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> FlatExtraRealized_                 ; // tnr_nonnegative_double
    std::vector<tnr_unrestricted_double> HoneymoonValueSpreadRealized_      ; // tnr_interest_rate (new)
    std::vector<tnr_unrestricted_double> PremiumHistoryRealized_            ; // tnr_unrestricted_double
    std::vector<tnr_unrestricted_double> SpecamtHistoryRealized_            ; // tnr_nonnegative_double
};

/// Specialization of struct template reconstitutor for this Model
/// and the base class that all its UDTs share.

template<> struct reconstitutor<datum_base, Input>
{
    typedef datum_base DesiredType;
    static DesiredType* reconstitute(any_member<Input>& m)
        {
        DesiredType* z = 0;
        z = exact_cast<ce_product_name         >(m); if(z) return z;
        z = exact_cast<datum_string            >(m); if(z) return z;
        z = exact_cast<mce_gen_basis           >(m); if(z) return z;
        z = exact_cast<mce_class               >(m); if(z) return z;
        z = exact_cast<mce_country             >(m); if(z) return z;
        z = exact_cast<mce_dbopt               >(m); if(z) return z;
        z = exact_cast<mce_defn_life_ins       >(m); if(z) return z;
        z = exact_cast<mce_defn_material_change>(m); if(z) return z;
        z = exact_cast<mce_from_point          >(m); if(z) return z;
        z = exact_cast<mce_fund_input_method   >(m); if(z) return z;
        z = exact_cast<mce_gen_acct_rate_type  >(m); if(z) return z;
        z = exact_cast<mce_gender              >(m); if(z) return z;
        z = exact_cast<mce_loan_rate_type      >(m); if(z) return z;
        z = exact_cast<mce_mec_avoid_method    >(m); if(z) return z;
        z = exact_cast<mce_mode                >(m); if(z) return z;
        z = exact_cast<mce_part_mort_table     >(m); if(z) return z;
        z = exact_cast<mce_pmt_strategy        >(m); if(z) return z;
        z = exact_cast<mce_premium_table       >(m); if(z) return z;
        z = exact_cast<mce_report_column       >(m); if(z) return z;
        z = exact_cast<mce_run_order           >(m); if(z) return z;
        z = exact_cast<mce_sa_strategy         >(m); if(z) return z;
        z = exact_cast<mce_sep_acct_rate_type  >(m); if(z) return z;
        z = exact_cast<mce_sep_basis           >(m); if(z) return z;
        z = exact_cast<mce_smoking             >(m); if(z) return z;
        z = exact_cast<mce_solve_target        >(m); if(z) return z;
        z = exact_cast<mce_solve_type          >(m); if(z) return z;
        z = exact_cast<mce_state               >(m); if(z) return z;
        z = exact_cast<mce_survival_limit      >(m); if(z) return z;
        z = exact_cast<mce_table_rating        >(m); if(z) return z;
        z = exact_cast<mce_term_adj_method     >(m); if(z) return z;
        z = exact_cast<mce_to_point            >(m); if(z) return z;
        z = exact_cast<mce_uw_basis            >(m); if(z) return z;
        z = exact_cast<mce_yes_or_no           >(m); if(z) return z;
        z = exact_cast<tnr_attained_age        >(m); if(z) return z;
        z = exact_cast<tnr_corridor_factor     >(m); if(z) return z;
        z = exact_cast<tnr_date                >(m); if(z) return z;
        z = exact_cast<tnr_duration            >(m); if(z) return z;
        z = exact_cast<tnr_issue_age           >(m); if(z) return z;
        z = exact_cast<tnr_month               >(m); if(z) return z;
        z = exact_cast<tnr_nonnegative_double  >(m); if(z) return z;
        z = exact_cast<tnr_nonnegative_integer >(m); if(z) return z;
        z = exact_cast<tnr_proportion          >(m); if(z) return z;
        z = exact_cast<tnr_unrestricted_double >(m); if(z) return z;
        return z;
        }
};

#endif // input_hpp

