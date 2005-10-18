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

// $Id: ihs_acctval.cpp,v 1.77 2005-10-18 00:10:17 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "copy_n_.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_rnddata.hpp"
#include "ihs_proddata.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "interest_rates.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "loads.hpp"
#include "materially_equal.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "stratified_algorithms.hpp"
#include "stratified_charges.hpp"
#include "surrchg_rates.hpp"

#include <boost/bind.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <limits>
#include <string>
#include <utility>

/*
We ideally want transaction functions to be reorderable.
That means each must be atomic and reentrant, with no dependencies.
To what extent is this feasible?

different sorts of variables:

constant input: never changes (e.g. COI rates)

changed by solves: e.g. withdrawals

changed by rules: e.g. withdrawals limited by minimum

changed and later reused: e.g. specamt (set in curr pass, used in guar pass)

These are not mutually exclusive. A withdrawal can change the specified
amount, but it must not wipe out the input specamt array (which can signal
reductions and increases).

Some rules change variables prospectively. For example, a withdrawal reduces
all future spec amts.

It might be helpful to prepare a table of transaction type cross variables
showing {accesses, modifies current year, modifies future years}

*/

//============================================================================
AccountValue::AccountValue(InputParms const& input)
    :BasicValues           (&input)
    ,DebugFilename         ("anonymous.debug")
    ,Debugging             (false)
    ,Solving               (e_solve_none != Input_->SolveType)
    ,SolvingForGuarPremium (false)
    ,ItLapsed              (false)
    ,ledger_(new Ledger(BasicValues::GetLedgerType(), BasicValues::GetLength()))
    ,ledger_invariant_     (new LedgerInvariant(BasicValues::GetLength()))
    ,ledger_variant_       (new LedgerVariant  (BasicValues::GetLength()))
    ,FirstYearPremiumExceedsRetaliationLimit(true)
{
    InvariantValues().Init(this);
// TODO ?? What are the values of the last two arguments here?
    VariantValues().Init(this, ExpAndGABasis, SABasis);
    // TODO ?? There are several variants. We have to initialize all of them.
    // This is probably best done through a function in class Ledger.
    // We haven't yet laid the groundwork for that, though.
    // If BasicValues changes, then this init becomes invalid
    //   e.g. solves change BasicValues

    // Iff partial mortality is used, save yearly values in a vector
    // for use elsewhere in this class, and store yearly inforce lives
    // (assuming no one ever lapses) in the invariant ledger object.
    //
    // A contract may be in force at the end of its maturity year,
    // and it's necessary to treat it that way because other year-end
    // composite values are multiplied by the number of lives inforce.
    // Of course, a contract is not normally in force after maturity.

    LMI_ASSERT
        (   InvariantValues().InforceLives.size()
        ==  static_cast<unsigned int>(1 + BasicValues::GetLength())
        );
    partial_mortality_q.resize(BasicValues::GetLength());
    // TODO ?? 'InvariantValues().InforceLives' may be thought of as
    // counting potential inforce lives: it does not reflect lapses.
    // It should either reflect lapses or be renamed. Meanwhile,
    // InforceLivesBoy() and InforceLivesEoy() may be used where
    // lapses should be taken into account.
    if(Input_->UsePartialMort)
        {
        double inforce_lives = Input_->NumIdenticalLives;
        InvariantValues().InforceLives[0] = inforce_lives;
        for(int j = 0; j < BasicValues::GetLength(); ++j)
            {
            partial_mortality_q[j] = GetPartMortQ(j);
            inforce_lives *= 1.0 - partial_mortality_q[j];
            InvariantValues().InforceLives[1 + j] = inforce_lives;
            }
        }
    else
        {
        InvariantValues().InforceLives.assign
            (InvariantValues().InforceLives.size()
            ,Input_->NumIdenticalLives
            );
        }

    OverridingEePmts    .resize(12 * BasicValues::GetLength());
    OverridingErPmts    .resize(12 * BasicValues::GetLength());

    OverridingLoan      .resize(BasicValues::GetLength());
    OverridingWD        .resize(BasicValues::GetLength());

    SurrChg_            .resize(BasicValues::GetLength());

    YearlyNoLapseActive .reserve(BasicValues::GetLength());
}

//============================================================================
AccountValue::~AccountValue()
{
}

//============================================================================
double AccountValue::RunAV()
{
/*
First run current, for solves and strategies. This determines
    payments
    specamt (increases, decreases, option changes)
    and hence surrchg
Then run other bases.

    guar and midpt values are useful only for illustrations
    for profit testing we want to avoid their overhead
    for solves we want only one run
        if solving on guar basis...does *that* basis determine pmts & specamt?
        it prolly should, so that the guar columns will show what's wanted
            otherwise the solve lacks meaning
        although I wonder how other illustration systems handle this
        it should also be possible to solve on a midpt basis as well
*/

    if(std::string::npos != Input_->Comments.find("idiosyncrasyZ"))
        {
        Debugging = true;
        DebugPrintInit();
        }

    double z = RunAllApplicableBases();

    FinalizeLifeAllBases();
    if(std::string::npos != Input_->Comments.find("idiosyncrasy_spew"))
        {
        std::ofstream os
            ("raw_output"
            ,std::ios_base::out | std::ios_base::trunc
            );
        ledger_->Spew(os);
        }

    return z;
}

//============================================================================
// TODO ?? Perhaps commutation functions could be used to speed up
// this rather expensive function.
void AccountValue::SetGuarPrem()
{
    GuarPremium = 0.0;
    if(BasicValues::IsSubjectToIllustrationReg())
        {
        GuarPremium = SolveGuarPremium();
        }
    LMI_ASSERT(GuarPremium < 1.0e100);
    ledger_->SetGuarPremium(GuarPremium);
}

//============================================================================
double AccountValue::RunOneBasis(e_run_basis const& a_Basis)
{
    if
        (  !BasicValues::IsSubjectToIllustrationReg()
        && e_mdptbasis == a_Basis
        )
        {
        fatal_error()
            << "Midpoint basis defined only for illustration-reg ledger."
            << LMI_FLUSH
            ;
        }

    double z = 0.0;
    if(Solving)
        {
// Apparently this should never be done because Solve() is called in
//   RunAllApplicableBases() .
// TODO ?? Do something more flexible.
//      LMI_ASSERT(a_Basis == Input_->SolveBasis);
//      z = Solve();
        }
    else
        {
        z = RunOneCell(a_Basis);
        }
    return z;
}

//============================================================================
// If not solving
//   if running one basis
//     just do that basis
//   if running all bases
//     run all bases
//
double AccountValue::RunAllApplicableBases()
{
    // LedgerType determines the set of bases required (enum e_run_basis).

    double z = 0.0;

    // TODO ?? Normally, running on the current basis determines the
    // overriding values for all components of outlay--premiums,
    // loans, and withdrawals. For a solve on any basis other than
    // current, the overriding values could be determined two ways:
    //
    // (1) on the current basis--but then the solve won't be right;
    //
    // (2) on the solve basis: but if it lapses on that basis, then
    // overriding values were not determined for later durations,
    // yet such values may be needed e.g. for the current basis.

    if(Solving)
        {
        // TODO ?? It seems odd to call a function of class InputParms
        // here--why isn't that done only during input?
        Input_->SetSolveDurations();

        if(e_run_curr_basis != SolveBasis)
            {
            RunOneBasis(e_run_basis(e_run_curr_basis));
            }

        z = Solve
            (Input_->SolveType
            ,Input_->SolveBegYear
            ,Input_->SolveEndYear
            ,Input_->SolveTarget
            ,Input_->SolveTgtCSV
            ,Input_->SolveTgtYear
            ,Input_->SolveBasis
            ,Input_->SolveSABasis
            );
        Solving = false;
        // TODO ?? Here we might save overriding parameters determined
        // on the solve basis.

        e_run_basis solve_run_basis;
        set_run_basis_from_separate_bases
            (solve_run_basis
            ,SolveBasis
            ,SolveSABasis
            );
        }
    // Run all bases, current first.
    std::vector<e_run_basis> const& run_bases = ledger_->GetRunBases();
    for
        (std::vector<e_run_basis>::const_iterator b = run_bases.begin()
        ;b != run_bases.end()
        ;++b
        )
        {
        RunOneBasis(*b);
        }
    return z;
}

