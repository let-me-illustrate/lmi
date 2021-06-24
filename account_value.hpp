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
#include "currency.hpp"
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
    friend currency SolveTest(); // Antediluvian.

  public:
    enum {months_per_year = 12};

    explicit AccountValue(Input const& input);
    AccountValue(AccountValue&&) = default;
    ~AccountValue() override = default;

    void RunAV();

    void SetDebugFilename    (std::string const&);

    void SolveSetPmts // Antediluvian.
        (currency a_Pmt
        ,int      ThatSolveBegYear
        ,int      ThatSolveEndYear
        );
    void SolveSetSpecAmt // Antediluvian.
        (currency a_Bft
        ,int      ThatSolveBegYear
        ,int      ThatSolveEndYear
        );
    void SolveSetLoans // Antediluvian.
        (currency a_Loan
        ,int      ThatSolveBegYear
        ,int      ThatSolveEndYear
        );
    void SolveSetWDs // Antediluvian.
        (currency a_WD
        ,int      ThatSolveBegYear
        ,int      ThatSolveEndYear
        );
    void SolveSetLoanThenWD // Antediluvian.
        (currency a_Amt
        ,int      ThatSolveBegYear
        ,int      ThatSolveEndYear
        );

    std::shared_ptr<Ledger const> ledger_from_av() const;

  private:
    AccountValue(AccountValue const&) = delete;
    AccountValue& operator=(AccountValue const&) = delete;

    LedgerInvariant const& InvariantValues() const;
    LedgerVariant   const& VariantValues  () const;

    int                    GetLength     () const;

    double   InforceLivesBoy        () const;
    double   InforceLivesEoy        () const;
    currency GetSepAcctAssetsInforce() const;

    void process_payment          (currency);
    void IncrementAVProportionally(currency);
    void IncrementAVPreferentially(currency, oenum_increment_account_preference);
    void process_deduction        (currency);
    void process_distribution     (currency);
    void DecrementAVProportionally(currency);
    void DecrementAVProgressively (currency, oenum_increment_account_preference);

    currency TotalAccountValue() const;
    currency CashValueFor7702() const;

    currency base_specamt(int year) const;
    currency term_specamt(int year) const;
    currency specamt_for_7702(int year) const;
    currency specamt_for_7702A(int year) const;

    void assert_pmts_add_up(char const* file, int line, int month);

    // We're not yet entirely sure how to handle ledger values. Right now,
    // we have pointers to a Ledger and also to its variant and invariant
    // parts. We put data into the parts, and then insert the parts into
    // the Ledger. It might seem better to work not through these distinct
    // "parts" but rather through references to components of the Ledger;
    // however, the present design permits a solve for NAIC illustration
    // reg guaranteed premium that (as desired) has no side effects--the
    // ledger "parts" it affects are simply thrown away.
    //
    // While we gather more information and consider this, all access comes
    // through the following functions.
    LedgerInvariant& InvariantValues();
    LedgerVariant  & VariantValues  ();

    void   RunOneCell              (mcenum_run_basis);
    void   RunOneBasis             (mcenum_run_basis);
    void   RunAllApplicableBases   ();
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

    void   SolveSetSpecAmt      (currency a_CandidateValue);
    void   SolveSetEePrem       (currency a_CandidateValue);
    void   SolveSetErPrem       (currency a_CandidateValue);
    void   SolveSetLoan         (currency a_CandidateValue);
    void   SolveSetWD           (currency a_CandidateValue);

    void   DebugPrint           ();

    void   SetClaims();

    // To support the notion of an M&E charge that depends on total case
    // assets, we provide these functions, which are designed to be
    // called by a distant module that has a pointer to an object of this
    // class. Processing must be split into two functions here so that
    // total assets for all lives combined can be ascertained just prior
    // to the point where interest is credited.

    // Process monthly transactions up to but excluding interest credit
    currency IncrementBOM
        (int    year
        ,int    month
        );
    // Credit interest and process all subsequent monthly transactions
    void IncrementEOM
        (int      year
        ,int      month
        ,currency assets_post_bom
        ,currency cum_pmts_post_bom
        );

    void IncrementEOY(int year);

    bool PrecedesInforceDuration(int year, int month);

    currency Solve(); // Antediluvian.
    currency Solve
        (mcenum_solve_type   a_SolveType
        ,int                 a_SolveBeginYear
        ,int                 a_SolveEndYear
        ,mcenum_solve_target a_SolveTarget
        ,currency            a_SolveTargetCsv
        ,int                 a_SolveTargetYear
        ,mcenum_gen_basis    a_SolveGenBasis
        ,mcenum_sep_basis    a_SolveSepBasis
        );

    currency SolveTest
        (currency a_CandidateValue
        ,void (AccountValue::*solve_set_fn)(currency)
        );

    currency SolveGuarPremium        ();

    void PerformSpecAmtStrategy();
    void PerformSupplAmtStrategy();
    currency CalculateSpecAmtFromStrategy
        (int                actual_year
        ,int                reference_year
        ,currency           explicit_value
        ,mcenum_sa_strategy strategy
        ) const;

    void PerformPmtStrategy(currency* a_Pmt); // Antediluvian.
    currency PerformEePmtStrategy       () const;
    currency PerformErPmtStrategy       () const;
    currency DoPerformPmtStrategy
        (mcenum_solve_type                       a_SolveForWhichPrem
        ,mcenum_mode                             a_CurrentMode
        ,mcenum_mode                             a_InitialMode
        ,double                                  a_TblMult
        ,std::vector<currency> const&            a_PmtVector
        ,std::vector<mcenum_pmt_strategy> const& a_StrategyVector
        ) const;

    void InitializeMonth            ();
    void TxExch1035                 ();
    void TxOptionChange             ();
    void TxSpecAmtChange            ();
    void TxTestGPT                  ();
    void TxPmt(); // Antediluvian.
    void TxAscertainDesiredPayment  ();
    void TxLimitPayment             (double a_maxpmt); // CURRENCY !! not currency?
    void TxRecognizePaymentFor7702A
        (currency a_pmt
        ,bool     a_this_payment_is_unnecessary
        );
    void TxAcceptPayment            (currency payment);
    currency GetPremLoad
        (currency a_pmt
        ,currency a_portion_exempt_from_premium_tax
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
    currency InterestCredited
        (currency principal
        ,double   monthly_rate
        ) const;

    bool     IsModalPmtDate          (mcenum_mode) const;
    bool     IsModalPmtDate          (); // Antediluvian.
    int      MonthsToNextModalPmtDate() const;
    currency anticipated_deduction   (mcenum_anticipated_deduction);

    currency minimum_specified_amount(bool issuing_now, bool term_rider) const;
    void     ChangeSpecAmtBy         (currency delta);
    void     ChangeSupplAmtBy        (currency delta);

    currency SurrChg                 () const;
    currency CSVBoost                () const;

    void     set_list_bill_year_and_month();
    void     set_list_bill_premium();
    void     set_modal_min_premium();

    void     SetMaxLoan              ();
    void     SetMaxWD                ();
    currency GetRefundableSalesLoad() const;

    void   ApplyDynamicMandE       (currency assets);

    void   SetMonthlyDetail(int enumerator, std::string const&);
    void   SetMonthlyDetail(int enumerator, double);
    void   SetMonthlyDetail(int enumerator, currency);
    void   DebugPrintInit();
    void   DebugEndBasis();

    void   EndTermRider(bool convert);

    void   CoordinateCounters();

    // Detailed monthly trace.
    std::string     DebugFilename;
    std::ofstream   DebugStream;
    std::vector<std::string> DebugRecord;

    currency        PriorAVGenAcct;
    currency        PriorAVSepAcct;
    currency        PriorAVRegLn;
    currency        PriorAVPrfLn;
    currency        PriorRegLnBal;
    currency        PriorPrfLnBal;

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

    currency GuarPremium;

    // These data members make Solve() arguments available to SolveTest().
    int                 SolveBeginYear_;
    int                 SolveEndYear_;
    mcenum_solve_target SolveTarget_;
    currency            SolveTargetCsv_;
    int                 SolveTargetDuration_;
    mcenum_gen_basis    SolveGenBasis_;
    mcenum_sep_basis    SolveSepBasis_;

    mcenum_run_basis RunBasis_;
    mcenum_gen_basis GenBasis_;
    mcenum_sep_basis SepBasis_;

    int      LapseMonth; // Antediluvian.
    int      LapseYear;  // Antediluvian.

    currency External1035Amount;
    currency Internal1035Amount;
    currency Dumpin;

    currency MlyNoLapsePrem;
    currency CumNoLapsePrem;
    bool     NoLapseActive;

    // Solves need to know when a no-lapse guarantee is active.
    // Prefer int here because vector<bool> is not a container.
    std::vector<int> YearlyNoLapseActive;

    // Ullage is any positive excess of amount requested over amount available.
    std::vector<currency> loan_ullage_;
    std::vector<currency> withdrawal_ullage_;

    currency CumPmts;
    currency TaxBasis;
    // This supports solves for tax basis. Eventually it should be
    // moved into the invariant-ledger class.
    std::vector<currency> YearlyTaxBasis;

    // Ee- and Er-GrossPmts aren't used directly in the AV calculations.
    // They must be kept separate for ledger output, and also for
    // tax basis calculations (when we fix that).
    std::vector<currency> GrossPmts;
    std::vector<currency> EeGrossPmts;
    std::vector<currency> ErGrossPmts;
    std::vector<currency> NetPmts;

    // Reproposal input.
    int      InforceYear;
    int      InforceMonth;
    currency InforceAVGenAcct;
    currency InforceAVSepAcct;
    currency InforceAVRegLn;
    currency InforceAVPrfLn;
    currency InforceRegLnBal;
    currency InforcePrfLnBal;
    currency InforceCumNoLapsePrem;
    currency InforceCumPmts;
    currency InforceTaxBasis;

    // Intermediate values.
    int      Year;
    int      Month;
    int      MonthsSinceIssue;
    bool     daily_interest_accounting;
    int      days_in_policy_month;
    int      days_in_policy_year;
    currency AVGenAcct;
    currency AVSepAcct;
    currency SepAcctValueAfterDeduction;
    double   GenAcctPaymentAllocation;
    double   SepAcctPaymentAllocation;
    currency NAAR;
    currency CoiCharge;
    currency RiderCharges;
    currency SpecAmtLoadBase;
    double   DacTaxRsv; // CURRENCY !! obsolete--always zero

    currency AVUnloaned; // Antediluvian.

    currency NetMaxNecessaryPremium;
    currency GrossMaxNecessaryPremium;
    currency NecessaryPremium;
    currency UnnecessaryPremium;

    // 7702A CVAT deemed cash value.
    currency Dcv;
    currency DcvDeathBft;
    currency DcvNaar;
    currency DcvCoiCharge;
    currency DcvTermCharge;
    currency DcvWpCharge;
    // For other riders like AD&D, charge for DCV = charge otherwise.

    // Honeymoon provision.
    bool     HoneymoonActive;
    currency HoneymoonValue;

    // 7702 GPT
    currency gpt_chg_sa_base_; // Cf. SpecAmtLoadBase
    currency GptForceout;
    currency YearsTotalGptForceout;

    // Intermediate values within annual or monthly loop only.
    currency    pmt;       // Antediluvian.
    mcenum_mode pmt_mode;  // Antediluvian.
    int         ModeIndex; // Antediluvian.

    currency GenAcctIntCred;
    currency SepAcctIntCred;
    currency RegLnIntCred;
    currency PrfLnIntCred;
    currency AVRegLn;
    currency AVPrfLn;
    currency RegLnBal;
    currency PrfLnBal;
    currency MaxLoan;
    currency UnusedTargetPrem;
    currency AnnualTargetPrem;
    currency MaxWD;
    currency GrossWD;
    currency NetWD;
    currency CumWD;

    currency wd;           // Antediluvian.
    double   mlyguarv;     // Antediluvian.

    // For GPT: SA, DB, and DBOpt before the day's transactions are applied.
    currency     OldSA;
    currency     OldDB;
    mcenum_dbopt OldDBOpt;

    // Permanent invariants are in class BasicValues; these are
    // annual invariants.
    double       YearsCorridorFactor;
    mcenum_dbopt YearsDBOpt;
    currency     YearsAnnualPolicyFee;
    currency     YearsMonthlyPolicyFee;
    double       YearsGenAcctIntRate;
    double       YearsSepAcctIntRate; // Rename: "Net"
    double       YearsSepAcctGrossRate;

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

    currency MonthsPolicyFees;
    currency SpecAmtLoad;
    // Premium load is (rounded) currency; its components are not.
    double   premium_load_;
    double   sales_load_;
    double   premium_tax_load_;
    double   dac_tax_load_;

    // Stratified loads are determined by assets and cumulative
    // payments immediately after the monthly deduction. Both are
    // stored at the proper moment, where they're constrained to be
    // nonnegative. Stratified loads happen to be used only for the
    // separate account.
    currency AssetsPostBom;
    currency CumPmtsPostBom;
    currency SepAcctLoad;

    double   ActualCoiRate;

    bool     SplitMinPrem;
    bool     UnsplitSplitMinPrem;

    int      list_bill_year_  {methuselah};
    int      list_bill_month_ {13};

    bool     TermCanLapse;
    bool     TermRiderActive;
    currency ActualSpecAmt;
    currency TermSpecAmt;
    currency TermDB;
    currency DB7702A;
    currency DBIgnoringCorr;
    currency DBReflectingCorr;

    currency deathbft; // Antediluvian.
    bool     haswp;    // Antediluvian.
    bool     hasadb;   // Antediluvian.

    currency ActualLoan;
    currency RequestedLoan;
    currency RequestedWD;

    currency AdbCharge;
    currency SpouseRiderCharge;
    currency ChildRiderCharge;
    currency WpCharge;
    currency TermCharge;

    currency MlyDed;
    currency mlydedtonextmodalpmtdate; // Antediluvian.

    currency YearsTotalCoiCharge;
    currency YearsTotalRiderCharges;
    double   YearsAVRelOnDeath;
    double   YearsLoanRepaidOnDeath;
    double   YearsGrossClaims;
    double   YearsDeathProceeds;
    double   YearsNetClaims;
    currency YearsTotalNetIntCredited;
    currency YearsTotalGrossIntCredited;
    currency YearsTotalLoanIntAccrued;
    currency YearsTotalPolicyFee;
    double   YearsTotalDacTaxLoad; // cumulative (unrounded) 'dac_tax_load_'
    currency YearsTotalSpecAmtLoad;
    currency YearsTotalSepAcctLoad;

    currency CumulativeSalesLoad;

    // Illustrated outlay must be the same for current, guaranteed,
    // and all other bases. Outlay components are set on whichever
    // basis governs, usually current, then stored for use with all
    // other bases.

    std::vector<currency> OverridingPmts; // Antediluvian.
    std::vector<currency> stored_pmts;    // Antediluvian.

    std::vector<currency> OverridingEePmts;
    std::vector<currency> OverridingErPmts;

    // We need no 'OverridingDumpin' because we simply treat dumpin as
    // employee premium.
    currency OverridingExternal1035Amount;
    currency OverridingInternal1035Amount;

    std::vector<currency> OverridingLoan;
    std::vector<currency> OverridingWD;

    std::vector<currency> SurrChg_; // Of uncertain utility.
};

//============================================================================
inline currency AccountValue::TotalAccountValue() const
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

#endif // account_value_hpp
