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

// $Id: accountvalue.cpp,v 1.16 2005-09-07 03:04:54 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include "alert.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "inputs.hpp"
#include "interest_rates.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
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

//============================================================================
AccountValue::AccountValue(InputParms const& input)
    :BasicValues       (input)
    ,DebugFilename     ("anonymous.debug")
    ,ledger_(new Ledger(BasicValues::GetLedgerType(), BasicValues::GetLength()))
    ,ledger_invariant_ (new LedgerInvariant(BasicValues::GetLength()))
    ,ledger_variant_   (new LedgerVariant  (BasicValues::GetLength()))
{
    GrossPmts  .resize(12);
    NetPmts    .resize(12);
}

//============================================================================
AccountValue::~AccountValue()
{
}

//============================================================================
double AccountValue::RunAV()
{
    InvariantValues().Init(this);
    OverridingPmts = InvariantValues().EePmt;
    Solving = e_solve_none != Input_->SolveType.value();
    return RunAllApplicableBases();
}

//============================================================================
double AccountValue::RunOneBasis(e_run_basis const& TheBasis)
{
    double z;
    if(Solving)
        {
        // TODO ?? Do something more flexible?
        // TODO ?? Isn't this unreachable?
        throw std::logic_error("This line had seemed to be unreachable.");
        LMI_ASSERT(TheBasis == Input_->SolveBasis.value());
        z = Solve();
        }
    else
        {
        z = RunOneCell(TheBasis);
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
    // set pmts, specamt, surrchg

    // Separate-account basis hardcoded because separate account not supported.
    e_run_basis run_basis;
    set_run_basis_from_separate_bases
        (run_basis
        ,e_basis(Input_->SolveBasis.value())
        ,e_sep_acct_basis(e_sep_acct_full)
        );

    double z = 0.0;
    if(Solving)
        {
        z = Solve();
        OverridingPmts = InvariantValues().EePmt;
        ledger_->SetOneLedgerVariant(run_basis, VariantValues());
        Solving = false;
        }
    ledger_->SetLedgerInvariant(InvariantValues());

    run_basis = e_run_basis(e_run_curr_basis);
    RunOneBasis(run_basis);
    ledger_->SetOneLedgerVariant(run_basis, VariantValues());

    run_basis = e_run_basis(e_run_guar_basis);
    RunOneBasis(run_basis);
    ledger_->SetOneLedgerVariant(run_basis, VariantValues());

    run_basis = e_run_basis(e_run_mdpt_basis);
    RunOneBasis(run_basis);
    ledger_->SetOneLedgerVariant(run_basis, VariantValues());

    return z;
}

//============================================================================
double AccountValue::RunOneCell(e_run_basis const& TheBasis)
{
    if(Solving)
        {
        // TODO ?? This seems wasteful. Track down the reason for doing it.
        InvariantValues().Init(this);
        }
    set_separate_bases_from_run_basis(TheBasis, ExpAndGABasis, SABasis);
    VariantValues().Init(this, ExpAndGABasis, SABasis);

    Debugging       = false;

    InforceYear      = Input_->InforceYear;
    InforceMonth     = Input_->InforceMonth;
    InforceAVGenAcct = Input_->InforceAVGenAcct;

    ItLapsed         = false;
    LapseMonth       = 0;
    LapseYear        = 0;

    TaxBasis         = 0.0;

    MaxLoan          = 0.0;
    RegLnBal         = 0.0;
    PrfLnBal         = 0.0;
    AVRegLn          = 0.0;
    AVPrfLn          = 0.0;

    AVUnloaned = InforceAVGenAcct - (AVRegLn + AVPrfLn);

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
    (e_run_basis const& a_TheBasis
    ,int                a_Year
    ,int                a_InforceMonth
    )
{
    Year = a_Year; // TODO ?? expunge?
    set_separate_bases_from_run_basis(a_TheBasis, ExpAndGABasis, SABasis);

// TODO ?? Solve...() should reset not inputs but...??

    // TODO ?? These variables are set in current run and used in
    // guaranteed and midpoint runs.
    YearsCoiRate0   = MortalityRates_->MonthlyCoiRates(ExpAndGABasis)[Year];

    YearsWpRate     = MortalityRates_->WpRates()[Year];
    YearsAdbRate    = MortalityRates_->AdbRates()[Year];
    haswp           = Input_->Status[0].HasWP.value();
    hasadb          = Input_->Status[0].HasADD.value();

    YearsGenAcctIntRate = InterestRates_->GenAcctNetRate
        (e_basis(ExpAndGABasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;

    pmt             = InvariantValues().EePmt[Year];
    YearsPremLoadTgt= Loads_->target_premium_load(ExpAndGABasis)[Year];
    YearsMlyPolFee  = Loads_->monthly_policy_fee(ExpAndGABasis)[Year];
    ActualSpecAmt   = InvariantValues().SpecAmt[Year];

    // These variables are set for each pass independently.
    mode            = InvariantValues().EeMode[Year];
    ModeIndex       = get_mode_index(mode);
    RequestedLoan   = Outlay_->new_cash_loans()[Year];
    wd              = Outlay_->withdrawals()[Year];
    mlyguarv        =
            1.0
        /   (1.0 + InterestRates_->GenAcctNetRate
                (e_basis(e_guarbasis)
                ,e_rate_period(e_monthly_rate)
                )[Year]
            );
    YearsSpecAmt    = DeathBfts_->specamt()[Year];
    YearsDBOpt      = DeathBfts_->dbopt()[Year];

    // For guaranteed-basis run, what loan rates should be used?
    YearsRegLnIntCredRate = InterestRates_->RegLnCredRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;
    YearsRegLnIntDueRate  = InterestRates_->RegLnDueRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;
    YearsPrfLnIntCredRate = InterestRates_->PrfLnCredRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;
    YearsPrfLnIntDueRate  = InterestRates_->PrfLnDueRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_monthly_rate)
        )[Year]
        ;

    YearsCorridorFactor = BasicValues::GetCorridorFactor()[Year];

    GrossPmts  .assign(12, 0.0);
    NetPmts    .assign(12, 0.0);

    // TODO ?? Strategy here?

    for(Month = a_InforceMonth; Month < 12; Month++)
        {
        DoMonth();
        if(ItLapsed)
            {
            return;
            }
        }

    VariantValues().AcctVal[Year] = AVUnloaned + AVRegLn + AVPrfLn;
    VariantValues().CSVNet[Year] = VariantValues().AcctVal[Year] - VariantValues().SurrChg[Year];
    // Update death benefit: "deathbft" currently holds benefit as of the
    //   beginning of month 12, but we want it as of the end of that month,
    //   in case the corridor or option 2 drove it up during the last month.
    //   TODO ?? needs end of year corridor factor, if it varies monthly?
    TxSetDeathBft();
    VariantValues().EOYDeathBft[Year] = deathbft;

    // TODO ?? Change one of these names, which differ only in the terminal 's'.
    InvariantValues().GrossPmt[Year] += std::accumulate(GrossPmts.begin(), GrossPmts.end(), 0.0);

    if(Debugging)
        {
//        DebugPrint(); // TODO ?? Need stream argument.
        }
}

//============================================================================
// Each month, process all transactions in order.
void AccountValue::DoMonth()
{
    if(ItLapsed)
        {
        return;
        }

    // TODO ?? not yet implemented:
    //   rounding of premium load
    //   multiple lives
    //   min and max prem (yearly and cumulative)
    //   excess loan
    //   where is loan capitalized?
    //   solve for WD then loan
    //   solve for endt?
    //   debug detail
    //   7702A
    //   CVT corridor
    //   rounding
    //   ADB and WP rates; WP rider duration
    //   preferred loans; loan parameterization
    //   guar accum
    //     curr SA, SC, pmts for guar
    //   general vs separate account
    //
    // NOT NEEDED IMMEDIATELY
    //   no lapse period
    //     increase premium during no lapse period, for solves at least
    //   monthly varying corridor
    //   tiered premium loads
    //   load per thousand
    //   surrender charges
    //   GPT
    //   multiple layers of coverage

    TxOptionChange();
    TxSpecAmtChange();
//    TxTestGPT();        // Not yet implemented.
    TxPmt();            // TODO ?? Incomplete.
    TxLoanRepay();      // TODO ?? Not called.

    TxSetBOMAV();
    TxSetCoiCharge();
    TxSetRiderDed();
    TxDoMlyDed();

    TxCreditInt();

    TxLoanInt();
    TxTakeWD();
    TxTakeLoan();

    TxTestLapse();
    TxDebug();
}

//============================================================================
inline bool AccountValue::IsModalPmtDate()
{
    return modal_pmt_this_month[ModeIndex][Month];
}

//============================================================================
inline int AccountValue::MonthsToNextModalPmtDate() const
{
    return months_to_next_modal_pmt[ModeIndex][Month];
}

//============================================================================
// Set specamt according to selected strategy, in every year.
void AccountValue::PerformSpecAmtStrategy()
{
    double SA = 0.0;
    switch(Input_->SAStrategy.value())
        {
        case e_sainputscalar:
            {
            SA = InvariantValues().SpecAmt[0];
            }
            break;
        case e_sainputvector: // Obsolete.
            {
            fatal_error()
                << "Varying specified amount not implemented."
                << " Specified amount set to scalar input value."
                << LMI_FLUSH
                ;
            SA = InvariantValues().SpecAmt[0];
            }
            break;
        case e_samaximum:
            {
            SA = GetModalMaxSpecAmt(InvariantValues().EeMode[0], InvariantValues().EePmt[0]);
            }
            break;
        case e_satarget:
            {
            SA = GetModalTgtSpecAmt(InvariantValues().EeMode[0], InvariantValues().EePmt[0]);
            }
            break;
        case e_samep:
            {
            fatal_error()
                << "Modified endowment premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            SA = InvariantValues().SpecAmt[0];
            }
            break;
        case e_saglp:
            {
            fatal_error()
                << "Guideline level premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            SA = InvariantValues().SpecAmt[0];
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input_->SAStrategy.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    SA = GetRoundingRules().round_specamt()(SA);

    for(int j = 0; j < BasicValues::GetLength(); j++)
        {
        InvariantValues().SpecAmt[j] = SA;
        }
}

//============================================================================
// Change death benefit option.
// Assumes surrender charge is not affected by this transaction.
// Assumes target premium rate is not affected by this transaction.
// Assumes change to option 2 mustn't decrease spec amt below minimum.
void AccountValue::TxOptionChange()
{
    // Illustrations allow option changes only on anniversary, but
    // not on zeroth anniversary.
    if(0 != Month || 0 == Year)
        {
        return;
        }

    // Nothing to do if no option change requested.
    if(YearsDBOpt.value() == DeathBfts_->dbopt()[Year - 1].value())
        {
        return;
        }

    // Nothing to do unless AV is positive.
    double AV = AVUnloaned + AVRegLn + AVPrfLn;
    if(AV <= 0.0)
        {
        return;
        }

    // Change specified amount, keeping amount at risk invariant.
    switch(YearsDBOpt.value())
        {
        case e_option1:
            {
            // Option 1: increase spec amt by AV.
            ActualSpecAmt += AV;
            }
            break;
        case e_option2:
            {
            // Option 2: decrease spec amt by AV, but not below min spec amt.
            ActualSpecAmt -= AV;
            ActualSpecAmt = std::max(ActualSpecAmt, MinSpecAmt);
            // An alternative is to lapse the policy.
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << YearsDBOpt.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    ActualSpecAmt = GetRoundingRules().round_specamt()(ActualSpecAmt);

    // Carry the new spec amt forward into all future years.
    for(int j = Year; j < BasicValues::GetLength(); j++)
        {
        InvariantValues().SpecAmt[j] = ActualSpecAmt;
        }
}

//============================================================================
// Specified amount change: increase or decrease.
// Ignores multiple layers of coverage: not correct for sel & ult COI rates.
// Assumes target premium rate is not affected by increases or decreases.
void AccountValue::TxSpecAmtChange()
{
// Make sure this is the right place to do this.
    // Illustrations allow increases and decreases only on anniversary
    //   but not on zeroth anniversary.
    if(0 != Month || 0 == Year)
        {
        return;
        }

    // Nothing to do if no increase or decrease requested.
    if(DeathBfts_->specamt()[Year] == DeathBfts_->specamt()[Year - 1])
        {
        return;
        }

    // Change specified amount.
    ActualSpecAmt = std::max(MinSpecAmt, DeathBfts_->specamt()[Year]);

    // Carry the new spec amt forward into all future years.
    for(int j = Year; j < BasicValues::GetLength(); j++)
        {
        InvariantValues().SpecAmt[j] = ActualSpecAmt;
        }
}

//============================================================================
// Set payment according to selected strategy, in each non-solve year.
void AccountValue::PerformPmtStrategy(double* a_Pmt)
{
    if
        (
            e_solve_ee_prem == Input_->SolveType.value()
        &&  Year >= Input_->SolveBegYear.value()
        &&  Year < std::min
                (Input_->SolveEndYear.value()
                ,BasicValues::GetLength()
                )
        )
        // Don't override premium during solve period.
        return;

    switch(Input_->EePmtStrategy.value())
        {
        case e_pmtinputscalar:
            {
            *a_Pmt = InvariantValues().EePmt[Year];
            }
            break;
        case e_pmtinputvector:
            {
            fatal_error()
                << "Varying premium not implemented."
                << " Payment set to scalar input value"
                << LMI_FLUSH
                ;
            *a_Pmt = InvariantValues().EePmt[Year];
            }
            break;
        case e_pmtminimum:
            {
            *a_Pmt = GetModalMinPrem
                (Year
                ,mode
                ,ActualSpecAmt
                );
            }
            break;
        case e_pmttarget:
            {
            *a_Pmt = GetModalTgtPrem
                (Year
                ,mode
                ,ActualSpecAmt
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
            *a_Pmt = InvariantValues().EePmt[Year];
            }
            break;
        case e_pmtglp:
            {
            fatal_error()
                << "Guideline level premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            *a_Pmt = InvariantValues().EePmt[Year];
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input_->EePmtStrategy.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
}

//============================================================================
// Process premium payment reflecting premium load.
// Contains hooks for guideline premium test; they need to be fleshed out.
// Ignores strategies such as pay guideline max--see PerformPmtStrategy().
// Ignores no-lapse periods and other death benefit guarantees.
// Some systems force monthly premium to be integral cents even though
//   mode is not monthly; TODO ?? is this something we need to do here?
// TODO ?? Tiered premium loads not implemented.
void AccountValue::TxPmt()
{
    // Do nothing if this is not a modal payment date.
    if(!IsModalPmtDate())
        {
        return;
        }

    // TODO ?? Put GPT stuff like forceout and premium limit here.

    // Pay premium.
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

    // TODO ?? Test maximum premium. Round it with Rounding.RoundMAXP .
//            (DB-AV)/YearsCorridorFactor - AV

    // Subtract premium load from gross premium yielding net premium.
    NetPmts[Month] = GetRoundingRules().round_net_premium()
        (GrossPmts[Month] * (1.0 - YearsPremLoadTgt)
        );
    // Should we instead do the following?
    //   calculate load as pmt * load rate and round the result
    //   subtract from pmt

    // Add net premium to unloaned account value.
    AVUnloaned += NetPmts[Month];
}

//============================================================================
void AccountValue::TxLoanRepay()
{
    // Illustrations allow loan repayment only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no loan repayment requested.
    if(0.0 <= RequestedLoan)
        {
        return;
        }

    AVUnloaned -= RequestedLoan;
    AVRegLn += RequestedLoan;    // TODO ?? Also preferred.
    InvariantValues().Loan[Year] = RequestedLoan;
}

//============================================================================
// Set account value before monthly deductions.
// TODO ?? Should this function live?
void AccountValue::TxSetBOMAV()
{
    AVUnloaned -= YearsMlyPolFee;
}

//============================================================================
// Set death benefit reflecting corridor and option 2.
void AccountValue::TxSetDeathBft(bool)
{
    // Total account value is unloaned plus loaned.
    double AV = AVUnloaned + AVRegLn + AVPrfLn;

    // Set death benefit reflecting corridor and death benefit option.
    switch(YearsDBOpt.value())
        {
        case e_option1:
            {
            // Option 1: specamt, or corridor times AV if greater.
            deathbft = std::max(ActualSpecAmt, YearsCorridorFactor * AV);
            }
            break;
        case e_option2:
            // Option 2: specamt plus AV, or corridor times AV if greater.
            // Negative AV doesn't decrease death benefit.
            deathbft = std::max
                (ActualSpecAmt + std::max(0.0, AV)
                ,YearsCorridorFactor * AV
                );
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << YearsDBOpt.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    deathbft = GetRoundingRules().round_death_benefit()(deathbft);

    // TODO ?? Here we should accumulate average death benefit for profit testing.
}

//============================================================================
// Set cost of insurance charge.
void AccountValue::TxSetCoiCharge()
{
    TxSetDeathBft();

    // Negative AV doesn't increase NAAR.
    NAAR = GetRoundingRules().round_naar()
        (deathbft * mlyguarv - (AVUnloaned + AVRegLn + AVPrfLn)
        );

    CoiCharge = GetRoundingRules().round_coi_charge()(NAAR * YearsCoiRate0);
}

//============================================================================
// Calculate rider charges.
void AccountValue::TxSetRiderDed()
{
    WpCharge = 0.0;
    if(haswp)
        {
        WpCharge = YearsWpRate * (CoiCharge + YearsMlyPolFee + AdbCharge);
        }

    AdbCharge = 0.0;
    if(hasadb)
        {
        AdbCharge = YearsAdbRate * std::min(500000.0, ActualSpecAmt);
        }
}

//============================================================================
// Subtract monthly deduction from unloaned account value.
void AccountValue::TxDoMlyDed()
{
    AVUnloaned -=             CoiCharge + AdbCharge + WpCharge;
    MlyDed = YearsMlyPolFee + CoiCharge + AdbCharge + WpCharge;
    mlydedtonextmodalpmtdate = MlyDed * MonthsToNextModalPmtDate();
}

//============================================================================
// Credit interest on loaned and unloaned account value separately.
void AccountValue::TxCreditInt()
{
    // Accrue interest on unloaned and loaned account value separately,
    //   but do not charge interest on negative account value.
    if(0.0 < AVUnloaned)
        {
        // TODO ?? Should each interest increment be rounded separately?
        double z = GetRoundingRules().round_interest_credit()
            (AVUnloaned * YearsGenAcctIntRate
            );
        AVUnloaned += z;
        }
    // Loaned account value cannot be negative.
    LMI_ASSERT(0.0 <= AVRegLn + AVPrfLn);
}

//============================================================================
// Accrue loan interest.
void AccountValue::TxLoanInt()
{
    // Nothing to do if there's no loan outstanding.
    if(0.0 == RegLnBal && 0.0 == PrfLnBal)
        {
        return;
        }

    // We may want to display credited interest separately.
    // TODO ?? Should each interest increment be rounded separately?
    RegLnIntCred = GetRoundingRules().round_interest_credit()
        (AVRegLn * YearsRegLnIntCredRate
        );
    PrfLnIntCred = GetRoundingRules().round_interest_credit()
        (AVPrfLn * YearsPrfLnIntCredRate
        );

    AVRegLn += RegLnIntCred;
    AVPrfLn += PrfLnIntCred;

    double RegLnIntAccrued = GetRoundingRules().round_interest_credit()
        (RegLnBal * YearsRegLnIntDueRate
        );
    double PrfLnIntAccrued = GetRoundingRules().round_interest_credit()
        (PrfLnBal * YearsPrfLnIntDueRate
        );

    RegLnBal += RegLnIntAccrued;
    PrfLnBal += PrfLnIntAccrued;
}

//============================================================================
// TODO ?? Min AV after WD not implemented--does max WD calculation take care of it?
void AccountValue::TxTakeWD()
{
    // Illustrations allow withdrawals only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no withdrawal requested.
    if(0.0 == wd)
        {
        return;
        }

    // For solves, we may wish to ignore min and max.

    // Impose minimum amount (if nonzero) on withdrawals.
    if(wd < MinWD)
        {
        wd = 0.0;
        }

    // Impose maximum amount.
    // If maximum exceeded, limit it.
    // TODO ?? Max WD and max loan formulas treat loan interest differently:
    //   max WD on a loaned policy: cannot become overloaned until next
    //     modal premium date;
    //   max loan: cannot become overloaned until end of policy year.
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

    switch(YearsDBOpt.value())
        {
        case e_option1:
            {
            // TODO ?? Spec amt reduced for option 1 even if in corridor?
            //   --taken care of by max WD formula
            // TODO ?? If WD causes spec amt < min spec amt, do we:
            //   set spec amt = min spec amt?
            //   reduce the WD?
            //   lapse the policy?
// TODO ??            ActualSpecAmt = std::min(ActualSpecAmt, deathbft - wd);
            ActualSpecAmt -= wd;
            ActualSpecAmt = std::max(ActualSpecAmt, MinSpecAmt);
            ActualSpecAmt = GetRoundingRules().round_specamt()(ActualSpecAmt);
            // TODO ?? If WD causes AV < min AV, do we:
            //   reduce the WD?
            //   lapse the policy?
            // Maybe it can't happen because of max WD defn?

            // Carry the new spec amt forward into all future years.
            for(int j = Year; j < BasicValues::GetLength(); j++)
                {
                InvariantValues().SpecAmt[j] = ActualSpecAmt;
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
                << YearsDBOpt.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // Deduct withdrawal fee.
    wd -= std::min(WDFee, wd * WDFeeRate);
    // TODO ?? This treats input WD as gross; it prolly should be net.

    InvariantValues().NetWD[Year] = wd;
// TODO ??    TaxBasis -= wd;
}

//============================================================================
// Take a new loan
void AccountValue::TxTakeLoan()
{
    // Illustrations allow loans only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no loan requested.
    if(RequestedLoan <= 0.0)
        {
        return;
        }

    // Impose maximum amount.
    // If maximum exceeded...limit it.
    // TODO ?? For solves, we may wish to ignore max.
    MaxLoan
        = AVUnloaned * 0.9    // TODO ?? Icky manifest constant.
        // - surrchg
        + (AVRegLn + AVPrfLn)
        - RegLnBal * (std::pow((1.0 + YearsRegLnIntDueRate), 12 - Month) - 1.0)
        - PrfLnBal * (std::pow((1.0 + YearsPrfLnIntDueRate), 12 - Month) - 1.0)
        - mlydedtonextmodalpmtdate;
    // Interest adjustment: d upper n where n is # months remaining in year.
    // Witholding this keeps policy from becoming overloaned before year end.
    double IntAdj = std::pow((1.0 + YearsRegLnIntDueRate), 12 - Month);
    IntAdj = (IntAdj - 1.0) / IntAdj;
    MaxLoan *= 1.0 - IntAdj;
    MaxLoan = std::max(0.0, MaxLoan);
    MaxLoan = GetRoundingRules().round_loan()(MaxLoan);

    // TODO ?? Preferred loan calculations go here.

    // Update loan AV, loan balance.

    if(MaxLoan < RequestedLoan)
        {
        RequestedLoan = MaxLoan;
        }

    AVUnloaned -= RequestedLoan;
    AVRegLn += RequestedLoan;    // TODO ?? Also preferred.
    InvariantValues().Loan[Year] = RequestedLoan;
}

//============================================================================
// Test for lapse.
void AccountValue::TxTestLapse()
{
// Perform no-lapse test.

    // If we're doing a solve, don't let it lapse--otherwise lapse would
    // introduce a discontinuity in the function for which we seek a root.
    if(Solving)
        {
        return;
        }

    // Otherwise if AV is negative or if overloaned, then lapse the policy.
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

//============================================================================
void AccountValue::SetDebugFilename(std::string const& s)
{
    DebugFilename = s;
}

// Stubs for member functions not implemented on this branch.

double AccountValue::ApportionNetMortalityReserve(double, double)
    {return 0.0;}
void   AccountValue::CoordinateCounters()
    {return;}
void   AccountValue::DebugRestart(std::string const&)
    {return;}
void   AccountValue::FinalizeLife(e_run_basis const&)
    {return;}
void   AccountValue::FinalizeLifeAllBases()
    {return;}
double AccountValue::GetCurtateNetClaimsInforce()
    {return 0.0;}
double AccountValue::GetProjectedCoiChargeInforce()
    {return 0.0;}
double AccountValue::GetLastCoiChargeInforce() const
    {return 0.0;}
double AccountValue::GetSepAcctAssetsInforce() const
    {return 0.0;}
void   AccountValue::GuessWhetherFirstYearPremiumExceedsRetaliationLimit()
    {return;}
double AccountValue::IncrementBOM(int, int, double)
    {return 0.0;}
void   AccountValue::IncrementEOM(int, int, double)
    {return;}
void   AccountValue::IncrementEOY(int)
    {return;}
void   AccountValue::InitializeLife(e_run_basis const&)
    {return;}
void   AccountValue::InitializeYear()
    {return;}
bool   AccountValue::PrecedesInforceDuration(int, int)
    {return false;}
void   AccountValue::SetClaims()
    {return;}
void   AccountValue::SetProjectedCoiCharge()
    {return;}
bool   AccountValue::TestWhetherFirstYearPremiumExceededRetaliationLimit()
    {return false;}


//============================================================================
double AccountValue::ibnr_as_months_of_mortality_charges()
{
    return Database_->Query(DB_ExpRatIBNRMult);
}

//============================================================================
double AccountValue::experience_rating_amortization_years()
{
    return Database_->Query(DB_ExpRatAmortPeriod);
}