//============================================================================
/// This implementation seems slightly unnatural because it strives
/// for similarity with run_census_in_parallel::operator(). For
/// instance, 'Year' and 'Month' aren't used directly as loop
/// counters, and the loop has no early-exit condition like
///   if(ItLapsed) break;
/// which isn't necessary anyway because all the functions it calls
/// contain such a condition.

double AccountValue::RunOneCell(e_run_basis const& a_Basis)
{
    GuessWhetherFirstYearPremiumExceedsRetaliationLimit();
restart:
    InitializeLife(a_Basis);

    for(int year = InforceYear; year < BasicValues::GetLength(); ++year)
        {
        Year = year;
        CoordinateCounters();
        InitializeYear();

        int inforce_month = (Year == InforceYear) ? InforceMonth : 0;
        for(int month = inforce_month; month < 12; ++month)
            {
            Month = month;
            CoordinateCounters();
            // Absent a group context, case-level k factor is unity:
            // because partial mortality has no effect, experience
            // rating is impossible. USER !! Explain this in user
            // documentation.
            IncrementBOM(year, month, 1.0);
            // TODO ?? PRESSING Adjusting this by inforce is wrong for
            // individual cells run as such, because they don't
            // reflect partial mortality.
            IncrementEOM
                (year
                ,month
                ,SepAcctValueAfterDeduction * InforceLivesBoy()
                ,CumPmts
                );
            }

        if(!TestWhetherFirstYearPremiumExceededRetaliationLimit())
            {
            // We could do this instead:
            //   InitializeLife(a_Basis);
            //   --year;
            // to satisfy the popular 'zero-tolerance' attitude toward
            // the goto statement, but that would be more unnatural.
            DebugRestart("First-year premium did not meet retaliation limit.");
            goto restart;
            }

        SetClaims();
        SetProjectedCoiCharge();
        IncrementEOY(year);
        }

    FinalizeLife(a_Basis);

    return TotalAccountValue();
}

//============================================================================
void AccountValue::InitializeLife(e_run_basis const& a_Basis)
{
    RateBasis = a_Basis;
    set_separate_bases_from_run_basis(RateBasis, ExpAndGABasis, SABasis);

// JOE I moved the next three lines of code up here from below. Reason:
// output showed wrong specamt if specamt strategy is target, non-MEC,
// or corridor--because first the specamt for output was set by this line
//   InvariantValues().Init(...
// before the specamt strategy was applied.
//
// The situation is really unsatisfactory.
// InvariantValues().Init() is called earlier in the ctor;
// then we call OldPerformSpecAmtStrategy(), which assigns values to
// InvariantValues().SpecAmt; then we call InvariantValues().Init() again.
// But calling InvariantValues().Init() again wiped out the SpecAmt, because
// it reinitialized it based on DeathBfts_::SpecAmt, so I called
// DeathBfts_->SetSpecAmt() in AccountValue::OldPerformSpecAmtStrategy().

    SetInitialValues();

    // TODO ?? This is a nasty workaround. It seems that some or all strategies
    // should be performed at a much higher level, say in Run*(). Without
    // the conditional here, guar prem is wrong for 7-pay spec amt strategy.
    if(!SolvingForGuarPremium)
        {
        // TODO ?? There's some code in FinalizeYear() below that
        // sets InvariantValues().EePmt to an annualized value, but
        // OldPerformSpecAmtStrategy() expects a modal value. The
        // annualized value is wiped out a few lines below anyway.
        // This is a crock to suppress an observed symptom.
        InvariantValues().Init(this);
        OldPerformSpecAmtStrategy();
        }

    // TODO ?? It seems wrong to initialize the ledger values here.
    // I believe, but do not know, that the only reason for doing
    // this is that the solve routines can change parameters in
    // class BasicValues or objects it contains, parameters which
    // determine ledger values that are used by the solve routines.
    // It would seem appropriate to treat such parameters instead
    // as local state of class AccountValue itself, or of a contained
    // class smaller than the ledger hierarchy--which we need anyway
    // for 7702 and 7702A. Or perhaps the solve functions should
    // manipulate the state of just those elements of the ledgers
    // that it needs to, to avoid the massive overhead of
    // unconditionally reinitializing all elements.
    VariantValues().Init(this, ExpAndGABasis, SABasis);
    InvariantValues().Init(this);

    OldDBOpt = InvariantValues().DBOpt[0];
    OldSA = InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0];
    // TODO ?? Shouldn't we increase initial SA if contract in corridor at issue?
    OldDB = OldSA;

    SurrChg_.assign(BasicValues::GetLength(), 0.0);

    if(0 == Year && 0 == Month)
        {
        AddSurrChgLayer(Year, InvariantValues().SpecAmt[Year]);
        }
    else
        {
        // TODO ?? Inforce surrchg needs more work.
        double prior_specamt = 0.0;
        for(int j = 0; j <= Year; ++j)
            {
            AddSurrChgLayer
                (j
                ,std::max(0.0, Input_->VectorSpecamtHistory[j] - prior_specamt)
                );
            prior_specamt = Input_->VectorSpecamtHistory[j];
            }
        }

    // TODO ?? We need to reconsider where the Irc7702 object gets created.
    // Perhaps it should be when initial DB is first known? No, needed for
    // solves. Then maybe we only need to move the call to Initialize7702?
    Irc7702_->Initialize7702
        (InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
        ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
        ,Get7702EffectiveDBOpt(InvariantValues().DBOpt[0])
        );

    // JOE--Try this instead of your second change to ldginvar.cpp
    // and particularly your change to basicval.hpp . It is always better
    // to add a const accessor than to publish a private member. Besides,
    // this avoids recalculating GLP and GSP. And does your code calculate
    // guideline premiums correctly if specified amount is determined by
    // a strategy or a solve?
    InvariantValues().InitGLP = Irc7702_->RoundedGLP();
    InvariantValues().InitGSP = Irc7702_->RoundedGSP();
    // TODO ?? JOE--If you accept this modification, would you do the same sort
    // of thing for the initial seven-pay premium as well please?

    // This is notionally called once per *current-basis* run
    // and actually called once per run, with calculations suppressed
    // for all other bases. TODO ?? How should we handle MEC-avoid
    // solves on bases other than current?

    bool inforce_is_mec =
           (
              0 != Input_->InforceYear
           || 0 != Input_->InforceMonth
           )
        && Input_->InforceIsMec
        ;
    InvariantValues().InforceIsMec = inforce_is_mec;
    bool mec_1035 =
              Input_->External1035ExchangeFromMec
           && 0.0 != Outlay_->external_1035_amount()
        ||    Input_->Internal1035ExchangeFromMec
           && 0.0 != Outlay_->internal_1035_amount()
        ;
    bool is_already_a_mec = inforce_is_mec || mec_1035;
    if(is_already_a_mec)
        {
        InvariantValues().IsMec    = true;
        InvariantValues().MecYear  = 0;
        InvariantValues().MecMonth = 0;
        }
    std::vector<double> pmts_7702a;
    std::vector<double> bfts_7702a;
    int length_7702a = std::min(7, BasicValues::GetLength());
    if(0 == InforceYear && 0 == InforceMonth)
        {
        // No need to initialize 'pmts_7702a' in this case.
        // This assumes the term rider can be treated as death benefit.
        bfts_7702a.push_back
            (   InvariantValues().SpecAmt[0]
            +   InvariantValues().TermSpecAmt[0]
            );
        }
    else
        {
        // Premium history starts at contract year zero.
        nonstd::copy_n_
            (Input_->VectorPremiumHistory.begin()
            ,length_7702a
            ,std::back_inserter(pmts_7702a)
            );
        // Specamt history starts at policy year zero.
        nonstd::copy_n_
            (Input_->VectorSpecamtHistory.begin() + Input_->InforceContractYear.operator int const&()
            ,length_7702a
            ,std::back_inserter(bfts_7702a)
            );
        }
    double lowest_death_benefit = Input_->InforceLeastDeathBenefit;
    if(0 == InforceYear && 0 == InforceMonth)
        {
        lowest_death_benefit = bfts_7702a.front();
        }
    Irc7702A_->Initialize7702A
        (e_run_curr_basis != RateBasis
        ,is_already_a_mec
        ,IssueAge
        ,BasicValues::EndtAge
        ,InforceYear
        ,InforceMonth
        ,Input_->InforceContractYear
        ,Input_->InforceContractMonth
        ,Input_->InforceAvBeforeLastMc
        ,lowest_death_benefit
        ,pmts_7702a
        ,bfts_7702a
        );

    daily_interest_accounting =
            std::string::npos
        !=  Input_->Comments.find("idiosyncrasy_daily_interest_accounting")
        ;

