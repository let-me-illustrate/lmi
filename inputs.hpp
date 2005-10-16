// Input parameters.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: inputs.hpp,v 1.8 2005-10-16 16:36:15 chicares Exp $

#ifndef inputs_hpp
#define inputs_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "inputstatus.hpp"
#include "xenumtypes.hpp"
#include "xrangetypes.hpp"

#include <iosfwd>
#include <string>
#include <vector>

class LMI_EXPIMP InputParms
{
  public:
    InputParms();
    virtual ~InputParms();

    bool operator==(InputParms const&) const;

    int YearsToMaturity() const;

    // TODO ?? See implementation file regarding 'const' on this function>
    void SetSolveDurations() const;
    int SumOfSepAcctFundAllocs() const;
    void ResetAllFunds(bool GeneralAccountAllowed);
    void EnforceConsistency();
    e_ledger_type LedgerType() const;

    bool NeedLoanRates() const;

    bool CheckAllowChangeToDBO2() const;
    bool CheckAllowDBO3() const;

    bool IsValid() const;
    std::string AgentFullName() const;
    std::string InsdFullName() const;

// TODO ?? Data should be private.
    std::string          ProductName          ; // TODO ?? Want validator.
    e_plan               Plan                 ; // TODO ?? Use 'ProductName' instead.

    r_positive_int       NumberOfLives        ;
    std::vector<InputStatus> Status           ; // TODO ?? Name should be plural.

    enum{NumberOfFunds = 30}; // TODO ?? Error to hardcode such a limit.
    // Percentage allocated to each separate account fund.
    // 100% minus the sum across the vector either:
    //   determines the general account allocation, if gen acct allowed; or
    //   must equal zero, if gen acct not allowed.
    std::vector<r_fund>          FundAllocs   ; // TODO ?? Want validator.
    std::vector<r_curr_int_rate> GenAcctRate  ;
    std::vector<r_curr_int_rate> SepAcctRate  ;
    std::vector<r_pmt>           EePremium    ;
    std::vector<r_pmt>           ErPremium    ;
    std::vector<e_mode>          EeMode       ;
    std::vector<e_mode>          ErMode       ;
    std::vector<r_loan>          Loan         ;
    std::vector<r_wd>            WD           ;
    std::vector<r_spec_amt>      SpecAmt      ;
    std::vector<e_dbopt>         DBOpt        ;
    std::vector<double>          Salary       ;
    std::vector<double>          IntegralTerm ;

    std::vector<double>          VectorAddonMonthlyCustodialFee;
    std::vector<double>          VectorAddonCompOnAssets;
    std::vector<double>          VectorAddonCompOnPremium;
    std::vector<double>          VectorNonUsCorridorFactor;
    std::vector<double>          VectorPartialMortalityMultiplier;
    std::vector<double>          VectorCurrentCoiMultiplier;
    std::vector<double>          VectorCurrentCoiGrading; // TODO ?? Obsolete: expunge.
    std::vector<double>          VectorCashValueEnhancementRate;
    std::vector<double>          VectorCorpTaxBracket;
    std::vector<double>          VectorIndvTaxBracket;
    std::vector<double>          VectorPolicyLevelFlatExtra;

    std::vector<e_sa_strategy>   VectorSpecifiedAmountStrategy;
    std::vector<e_pmt_strategy>  VectorIndvPaymentStrategy;
    std::vector<e_pmt_strategy>  VectorCorpPaymentStrategy;
    std::vector<e_loan_strategy> VectorNewLoanStrategy;
    std::vector<e_wd_strategy>   VectorWithdrawalStrategy;

    std::vector<double>          VectorHoneymoonValueSpread;
    std::vector<r_pmt>           VectorPremiumHistory;
    std::vector<r_spec_amt>      VectorSpecamtHistory;

    r_dumpin             Dumpin                      ;
    r_dumpin1035         External1035ExchangeAmount  ;
    r_basis1035          External1035ExchangeBasis   ;
    e_yes_or_no          External1035ExchangeFromMec ;
    r_dumpin1035         Internal1035ExchangeAmount  ;
    r_basis1035          Internal1035ExchangeBasis   ;
    e_yes_or_no          Internal1035ExchangeFromMec ;

    e_solve_tgt_at       SolveTgtAtWhich      ;
    r_solve_tgt_time     SolveTgtTime         ;
    e_solve_from         SolveFromWhich       ;
    r_solve_beg_time     SolveBegTime         ;
    e_solve_to           SolveToWhich         ;
    r_solve_end_time     SolveEndTime         ;

