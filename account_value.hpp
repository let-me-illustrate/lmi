// Account value.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: account_value.hpp,v 1.28 2005-09-11 22:19:10 chicares Exp $

#ifndef account_value_hpp
#define account_value_hpp

#include "config.hpp"

#include "basic_values.hpp"
#include "expimp.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <iosfwd>
#include <fstream>
#include <string>
#include <vector>

// Accumulates account values in four distinct accounts:
//   general account (unloaned)
//   separate account
//   regular loans
//   preferred loans

class InputParms;
class Ledger;
class LedgerInvariant;
class LedgerVariant;
class DebugView;

class LMI_EXPIMP AccountValue
    :protected BasicValues
    ,private boost::noncopyable
{
    friend class SolveHelper;
    friend class run_census_in_parallel;
    friend double SolveTest();

  public:
    enum {months_per_year = 12};

    explicit AccountValue(InputParms const& input);
    ~AccountValue();

    double RunAV                ();

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

    boost::shared_ptr<Ledger const> ledger_from_av() const;

  private:
    LedgerInvariant const& InvariantValues() const;
    LedgerVariant   const& VariantValues  () const;

    int                    GetLength     () const;
    e_ledger_type const&   GetLedgerType () const;

    double GetInforceLives            () const;
    double GetSepAcctAssetsInforce    () const;
    double GetLastCoiChargeInforce    () const;

    void process_payment          (double);
    void IncrementAVProportionally(double);
    void IncrementAVPreferentially(double, e_increment_account_preference);
    void process_deduction        (double);
    void process_distribution     (double);
    void DecrementAVProportionally(double);
    void DecrementAVProgressively (double, e_increment_account_preference);

    double TotalAccountValue() const;
    double CashValueFor7702() const;

    // We're not yet entirely sure how to handle ledger values. Right now,
    // we have pointers to a Ledger and also to its variant and invariant
    // parts. We put data into the parts, and then insert the parts into
    // the Ledger. At this moment it seems best to work not through these
    // "parts" but rather through references to components of the Ledger.
    // While we gather more information and consider this, all access comes
    // through the following functions.
    LedgerInvariant& InvariantValues();
    LedgerVariant  & VariantValues  ();

    double RunOneCell              (e_run_basis const&);
    double RunOneBasis             (e_run_basis const&);
    double RunAllApplicableBases   ();
    void   InitializeLife          (e_run_basis const&);
    void   FinalizeLife            (e_run_basis const&);
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
        (e_run_basis const& a_TheBasis
        ,int                a_Year
        ,int                a_InforceMonth = 0
        );

    void   SolveSetSpecAmt      (double a_CandidateValue);
    void   SolveSetEePrem       (double a_CandidateValue);
    void   SolveSetErPrem       (double a_CandidateValue);
    void   SolveSetLoan         (double a_CandidateValue);
    void   SolveSetWD           (double a_CandidateValue);
    void   SolveSetWDThenLoan   (double a_CandidateValue);

    void   DebugPrint(std::ostream& os) const; // Antediluvian.
    void   DebugPrint           ();
    void   DebugRestart         (std::string const& reason);

    void   SetClaims();
    double GetCurtateNetClaimsInforce();
    void   SetProjectedCoiCharge();
    double GetProjectedCoiChargeInforce();
    double ApportionNetMortalityReserve
        (double case_net_mortality_reserve
        ,double case_years_net_mortchgs
        );
    double experience_rating_amortization_years() const;
    double ibnr_as_months_of_mortality_charges() const;

    void GuessWhetherFirstYearPremiumExceedsRetaliationLimit();
    bool TestWhetherFirstYearPremiumExceededRetaliationLimit();

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
        (int year
        ,int month
        ,double MandE
        );

    void IncrementEOY(int year);

    bool PrecedesInforceDuration(int year, int month);

    double Solve(); // Antediluvian.
    double Solve
        (e_solve_type const&     a_SolveType
        ,int                     a_SolveBegYear
        ,int                     a_SolveEndYear
        ,e_solve_target const&   a_SolveTarget
        ,int                     a_SolveTgtCSV
        ,int                     a_SolveTgtYear
        ,e_basis const&          a_SolveBasis
        ,e_sep_acct_basis const& a_SolveSABasis
        );

    double SolveTest               (double a_CandidateValue);
    void SolveSetTargetValueAndDuration
        (e_solve_target const&  a_SolveTarget
        ,double                 a_SolveTgtCSV
        ,int                    a_SolveTgtYear
        );

    double SolveGuarPremium        ();

    double GetPartMortQ            (int year) const;

    void PerformSpecAmtStrategy(); // Antediluvian.
    double CalculateSpecAmtFromStrategy
        (int actual_year
        ,int reference_year
        ) const;
    void OldPerformSpecAmtStrategy  ();
    void NewPerformSpecAmtStrategy  ();
    void PerformSpecAmtStrategy
        (double*                    sa
        ,e_sa_strategy&             strategy
        ,e_solve_type const&        SolveForWhichPrem
        ,e_mode const&              m
        ,std::vector<double> const& SAVector
        );

    void PerformPmtStrategy(double* a_Pmt); // Antediluvian.
    double PerformEePmtStrategy       () const;
    double PerformErPmtStrategy       () const;
    double DoPerformPmtStrategy
        (e_solve_type const&                a_SolveForWhichPrem
        ,e_mode const&                      a_CurrentMode
        ,e_mode const&                      a_InitialMode
        ,double                             a_TblMult
        ,std::vector<double> const&         a_PmtVector
        ,std::vector<e_pmt_strategy> const& a_StrategyVector
        ) const;

    void InitializeMonth            ();
    void TxExch1035                 ();
    void IncreaseSpecAmtToAvoidMec  ();
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
    double GetPremTaxLoad(double payment);
    void TxLoanRepay             ();

    void TxSetBOMAV              ();
    void TxTestHoneymoonForExpiration();
    void TxSetTermAmt            ();
    void TxSetDeathBft           (bool force_eoy_behavior = false);
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

    bool   IsModalPmtDate          (e_mode const& m) const;
    bool   IsModalPmtDate          (); // Antediluvian.
    int    MonthsToNextModalPmtDate() const;
    double anticipated_deduction   (e_anticipated_deduction const&);

    void   ChangeSpecAmtBy         (double delta);
    void   ChangeSurrChgSpecAmtBy  (double delta);
    void   AddSurrChgLayer         (int year, double delta_specamt);
    void   ReduceSurrChg           (int year, double partial_surrchg);
    double SurrChg                 ();

    void   SetMaxLoan              ();
    void   SetMaxWD                ();
    double GetRefundableSalesLoad  () const;
    double DetermineSpecAmtLoad    ();
    double DetermineAcctValLoadBOM (); // TODO ?? eradicate
    double DetermineAcctValLoadAMD ();

    void   ApplyDynamicMandE          (double assets);
    void   ApplyDynamicSepAcctLoadAMD (double assets, double cumpmts);

    void   SetMonthlyDetail(int enumerator, std::string const& s);
    void   SetMonthlyDetail(int enumerator, double d);
    void   DebugPrintInit();
    void   DebugEndBasis();

    void   EndTermRider();

    void   CoordinateCounters();

    // First-year premium determines whether SD waives premium tax
    // retaliation, although SD isn't hardcoded anywhere and this
    // is a generic rule expressed in data files. This function tells
    // us up front whether the minimum premium for tiering may be met.
    // Used with 'FirstYearPremiumExceedsRetaliationLimit'. This
    // is a naive lookahead: GPT limitations or MEC avoidance may
    // reduce the premium below the threshold, so it's useful only as
    // a hint. Premium exempt from premium tax (e.g. internal 1035
    // exchanges when the database makes them so exempt) is excluded.
    double TaxableFirstYearPlannedPremium() const;

    // Monthly calculation detail.
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

    boost::shared_ptr<Ledger         > ledger_;
    boost::shared_ptr<LedgerInvariant> ledger_invariant_;
    boost::shared_ptr<LedgerVariant  > ledger_variant_;

    e_increment_method             deduction_method;
    e_increment_account_preference deduction_preferred_account;
    e_increment_method             distribution_method;
    e_increment_account_preference distribution_preferred_account;
    e_allocation_method            ee_premium_allocation_method;
    e_increment_account_preference ee_premium_preferred_account;
    e_allocation_method            er_premium_allocation_method;
    e_increment_account_preference er_premium_preferred_account;

    double GuarPremium;

    // If solving for endowment, we use endowment duration instead of
    // input target duration. But we don't want to change the input
    // value--we just want to override it. If input duration was other
    // than endowment, then we want that value still to be there if
    // the goal is later changed to use input duration.
    int    EffectiveSolveTargetYear;
    double SolveTargetValue;

    int              SolveBegYear;
    int              SolveEndYear;
    e_basis          SolveBasis;
    e_sep_acct_basis SolveSABasis;

    e_run_basis      RateBasis;
    e_basis          ExpAndGABasis;
    e_sep_acct_basis SABasis;

    int         LapseMonth; // Antediluvian.
    int         LapseYear;  // Antediluvian.

    double External1035Amount;
    double Internal1035Amount;
    double Dumpin;

    double MlyNoLapsePrem;
    double CumNoLapsePrem;
    bool   NoLapseActive;
    // Solve routines need to know period when no lapse was active; we
    // represent it as int rather than bool so we can use accumulate().
    std::vector<int> YearlyNoLapseActive;

    double RiderDeductions;

    // These two things can be different: IRC section 72
    double CumPmts;
    double TaxBasis;

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
    // TODO ?? Fold comments into names?
    double  GenAcctAlloc;   // pmt allocation to gen acct
    double  SepAcctAlloc;   // pmt allocation to sep acct
    double  NAAR;
    double  CoiCharge;
    double  NetCoiCharge;
    double  SpecAmtLoadBase;
    // TODO ?? Separating different types of sepacct load is probably unneeded.
    double  AVSepAcctLoadBaseBOM; // TODO ?? eradicate
    double  AVSepAcctLoadBaseAMD;
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
    e_mode      mode;      // Antediluvian.
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
    double  PolicyYearRunningTotalPremiumSubjectToPremiumTax;
    double  MaxWD;
    double  GrossWD;
    double  NetWD;
    double  CumWD;

    double      wd;           // Antediluvian.
    double      mlyguarv;     // Antediluvian.

    // For GPT: SA, DB, and DBOpt before the day's transactions are applied.
    double  OldSA;
    double  OldDB;
    e_dbopt OldDBOpt;

    // Permanent invariants are in class BasicValues; these are
    // annual invariants.
    double  YearsCorridorFactor;
    e_dbopt YearsDBOpt;
    double  YearsSpecAmt;
    double  YearsAnnPolFee;
    double  YearsMlyPolFee;
    double  YearsGenAcctIntRate;
    double  YearsSepAcctIntRate;

    // Separate components of separate account interest rate: all
    // except 'YearsSepAcctGrossRate' are annual.
    double  YearsSepAcctGrossRate;
    double  YearsSepAcctMandERate;
    double  YearsSepAcctIMFRate;
    double  YearsSepAcctABCRate;
    double  YearsSepAcctSVRate;

    double  YearsDcvIntRate;

    double  YearsHoneymoonValueRate;
    double  YearsPostHoneymoonGenAcctIntRate;

    double  YearsRegLnIntCredRate;
    double  YearsPrfLnIntCredRate;
    double  YearsRegLnIntDueRate;
    double  YearsPrfLnIntDueRate;
    double  YearsSurrChgPremMult;
    double  YearsSurrChgAVMult;
    double  YearsSurrChgSAMult;
    double  YearsCoiRate0;
    double  YearsCoiRate1;
    double  YearsCoiRate2;
    double  Years7702CoiRate;
    double  YearsAdbRate;
    double  YearsTermRate;
    double  YearsWpRate;
    double  YearsSpouseRiderRate;
    double  YearsChildRiderRate;
    double  YearsPremLoadTgt;
    double  YearsPremLoadExc;
    double  YearsTotLoadTgt;
    double  YearsTotLoadExc;
    double  YearsTotLoadTgtLowestPremtax;
    double  YearsTotLoadExcLowestPremtax;
    double  YearsSalesLoadTgt;
    double  YearsSalesLoadExc;
    double  YearsSpecAmtLoad;
    double  YearsAcctValLoadBOM; // TODO ?? eradicate
    double  YearsAcctValLoadAMD;
    double  YearsSalesLoadRefundRate;
    double  YearsPremTaxLoadRate;
    double  YearsDacTaxLoadRate;

    double  case_k_factor;
    double  ActualCoiRate;

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

    // The spec amt used as the basis for surrender charges is not
    // always the current spec amt, but rather the original spec amt
    // adjusted for withdrawals only.
    double  SurrChgSpecAmt;

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
    double  YearsAVRelOnDeath;
    double  YearsGrossClaims;
    double  YearsNetClaims;
    double  YearsTotalNetIntCredited;
    double  YearsTotalGrossIntCredited;
    double  YearsTotalExpRsvInt;
    double  YearsTotalAnnPolFee;
    double  YearsTotalMlyPolFee;
    double  YearsTotalPremTaxLoad;
    double  YearsTotalPremTaxLoadInStateOfDomicile;
    double  YearsTotalPremTaxLoadInStateOfJurisdiction;
    double  YearsTotalDacTaxLoad;
    double  YearsTotalSpecAmtLoad;
    double  YearsTotalAcctValLoadBOM; // TODO ?? eradicate
    double  YearsTotalAcctValLoadAMD;

    std::vector<double> partial_mortality_q;

    // For experience rating.
    double  apportioned_net_mortality_reserve;
    double  CoiRetentionRate;
    double  ExperienceRatingAmortizationYears;
    double  IbnrAsMonthsOfMortalityCharges;
    double  NextYearsProjectedCoiCharge;
    double  YearsTotalNetCoiCharges;

    double  CumulativeSalesLoad;
    bool    FirstYearPremiumExceedsRetaliationLimit;

    // Illustrated outlay must be the same for current, guaranteed,
    // and all other bases. Outlay components are set on whichever
    // basis governs, usually current, then stored for use with all
    // other bases.

    std::vector<double> OverridingPmts; // Antediluvian.

    std::vector<double> OverridingEePmts;
    std::vector<double> OverridingErPmts;

    // We need no 'OverridingDumpin' because we simply treat dumpin as
    // employee premium.
    double OverridingExternal1035Amount;
    double OverridingInternal1035Amount;

    std::vector<double> OverridingLoan;
    std::vector<double> OverridingWD;

    std::vector<double> SurrChg_;
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
inline boost::shared_ptr<Ledger const> AccountValue::ledger_from_av() const
{
    return ledger_;
}

//============================================================================
inline int AccountValue::GetLength() const
{
    return BasicValues::GetLength();
}

//============================================================================
inline e_ledger_type const& AccountValue::GetLedgerType() const
{
    return BasicValues::GetLedgerType();
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