/* TODO expunge?
    OldPerformSpecAmtStrategy();
*/
}

//============================================================================
void AccountValue::FinalizeLife(e_run_basis const& a_Basis)
{
    LMI_ASSERT(RateBasis == a_Basis);

    DebugEndBasis();

    if(SolvingForGuarPremium)
        {
        return;
        }

    if(e_run_curr_basis == RateBasis)
        {
        ledger_->SetLedgerInvariant(InvariantValues());
        }
    ledger_->SetOneLedgerVariant(a_Basis, VariantValues());
}

//============================================================================
void AccountValue::FinalizeLifeAllBases()
{
    ledger_->ZeroInforceAfterLapse();
    SetGuarPrem();
}

//============================================================================
void AccountValue::SetInitialValues()
{
    // These inforce things belong in input struct.
    // TODO ?? The list is not complete; others will be required:
    // payment history; surrender charges; DCV history?
    InforceYear                 = Input_->InforceYear            ;
    InforceMonth                = Input_->InforceMonth           ;
    InforceAVGenAcct            = Input_->InforceAVGenAcct       ;
    InforceAVSepAcct            = Input_->InforceAVSepAcct       ;
    InforceAVRegLn              = Input_->InforceAVRegLn         ;
    InforceAVPrfLn              = Input_->InforceAVPrfLn         ;
    InforceRegLnBal             = Input_->InforceRegLnBal        ;
    InforcePrfLnBal             = Input_->InforcePrfLnBal        ;
    InforceCumNoLapsePrem       = Input_->InforceCumNoLapsePrem  ;
    InforceCumPmts              = Input_->InforceCumPmts         ;
    InforceTaxBasis             = Input_->InforceTaxBasis        ;

    Year                        = InforceYear;
    Month                       = InforceMonth;
    CoordinateCounters();

    DB7702A                     = 0.0;  // TODO ?? This seems silly.

    AVRegLn                     = InforceAVRegLn;
    AVPrfLn                     = InforceAVPrfLn;
    RegLnBal                    = InforceRegLnBal;
    PrfLnBal                    = InforcePrfLnBal;
    AVGenAcct                   = InforceAVGenAcct;
    AVSepAcct                   = InforceAVSepAcct;

    if(Input_->AvgFund || Input_->OverrideFundMgmtFee)
        {
        SepAcctPaymentAllocation = 1.0;
        }
    else if(100 == Input_->SumOfSepAcctFundAllocs())
        {
        // Because 100 * .01 does not exactly equal unity, treat 100%
        // as a special case to avoid catastrophic cancellation when
        // calculating general-account allocation as the difference
        // between this quantity and unity.
        SepAcctPaymentAllocation = 1.0;
        }
    else
        {
        SepAcctPaymentAllocation = .01 * Input_->SumOfSepAcctFundAllocs();
        }

    GenAcctPaymentAllocation = 1.0 - SepAcctPaymentAllocation;

    if(!Database_->Query(DB_AllowGenAcct) && 0.0 != GenAcctPaymentAllocation)
        {
        fatal_error()
            << "No general account is allowed for this product, but "
            << "allocation to general account is "
            << GenAcctPaymentAllocation
            << " ."
            << LMI_FLUSH
            ;
        }

    if(!Database_->Query(DB_AllowSepAcct) && 0.0 != SepAcctPaymentAllocation)
        {
        fatal_error()
            << "No separate account is allowed for this product, but "
            << " total allocation to separate accounts is "
            << SepAcctPaymentAllocation
            << " ."
            << LMI_FLUSH
            ;
        }

    MaxLoan                     = 0.0;

    GenAcctIntCred              = 0.0;
    SepAcctIntCred              = 0.0;
    RegLnIntCred                = 0.0;
    PrfLnIntCred                = 0.0;

    MaxWD                       = 0.0;
    GrossWD                     = 0.0;
    NetWD                       = 0.0;

    CumPmts                     = InforceCumPmts;
    TaxBasis                    = InforceTaxBasis;
    MlyNoLapsePrem              = 0.0;
    CumNoLapsePrem              = InforceCumNoLapsePrem;

    YearlyNoLapseActive.assign(BasicValues::GetLength(), true);
    NoLapseActive               = true;
    if(NoLapseOpt1Only && e_option1 != DeathBfts_->dbopt()[0])
        {
        NoLapseActive           = false;
        }
    if(NoLapseUnratedOnly && Input_->Status[0].IsPolicyRated())
        {
        NoLapseActive           = false;
        }

    RiderDeductions             = 0.0;

    TermRiderActive             = true;
    TermDB                      = 0.0;

    // Assume by default that the policy never lapses or becomes a MEC,
    // so that the lapse and MEC durations are the last possible month.
    // TODO ?? Last possible month? Why? In that month, it doesn't quite
    // lapse, and it's certainly wrong to assume it becomes a MEC then.
    ItLapsed                    = false;
    VariantValues().LapseMonth  = 11;
    VariantValues().LapseYear   = BasicValues::GetLength();
// TODO ?? Length should be Database_->Query(DB_EndtAge);

    InvariantValues().IsMec     = false;
    InvariantValues().MecMonth  = 11;
    InvariantValues().MecYear   = BasicValues::GetLength();
    Dcv                         = Input_->InforceDcv;
    DcvDeathBft                 = 0.0;
    DcvNaar                     = 0.0;
    DcvCoiCharge                = 0.0;
    DcvTermCharge               = 0.0;
    DcvWpCharge                 = 0.0;

    HoneymoonActive             = false;
    HoneymoonValue              = -std::numeric_limits<double>::max();
    if(e_currbasis == ExpAndGABasis)
        {
        HoneymoonActive = Input_->HasHoneymoon;
        if(0 != Year || 0 != Month)
            {
            HoneymoonActive =
                   HoneymoonActive
                && 0.0 < Input_->InforceHoneymoonValue
                ;
            }
        if(HoneymoonActive)
            {
            HoneymoonValue = Input_->InforceHoneymoonValue;
            }
        }

    CoiCharge                   = 0.0;
    NetCoiCharge                = 0.0;
    MlyDed                      = 0.0;
    CumulativeSalesLoad         = 0.0;

    CoiRetentionRate                  = Database_->Query(DB_ExpRatCOIRetention);
    ExperienceRatingAmortizationYears = Database_->Query(DB_ExpRatAmortPeriod);
    IbnrAsMonthsOfMortalityCharges    = Database_->Query(DB_ExpRatIBNRMult);

    Dumpin             = Outlay_->dumpin();
    External1035Amount = Outlay_->external_1035_amount();
    Internal1035Amount = Outlay_->internal_1035_amount();

    ee_premium_allocation_method   = static_cast<e_allocation_method>
        (static_cast<int>(Database_->Query(DB_EePremMethod))
        );
    ee_premium_preferred_account   = static_cast<e_increment_account_preference>
        (static_cast<int>(Database_->Query(DB_EePremAcct))
        );
    er_premium_allocation_method   = static_cast<e_allocation_method>
        (static_cast<int>(Database_->Query(DB_ErPremMethod))
        );
    er_premium_preferred_account   = static_cast<e_increment_account_preference>
        (static_cast<int>(Database_->Query(DB_ErPremAcct))
        );
    deduction_method               = static_cast<e_increment_method>
        (static_cast<int>(Database_->Query(DB_DeductionMethod))
        );
    deduction_preferred_account    = static_cast<e_increment_account_preference>
        (static_cast<int>(Database_->Query(DB_DeductionAcct))
        );
    distribution_method            = static_cast<e_increment_method>
        (static_cast<int>(Database_->Query(DB_DistributionMethod))
        );
    distribution_preferred_account = static_cast<e_increment_account_preference>
        (static_cast<int>(Database_->Query(DB_DistributionAcct))
        );

    // If any account preference is the separate account, then a
    // separate account must be available.
    if
        (    e_prefer_separate_account == ee_premium_preferred_account
        ||   e_prefer_separate_account == er_premium_preferred_account
        ||   e_prefer_separate_account == deduction_preferred_account
        ||   e_prefer_separate_account == distribution_preferred_account
        )
        {
        LMI_ASSERT(Database_->Query(DB_AllowSepAcct));
        }
    // If any account preference for premium is the general account,
    // then payment into the separate account must be permitted; but
    // even a product that doesn't permit that might have a general
    // account, e.g. for loans or deductions.
    if
        (    e_prefer_separate_account == ee_premium_preferred_account
        ||   e_prefer_separate_account == er_premium_preferred_account
        )
        {
        LMI_ASSERT(Database_->Query(DB_AllowSepAcct));
        }
}