    e_solve_type         SolveType            ;
    r_solve_beg_year     SolveBegYear         ;
    r_solve_end_year     SolveEndYear         ;
    e_solve_target       SolveTarget          ;
    r_solve_tgt_csv      SolveTgtCSV          ;
    r_solve_tgt_year     SolveTgtYear         ;
    e_basis              SolveBasis           ;
    e_sep_acct_basis     SolveSABasis         ;

    e_pmt_strategy       EePmtStrategy        ; // obsolescent
    e_pmt_strategy       ErPmtStrategy        ; // obsolescent
    e_sa_strategy        SAStrategy           ; // obsolescent
    e_post_ret_sa_strategy PostRetType        ; // obsolescent
    r_nonnegative_double PostRetAmt           ; // obsolescent
    r_nonnegative_double PostRetPct           ; // obsolescent

    e_int_rate_type      IntRateTypeGA        ;
    e_int_rate_type      IntRateTypeSA        ;
    r_loan_int_rate      LoanIntRate          ;
    e_loan_rate_type     LoanRateType         ;

    r_nonnegative_double ExperienceRatingInitialKFactor;
    // TODO ?? Either this should have a distinct type, or the type
    // used should be generalized, e.g., to mean any positive interest
    // rate.
    r_loan_int_rate      ExperienceReserveRate;
    r_unchecked_double   InforceExperienceReserve;
    e_yes_or_no          OverrideExperienceReserveRate;

    e_yes_or_no          PayLoanInt           ; // Pay loan interest in cash
    e_yes_or_no          WDToBasisThenLoan    ;

    e_yes_or_no          AvgFund              ; // Use average fund charge
    e_yes_or_no          OverrideFundMgmtFee  ; // TODO ?? explain
    e_fund_input_method  FundChoiceType       ;
    r_nonnegative_double InputFundMgmtFee     ; // TODO ?? explain

    // Composites can be run life by life, or month by month.
    // TODO ?? It makes little sense to let this vary across lives in
    // a census. Ultimately it should be moved to a census-level
    // class, but none exists at present.
    e_run_order          RunOrder             ;

    // Note these differences:
    //   NumIdenticalLives is the number of *identical* lives that this
    //     class object represents. The case (census) could contain other
    //     lives as well. This parameter has no effect on an individual
    //     illustration--it affects the composite only, and can be
    //     thought of as representing the number of identical lives
    //     that a single census entry represents.
    //   NumberOfLives is the number of lives on one policy: 2 for survivorship
    //
    r_nonnegative_int    NumIdenticalLives    ;
    // TODO ?? Should not vary across lives in a census
    e_yes_or_no          UseExperienceRating  ;
    e_yes_or_no          UsePartialMort       ;
    e_part_mort_table    PartialMortTable     ;
    r_nonnegative_double PartialMortTableMult ;

    std::string          InsdFirstName        ;
    std::string          InsdMiddleName       ;
    std::string          InsdLastName         ;
    std::string          InsdAddr1            ;
    std::string          InsdAddr2            ;
    std::string          InsdCity             ;
    e_state              InsdState            ;
    std::string          InsdZipCode          ; // TODO ?? Want validator.
    std::string          InsdEeClass          ;
// TODO Prefer 'Corp'- to 'Sponsor'- .
    std::string          SponsorFirstName     ;
    std::string          SponsorAddr1         ;
    std::string          SponsorAddr2         ; // TODO ?? Not useful.
    std::string          SponsorCity          ;
    e_state              SponsorState         ;
    std::string          SponsorZipCode       ; // TODO ?? Want validator.
    // Even in the US, the format of corporate tax ID numbers varies by state.
    std::string          SponsorTaxpayerID    ;

    std::string          AgentFirstName       ;
    std::string          AgentMiddleName      ;
    std::string          AgentLastName        ;
    std::string          AgentAddr1           ;
    std::string          AgentAddr2           ;
    std::string          AgentCity            ;
    e_state              AgentState           ;
    std::string          AgentZipCode         ; // TODO ?? Want validator.
    std::string          AgentPhone           ; // TODO ?? Want validator.
    std::string          AgentID              ;

    e_premium_table      EePremTableNum       ;
    r_nonnegative_double EePremTableMult      ;
    e_premium_table      ErPremTableNum       ;
    r_nonnegative_double ErPremTableMult      ;
    mutable r_duration   WDBegYear            ; // obsolescent
    mutable r_duration   WDEndYear            ; // obsolescent
    mutable r_duration   LoanBegYear          ; // obsolescent
    mutable r_duration   LoanEndYear          ; // obsolescent
    r_date               EffDate              ; // Want validator.

