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

// $Id: account_value.hpp,v 1.1 2005-02-05 03:02:41 chicares Exp $

#ifndef account_value_hpp
#define account_value_hpp

#include "config.hpp"

#include "basic_values.hpp"
#include "expimp.hpp"

#include <boost/scoped_ptr.hpp>

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
class TLedger;
class TLedgerInvariant;
class TLedgerVariant;
class DebugView;

class LMI_EXPIMP AccountValue
    :protected BasicValues
{
    friend class SolveHelper;
    friend class CensusView;

  public:
    enum {months_per_year = 12};

    explicit AccountValue(InputParms const& input);
    ~AccountValue();

    double RunAV                ();
    double PerformRun           (e_run_basis const&);
    void   InitializeLife       (e_run_basis const&);
    void   FinalizeLife         (e_run_basis const&);
    void   FinalizeLifeAllBases ();
    void   SetGuarPrem          ();

    void SetDebugFilename    (std::string const&);
    void SetProject12Filename(std::string const&);

    TLedger const&       GetLedgerValues () const;
    int                  GetLength       () const;
    e_ledger_type const& GetLedgerType   () const;

    double GetInforceLives         () const;

    void   GuessWhetherFirstYearPremiumExceedsRetaliationLimit();
    bool   TestWhetherFirstYearPremiumExceededRetaliationLimit();

    double GetSepAcctAssetsInforce () const;
    double GetNetCOI               () const;
    double GetLastCOIChargeInforce () const;
    double GetIBNRContrib          () const;
    double GetExpRatReserve        () const;
    double GetExpRatReserveNonforborne() const;

    // Antediluvian.
    double Solve();
    void SolveSetPmts
        (double a_Pmt
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetSpecAmt
        (double a_Bft
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetLoans
        (double a_Loan
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetWDs
        (double a_WD
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetLoanThenWD
        (double a_Amt
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );

    TLedger const& WorkingValues();
    TLedger const& CurrValues();
    TLedger const& MdptValues();
    TLedger const& GuarValues();

  private:
    AccountValue(AccountValue const&);
    AccountValue& operator=(AccountValue const&);

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
    // we have pointers to a TLedger and also to its variant and invariant
    // parts. We put data into the parts, and then insert the parts into
    // the TLedger. At this moment it seems best to work not through these
    // "parts" but rather through references to components of the TLedger.
    // While we gather more information and consider this, all access comes
    // through the following functions.
    TLedgerInvariant&       InvariantValues();
    TLedgerInvariant const& InvariantValues() const;
    TLedgerVariant&         VariantValues  ();
    TLedgerVariant const&   VariantValues  () const;

    double PerformRunMonthByMonth  (e_run_basis const&);
    double PerformRunLifeByLife    (e_run_basis const&);
    double RunOneBasis             (e_run_basis const&);
    double RunAllApplicableBases   ();
    void   DoYear                  (int InforceMonth = 0);
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

    double UpdateExpRatReserveBOM(double CaseExpRatMlyIntRate);
    void UpdateExpRatReserveEOM
        (double CaseYearsCOICharges
        ,double CaseMonthsClaims
        );
    double UpdateExpRatReserveForPersistency
        (double a_PersistencyAdjustment
        );
    // For experience rating, record claims paid and AV released on
    // death before adjusting the experience fund for deaths.
    void   SetClaims();
    double GetCurtateNetClaimsInforce();
    double GetStabResContrib();
    void SetExpRatRfd
        (double CaseYearsCOICharges
        ,double CaseExpRfd
        );

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
        ,double CaseExpRatReserve
        );
    // Credit interest and process all subsequent monthly transactions
    double IncrementEOM
        (int year
        ,int month
        ,double MandE
        );
    // Increment year, update curtate inforce factor
    double IncrementEOY
        (int year
        );

    bool PrecedesInforceDuration(int year, int month);

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

    // Not inline--goes through class MortalityRates, and we don't
    // want to put that class's full declaration in the header.
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
    void TxOptChg                   ();
    void TxSpecAmtChg               ();
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
    void TxSetDeathBft           ();
    void TxSetCOI                ();
    void TxSetRiderDed           ();
    void TxDoMlyDed              ();

    void TxTakeSepAcctLoad       ();
    void TxCreditInt             ();
    void TxLoanInt               ();
    void TxTakeWD                ();
    void TxTakeLoan              ();
    void TxCapitalizeLoan        ();

    void TxDebitExpRatRsvChg     ();

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
    double DetermineAcctValLoadBOM ();
    double DetermineAcctValLoadAMD ();
    void   RecalculateGDBPrem      ();

    void   ApplyDynamicMandE          (double assets);
    void   ApplyDynamicSepAcctLoadAMD (double assets);

    void   SetMonthlyDetail(int enumerator, std::string const& s);
    void   SetMonthlyDetail(int enumerator, double d);
    void   DebugPrintInit();
    void   DebugEndBasis();

    void   Project12MosPrintInit();
    void   Project12MosPrintMonth();
    void   Project12MosPrintEnd();

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

    // 12-month projection (akin to annual report, but prospective).
    std::string     Project12MosFilename;
    std::ofstream   Project12MosStream;

    double          PriorAVGenAcct;
    double          PriorAVSepAcct;
    double          PriorAVRegLn;
    double          PriorAVPrfLn;
    double          PriorRegLnBal;
    double          PriorPrfLnBal;

    // mode flags
    bool            Debugging;
    bool            Projecting12Mos;
    bool            Solving;
    bool            SolvingForGuarPremium;
    bool            ItLapsed;

    boost::scoped_ptr<TLedger         > LedgerValues;
    boost::scoped_ptr<TLedgerInvariant> LedgerInvariant;
    boost::scoped_ptr<TLedgerVariant  > LedgerVariant;

    boost::scoped_ptr<TLedger> WorkingValues_; // Antediluvian.
    boost::scoped_ptr<TLedger> CurrValues_;    // Antediluvian.
    boost::scoped_ptr<TLedger> MdptValues_;    // Antediluvian.
    boost::scoped_ptr<TLedger> GuarValues_;    // Antediluvian.

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
    std::vector<double> Corridor; // Antediluvian.
    std::vector<double> WPRates;  // Antediluvian.
    std::vector<double> ADDRates; // Antediluvian.

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
    double  InforceLives;
    double  InforceFactor;

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
    double  COI;    // TODO ?? Call it COIChg instead?
    double  SpecAmtLoadBase;
    // TODO ?? Separating different types of sepacct load is probably unneeded.
    double  AVSepAcctLoadBaseBOM;
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
    double  DcvCoi;
    double  DcvTermChg;
    double  DcvWpChg;
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
    double  YearsCOIRate0;
    double  YearsCOIRate1;
    double  YearsCOIRate2;
    double  Years7702CoiRate;
    double  YearsADDRate;
    double  YearsTermRate;
    double  YearsWPRate;
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
    double  YearsAcctValLoadBOM;
    double  YearsAcctValLoadAMD;
    double  YearsSalesLoadRefundRate;
    double  YearsPremTaxLoadRate;
    double  YearsDacTaxLoadRate;

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
    bool        hasadd;   // Antediluvian.

    // The spec amt used as the basis for surrender charges is not
    // always the current spec amt, but rather the original spec amt
    // adjusted for withdrawals only.
    double  SurrChgSpecAmt;

    double  ActualLoan;
    double  RequestedLoan;
    double  RequestedWD;

    double  ADDChg;
    double  SpouseRiderChg;
    double  ChildRiderChg;
    double  WPChg;
    double  TermChg;

    double  MlyDed;
    double  mlydedtonextmodalpmtdate; // Antediluvian.

    double  YearsTotalCOICharge;
    double  YearsAVRelOnDeath;
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
    double  YearsTotalAcctValLoadBOM;
    double  YearsTotalAcctValLoadAMD;

    // for experience rating
    double  YearsTotalNetCOIs;
    double  ExpRatReserve;
    double  ExpRatStabReserve;
    double  ExpRatIBNRReserve;
    double  ExpRatRfd;

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
inline TLedgerVariant& AccountValue::VariantValues()    // temporary artifact
{
    return *LedgerVariant;
}

//============================================================================
inline TLedgerVariant const& AccountValue::VariantValues() const
{
    return *LedgerVariant;
}

//============================================================================
inline TLedgerInvariant& AccountValue::InvariantValues()    // temporary artifact
{
    return *LedgerInvariant;
}

//============================================================================
inline TLedgerInvariant const& AccountValue::InvariantValues() const
{
    return *LedgerInvariant;
}

//============================================================================
inline TLedger const& AccountValue::GetLedgerValues() const
{
    return *LedgerValues;
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
inline double AccountValue::GetInforceLives() const
{
    return InforceLives;
}

#endif // account_value_hpp

