// Account value.
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

// $Id: accountvalue.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "accountvalue.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "deathbenefits.hpp"
#include "inputs.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "rounding_rules.hpp"
#include "xenumtypes.hpp"

#include <algorithm> // std::max(), std::min()
#include <cmath>     // std::pow()
#include <numeric>   // std::accumulate()

namespace
{
    // This table lookup approach is faster than determining
    //   whether this is a modal payment date
    //     0 == Month % (12 / mode.value());
    //   or how many months until the next modal payment date
    //     1 + (11 - Month) % (12 / mode.value());
    // by arithmetic, by a large enough margin to make the
    // program noticeably faster.

    int const modal_pmt_this_month[4][12] =
        {
            { 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0} // A
        ,   { 1,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0} // S
        ,   { 1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0} // Q
        ,   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1} // M
        };
    int const months_to_next_modal_pmt[4][12] =
        {
            {12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1} // A
        ,   { 6,  5,  4,  3,  2,  1,  6,  5,  4,  3,  2,  1} // S
        ,   { 3,  2,  1,  3,  2,  1,  3,  2,  1,  3,  2,  1} // Q
        ,   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1} // M
        };

    int get_mode_index(e_mode const& a_mode)
    {
    switch(a_mode.value())
        {
        case e_annual:     return 0; // A
        case e_semiannual: return 1; // S
        case e_quarterly:  return 2; // Q
        case e_monthly:    return 3; // M
        default:
            {
            fatal_error()
                << "Case '"
                << a_mode.value()
                << "' not found."
                << LMI_FLUSH
                ;
            return 0;
            }
        }
    }
} // Unnamed namespace.

/*
TODO ??
We want transaction functions to be reorderable. That means each must be
atomic--reentrant, if you will. Is this feasible?

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

/*
TODO ?? Not used yet.
static double const GPTCorridor[101] =
{
//    0     1     2     3     4     5     6     7     8     9
    2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 00
    2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 10
    2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 20
    2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 30
    2.50, 2.43, 2.36, 2.29, 2.22, 2.15, 2.09, 2.03, 1.97, 1.91, // 40
    1.85, 1.78, 1.71, 1.64, 1.57, 1.50, 1.46, 1.42, 1.38, 1.34, // 50
    1.30, 1.28, 1.26, 1.24, 1.22, 1.20, 1.19, 1.18, 1.17, 1.16, // 60
    1.15, 1.13, 1.11, 1.09, 1.07, 1.05, 1.05, 1.05, 1.05, 1.05, // 70
    1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, // 80
    1.05, 1.04, 1.03, 1.02, 1.01, 1.00, 1.00, 1.00, 1.00, 1.00, // 90
    1.00                                                        // 100
};
*/

// INELEGANT !! Many member variables are initialized not in ctors,
// but rather in common initialization functions.

//============================================================================
AccountValue::AccountValue()
    :BasicValues()
    ,TLedger()
{
    Alloc(100); // TODO ?? Manifest constant
//    Init(); // TODO ?? Needed?
}

//============================================================================
AccountValue::AccountValue(InputParms const& input)
    :BasicValues(input)
    ,TLedger(BasicValues::GetLength())
{
    Alloc(BasicValues::GetLength());
//    Init(); // TODO ?? Needed?
}

//============================================================================
AccountValue::AccountValue(AccountValue const& obj)
    :BasicValues(obj)
    ,TLedger(BasicValues::GetLength())
{
    Alloc(BasicValues::GetLength());
    Copy(obj);
}

//============================================================================
AccountValue& AccountValue::operator=(AccountValue const& obj)
{
    if(this != &obj)
        {
        Destroy();
        Alloc(obj.BasicValues::GetLength());
        Copy(obj);
        }
    return *this;
}

//============================================================================
AccountValue::~AccountValue()
{
    Destroy();
}

//============================================================================
void AccountValue::Alloc(int len)
{
    GrossPmts  .resize(12);
    NetPmts    .resize(12);
    Corridor   .reserve(len);
    CurrValues = new TLedger(len);
    MdptValues = new TLedger(len);
    GuarValues = new TLedger(len);
}

