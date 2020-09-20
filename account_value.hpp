// Account value.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef account_value_hpp
#define account_value_hpp

#include "config.hpp"

#include "basic_values.hpp"
#include "oecumenic_enumerations.hpp"
#include "so_attributes.hpp"

#include <fstream>
#include <iosfwd>
#include <memory>                       // shared_ptr
#include <string>
#include <vector>

// Accumulates account values in four distinct accounts:
//   general account (unloaned)
//   separate account
//   regular loans
//   preferred loans

class Input;
class Ledger;
class LedgerInvariant;
class LedgerVariant;

class LMI_SO AccountValue final
    :protected BasicValues
{
    friend class SolveHelper;
    friend class run_census_in_parallel;
    friend double SolveTest();

  public:
    enum {months_per_year = 12};

    explicit AccountValue(Input const& input);
    AccountValue(AccountValue&&) = default;
    ~AccountValue() override = default;

    void RunAV();

    void SetDebugFilename    (std::string const&);

    void SolveSetPmts // Antediluvian.
        (double a_Pmt
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetSpecAmt // Antediluvian.
        (double a_Bft
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetLoans // Antediluvian.
        (double a_Loan
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetWDs // Antediluvian.
        (double a_WD
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetLoanThenWD // Antediluvian.
        (double a_Amt
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );

    std::shared_ptr<Ledger const> ledger_from_av() const;

  private:
    AccountValue(AccountValue const&) = delete;
    AccountValue& operator=(AccountValue const&) = delete;

    LedgerInvariant const& InvariantValues() const;
    LedgerVariant   const& VariantValues  () const;

    int                    GetLength     () const;

    double InforceLivesBoy         () const;
    double InforceLivesEoy         () const;
    double GetSepAcctAssetsInforce () const;

    void process_payment          (double);
    void IncrementAVProportionally(double);
    void IncrementAVPreferentially(double, oenum_increment_account_preference);
    void process_deduction        (double);
    void process_distribution     (double);
    void DecrementAVProportionally(double);
    void DecrementAVProgressively (double, oenum_increment_account_preference);

    double TotalAccountValue() const;
    double CashValueFor7702() const;

    double base_specamt(int year) const;
    double term_specamt(int year) const;
    double specamt_for_7702(int year) const;
    double specamt_for_7702A(int year) const;

    // We're not yet entirely sure how to handle ledger values. Right now,
    // we have pointers to a Ledger and also to its variant and invariant
    // parts. We put data into the parts, and then insert the parts into
    // the Ledger. At this moment it seems best to work not through these
    // "parts" but rather through references to components of the Ledger.
    // While we gather more information and consider this, all access comes
    // through the following functions.
    LedgerInvariant& InvariantValues();
    LedgerVariant  & VariantValues  ();

    double RunOneCell              (mcenum_run_basis);
    double RunOneBasis             (mcenum_run_basis);
    double RunAllApplicableBases   ();
    void   InitializeLife          (mcenum_run_basis);
    void   FinalizeLife            (mcenum_run_basis);
    void   FinalizeLifeAllBases    ();
    void   SetGuarPrem             ();
    void   InitializeYear          ();
    void   InitializeSpecAmt       ();
    void   FinalizeYear            ();
    void   DoMonth(); // Antediluvian.
    void   DoMonthDR               ();
    void   DoMonthCR               ();
    void   SetInitialValues        ();
    void   SetAnnualInvariants     ();

    void DoYear // Antediluvian.
        (mcenum_run_basis a_TheBasis
        ,int              a_Year
        ,int              a_InforceMonth = 0
        );

    void   SolveSetSpecAmt      (double a_CandidateValue);
    void   SolveSetEePrem       (double a_CandidateValue);
    void   SolveSetErPrem       (double a_CandidateValue);
    void   SolveSetLoan         (double a_CandidateValue);
    void   SolveSetWD           (double a_CandidateValue);

    void   DebugPrint           ();

    void   SetClaims();
    double GetCurtateNetClaimsInforce    () const;
    double GetCurtateNetCoiChargeInforce () const;
    void   SetProjectedCoiCharge         ();
    double GetProjectedCoiChargeInforce  () const;
    double ApportionNetMortalityReserve(double reserve_per_life_inforce);
    double experience_rating_amortization_years() const;
    double ibnr_as_months_of_mortality_charges() const;

    // To support the notion of an M&E charge that depends on total case
    // assets, we provide these functions, which are designed to be
    // called by a distant module that has a pointer to an object of this
    // class. Processing must be split into two functions here so that
    // total assets for all lives combined can be ascertained just prior
    // to the point where interest is credited.

    // Process monthly transactions up to but excluding interest credit
    double IncrementBOM
        (int year
        ,int month
        ,double a_case_k_factor
        );
    // Credit interest and process all subsequent monthly transactions
    void IncrementEOM
        (int    year
        ,int    month
        ,double assets_post_bom
        ,double cum_pmts_post_bom
        );

    void IncrementEOY(int year);

    bool PrecedesInforceDuration(int year, int month);

    double Solve(); // Antediluvian.
    double Solve
        (mcenum_solve_type   a_SolveType
        ,int                 a_SolveBeginYear
        ,int                 a_SolveEndYear
        ,mcenum_solve_target a_SolveTarget
        ,double              a_SolveTargetCsv
        ,int                 a_SolveTargetYear
        ,mcenum_gen_basis    a_SolveGenBasis
        ,mcenum_sep_basis    a_SolveSepBasis
        );

    double SolveTest               (double a_CandidateValue);

    double SolveGuarPremium        ();

    void PerformSpecAmtStrategy();
    void PerformSupplAmtStrategy();
    double CalculateSpecAmtFromStrategy
        (int                actual_year
        ,int                reference_year
        ,double             explicit_value
        ,mcenum_sa_strategy strategy
        ) const;

    void PerformPmtStrategy(double* a_Pmt); // Antediluvian.
    double PerformEePmtStrategy       () const;
    double PerformErPmtStrategy       () const;
    double DoPerformPmtStrategy
        (mcenum_solve_type                       a_SolveForWhichPrem
        ,mcenum_mode                             a_CurrentMode
        ,mcenum_mode                             a_InitialMode
        ,double                                  a_TblMult
        ,std::vector<double> const&              a_PmtVector
        ,std::vector<mcenum_pmt_strategy> const& a_StrategyVector
        ) const;

    void InitializeMonth            ();
    void TxExch1035                 ();
    void TxOptionChange             ();
    void TxSpecAmtChange            ();
    void TxTestGPT                  ();
    void TxPmt(); // Antediluvian.
    void TxAscertainDesiredPayment  ();
    void TxLimitPayment             (double a_maxpmt);
    void TxRecognizePaymentFor7702A
        (double a_pmt
        ,bool   a_this_payment_is_unnecessary
        );
    void TxAcceptPayment            (double payment);
    double GetPremLoad
        (double a_pmt
        ,double a_portion_exempt_from_premium_tax
        );
    void TxLoanRepay             ();

    void TxSetBOMAV              ();
    void TxTestHoneymoonForExpiration();
    void TxSetTermAmt            ();
    void TxSetDeathBft           ();
    void TxSetCoiCharge          ();
    void TxSetRiderDed           ();
    void TxDoMlyDed              ();

    void TxTakeSepAcctLoad       ();
    void TxCreditInt             ();
    void TxLoanInt               ();
    void TxTakeWD                ();
    void TxTakeLoan              ();
    void TxCapitalizeLoan        ();

    void TxTestLapse             ();
    void TxDebug                 ();

    void FinalizeMonth           ();

    // Reflects optional daily interest accounting.
    double ActualMonthlyRate    (double monthly_rate) const;
    double InterestCredited
        (double principal
        ,double monthly_rate
        ) const;

    bool   IsModalPmtDate          (mcenum_mode) const;
    bool   IsModalPmtDate          (); // Antediluvian.
    int    MonthsToNextModalPmtDate() const;
    double anticipated_deduction   (mcenum_anticipated_deduction);

    double minimum_specified_amount(bool issuing_now, bool term_rider) const;
    void   ChangeSpecAmtBy         (double delta);
    void   ChangeSupplAmtBy        (double delta);

    double SurrChg                 () const;
    double CSVBoost                () const;

    void   set_list_bill_year_and_month();
    void   set_list_bill_premium();
    void   set_modal_min_premium();

    void   SetMaxLoan              ();
    void   SetMaxWD                ();
    double GetRefundableSalesLoad  () const;

    void   ApplyDynamicMandE       (double assets);

    void   SetMonthlyDetail(int enumerator, std::string const& s);
    void   SetMonthlyDetail(int enumerator, double d);
    void   DebugPrintInit();
    void   DebugEndBasis();

    void   EndTermRider(bool convert);

    void   CoordinateCounters();

    // Detailed monthly trace.
    std::string     DebugFilename;
    std::ofstream   DebugStream;
    std::vector<std::string> DebugRecord;

    double          PriorAVGenAcct;
    double          PriorAVSepAcct;
    double          PriorAVRegLn;
    double          PriorAVPrfLn;
    double          PriorRegLnBal;
    double          PriorPrfLnBal;

    // Mode flags.
    bool            Debugging;
    bool            Solving;
    bool            SolvingForGuarPremium;
    bool            ItLapsed;

    std::shared_ptr<Ledger         > ledger_;
    std::shared_ptr<LedgerInvariant> ledger_invariant_;
    std::shared_ptr<LedgerVariant  > ledger_variant_;

    oenum_increment_method             deduction_method;
    oenum_increment_account_preference deduction_preferred_account;
    oenum_increment_method             distribution_method;
    oenum_increment_account_preference distribution_preferred_account;
    oenum_allocation_method            ee_premium_allocation_method;
    oenum_increment_account_preference ee_premium_preferred_account;
    oenum_allocation_method            er_premium_allocation_method;
    oenum_increment_account_preference er_premium_preferred_account;

    double GuarPremium;

    // These data members make Solve() arguments available to SolveTest().
    int                 SolveBeginYear_;
    int                 SolveEndYear_;
    mcenum_solve_target SolveTarget_;
    double              SolveTargetCsv_;
    int                 SolveTargetDuration_;
    mcenum_gen_basis    SolveGenBasis_;
    mcenum_sep_basis    SolveSepBasis_;

    mcenum_run_basis RunBasis_;
    mcenum_gen_basis GenBasis_;
    mcenum_sep_basis SepBasis_;

    int         LapseMonth; // Antediluvian.
    int         LapseYear;  // Antediluvian.

    double External1035Amount;
    double Internal1035Amount;
    double Dumpin;

    double MlyNoLapsePrem;
    double CumNoLapsePrem;
    bool   NoLapseActive;

    // Solves need to know when a no-lapse guarantee is active.
    // Prefer int here because vector<bool> is not a container.
    std::vector<int> YearlyNoLapseActive;

    // Ullage is any positive excess of amount requested over amount available.
    std::vector<double> loan_ullage_;
    std::vector<double> withdrawal_ullage_;

    double CumPmts;
    double TaxBasis;
    // This supports solves for tax basis. Eventually it should be
    // moved into the invariant-ledger class.
    std::vector<double> YearlyTaxBasis;

    // Ee- and Er-GrossPmts aren't used directly in the AV calculations.
    // They must be kept separate for ledger output, and also for
    // tax basis calculations (when we fix that).
    std::vector<double> GrossPmts;
    std::vector<double> EeGrossPmts;
    std::vector<double> ErGrossPmts;
    std::vector<double> NetPmts;

    // Reproposal input.
    int     InforceYear;
    int     InforceMonth;
    double  InforceAVGenAcct;
    double  InforceAVSepAcct;
    double  InforceAVRegLn;
    double  InforceAVPrfLn;
    double  InforceRegLnBal;
    double  InforcePrfLnBal;
    double  InforceCumNoLapsePrem;
    double  InforceBasis;
    double  InforceCumPmts;
    double  InforceTaxBasis;
    double  InforceLoanBalance;

    // Intermediate values.
    int     Year;
    int     Month;
    int     MonthsSinceIssue;
    bool    daily_interest_accounting;
    int     days_in_policy_month;
    int     days_in_policy_year;
    double  AVGenAcct;
    double  AVSepAcct;
    double  SepAcctValueAfterDeduction;
    double  GenAcctPaymentAllocation;
    double  SepAcctPaymentAllocation;
    double  NAAR;
    double  CoiCharge;
    double  RiderCharges;
    double  NetCoiCharge;
    double  SpecAmtLoadBase;
    double  DacTaxRsv;

    double  AVUnloaned; // Antediluvian.

    double  NetMaxNecessaryPremium;
    double  GrossMaxNecessaryPremium;
    double  NecessaryPremium;
    double  UnnecessaryPremium;

    // 7702A CVAT deemed cash value.
    double  Dcv;
    double  DcvDeathBft;
    double  DcvNaar;
    double  DcvCoiCharge;
    double  DcvTermCharge;
    double  DcvWpCharge;
    // For other riders like AD&D, charge for DCV = charge otherwise.

    // Honeymoon provision.
    bool    HoneymoonActive;
    double  HoneymoonValue;

    // 7702 GPT
    double  GptForceout;
    double  YearsTotalGptForceout;

    // Intermediate values within annual or monthly loop only.
    double      pmt;       // Antediluvian.
    mcenum_mode pmt_mode;  // Antediluvian.
    int         ModeIndex; // Antediluvian.

    double  GenAcctIntCred;
    double  SepAcctIntCred;
    double  RegLnIntCred;
    double  PrfLnIntCred;
    double  AVRegLn;
    double  AVPrfLn;
    double  RegLnBal;
    double  PrfLnBal;
    double  MaxLoan;
    double  UnusedTargetPrem;
    double  AnnualTargetPrem;
    double  MaxWD;
    double  GrossWD;
    double  NetWD;
    double  CumWD;

    double      wd;           // Antediluvian.
    double      mlyguarv;     // Antediluvian.

    // For GPT: SA, DB, and DBOpt before the day's transactions are applied.
    double       OldSA;
    double       OldDB;
    mcenum_dbopt OldDBOpt;

    // Permanent invariants are in class BasicValues; these are
    // annual invariants.
    double       YearsCorridorFactor;
    mcenum_dbopt YearsDBOpt;
    double       YearsAnnualPolicyFee;
    double       YearsMonthlyPolicyFee;
    double       YearsGenAcctIntRate;
    double       YearsSepAcctIntRate;

    double       YearsDcvIntRate;

    double       YearsHoneymoonValueRate;
    double       YearsPostHoneymoonGenAcctIntRate;

    double       YearsRegLnIntCredRate;
    double       YearsPrfLnIntCredRate;
    double       YearsRegLnIntDueRate;
    double       YearsPrfLnIntDueRate;

    double       YearsCoiRate0;
    double       YearsCoiRate1;
    double       YearsCoiRate2;
    double       YearsDcvCoiRate;
    double       YearsAdbRate;
    double       YearsTermRate;
    double       YearsWpRate;
    double       YearsSpouseRiderRate;
    double       YearsChildRiderRate;
    double       YearsPremLoadTgt;
    double       YearsPremLoadExc;
    double       YearsTotLoadTgt;
    double       YearsTotLoadExc;
    double       YearsTotLoadTgtLowestPremtax;
    double       YearsTotLoadExcLowestPremtax;
    double       YearsSalesLoadTgt;
    double       YearsSalesLoadExc;
    double       YearsSpecAmtLoadRate;
    double       YearsSepAcctLoadRate;
    double       YearsSalesLoadRefundRate;
    double       YearsDacTaxLoadRate;

    double  MonthsPolicyFees;
    double  SpecAmtLoad;
    double  premium_load_;
    double  sales_load_;
    double  premium_tax_load_;
    double  dac_tax_load_;

    // Stratified loads are determined by assets and cumulative
    // payments immediately after the monthly deduction. Both are
    // stored at the proper moment, where they're constrained to be
    // nonnegative. Stratified loads happen to be used only for the
    // separate account.
    double  AssetsPostBom;
    double  CumPmtsPostBom;
    double  SepAcctLoad;

    double  case_k_factor;
    double  ActualCoiRate;

    bool    SplitMinPrem;
    bool    UnsplitSplitMinPrem;

    int     list_bill_year_  {methuselah};
    int     list_bill_month_ {13};

    bool    TermCanLapse;
    bool    TermRiderActive;
    double  ActualSpecAmt;
    double  TermSpecAmt;
    double  TermDB;
    double  DB7702A;
    double  DBIgnoringCorr;
    double  DBReflectingCorr;

    double      deathbft; // Antediluvian.
    bool        haswp;    // Antediluvian.
    bool        hasadb;   // Antediluvian.

    double  ActualLoan;
    double  RequestedLoan;
    double  RequestedWD;

    double  AdbCharge;
    double  SpouseRiderCharge;
    double  ChildRiderCharge;
    double  WpCharge;
    double  TermCharge;

    double  MlyDed;
    double  mlydedtonextmodalpmtdate; // Antediluvian.

    double  YearsTotalCoiCharge;
    double  YearsTotalRiderCharges;
    double  YearsAVRelOnDeath;
    double  YearsLoanRepaidOnDeath;
    double  YearsGrossClaims;
    double  YearsDeathProceeds;
    double  YearsNetClaims;
    double  YearsTotalNetIntCredited;
    double  YearsTotalGrossIntCredited;
    double  YearsTotalLoanIntAccrued;
    double  YearsTotalPolicyFee;
    double  YearsTotalDacTaxLoad;
    double  YearsTotalSpecAmtLoad;
    double  YearsTotalSepAcctLoad;

    // For experience rating.
    double  CoiRetentionRate;
    double  ExperienceRatingAmortizationYears;
    double  IbnrAsMonthsOfMortalityCharges;
    double  NextYearsProjectedCoiCharge;
    double  YearsTotalNetCoiCharge;

    double  CumulativeSalesLoad;

    // Illustrated outlay must be the same for current, guaranteed,
    // and all other bases. Outlay components are set on whichever
    // basis governs, usually current, then stored for use with all
    // other bases.

    std::vector<double> OverridingPmts; // Antediluvian.
    std::vector<double> stored_pmts;    // Antediluvian.

    std::vector<double> OverridingEePmts;
    std::vector<double> OverridingErPmts;

    // We need no 'OverridingDumpin' because we simply treat dumpin as
    // employee premium.
    double OverridingExternal1035Amount;
    double OverridingInternal1035Amount;

    std::vector<double> OverridingLoan;
    std::vector<double> OverridingWD;

    std::vector<double> SurrChg_; // Of uncertain utility.
};

//============================================================================
inline double AccountValue::TotalAccountValue() const
{
    return AVGenAcct + AVSepAcct + AVRegLn + AVPrfLn;
}

//============================================================================
inline LedgerVariant& AccountValue::VariantValues() // Temporary artifact?
{
    return *ledger_variant_;
}

//============================================================================
inline LedgerVariant const& AccountValue::VariantValues() const
{
    return *ledger_variant_;
}

//============================================================================
inline LedgerInvariant& AccountValue::InvariantValues() // Temporary artifact?
{
    return *ledger_invariant_;
}

//============================================================================
inline LedgerInvariant const& AccountValue::InvariantValues() const
{
    return *ledger_invariant_;
}

//============================================================================
inline int AccountValue::GetLength() const
{
    return BasicValues::GetLength();
}

//============================================================================
inline double AccountValue::experience_rating_amortization_years() const
{
    return ExperienceRatingAmortizationYears;
}

//============================================================================
inline double AccountValue::ibnr_as_months_of_mortality_charges() const
{
    return IbnrAsMonthsOfMortalityCharges;
}

#endif // account_value_hpp