//============================================================================
// Process monthly transactions up to but excluding interest credit
double AccountValue::IncrementBOM
    (int    year
    ,int    month
    ,double a_case_k_factor
    )
{
    // TODO ?? We don't start at InforceYear, because issue years may differ
    // between lives and we have not coded support for that yet.
    if(ItLapsed || BasicValues::GetLength() <= year)
        {
        // Return value is total assets. After the policy has lapsed or
        // matured, there are no assets.
        return 0.0;
        }

    // Paranoid check.
    LMI_ASSERT(year == Year);
    LMI_ASSERT(month == Month);
    LMI_ASSERT(MonthsSinceIssue == Month + 12 * Year);
    if(daily_interest_accounting)
        {
        LMI_ASSERT(365 <= days_in_policy_year && days_in_policy_year <= 366);
        LMI_ASSERT(28 <= days_in_policy_month && days_in_policy_month <= 31);
        }
    if
        (   year != Year
        ||  month != Month
        ||  MonthsSinceIssue != Month + 12 * Year
        ||  daily_interest_accounting && !(365 <= days_in_policy_year && days_in_policy_year <= 366)
        ||  daily_interest_accounting && !(28 <= days_in_policy_month && days_in_policy_month <= 31)
        )
        {
        fatal_error()
            << "Expected year = "  << Year
            << "; actual year is  " << year << ".\n"
            << "Expected month = " << Month
            << "; actual month is " << month << ".\n"
            << "Expected MonthsSinceIssue = " << (Month + 12 * Year)
            << "; actual value is " << MonthsSinceIssue << ".\n"
            << "Days in policy year = " << days_in_policy_year << ".\n"
            << "Days in policy month = " << days_in_policy_month << ".\n"
            << LMI_FLUSH
            ;
        }

    if
        (   Input_->UseExperienceRating
        &&  e_currbasis == ExpAndGABasis
        )
        {
        case_k_factor = a_case_k_factor;
        }

    DoMonthDR();
    return TotalAccountValue();
}

//============================================================================
// Credit interest and process all subsequent monthly transactions
void AccountValue::IncrementEOM
    (int year
    ,int month
    ,double assets_post_bom
    ,double cum_pmts_post_bom
    )
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

    // Paranoid check.
    LMI_ASSERT(year == Year);
    LMI_ASSERT(month == Month);
    LMI_ASSERT(MonthsSinceIssue == Month + 12 * Year);
    if(daily_interest_accounting)
        {
        LMI_ASSERT(365 <= days_in_policy_year && days_in_policy_year <= 366);
        LMI_ASSERT(28 <= days_in_policy_month && days_in_policy_month <= 31);
        }

    // Save arguments, constraining their values to be nonnegative,
    // for calculating banded and tiered quantities.
    AssetsPostBom  = std::max(0.0, assets_post_bom  );
    CumPmtsPostBom = std::max(0.0, cum_pmts_post_bom);

    DoMonthCR();
}

//============================================================================
void AccountValue::IncrementEOY(int year)
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

    // Paranoid check.
    LMI_ASSERT(year == Year);

    FinalizeYear();
}

//============================================================================
bool AccountValue::PrecedesInforceDuration(int year, int month)
{
    return year < InforceYear || (year == InforceYear && month < InforceMonth);
}

//============================================================================
void AccountValue::InitializeYear()
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

// TODO ?? Solve...() should reset not inputs but...something else?
    SetAnnualInvariants();

    MonthsPolicyFees            = 0.0;
    SpecAmtLoad                 = 0.0;

    AssetsPostBom               = 0.0;
    CumPmtsPostBom              = 0.0;
    SepAcctLoad                 = 0.0;

    YearsTotalCoiCharge         = 0.0;
    YearsAVRelOnDeath           = 0.0;
    YearsGrossClaims            = 0.0;
    YearsNetClaims              = 0.0;
    YearsTotalNetIntCredited    = 0.0;
    YearsTotalGrossIntCredited  = 0.0;
    YearsTotalNetCoiCharges     = 0.0;
    YearsTotalPolicyFee         = 0.0;
    YearsTotalPremTaxLoad       = 0.0;
    YearsTotalPremTaxLoadInStateOfDomicile = 0.0;
    YearsTotalPremTaxLoadInStateOfJurisdiction = 0.0;
    YearsTotalDacTaxLoad        = 0.0;
    YearsTotalSpecAmtLoad       = 0.0;
    YearsTotalSepAcctLoad       = 0.0;
    YearsTotalGptForceout       = 0.0;

    NextYearsProjectedCoiCharge = 0.0;

    PolicyYearRunningTotalPremiumSubjectToPremiumTax = 0.0;

    DacTaxRsv                   = 0.0;

    RequestedLoan       = Outlay_->new_cash_loans()[Year];
    ActualLoan          = RequestedLoan; // TODO ?? Why not zero?

    GrossPmts   .assign(12, 0.0);
    EeGrossPmts .assign(12, 0.0);
    ErGrossPmts .assign(12, 0.0);
    NetPmts     .assign(12, 0.0);

    InitializeSpecAmt();
}