    e_defn_life_ins      DefnLifeIns          ;
    e_defn_material_change DefnMaterialChange ;
    e_mec_avoid_method   AvoidMec             ;

    e_yes_or_no          RetireesCanEnroll    ;
    e_uw_basis           GroupUWType          ;
    e_yes_or_no          BlendMortGender      ;
    e_yes_or_no          BlendMortSmoking     ;
    r_proportion         MaleProportion       ;
    r_proportion         NonsmokerProportion  ;

    r_proportion         InitTermProportion   ;
    e_term_adj_method    TermAdj              ;

    mutable r_duration   EePremEndYear        ; // obsolescent
    mutable r_duration   ErPremEndYear        ; // obsolescent

    e_yes_or_no          IncludeInComposite   ;
    std::string          Comments             ;
    e_yes_or_no          AmortizePremLoad     ;

    r_inforce_year       InforceYear          ;
    r_inforce_month      InforceMonth         ;
    r_nonnegative_double InforceAVGenAcct     ;
    r_nonnegative_double InforceAVSepAcct     ;
    r_nonnegative_double InforceAVRegLn       ;
    r_nonnegative_double InforceAVPrfLn       ;
    r_nonnegative_double InforceRegLnBal      ;
    r_nonnegative_double InforcePrfLnBal      ;
    r_nonnegative_double InforceCumNoLapsePrem;
    r_nonnegative_double InforceCumPmts       ;

    // TODO ?? 'Extra*' variables are obsolete: use vector instead.
    r_nonnegative_double ExtraAssetComp       ; // obsolescent
    int                  ExtraAssetCompDur    ; // obsolescent
    r_nonnegative_double ExtraPremComp        ; // obsolescent
    int                  ExtraPremCompDur     ; // obsolescent
    r_unchecked_double   ExtraPolFee          ; // obsolescent

    e_country            Country              ;
    e_yes_or_no          OverrideCOIMultiplier;
    r_nonnegative_double CountryCOIMultiplier ;

    e_survival_limit     SurviveToType        ;
    r_duration           SurviveToYear        ;
    r_attained_age       SurviveToAge         ;

    r_nonnegative_double MaxNAAR              ;
    r_double_ge_one      NonUSCorridor        ;

    e_yes_or_no          HasChildRider        ;
    r_unchecked_double   ChildRiderAmount     ;
    e_yes_or_no          HasSpouseRider       ;
    r_unchecked_double   SpouseRiderAmount    ;
    r_nonnegative_int    SpouseIssueAge       ;

    std::string          Franchise            ;
    std::string          PolicyNumber         ;
// TODO ?? Constraints unknown.
    r_date               PolicyDate           ; // Want validator.
    r_unchecked_double   InforceTaxBasis      ;
    r_unchecked_double   InforceCumGlp        ;
    r_unchecked_double   InforceGlp           ;
    r_unchecked_double   InforceGsp           ;
    r_unchecked_double   Inforce7pp           ;
    e_yes_or_no          InforceIsMec         ;
// TODO ?? Constraints not yet ascertained.
    r_date               LastMatChgDate       ; // Want validator.
    r_nonnegative_double InforceDcv           ;
    r_unchecked_double   InforceDcvDb         ; // DCV death benefit--expunge?
    r_nonnegative_double InforceAvBeforeLastMc;
    r_nonnegative_double InforceHoneymoonValue;
    r_contract_year      InforceContractYear  ;
    r_contract_month     InforceContractMonth ;
    r_nonnegative_double InforceLeastDeathBenefit;
    e_state              StateOfJurisdiction  ;

    r_nonnegative_double SalarySAPct          ;
    r_nonnegative_double SalarySACap          ;
    r_nonnegative_double SalarySAOffset       ;

    e_yes_or_no          HasHoneymoon         ;
    r_unchecked_double   PostHoneymoonSpread  ;

    e_yes_or_no CreateSupplementalReport        ;
    std::string SupplementalReportColumn00      ;
    std::string SupplementalReportColumn01      ;
    std::string SupplementalReportColumn02      ;
    std::string SupplementalReportColumn03      ;
    std::string SupplementalReportColumn04      ;
    std::string SupplementalReportColumn05      ;
    std::string SupplementalReportColumn06      ;
    std::string SupplementalReportColumn07      ;
    std::string SupplementalReportColumn08      ;
    std::string SupplementalReportColumn09      ;
    std::string SupplementalReportColumn10      ;
    std::string SupplementalReportColumn11      ;

  private:
    int Length;
};

#endif // inputs_hpp