//============================================================================
void AccountValue::Copy(AccountValue const& obj)
{
    Database          = obj.Database; // TODO ?? Reconsider this.
    GrossPmts         = obj.GrossPmts;
    NetPmts           = obj.NetPmts  ;
    Corridor          = obj.Corridor ;
    CurrValues        = obj.CurrValues;
    MdptValues        = obj.MdptValues;
    GuarValues        = obj.GuarValues;
    OverridingPmts    = obj.OverridingPmts;
}

//============================================================================
void AccountValue::Destroy()
{
    delete CurrValues;
    delete MdptValues;
    delete GuarValues;
}

//============================================================================
void AccountValue::Init()
{
}

//============================================================================
double AccountValue::Run(ERunBasis RunBasis)
{
/*
First run current, for solves and strategies. This determines
    payments
    specamt (increases, decreases, option changes)
    and hence surrchg
    These should be saved...where? *
Then run guaranteed.
Then run midpoint

TODO ?? More generally--what do we need Run() to do?
    guar and midpt values are useful only for illustrations
    for profit testing we want to avoid their overhead
    for solves we want only one run
        if solving on guar basis...does *that* basis determine pmts & specamt?
        it prolly should, so that the guar columns will show what's wanted
            otherwise the solve lacks meaning
        although I wonder how other illustration systems handle this
        it should also be possible to solve on a midpt basis as well

TODO ?? * saved...where
    the answer to this above question is "here"
    this function should take an enum arg
        CurrBasisOnly
        GuarAndCurrBasesOnly
        AllBases
*/

    TLedger::Init(this);
    OverridingPmts = Pmt;

    double z = 0.0;
    Solving = e_solve_none != Input->SolveType.value();
    switch(RunBasis)
        {
        case CurrBasisOnly:
            {
            z = RunOneBasis(e_currbasis);
            }
            break;
        case MdptBasisOnly:
            {
            z = RunOneBasis(e_mdptbasis);
            }
            break;
        case GuarBasisOnly:
            {
            z = RunOneBasis(e_guarbasis);
            }
            break;
        case AllBases:
            {
            // Do them all
            z = RunAllBases();
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << RunBasis
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    return z;
}

/*
If not solving
    if running one basis
        just do that basis
    if running all bases
        run all bases
*/

//============================================================================
double AccountValue::RunOneBasis(enum_basis TheBasis)
{
    double z;
    if(Solving)
        {
        LMI_ASSERT(TheBasis == Input->SolveBasis.value()); // TODO ?? Do something more flexible?
        z = Solve();
        }
    else
        z = PerformRun(TheBasis);
    return z;
}

//============================================================================
double AccountValue::RunAllBases()
{
    // set pmts, specamt, surrchg
    double z = 0.0;
    if(Solving) switch(Input->SolveBasis.value())
        {
        case e_currbasis:
            {
            z = Solve();
            OverridingPmts = Pmt;
            *CurrValues = *this;
            Solving = false;
            }
            break;
        case e_guarbasis:
            {
            z = Solve();
            OverridingPmts = Pmt;
            *GuarValues = *this;
            Solving = false;
            }
            break;
        case e_mdptbasis:
            {
            z = Solve();
            OverridingPmts = Pmt;
            *MdptValues = *this;
            Solving = false;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input->SolveBasis.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    RunOneBasis(e_currbasis);
    *CurrValues = *this;
    RunOneBasis(e_guarbasis);
    *GuarValues = *this;
    RunOneBasis(e_mdptbasis);
    *MdptValues = *this;

    return z;
}

//============================================================================
double AccountValue::PerformRun(enum_basis TheBasis)
{
    TLedger::Init(this);    // TODO ?? Shouldn't be here.

    // TODO ?? This should be a command?
    Debugging       = false;

    // TODO ?? These three things belong in input struct.
    int InforceYear  = 0;
    int InforceMonth = 0;
    InforceAV        = 0.0;

    ItLapsed         = false;
    LapseMonth       = 0;
    LapseYear        = 0;

    Basis            = 0.0;
    CumTgtPrem       = 0.0;
    CumMinPrem       = 0.0;

    MaxLoan          = 0.0;
    RegLnBal         = 0.0;
    PrfLnBal         = 0.0;
    AVRegLn          = 0.0;
    AVPrfLn          = 0.0;

    AVUnloaned = InforceAV - (AVRegLn + AVPrfLn);

// TODO ?? For GPT:
//    Corridor = GPTCorridor + BasicValues::GetIssueAge();

    Corridor = actuarial_table
        (CurrentTableFile()
        ,static_cast<long int>(Database->Query(DB_CorridorTable))
        ,Input->Status[0].IssueAge.value()
        ,BasicValues::GetLength()
        );

    WPRates = actuarial_table
        (CurrentTableFile()
        ,static_cast<long int>(Database->Query(DB_WPTable))
        ,Input->Status[0].IssueAge.value()
        ,BasicValues::GetLength()
        );

    ADDRates = actuarial_table
        (CurrentTableFile()
        ,static_cast<long int>(Database->Query(DB_ADDTable))
        ,Input->Status[0].IssueAge.value()
        ,BasicValues::GetLength()
        );

    PerformSpecAmtStrategy();

    for(Year = InforceYear; Year < BasicValues::GetLength(); Year++)
        {
        if(!ItLapsed)
            {
            DoYear(TheBasis, Year, (Year == InforceYear) ? InforceMonth : 0);
            }
        }

    return AVUnloaned + AVRegLn + AVPrfLn;
}

//============================================================================
void AccountValue::DoYear
    (enum_basis a_TheBasis
    ,int        a_Year
    ,int        a_InforceMonth
    )
{
    Year = a_Year; // TODO ?? expunge?

// Variables like "Pmt" come from base class TLedger
// TODO ?? Solve...() should reset not inputs but...??

    // TODO ?? These variables are set in current run and used in guar and midpt
    coirate         = MortalityRates_->MonthlyCoiRates(a_TheBasis)[Year];

    wprate          = WPRates[Year];
    addrate         = ADDRates[Year];
    haswp           = Input->Status[0].HasWP.value();
    hasadd          = Input->Status[0].HasADD.value();

    mlycurrint      = InterestRates_->GenAcctNetRate
        (e_basis(a_TheBasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;

    pmt             = Pmt[Year];
    premloadtgt     = Loads_->target_premium_load(a_TheBasis)[Year];
    polfee          = Loads_->monthly_policy_fee(a_TheBasis)[Year];
    actualspecamt   = SpecAmt[Year];

    // These variables are set for each pass independently
    mode            = Mode[Year];
    ModeIndex       = get_mode_index(mode);
    loan            = Outlay_->new_cash_loans()[Year];
    wd              = Outlay_->withdrawals()[Year];
    mlyguarv        =
            1.0
        /   (1.0 + InterestRates_->GenAcctNetRate
                (e_basis(e_guarbasis)
                ,e_rate_period(e_monthly_rate)
                )[Year]
            );
    inputspecamt    = DeathBfts->GetSpecAmt()[Year];
    dbopt           = DeathBfts->GetDBOpt()[Year];

    // for guar basis run, what loan rates do we use?
    MlyRegLnCredIntRate = InterestRates_->RegLnCredRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;
    MlyRegLnDueIntRate  = InterestRates_->RegLnDueRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;
    MlyPrfLnCredIntRate = InterestRates_->PrfLnCredRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;
    MlyPrfLnDueIntRate  = InterestRates_->PrfLnDueRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;

    CorridorFactor = Corridor[Year];

    GrossPmts  .assign(12, 0.0);
    NetPmts    .assign(12, 0.0);

    // TODO ?? strategy here?

    for(Month = a_InforceMonth; Month < 12; Month++)
        {
        DoMonth();
        if(ItLapsed)
            {
            return;
            }
        }

    AcctVal[Year] = AVUnloaned + AVRegLn + AVPrfLn;
    CSV[Year] = AcctVal[Year] - SurrChg[Year];
    // Update death benefit: "deathbft" currently holds benefit as of the
    //   beginning of month 12, but we want it as of the end of that month,
    //   in case the corridor or option 2 drove it up during the last month.
    //   TODO ?? needs end of year corridor factor, if it varies monthly?
    TxSetDeathBft();
    EOYDeathBft[Year] = deathbft;

    // TODO ?? change one of these names?
    GrossPmt[Year] += std::accumulate(GrossPmts.begin(), GrossPmts.end(), 0.0);

    if(Debugging)
        {
//        DebugPrint(); // TODO ?? need stream arg
        }
}

//============================================================================
// Each month, process all transactions in order
void AccountValue::DoMonth()
{
    if(ItLapsed)
        {
        return;
        }

    // TODO ?? General needs:
    // multiple lives
    // testing plan; regression testing; walkthrough
        // debug detail
    // printing: TRichEdit
    // file open and save: like Options|Save?
    //   validation?
    // online help: TRichEdit authoring tool?

// TODO ?? multiple lives

// TODO ??
    // display: spreadsheet with each life's parms listed in cols
    // cmds:
        // Import
            // or enter directly in SS
        // Input
            // Which parms vary by life?
            // Duplicate those that don't ("Rules")
        // Validate (why's this hard?)
        // Run
            // array of...inputparms?
            // one ledger to hold composite
            // one inputparms thing to hold group scenario?
            // calculate current, midpoint, guar
            // what to show on SS when each one done?
                // nothing--show progress dialog instead
        // Ledger
            // SS window with ledger for one life
        // Print all
            // Run all and print
            //Summary

    // TODO ?? questions and comments:
    //   rounding of premium load

    // TODO ?? not yet implemented:
    //   min and max prem (yearly and cumulative)
    //   excess loan
    //   where is loan capitalized?
    //   solve for WD then loan
    //   solve for endt?
    //   debug detail
    //   TAMRA
    //   CVT corridor
    //   rounding
    //   ADD and WP rider rates; WP rider duration
    //   preferred loans; loan parameterization
    //   guar accum
    //     curr SA, SC, pmts for guar
    //   general vs separate account

    // TODO ?? NOT NEEDED IMMEDIATELY
    //   no lapse period
    //     increase premium during no lapse period, for solves at least
    //   monthly varying corridor
    //   tiered premium loads
    //   load per thousand
    //   surrender charges
    //   GPT
    //   multiple layers of coverage

    TxOptChg();
    TxSpecAmtChg();
    TxSet7702();        // Not yet implemented
    TxPmt();            // Incomplete
    TxLoanRepay();      // Not called...

    TxSetBOMAV(); // TODO ?? Do we need this function?
    TxSetCOI();
    TxSetRiderDed();
    TxDoMlyDed();

    TxCreditInt();

    TxLoanInt();
    TxTakeWD();
    TxTakeLoan();

    TxTestLapse();
    TxDebug(); // TODO ?? Not yet implemented.
}

//============================================================================
inline bool AccountValue::IsModalPmtDate()
{
    return modal_pmt_this_month[ModeIndex][Month];
}

//============================================================================
inline int AccountValue::MonthsToNextModalPmtDate()
{
    return months_to_next_modal_pmt[ModeIndex][Month];
}

//============================================================================
// Sets spec amt according to selected strategy, in every year
void AccountValue::PerformSpecAmtStrategy()
{
    double SA = 0.0;
    switch(Input->SAStrategy.value())
        {
        case e_sainputscalar:
            {
            SA = SpecAmt[0];
            }
            break;
        case e_sainputvector: // Obsolete.
            {
            fatal_error()
                << "Varying specified amount not implemented."
                << " Specified amount set to scalar input value."
                << LMI_FLUSH
                ;
            SA = SpecAmt[0];
            }
            break;
        case e_samaximum:
            {
            SA = GetModalMaxSpecAmt(Mode[0], Pmt[0]);
            }
            break;
        case e_satarget:
            {
            SA = GetModalTgtSpecAmt(Mode[0], Pmt[0]);
            }
            break;
        case e_samep:
            {
            fatal_error()
                << "Modified endowment premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            SA = SpecAmt[0];
            }
            break;
        case e_saglp:
            {
            fatal_error()
                << "Guideline level premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            SA = SpecAmt[0];
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input->SAStrategy.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    SA = GetRoundingRules().round_specamt()(SA);

    for(int j = 0; j < BasicValues::GetLength(); j++)
        {
        SpecAmt[j] = SA;
        }
}

//============================================================================
// Death benefit option change
// Assumes surrender charge is not affected by this transaction
// Assumes target premium rate is not affected by this transaction
// Assumes change to option 2 mustn't decrease spec amt below minimum
void AccountValue::TxOptChg()
{
    // Illustrations allow option changes only on anniversary
    //   but not on zeroth anniversary
    if(0 != Month || 0 == Year)
        {
        return;
        }

    // Nothing to do if no option change requested
    if(dbopt.value() == DeathBfts->GetDBOpt()[Year - 1].value())
        {
        return;
        }

    // Nothing to do unless AV is positive
    double AV = AVUnloaned + AVRegLn + AVPrfLn;
    if(AV <= 0.0)
        {
        return;
        }

    // Change specified amount, keeping amount at risk invariant
    switch(dbopt.value())
        {
        case e_option1:
            {
            // Option 1: increase spec amt by AV
            actualspecamt += AV;
            }
            break;
        case e_option2:
            {
            // Option 2: decrease spec amt by AV, but not below min spec amt
            actualspecamt -= AV;
            actualspecamt = std::max(actualspecamt, MinSpecAmt);
            // An alternative is to lapse the policy.
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << dbopt.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    actualspecamt = GetRoundingRules().round_specamt()(actualspecamt);

    // Carry the new spec amt forward into all future years
    for(int j = Year; j < BasicValues::GetLength(); j++)
        {
        SpecAmt[j] = actualspecamt;
        }
}

//============================================================================
// Specified amount change: increase or decrease
// Ignores multiple layers of coverage: not correct for sel & ult COIs
// Assumes target premium rate is not affected by increases or decreases
void AccountValue::TxSpecAmtChg()
{
// Make sure this is the right place to do this
    // Illustrations allow increases and decreases only on anniversary
    //   but not on zeroth anniversary
    if(0 != Month || 0 == Year)
        {
        return;
        }

    // Nothing to do if no increase or decrease requested
    if(DeathBfts->GetSpecAmt()[Year] == DeathBfts->GetSpecAmt()[Year - 1])
        {
        return;
        }

    // Change specified amount
    actualspecamt = std::max(MinSpecAmt, DeathBfts->GetSpecAmt()[Year]);

    // Carry the new spec amt forward into all future years        
    for(int j = Year; j < BasicValues::GetLength(); j++)
        {
        SpecAmt[j] = actualspecamt;
        }
}

//============================================================================
void AccountValue::TxSet7702()
{
// SOMEDAY !! Need 7702 implementation.
}

//============================================================================
// Sets payment according to selected strategy, in each non-solve year
void AccountValue::PerformPmtStrategy(double* a_Pmt)
{
    if
        (
           e_solve_ee_prem == Input->SolveType.value()
        && Year >= Input->SolveBegYear.value()
        && Year < std::min
            (Input->SolveEndYear.value()
            ,BasicValues::GetLength()
            )
        )
        // Don't override premium during solve period
        return;

    switch(Input->EePmtStrategy.value())
        {
        case e_pmtinputscalar:
            {
            *a_Pmt = Pmt[Year];
            }
            break;
        case e_pmtinputvector:
            {
            fatal_error()
                << "Varying premium not implemented."
                << " Payment set to scalar input value"
                << LMI_FLUSH
                ;
            *a_Pmt = Pmt[Year];
            }
            break;
        case e_pmtminimum:
            {
            *a_Pmt = GetModalMinPrem
                (Year
                ,mode
                ,actualspecamt
                );
            }
            break;
        case e_pmttarget:
            {
            *a_Pmt = GetModalTgtPrem
                (Year
                ,mode
                ,actualspecamt
                );
            }
            break;
        case e_pmtmep:
            {
            fatal_error()
                << "Modified endowment premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            *a_Pmt = Pmt[Year];
            }
            break;
        case e_pmtglp:
            {
            fatal_error()
                << "Guideline level premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            *a_Pmt = Pmt[Year];
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input->EePmtStrategy.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
}

//============================================================================
// Process premium payment reflecting premium load
// Contains hooks for guideline premium test; they need to be fleshed out
// Ignores strategies such as pay guideline max--see PerformPmtStrategy()
// Ignores no-lapse periods and other death benefit guarantees
// Some systems force monthly premium to be integral cents even though
//   mode is not monthly; TODO ?? is this something we need to do here?
// TODO ?? Tiered premium loads not implemented
void AccountValue::TxPmt()
{
    // Do nothing if this is not a modal payment date
    if(!IsModalPmtDate())
        {
        return;
        }

//    TODO ?? double ForceOut = 0.0;
//    TODO ?? double GuidelinePremLimit = 0.0;

    // Pay premium
    PerformPmtStrategy(&pmt);
    GrossPmts[Month] = pmt;
    if(0 == Year && 0 == Month)
        {
        double TotalDumpin =
              Outlay_->dumpin()
            + Outlay_->external_1035_amount()
            + Outlay_->internal_1035_amount()
            ;
        GrossPmts[Month] += TotalDumpin;
        }

    // TODO ?? Test maximum premium.
    // TODO ?? Round it with Rounding.RoundMAXP .
//            (DB-AV)/CorridorFactor - AV

    // Subtract premium load from gross premium yielding net premium
    NetPmts[Month] = GetRoundingRules().round_net_premium()
        (GrossPmts[Month] * (1.0 - premloadtgt)
        );
    // Should we instead do the following?
    //   calculate load as pmt * load rate and round the result
    //   subtract from pmt

    // Add net premium to unloaned account value
    AVUnloaned += NetPmts[Month];
}

//============================================================================
void AccountValue::TxLoanRepay()
{
// loan repayment
    // Illustrations allow loan repayment only on anniversary
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no loan repayment requested
    if(0.0 <= loan)
        {
        return;
        }

    AVUnloaned -= loan;
    AVRegLn += loan;    // TODO ?? also preferred...
    Loan[Year] = loan;
}

//============================================================================
// Set account value before monthly deductions
// TODO ?? Should this function live?
void AccountValue::TxSetBOMAV()
{
    AVUnloaned -= polfee;
}

//============================================================================
// Set death benefit reflecting corridor and option 2
void AccountValue::TxSetDeathBft()
{
    // Total account value is unloaned plus loaned
    double AV = AVUnloaned + AVRegLn + AVPrfLn;

    // Set death benefit reflecting corridor and death benefit option
    switch(dbopt.value())
        {
        case e_option1:
            {
            // Option 1: specamt, or corridor times AV if greater
            deathbft = std::max(actualspecamt, CorridorFactor * AV);
            }
            break;
        case e_option2:
            // Option 2: specamt plus AV, or corridor times AV if greater
            // Note negative AV doesn't decrease death benefit
            deathbft = std::max
                (actualspecamt + std::max(0.0, AV)
                ,CorridorFactor * AV
                );
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << dbopt.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    deathbft = GetRoundingRules().round_death_benefit()(deathbft);

    // TODO ?? Here we should accumulate average death benefit for profit testing
}

//============================================================================
// Set cost of insurance charge
void AccountValue::TxSetCOI()
{
    TxSetDeathBft();
    // Net amount at risk: death benefit discounted one month at
    //   guaranteed interest, minus account value if nonnegative
    // Note negative AV doesn't increase NAAR

    NAAR = GetRoundingRules().round_naar()
        (deathbft * mlyguarv - (AVUnloaned + AVRegLn + AVPrfLn)
        );

    // Cost of insurance is COI rate times net amount at risk
    COI = GetRoundingRules().round_coi_charge()(NAAR * coirate);
}

//============================================================================
// Calculate rider charges
void AccountValue::TxSetRiderDed()
{
    WPChg = 0.0;
    if(haswp)
        {
        WPChg = wprate * (COI + polfee + ADDChg);
        }

    ADDChg = 0.0;
    if(hasadd)
        {
        ADDChg = addrate * std::min(500000.0, actualspecamt);
        }
}

//============================================================================
// Subtract monthly deductions from unloaned account value
void AccountValue::TxDoMlyDed()
{
    // Subtract mortality and rider deductions from unloaned account value
    AVUnloaned -= COI + ADDChg + WPChg;
    mlyded = polfee + COI + ADDChg + WPChg;
    mlydedtonextmodalpmtdate = mlyded * MonthsToNextModalPmtDate();
}

//============================================================================
// Credit interest on loaned and unloaned account value separately
void AccountValue::TxCreditInt()
{
    // Accrue interest on unloaned and loaned account value separately
    //   but do not charge interest on negative account value
    if(0.0 < AVUnloaned)
        {
        // TODO ?? Should each interest increment be rounded separately?
        double z = GetRoundingRules().round_interest_credit()
            (AVUnloaned * mlycurrint
            );
        AVUnloaned += z;
        }
    LMI_ASSERT(0.0 <= AVRegLn + AVPrfLn); // loaned account value cannot be negative
// TODO ??   TxLoanInt() handles loan interest
}

//============================================================================
// Accrue loan interest
void AccountValue::TxLoanInt()
{
    // Nothing to do if there's no loan outstanding
    if(0.0 == RegLnBal && 0.0 == PrfLnBal)
        {
        return;
        }

    // We may want to display credited interest separately.
    // TODO ?? Should each interest increment be rounded separately
    RegLnIntCred = GetRoundingRules().round_interest_credit()
        (AVRegLn * MlyRegLnCredIntRate
        );
    PrfLnIntCred = GetRoundingRules().round_interest_credit()
        (AVPrfLn * MlyPrfLnCredIntRate
        );

    AVRegLn += RegLnIntCred;
    AVPrfLn += PrfLnIntCred;

    double RegLnIntAccrued = GetRoundingRules().round_interest_credit()
        (RegLnBal * MlyRegLnDueIntRate
        );
    double PrfLnIntAccrued = GetRoundingRules().round_interest_credit()
        (PrfLnBal * MlyPrfLnDueIntRate
        );

    RegLnBal += RegLnIntAccrued;
    PrfLnBal += PrfLnIntAccrued;
}

//============================================================================
// TODO ?? Min AV after WD not implemented--does max WD calculation take care of it?
void AccountValue::TxTakeWD()
{
    // Illustrations allow withdrawals only on anniversary
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no withdrawal requested
    if(0.0 == wd)
        {
        return;
        }

    // For solves, we may wish to ignore min and max

    // Impose minimum amount (if nonzero) on withdrawals
    if(wd < MinWD)
        {
        wd = 0.0;
        }

    // Impose maximum amount
    // If maximum exceeded, limit it.
    // TODO ?? Max WD and max loan formulas treat loan interest differently
    //   max WD on a loaned policy: cannot become overloaned until next
    //     modal premium date
    //   max loan: cannot become overloaned until end of policy year
    double MaxWD
        = AVUnloaned
        + (AVRegLn  + AVPrfLn)
        - (RegLnBal + PrfLnBal)
        - mlydedtonextmodalpmtdate;
    if(MaxWD < wd)
        {
        wd = MaxWD;
        }

    AVUnloaned -= wd;

    switch(dbopt.value())
        {
        case e_option1:
            {
            // TODO ?? Spec amt reduced for option 1 even if in corridor?
            //   --taken care of by max WD formula
            // TODO ?? If WD causes spec amt < min spec amt, do we:
            //   set spec amt = min spec amt?
            //   reduce the WD?
            //   lapse the policy?
// TODO ??            actualspecamt = std::min(actualspecamt, deathbft - wd);
            actualspecamt -= wd;
            actualspecamt = std::max(actualspecamt, MinSpecAmt);
            actualspecamt = GetRoundingRules().round_specamt()(actualspecamt);
            // TODO ?? If WD causes AV < min AV, do we:
            //   reduce the WD?
            //   lapse the policy?
            // Maybe it can't happen cuz of max WD defn?

            // Carry the new spec amt forward into all future years
            for(int j = Year; j < BasicValues::GetLength(); j++)
                {
                SpecAmt[j] = actualspecamt;
                }
            }
            break;
        case e_option2:
            ;
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << dbopt.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // Deduct withdrawal fee
    wd -= std::min(WDFee, wd * WDFeeRate);
    // TODO ?? This treats input WD as gross; it prolly should be net

    WD[Year] = wd;
// TODO ??    Basis -= wd;
}

//============================================================================
// Take a new loan
void AccountValue::TxTakeLoan()
{
    // Illustrations allow loans only on anniversary
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no loan requested
    if(loan <= 0.0)
        {
        return;
        }

    // Impose maximum amount.
    // If maximum exceeded...limit it.
    // TODO ?? For solves, we may wish to ignore max.
    MaxLoan
        = AVUnloaned * 0.9    // TODO ?? icky manifest constant
        // - surrchg
        + (AVRegLn + AVPrfLn)
        - RegLnBal * (std::pow((1.0 + MlyRegLnDueIntRate), 12 - Month) - 1.0)
        - PrfLnBal * (std::pow((1.0 + MlyPrfLnDueIntRate), 12 - Month) - 1.0)
        - mlydedtonextmodalpmtdate;
    // Interest adjustment: d upper n where n is # months remaining in year
    //   witholding this keeps policy from becoming overloaned before year end
    double IntAdj = std::pow((1.0 + MlyRegLnDueIntRate), 12 - Month);
    IntAdj = (IntAdj - 1.0) / IntAdj;
    MaxLoan *= 1.0 - IntAdj;
    MaxLoan = std::max(0.0, MaxLoan);
    MaxLoan = GetRoundingRules().round_loan()(MaxLoan);

    // TODO ?? preferred loan calculations go here

    // update loan AV, loan bal

    if(MaxLoan < loan)
        {
        loan = MaxLoan;
        }

    AVUnloaned -= loan;
    AVRegLn += loan;    // TODO ?? also preferred...
    Loan[Year] = loan;
}

//============================================================================
// Test for lapse
void AccountValue::TxTestLapse()
{
// TODO ?? Don't say 'basis' when 'cum prem' is meant.
//    BasisMin += minprem;
//    BasisTgt += tgtprem;
// Perform no-lapse test

    // If we're doing a solve, don't let it lapse--otherwise lapse would
    // introduce a discontinuity in the function for which we seek a root
    if(Solving)
        {
        return;
        }

    // Otherwise if AV is negative or if overloaned, then lapse the policy
    else if
        (
            (AVUnloaned + AVRegLn + AVPrfLn < 0.0)
        ||  (MaxLoan < RegLnBal + PrfLnBal)
        )
        {
        LapseMonth = Month;
        LapseYear = Year;
        ItLapsed = true;
        }
}

//============================================================================
void AccountValue::TxDebug()
{
// TODO ?? Not yet implemented.
}


/*
TODO ??

Lots of small classes?
    DB, pmts, mort rates, int rates
    small classes easier to understand
    use them only for input, or for operations?

Coherence?
    example: payment needs loads (const), and pmts and 7702 (not const)
    example: COI ded needs DB, mort, int (all const); yet subdivide further:
        Set DB (needs 7702)
        Set NAAR (needs DB, int)
        Set COI

Slower because of extra indirection
    get references at top of loop? breaks encapsulation

*/