//============================================================================
// MEC avoidance may require issuing a contract at a higher specamt than input.
void AccountValue::InitializeSpecAmt()
{
    Irc7702_->UpdateBOY7702();
    Irc7702A_->UpdateBOY7702A(Year);

    YearsSpecAmt        = DeathBfts_->specamt()[Year];

    // TODO ?? These variables are set in current run and used in guar and midpt.
    ActualSpecAmt       = InvariantValues().SpecAmt[Year];
    TermSpecAmt         = InvariantValues().TermSpecAmt[Year];

    int target_year = Year;
    if(Database_->Query(DB_TgtPmFixedAtIssue))
        {
        target_year = 0;
        }
    MlyNoLapsePrem = GetModalTgtPrem
        (target_year
        ,e_mode(e_monthly)
        ,InvariantValues().SpecAmt[target_year]
        );
    UnusedTargetPrem = GetModalTgtPrem
        (target_year
        ,e_mode(e_annual)
        ,InvariantValues().SpecAmt[target_year]
        );
    AnnualTargetPrem = UnusedTargetPrem;

/*
// TODO ?? This code might be preferable if it worked correctly.
//
// Motivation for GetTgtPrem(): encapsulate calculations that need to
// return the exact target premium, respecting all arcana such as
// 'DB_TgtPmFixedAtIssue'.
//
// Defect in its implementation: specamt is passed as an argument, and
// it's easy to get that wrong, as it is here. Real encapsulation
// shouldn't need arguments.
//
// Problems this might cause elsewhere: apparently GPT calculations
// use this function with the then-current specamt. That might be
// correct, though, because the B and C contracts in the A+B-C
// formula are deemed to have current issue dates.

    MlyNoLapsePrem = GetTgtPrem
        (Year
        ,InvariantValues().SpecAmt[Year]
        ,InvariantValues().DBOpt[Year]
        ,e_mode(e_monthly)
        );

    // Target premium is annual mode. Using a different mode seems
    // conceivable, but if the ee and er both pay, and on different
    // modes, then it would be unclear which mode to choose.
    UnusedTargetPrem = GetTgtPrem
        (Year
        ,InvariantValues().SpecAmt[Year]
        ,InvariantValues().DBOpt[Year]
        ,e_mode(e_annual)
        );
    AnnualTargetPrem = UnusedTargetPrem;
*/

    if(0 == Year)
        {
        InvariantValues().InitTgtPrem = AnnualTargetPrem;
        }

    SurrChgSpecAmt = InvariantValues().SpecAmt[0];
    LMI_ASSERT(0.0 <= SurrChgSpecAmt);
    // TODO ?? SurrChgSpecAmt is not used yet.

    // TODO ?? Perform specamt strategy here?
}

//============================================================================
void AccountValue::AddSurrChgLayer(int year, double delta_specamt)
{
    if(!SurrChgOnIncr || 0.0 == delta_specamt)
        {
        return;
        }

// TODO ?? It should be something like this:
//    rate = delta_specamt * TempDatabase.Query(DB_SurrChgSAMult);
// but for the moment we resort to this kludge:
    double z = delta_specamt * MortalityRates_->TargetPremiumRates()[year];

    std::vector<double> new_layer;
    std::transform
        (SurrChgRates_->SpecamtRateDurationalFactor().begin()
        ,SurrChgRates_->SpecamtRateDurationalFactor().end() - year
        ,std::inserter(new_layer, new_layer.begin())
        ,boost::bind
            (round_surrender_charge
            ,boost::bind(std::multiplies<double>(), _1, z)
            )
        );

    std::transform
        (year + SurrChg_.begin()
        ,       SurrChg_.end()
        ,       new_layer.begin()
        ,year + SurrChg_.begin()
        ,std::plus<double>()
        );

#ifdef DEBUGGING_SC
double target_premium_rate = MortalityRates_->TargetPremiumRates()[year];
std::ofstream os
        ("surrchg.txt"
        ,   std::ios_base::out
          | std::ios_base::ate
          | std::ios_base::app
        );
os
<< "AccountValue::AddSurrChgLayer(): \n   "
        << "\n\t delta_specamt = " << delta_specamt
        << "\n\t year = " << year
        << "\n\t InforceYear = " << InforceYear
        << "\n\t Year = " << Year
        << "\n\t InforceMonth = " << InforceMonth
        << "\n\t Month = " << Month
        << "\n\t z = " << z
        << "\n\t target_premium_rate = " << target_premium_rate
        ;

os << "\n\t SurrChgRates_->SpecamtRateDurationalFactor() = ";
std::copy(SurrChgRates_->SpecamtRateDurationalFactor().begin(), SurrChgRates_->SpecamtRateDurationalFactor().end(), std::ostream_iterator<double>(os, " "));
os << "\n\t new_layer = ";
std::copy(new_layer.begin(), new_layer.end(), std::ostream_iterator<double>(os, " "));
os << "\n\t SurrChg_ = ";
std::copy(SurrChg_.begin(), SurrChg_.end(), std::ostream_iterator<double>(os, " "));
os << "\n\n";
#endif // DEBUGGING_SC
}

//============================================================================
// Upon partial surrender, multiply current and future surrchg by
//   1 - (partial surrchg / full surrchg)
void AccountValue::ReduceSurrChg(int year, double partial_surrchg)
{
#ifdef DEBUGGING_SC
    std::ofstream os
        ("trace.txt"
        ,   std::ios_base::out
          | std::ios_base::ate
          | std::ios_base::app
        );
    int year0 = std::min(0 + year, BasicValues::GetLength());
    int year1 = std::min(1 + year, BasicValues::GetLength());
    int year2 = std::min(2 + year, BasicValues::GetLength());
    int year3 = std::min(3 + year, BasicValues::GetLength());
    int year4 = std::min(4 + year, BasicValues::GetLength());
    int year5 = std::min(5 + year, BasicValues::GetLength());
    os
        << "\n ReduceSurrChg() [before]:"
        << "\n Year = " << Year
        << "\n Month = " << Month
        << "\n year = " << year
        << "\n partial_surrchg = " << partial_surrchg
        << "\n (partial_surrchg / SurrChg_[year]) = " << (partial_surrchg / SurrChg_[year])
        << "\n SurrChg_[year0] = " << SurrChg_[year0]
        << "\n SurrChg_[year1] = " << SurrChg_[year1]
        << "\n SurrChg_[year2] = " << SurrChg_[year2]
        << "\n SurrChg_[year3] = " << SurrChg_[year3]
        << "\n SurrChg_[year4] = " << SurrChg_[year4]
        << "\n SurrChg_[year5] = " << SurrChg_[year5]
        << std::endl
        ;
#endif // DEBUGGING_SC

    if(!SurrChgOnIncr || 0.0 == partial_surrchg)
        {
        return;
        }
    // We don't assert the condition because this function might
    // be called for a product that has no tabular surrender charge.
    if(0.0 != SurrChg_[year])
        {
        double multiplier = 1.0 - partial_surrchg / SurrChg_[year];
        std::transform
            (year + SurrChg_.begin()
            ,       SurrChg_.end()
            ,year + SurrChg_.begin()
            ,boost::bind
                (round_surrender_charge
                ,boost::bind(std::multiplies<double>(), _1, multiplier)
                )
            );
        }

#ifdef DEBUGGING_SC
    os
        << "\n ReduceSurrChg() [after]:"
        << "\n multiplier = " << (1.0 - partial_surrchg / SurrChg_[year])
        << "\n SurrChg_[year0] = " << SurrChg_[year0]
        << "\n SurrChg_[year1] = " << SurrChg_[year1]
        << "\n SurrChg_[year2] = " << SurrChg_[year2]
        << "\n SurrChg_[year3] = " << SurrChg_[year3]
        << "\n SurrChg_[year4] = " << SurrChg_[year4]
        << "\n SurrChg_[year5] = " << SurrChg_[year5]
        << std::endl
        ;
#endif // DEBUGGING_SC
}

//============================================================================
// Surrender charge. Only simple multiplicative parts are implemented.
//
// SOMEDAY !! Table support and UL model reg formulas should be added.
//
double AccountValue::SurrChg()
{
    // We permit negative surrender-charge factors. But we don't
    // allow those factors, regardless of sign, to be multiplied by
    // negative base amounts, which would result in surrender charge
    // components having an unexpected sign.

//    LMI_ASSERT(0.0 <= CumPmts); // TODO ?? Fails on a few test cases: should it?
    LMI_ASSERT(0.0 <= InvariantValues().SpecAmt[0]);

    return
            YearsSurrChgAVMult      * std::max(0.0, TotalAccountValue())
        +   YearsSurrChgPremMult    * std::max(0.0, CumPmts)
        +   SurrChg_[Year]
// TODO ?? expunge this and its antecedents:
//        +   YearsSurrChgSAMult      * InvariantValues().SpecAmt[0]
        -   Input_->VectorCashValueEnhancementRate[Year] * std::max(0.0, TotalAccountValue())
        ;
}

//============================================================================
void AccountValue::SetClaims()
{
    if(!Input_->UsePartialMort || ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

    // Update death benefit. 'DBReflectingCorr' currently equals the
    // death benefit as of the beginning of the twelfth month, but its
    // end-of-year value (as of the end of the twelfth month) is
    // needed.

    TxSetDeathBft(true);
    TxSetTermAmt();

    // Amounts such as claims and account value released on death
    // are multiplied by the beginning-of-year inforce factor when
    // a composite is produced; it would be incorrect to multiply
    // them by the inforce factor here because individual-cell
    // ledgers do not reflect partial mortality. This calculation
    // assumes that partial mortality is curtate.

    YearsGrossClaims =  partial_mortality_q[Year] * DBReflectingCorr;
    YearsAVRelOnDeath = partial_mortality_q[Year] * TotalAccountValue();
    YearsNetClaims = material_difference(YearsGrossClaims, YearsAVRelOnDeath);
}

//============================================================================
// Proxy for next year's COI charge, used only for experience rating.
void AccountValue::SetProjectedCoiCharge()
{
    if
        (   ItLapsed
        ||  BasicValues::GetLength() <= Year
        ||  !Input_->UseExperienceRating
        ||  !e_currbasis == ExpAndGABasis
        )
        {
        return;
        }

    // Project a charge of zero for the year after maturity.
    //
    // This is written separately to emphasize its meaning, though it
    // obviously could be combined with the above '<=' comparison.
    //
    if(BasicValues::GetLength() == 1 + Year)
        {
        return;
        }

    TxSetDeathBft(true);
    TxSetTermAmt();
    double this_years_terminal_naar = material_difference
        (DBReflectingCorr + TermDB
        ,TotalAccountValue()
        );
    this_years_terminal_naar = std::max(0.0, this_years_terminal_naar);
    double next_years_coi_rate = GetBandedCoiRates(ExpAndGABasis, ActualSpecAmt)[1 + Year];

    NextYearsProjectedCoiCharge =
            12.0
        *   this_years_terminal_naar
        *   next_years_coi_rate
        ;
}

//============================================================================
void AccountValue::FinalizeYear()
{
    double total_av = TotalAccountValue();
    double surr_chg = SurrChg();
    double csv_net =
          total_av
        - surr_chg
        - (RegLnBal + PrfLnBal)
        + GetRefundableSalesLoad()
//        + ExpRatReserve // This would be added if it existed.
        ;
    csv_net = std::max(csv_net, HoneymoonValue);
    csv_net = std::max(csv_net, 0.0);

    // 7702(f)(2)(A)
    double cv_7702 =
          total_av
        + GetRefundableSalesLoad()
//        + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        ;
    // Increase by negative surrender charge. If some components of
    // the surrender charge are negative while others are positive,
    // consider only their sum here, instead of considering the sign
    // of each component individually.
    if(surr_chg < 0.0)
        {
        cv_7702 -= surr_chg;
        }
    cv_7702 = std::max(cv_7702, HoneymoonValue);

    VariantValues().AcctVal     [Year] = total_av;
    VariantValues().DacTaxRsv   [Year] = DacTaxRsv;
    VariantValues().CSVNet      [Year] = csv_net;
    VariantValues().CV7702      [Year] = cv_7702;

    // Update death benefit. 'DBReflectingCorr' currently equals the
    // death benefit as of the beginning of the twelfth month, but its
    // end-of-year value (as of the end of the twelfth month) is
    // needed.

    TxSetDeathBft(true);
    TxSetTermAmt();
    // post values to LedgerVariant
    InvariantValues().TermSpecAmt   [Year] = TermSpecAmt;
    VariantValues().TermPurchased   [Year] = TermDB;
    // Add term rider DB
    VariantValues().BaseDeathBft    [Year] = DBReflectingCorr;
    VariantValues().EOYDeathBft     [Year] = DBReflectingCorr + TermDB;

/*
    // AV already includes any experience refund credited, but it's
    // forborne among the survivors. That was the right thing to use
    // for calculating the corridor death benefit for each individual.
    // But it's not the right thing to multiply by EOY inforce and add
    // into a composite. The right thing is for the composite to sum
    // the AV plus *cash* refund instead of forborne, multiplied by EOY
    // inforce. Otherwise an account-value rollforward cross-check
    // wouldn't work.
    //
    // TODO ?? Maybe the AV before this adjustment is what we really want
    // to display for an individual illustration. That's what we did
    // originally, and I'm not at all sure it's right to change it
    // now.
    VariantValues().AcctVal         [Year] =
          TotalAccountValue()
        + VariantValues().ExpRatRfdCash     [Year]
        - VariantValues().ExpRatRfdForborne [Year]
        ;
*/

    // Monthly deduction detail
    //
    // TODO ?? This is done only if the policy is in force at the end of the
    // year; but if it lapses during the year, should things that happened
    // during the year of lapse be included in a composite?
    VariantValues().COICharge       [Year] = YearsTotalCoiCharge        ;
    VariantValues().AVRelOnDeath    [Year] = YearsAVRelOnDeath          ;
    VariantValues().ClaimsPaid      [Year] = YearsGrossClaims           ;
    VariantValues().NetClaims       [Year] = YearsNetClaims             ;
    VariantValues().NetIntCredited  [Year] = YearsTotalNetIntCredited   ;
    VariantValues().GrossIntCredited[Year] = YearsTotalGrossIntCredited ;
    VariantValues().NetCOICharge    [Year] = YearsTotalNetCoiCharges    ;
    VariantValues().PolicyFee       [Year] = YearsTotalPolicyFee        ;
    VariantValues().PremTaxLoad     [Year] = YearsTotalPremTaxLoad      ;
    VariantValues().DacTaxLoad      [Year] = YearsTotalDacTaxLoad       ;
    VariantValues().SpecAmtLoad     [Year] = YearsTotalSpecAmtLoad      ;
    VariantValues().SepAcctLoad     [Year] = YearsTotalSepAcctLoad      ;

    // Record dynamic interest rate in ledger object.
    //
    // TODO ?? Actually, 'YearsSepAcctIntRate' holds the dynamic rate
    // for the current month--here, the year's last month. It would be
    // more accurate to use an average across twelve months.
    //
    // TODO ?? The annual rate is calculated in the interest-rate
    // class's dynamic callback function; it would be better to pass
    // it here, instead of invoking a costly transcendental function
    // that loses accuracy.
    //
    // TODO ?? Monthly rates should be recorded too.
    //
    if(MandEIsDynamic)
        {
        VariantValues().RecordDynamicSepAcctRate
            (InterestRates_->SepAcctNetRate
                (SABasis
                ,ExpAndGABasis
                ,e_rate_period(e_annual_rate)
                )
                [Year]
            ,InterestRates_->SepAcctNetRate
                (SABasis
                ,ExpAndGABasis
                ,e_rate_period(e_monthly_rate)
                )
                [Year]
            ,Year
            );
        }

    if(e_run_curr_basis == RateBasis)
        {
        InvariantValues().GrossPmt  [Year]  = 0.0;
        InvariantValues().EeGrossPmt[Year]  = 0.0;
        InvariantValues().ErGrossPmt[Year]  = 0.0;
        VariantValues  ().NetPmt    [Year]  = 0.0;

        // TODO ?? This is a temporary workaround until we do it right.
        // Forceouts should be a distinct component, passed separately
        // to ledger values. Probably we should treat 1035 exchanges
        // and NAAR 'forceouts' the same way.
        InvariantValues().GrossPmt  [Year]  -= YearsTotalGptForceout;
        InvariantValues().EeGrossPmt[Year]  -= YearsTotalGptForceout;
        VariantValues  ().NetPmt    [Year]  -= YearsTotalGptForceout;

        for(int j = 0; j < 12; ++j)
            {
            LMI_ASSERT(materially_equal(GrossPmts[j], EeGrossPmts[j] + ErGrossPmts[j]));
            InvariantValues().GrossPmt  [Year]  += GrossPmts[j];
            InvariantValues().EeGrossPmt[Year]  += EeGrossPmts[j];
            InvariantValues().ErGrossPmt[Year]  += ErGrossPmts[j];
            VariantValues  ().NetPmt    [Year]  += NetPmts[j];
            }
        if(0 == Year)
            {
            InvariantValues().InitPrem = InvariantValues().GrossPmt[Year];
            }
        LMI_ASSERT
            (materially_equal
                (   InvariantValues().GrossPmt  [Year]
                ,   InvariantValues().EeGrossPmt[Year]
                +   InvariantValues().ErGrossPmt[Year]
                )
            );
        InvariantValues().Outlay[Year] =
                InvariantValues().GrossPmt   [Year]
            -   InvariantValues().NetWD      [Year]
            -   InvariantValues().NewCashLoan[Year]
            ;

        InvariantValues().EePmt[Year] = InvariantValues().EeGrossPmt[Year];
        InvariantValues().ErPmt[Year] = InvariantValues().ErGrossPmt[Year];

        InvariantValues().GptForceout[Year] = YearsTotalGptForceout;

// TODO ?? Not yet implemented.
//        InvariantValues().NaarForceout[Year] = InvariantValues().ErGrossPmt[Year];

        // TODO ?? This should also incorporate:
        //   asset-tiered compensation as in ApplyDynamicSepAcctLoad()
        //   input 'extra' compensation (on premium and assets)
        double commission =
                InvariantValues().GrossPmt[Year]
            -   TieredGrossToNet
                    (InvariantValues().GrossPmt[Year]
                    ,AnnualTargetPrem
                    ,CompTarget[Year]
                    ,CompExcess[Year]
                    )
            ;
        InvariantValues().ProducerCompensation[Year] =
//                AssetComp[Year] * AVSepAcct // original
                AssetComp[Year] * (AVSepAcct + AVGenAcct) // workaround
// TODO ?? Rethink this.
            +   commission
            ;
        }
}

//============================================================================
void AccountValue::SetAnnualInvariants()
{
    YearsCorridorFactor     = GetCorridorFactor()[Year];
    YearsDBOpt              = DeathBfts_->dbopt()[Year];
    YearsMonthlyPolicyFee   = Loads_->monthly_policy_fee(ExpAndGABasis)[Year];
    YearsAnnualPolicyFee    = Loads_->annual_policy_fee(ExpAndGABasis)[Year];

    YearsGenAcctIntRate     = InterestRates_->GenAcctNetRate
        (ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;
    YearsSepAcctIntRate     = InterestRates_->SepAcctNetRate
        (SABasis
        ,ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;

    YearsDcvIntRate         = GetMly7702iGlp()[Year];
    YearsHoneymoonValueRate = InterestRates_->HoneymoonValueRate
        (ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;
    YearsPostHoneymoonGenAcctIntRate = InterestRates_->PostHoneymoonGenAcctRate
        (ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;

    YearsRegLnIntCredRate   = InterestRates_ ->RegLnCredRate
        (ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;
    YearsPrfLnIntCredRate   = InterestRates_ ->PrfLnCredRate
        (ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;
    YearsRegLnIntDueRate    = InterestRates_ ->RegLnDueRate
        (ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;
    YearsPrfLnIntDueRate    = InterestRates_ ->PrfLnDueRate
        (ExpAndGABasis
        ,e_rate_period(e_monthly_rate)
        )
        [Year]
        ;

    YearsCoiRate0           = MortalityRates_->MonthlyCoiRatesBand0(ExpAndGABasis)[Year];
    YearsCoiRate1           = MortalityRates_->MonthlyCoiRatesBand1(ExpAndGABasis)[Year];
    YearsCoiRate2           = MortalityRates_->MonthlyCoiRatesBand2(ExpAndGABasis)[Year];
    Years7702CoiRate        = GetMly7702qc                     ()             [Year];
    YearsAdbRate            = MortalityRates_->AdbRates        ()             [Year];
    YearsTermRate           = MortalityRates_->MonthlyTermCoiRates(ExpAndGABasis)[Year];
    YearsWpRate             = MortalityRates_->WpRates         ()             [Year];
    YearsSpouseRiderRate    = MortalityRates_->SpouseRiderRates(ExpAndGABasis)[Year];
    YearsChildRiderRate     = MortalityRates_->ChildRiderRates ()             [Year];

    YearsSurrChgPremMult    = SurrChgRates_->RatePerDollarOfPremium()         [Year];

/* TODO ?? expunge
    YearsSurrChgAVMult      = SurrChgRates_->RatePerDollarOfAcctval()         [Year];
    YearsSurrChgSAMult      = SurrChgRates_->RatePerDollarOfSpecamt()         [Year];
*/

    YearsSurrChgAVMult =
          SurrChgRates_->RatePerDollarOfAcctval()      [Year]
        * SurrChgRates_->AcctvalRateDurationalFactor() [Year]
        ;
/*
    YearsSurrChgSAMult =
          SurrChgRates_->RatePerDollarOfSpecamt()      [Year]
        * SurrChgRates_->SpecamtRateDurationalFactor() [Year]
        ;
*/
    YearsTotLoadTgt         = Loads_->target_total_load  (ExpAndGABasis)[Year];
    YearsTotLoadExc         = Loads_->excess_total_load  (ExpAndGABasis)[Year];
    YearsTotLoadTgtLowestPremtax = Loads_->target_premium_load_7702_lowest_premium_tax()[Year];
    YearsTotLoadExcLowestPremtax = Loads_->excess_premium_load_7702_lowest_premium_tax()[Year];
    YearsPremLoadTgt        = Loads_->target_premium_load(ExpAndGABasis)[Year];
    YearsPremLoadExc        = Loads_->excess_premium_load(ExpAndGABasis)[Year];
    YearsSalesLoadTgt       = Loads_->target_sales_load  (ExpAndGABasis)[Year];
    YearsSalesLoadExc       = Loads_->excess_sales_load  (ExpAndGABasis)[Year];
    YearsSpecAmtLoadRate    = Loads_->specified_amount_load (ExpAndGABasis)[Year];
    YearsSepAcctLoadRate    = Loads_->separate_account_load (ExpAndGABasis)[Year];
    YearsSalesLoadRefundRate= Loads_->refundable_sales_load_proportion()[Year];
    YearsPremTaxLoadRate    = Loads_->premium_tax_load                ()[Year];
    YearsDacTaxLoadRate     = Loads_->dac_tax_load                    ()[Year];
}

//============================================================================
double AccountValue::TaxableFirstYearPlannedPremium() const
{
// TODO ?? 'WaivePmTxInt1035' is not respected elsewhere, but should be.
    double z =
          PerformEePmtStrategy() * InvariantValues().EeMode[0]
        + PerformErPmtStrategy() * InvariantValues().ErMode[0]
        + Outlay_->dumpin()
        + Outlay_->external_1035_amount()
        ;
    if(!WaivePmTxInt1035)
        {
        z += Outlay_->internal_1035_amount();
        }
    return z;
}

//============================================================================
void AccountValue::GuessWhetherFirstYearPremiumExceedsRetaliationLimit()
{
    // TODO ?? Probably we should make PerformE[er]PmtStrategy() take
    // 'Year' as an argument. Until then, we need a dummy initial
    // value here.
    //
    // TODO ?? For inforce, we really need the admin system to say
    //   - whether the first-year retaliation limit was exceeded; and
    //   - how much premium has already been paid for inforce
    //       contracts that are still in the first policy year.

    Year  = Input_->InforceYear;
    Month = Input_->InforceMonth;
    CoordinateCounters();
    FirstYearPremiumExceedsRetaliationLimit =
           FirstYearPremiumRetaliationLimit
        <= TaxableFirstYearPlannedPremium()
        ;
}

//============================================================================
bool AccountValue::TestWhetherFirstYearPremiumExceededRetaliationLimit()
{
    if
        (  0 == Year
        && FirstYearPremiumExceedsRetaliationLimit
        && CumPmts < FirstYearPremiumRetaliationLimit
        )
        {
        FirstYearPremiumExceedsRetaliationLimit = false;
        return false;
        }
    else
        {
        return true;
        }
}

//============================================================================
double AccountValue::GetPartMortQ(int a_year) const
{
    LMI_ASSERT(a_year <= BasicValues::GetLength());
    if(!Input_->UsePartialMort || ItLapsed)
        {
        return 0.0;
        }
    if
        (   MaxSurvivalDur <= a_year
        ||  a_year == BasicValues::GetLength()
        )
        {
        return 1.0;
        }

    double z =
          MortalityRates_->PartialMortalityQ()[a_year]
        * Input_->VectorPartialMortalityMultiplier[a_year]
        ;
    return std::max(0.0, std::min(1.0, z));
}

//============================================================================
double AccountValue::GetSepAcctAssetsInforce() const
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return 0.0;
        }

    return SepAcctValueAfterDeduction * InvariantValues().InforceLives[Year];
}

//============================================================================
double AccountValue::GetNetCoiChargeInforce() const
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return 0.0;
        }

    return NetCoiCharge * InforceLivesBoy();
}

//============================================================================
double AccountValue::GetCurtateNetClaimsInforce()
{
    if(!Input_->UsePartialMort || ItLapsed || BasicValues::GetLength() <= Year)
        {
        return 0.0;
        }
    return YearsNetClaims * InvariantValues().InforceLives[Year];
}

//============================================================================
double AccountValue::GetProjectedCoiChargeInforce()
{
    if
        (   ItLapsed
        ||  BasicValues::GetLength() <= Year
        ||  !Input_->UseExperienceRating
        ||  !e_currbasis == ExpAndGABasis
        )
        {
        return 0.0;
        }
    return NextYearsProjectedCoiCharge * InforceLivesEoy();
}

//============================================================================
// The experience-rating mortality reserve isn't actually held in
// individual certificates: it really exists only at the case level.
// Yet it is apportioned among certificates in order to conform to the
// design invariant that a composite is a weighted sum of cells.
//
// The return value, added across cells, should reproduce the total
// total reserve at the case level, as the caller may assert.
//
double AccountValue::ApportionNetMortalityReserve
    (double reserve_per_life_inforce
    )
{
    if
        (   ItLapsed
        ||  BasicValues::GetLength() <= Year
        ||  !Input_->UseExperienceRating
        ||  !e_currbasis == ExpAndGABasis
        )
        {
        return 0.0;
        }

    double inforce_factor =
        (0.0 != Input_->NumIdenticalLives)
        ? InforceLivesEoy() / Input_->NumIdenticalLives
        : 0.0
        ;

    double apportioned_reserve = reserve_per_life_inforce * inforce_factor;

    // The experience-rating reserve can't be posted to the ledger in
    // FinalizeYear(), which is run before the reserve is calculated.
    // The projected COI charge and K factor are posted to the ledger
    // here as well, simply for uniformity.
    VariantValues().ExperienceReserve [Year] = apportioned_reserve;
    VariantValues().ProjectedCoiCharge[Year] = NextYearsProjectedCoiCharge;
    VariantValues().KFactor           [Year] = case_k_factor;

    return apportioned_reserve * Input_->NumIdenticalLives;
}

//============================================================================
double AccountValue::InforceLivesBoy() const
{
    if
        (   ItLapsed
        ||  BasicValues::GetLength() <= Year
        )
        {
        return 0.0;
        }
    unsigned int index = Year;
    LMI_ASSERT(index < InvariantValues().InforceLives.size());
    return InvariantValues().InforceLives[index];
}

//============================================================================
double AccountValue::InforceLivesEoy() const
{
    if
        (   ItLapsed
        ||  BasicValues::GetLength() <= Year
        )
        {
        return 0.0;
        }
    unsigned int index = 1 + Year;
    LMI_ASSERT(index < InvariantValues().InforceLives.size());
    return InvariantValues().InforceLives[index];
}

//============================================================================
void AccountValue::CoordinateCounters()
{
    MonthsSinceIssue = Month + 12 * Year;

    if(daily_interest_accounting)
        {
        calendar_date current_anniversary = Input_->EffDate;
        current_anniversary.add_years(Year, true);
        calendar_date next_anniversary = Input_->EffDate;
        next_anniversary.add_years(1 + Year, true);
        // This alternative
        //   days_in_policy_year = current_anniversary.days_in_year();
        // is not used: a policy issued in a leap year after its leap
        // day is unaffected by that leap day in the approximate
        // algorithm used here.
        days_in_policy_year =
                next_anniversary.julian_day_number()
            -   current_anniversary.julian_day_number()
            ;
        LMI_ASSERT(365 <= days_in_policy_year && days_in_policy_year <= 366);

        calendar_date current_monthiversary = Input_->EffDate;
        current_monthiversary.add_years_and_months(Year, Month, true);
        calendar_date next_monthiversary = Input_->EffDate;
        next_monthiversary.add_years_and_months(Year, 1 + Month, true);
        days_in_policy_month =
                next_monthiversary.julian_day_number()
            -   current_monthiversary.julian_day_number()
            ;

        LMI_ASSERT(28 <= days_in_policy_month && days_in_policy_month <= 31);

//        LMI_ASSERT
//            (   days_in_policy_month
//            ==  current_monthiversary.days_in_month()
//            );
/*
// This would be wrong for ill-conditioned effective dates.
// Example:
//   effective date 2000-03-29
//   tenth    monthiversary 2001-01-29
//   eleventh monthiversary 2001-02-28 (not the twenty-ninth)
        if(days_in_policy_month !=  current_monthiversary.days_in_month())
            {
            warning()
                << Year << " Year; " << Month << " Month\n"
                << current_monthiversary.year() << '-'
                << current_monthiversary.month() << '-'
                << current_monthiversary.day() << " calendar date BOM\n"
                << days_in_policy_month << " days_in_policy_month does not equal\n"
                << current_monthiversary.days_in_month() << " current_monthiversary.days_in_month()\n"
                << LMI_FLUSH
                ;
            }
*/
        }
}

