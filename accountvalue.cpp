// Account value.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "input.hpp" // Magic static function.
#include "interest_rates.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "loads.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"

#include <algorithm> // std::max(), std::min()
#include <cmath>     // std::pow()
#include <numeric>   // std::accumulate()

namespace
{
    // This table lookup approach is faster than determining
    //   whether this is a modal payment date
    //     0 == Month % (12 / mode);
    //   or how many months until the next modal payment date
    //     1 + (11 - Month) % (12 / mode);
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

    int get_mode_index(mcenum_mode a_mode)
    {
    switch(a_mode)
        {
        case mce_annual:     return 0; // A
        case mce_semiannual: return 1; // S
        case mce_quarterly:  return 2; // Q
        case mce_monthly:    return 3; // M
        default:
            {
            fatal_error() << "Case " << a_mode << " not found." << LMI_FLUSH;
            throw "Unreachable--silences a compiler diagnostic.";
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
AccountValue::AccountValue(Input const& input)
    :BasicValues       (Input::magically_rectify(input))
    ,DebugFilename     ("anonymous.monthly_trace")
    ,ledger_(new Ledger(BasicValues::GetLedgerType(), BasicValues::GetLength()))
    ,ledger_invariant_ (new LedgerInvariant(BasicValues::GetLength()))
    ,ledger_variant_   (new LedgerVariant  (BasicValues::GetLength()))
    ,RunBasis_         (mce_run_gen_curr_sep_full)
    ,GenBasis_         (mce_gen_curr)
    ,SepBasis_         (mce_sep_full)
    ,mode              (mce_annual)
    ,OldDBOpt          (mce_option1)
    ,YearsDBOpt        (mce_option1)
{
    GrossPmts  .resize(12);
    NetPmts    .resize(12);
}

//============================================================================
AccountValue::~AccountValue()
{
}

//============================================================================
boost::shared_ptr<Ledger const> AccountValue::ledger_from_av() const
{
    LMI_ASSERT(ledger_.get());
    return ledger_;
}

//============================================================================
double AccountValue::RunAV()
{
    InvariantValues().Init(this);
    OverridingPmts = InvariantValues().EePmt;
    Solving = mce_solve_none != yare_input_.SolveType;
    return RunAllApplicableBases();
}

//============================================================================
double AccountValue::RunOneBasis(mcenum_run_basis TheBasis)
{
    double z;
    if(Solving)
        {
        // TODO ?? Isn't this unreachable?
        throw std::logic_error("This line had seemed to be unreachable.");
//        LMI_ASSERT(TheBasis corresponds to yare_input_.SolveExpenseGeneralAccountBasis);
//        z = Solve();
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
    mcenum_run_basis run_basis;
    set_run_basis_from_cloven_bases
        (run_basis
        ,yare_input_.SolveExpenseGeneralAccountBasis
        ,mce_sep_full
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

    run_basis = mce_run_gen_curr_sep_full;
    RunOneBasis(run_basis);
    ledger_->SetOneLedgerVariant(run_basis, VariantValues());

    run_basis = mce_run_gen_guar_sep_full;
    RunOneBasis(run_basis);
    ledger_->SetOneLedgerVariant(run_basis, VariantValues());

    run_basis = mce_run_gen_mdpt_sep_full;
    RunOneBasis(run_basis);
    ledger_->SetOneLedgerVariant(run_basis, VariantValues());

    return z;
}

//============================================================================
double AccountValue::RunOneCell(mcenum_run_basis TheBasis)
{
    if(Solving)
        {
        // TODO ?? This seems wasteful. Track down the reason for doing it.
        InvariantValues().Init(this);
        }

    RunBasis_ = TheBasis;
    set_cloven_bases_from_run_basis(RunBasis_, GenBasis_, SepBasis_);

    VariantValues().Init(*this, GenBasis_, SepBasis_);

    Debugging       = false;

    InforceYear      = yare_input_.InforceYear;
    InforceMonth     = yare_input_.InforceMonth;
    InforceAVGenAcct = yare_input_.InforceGeneralAccountValue;

    ItLapsed         = false;
    LapseMonth       = 0;
    LapseYear        = 0;

    TaxBasis         = 0.0;

    MaxLoan          = 0.0;
    RegLnBal         = 0.0;
    PrfLnBal         = 0.0;
    AVRegLn          = 0.0;
    AVPrfLn          = 0.0;

    // 'InforceAVGenAcct' is unloaned only; this branch wasn't
    // designed to allow inforce loans.
    AVUnloaned = InforceAVGenAcct;

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
    (mcenum_run_basis a_TheBasis
    ,int              a_Year
    ,int              a_InforceMonth
    )
{
    Year = a_Year; // TODO ?? expunge?

    RunBasis_ = a_TheBasis;
    set_cloven_bases_from_run_basis(RunBasis_, GenBasis_, SepBasis_);

// TODO ?? Solve...() should reset not inputs but...?

    // TODO ?? These variables are set in current run and used in
    // guaranteed and midpoint runs.
    YearsCoiRate0   = MortalityRates_->MonthlyCoiRates(GenBasis_)[Year];

    YearsWpRate     = MortalityRates_->WpRates()[Year];
    YearsAdbRate    = MortalityRates_->AdbRates()[Year];
    haswp           = yare_input_.WaiverOfPremiumBenefit;
    hasadb          = yare_input_.AccidentalDeathBenefit;

    YearsGenAcctIntRate = InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_monthly_rate
        )[Year]
        ;

    pmt                   = InvariantValues().EePmt[Year];
    YearsPremLoadTgt      = Loads_->target_premium_load(GenBasis_)[Year];
    YearsMonthlyPolicyFee = Loads_->monthly_policy_fee(GenBasis_)[Year];
    ActualSpecAmt         = InvariantValues().SpecAmt[Year];

    // These variables are set for each pass independently.
    mode            = InvariantValues().EeMode[Year].value();
    ModeIndex       = get_mode_index(mode);
    RequestedLoan   = Outlay_->new_cash_loans()[Year];
    wd              = Outlay_->withdrawals()[Year];
    mlyguarv        =
            1.0
        /   (1.0 + InterestRates_->GenAcctNetRate
                (mce_gen_guar
                ,mce_monthly_rate
                )[Year]
            );
    YearsSpecAmt    = DeathBfts_->specamt()[Year];
    YearsDBOpt      = DeathBfts_->dbopt()[Year];

    // For guaranteed-basis run, what loan rates should be used?
    YearsRegLnIntCredRate = InterestRates_->RegLnCredRate
        (mce_gen_curr
        ,mce_monthly_rate
        )[Year]
        ;
    YearsRegLnIntDueRate  = InterestRates_->RegLnDueRate
        (mce_gen_curr
        ,mce_monthly_rate
        )[Year]
        ;
    YearsPrfLnIntCredRate = InterestRates_->PrfLnCredRate
        (mce_gen_curr
        ,mce_monthly_rate
        )[Year]
        ;
    YearsPrfLnIntDueRate  = InterestRates_->PrfLnDueRate
        (mce_gen_curr
        ,mce_monthly_rate
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
    TxSetDeathBft();
    VariantValues().EOYDeathBft[Year] = deathbft;

    // TODO ?? Change one of these names, which differ only in the terminal 's'.
    InvariantValues().GrossPmt[Year] += std::accumulate(GrossPmts.begin(), GrossPmts.end(), 0.0);
    InvariantValues().Outlay[Year] =
            InvariantValues().GrossPmt   [Year]
        -   InvariantValues().NetWD      [Year]
        -   InvariantValues().NewCashLoan[Year]
        ;

    if(Debugging)
        {
//        DebugPrint(); // IHS !! Implemented in lmi.
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

    // IHS !! Implemented in lmi but not here:
    //   rounding of premium load
    //   multiple lives
    //   min and max prem (yearly and cumulative)
    //   excess loan
    //   where is loan capitalized?
    //   solve for WD then loan
    //   solve for endt?
    //   monthly trace
    //   7702A
    //   CVAT corridor
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
    TxPmt();            // IHS !! Incomplete here; better in lmi.
    TxLoanRepay();

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
    switch(yare_input_.SpecifiedAmountStrategy[0])
        {
        case mce_sa_input_scalar:
            {
            SA = InvariantValues().SpecAmt[0];
            }
            break;
        case mce_sa_maximum:
            {
            SA = GetModalMaxSpecAmt(InvariantValues().EeMode[0].value(), InvariantValues().EePmt[0]);
            }
            break;
        case mce_sa_target:
            {
            SA = GetModalTgtSpecAmt(InvariantValues().EeMode[0].value(), InvariantValues().EePmt[0]);
            }
            break;
        case mce_sa_mep:
            {
            fatal_error()
                << "Modified endowment premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            SA = InvariantValues().SpecAmt[0];
            }
            break;
        case mce_sa_glp:
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
                << "Case "
                << yare_input_.SpecifiedAmountStrategy[0]
                << " not found."
                << LMI_FLUSH
                ;
            }
        }

    SA = round_specamt()(SA);

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
    if(DeathBfts_->dbopt()[Year - 1] == YearsDBOpt)
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
    switch(YearsDBOpt)
        {
        case mce_option1:
            {
            // Option 1: increase spec amt by AV.
            ActualSpecAmt += AV;
            }
            break;
        case mce_option2:
            {
            // Option 2: decrease spec amt by AV, but not below min spec amt.
            ActualSpecAmt -= AV;
            ActualSpecAmt = std::max(ActualSpecAmt, MinSpecAmt);
            // An alternative is to lapse the policy.
            }
            break;
        default:
            {
            fatal_error() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }
    ActualSpecAmt = round_specamt()(ActualSpecAmt);

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
    // Don't override premium during solve period.
    if
        (
            mce_solve_ee_prem == yare_input_.SolveType
        &&  yare_input_.SolveBeginYear <= Year
        &&  Year < std::min
                (yare_input_.SolveEndYear
                ,BasicValues::GetLength()
                )
        )
        {
        return;
        }

    switch(yare_input_.PaymentStrategy[0])
        {
        case mce_pmt_input_scalar:
            {
            *a_Pmt = InvariantValues().EePmt[Year];
            }
            break;
        case mce_pmt_minimum:
            {
            *a_Pmt = GetModalMinPrem
                (Year
                ,mode
                ,ActualSpecAmt
                );
            }
            break;
        case mce_pmt_target:
            {
            *a_Pmt = GetModalTgtPrem
                (Year
                ,mode
                ,ActualSpecAmt
                );
            }
            break;
        case mce_pmt_mep:
            {
            fatal_error()
                << "Modified endowment premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            *a_Pmt = InvariantValues().EePmt[Year];
            }
            break;
        case mce_pmt_glp:
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
                << "Case "
                << yare_input_.PaymentStrategy[0]
                << " not found."
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
// IHS !! Tiered premium implemented in lmi, but not here.
void AccountValue::TxPmt()
{
    // Do nothing if this is not a modal payment date.
    if(!IsModalPmtDate())
        {
        return;
        }

    // IHS !! Put GPT stuff like forceout and premium limit here--see lmi.

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
    NetPmts[Month] = round_net_premium()
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
    AVRegLn += RequestedLoan;    // IHS !! Preferred loans--see lmi.
    InvariantValues().NewCashLoan[Year] = RequestedLoan;
}

//============================================================================
// Set account value before monthly deductions.
// IHS !! This function seems bogus here; the lmi implementation does much more.
void AccountValue::TxSetBOMAV()
{
    AVUnloaned -= YearsMonthlyPolicyFee;
}

//============================================================================
// Set death benefit reflecting corridor and option 2.
void AccountValue::TxSetDeathBft(bool)
{
    // Total account value is unloaned plus loaned.
    double AV = AVUnloaned + AVRegLn + AVPrfLn;

    // Set death benefit reflecting corridor and death benefit option.
    switch(YearsDBOpt)
        {
        case mce_option1:
            {
            // Option 1: specamt, or corridor times AV if greater.
            deathbft = std::max(ActualSpecAmt, YearsCorridorFactor * AV);
            }
            break;
        case mce_option2:
            // Option 2: specamt plus AV, or corridor times AV if greater.
            // Negative AV doesn't decrease death benefit.
            deathbft = std::max
                (ActualSpecAmt + std::max(0.0, AV)
                ,YearsCorridorFactor * AV
                );
            break;
        default:
            {
            fatal_error() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }

    deathbft = round_death_benefit()(deathbft);

    // SOMEDAY !! Accumulate average death benefit for profit testing here.
}

//============================================================================
// Set cost of insurance charge.
void AccountValue::TxSetCoiCharge()
{
    TxSetDeathBft();

    // Negative AV doesn't increase NAAR.
    NAAR = round_naar()(deathbft * mlyguarv - (AVUnloaned + AVRegLn + AVPrfLn));

    CoiCharge = round_coi_charge()(NAAR * YearsCoiRate0);
}

//============================================================================
// Calculate rider charges.
void AccountValue::TxSetRiderDed()
{
    WpCharge = 0.0;
    if(haswp)
        {
        WpCharge =
                YearsWpRate
            *   (CoiCharge + YearsMonthlyPolicyFee + AdbCharge)
            ;
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
    MlyDed = YearsMonthlyPolicyFee + CoiCharge + AdbCharge + WpCharge;
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
        // IHS !! Each interest increment is rounded separately in lmi.
        double z = round_interest_credit()(AVUnloaned * YearsGenAcctIntRate);
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
    // IHS !! Each interest increment is rounded separately in lmi.
    RegLnIntCred = round_interest_credit()(AVRegLn * YearsRegLnIntCredRate);
    PrfLnIntCred = round_interest_credit()(AVPrfLn * YearsPrfLnIntCredRate);

    AVRegLn += RegLnIntCred;
    AVPrfLn += PrfLnIntCred;

    double RegLnIntAccrued = round_interest_credit()(RegLnBal * YearsRegLnIntDueRate);
    double PrfLnIntAccrued = round_interest_credit()(PrfLnBal * YearsPrfLnIntDueRate);

    RegLnBal += RegLnIntAccrued;
    PrfLnBal += PrfLnIntAccrued;
}

//============================================================================
// IHS !! Min AV after WD not implemented here, though max WD calculation
// may take care of it. It is implemented in lmi.
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
    // IHS !! Max WD and max loan formulas treat loan interest differently:
    //   max WD on a loaned policy: cannot become overloaned until next
    //     modal premium date;
    //   max loan: cannot become overloaned until end of policy year.
    // However, lmi provides a variety of implementations instead of
    // only one.
    double MaxWD =
          AVUnloaned
        + (AVRegLn  + AVPrfLn)
        - (RegLnBal + PrfLnBal)
        - mlydedtonextmodalpmtdate;
    if(MaxWD < wd)
        {
        wd = MaxWD;
        }

    AVUnloaned -= wd;

    switch(YearsDBOpt)
        {
        case mce_option1:
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
            ActualSpecAmt = round_specamt()(ActualSpecAmt);
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
        case mce_option2:
            ;
            break;
        default:
            {
            fatal_error() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }

    // Deduct withdrawal fee.
    wd -= std::min(WDFee, wd * WDFeeRate);
    // IHS !! This treats input WD as gross; it prolly should be net. But compare lmi.

    InvariantValues().NetWD[Year] = wd;
// IHS !!    TaxBasis -= wd; // Withdrawals are subtracted from basis in lmi.
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
    // IHS !! For solves, the lmi branch uses an 'ullage' concept.
    MaxLoan =
          AVUnloaned * 0.9    // IHS !! Icky manifest constant--lmi uses a database entity.
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
    MaxLoan = round_loan()(MaxLoan);

    // IHS !! Preferred loan calculations would go here: implemented in lmi.

    // Update loan AV, loan balance.

    if(MaxLoan < RequestedLoan)
        {
        RequestedLoan = MaxLoan;
        }

    AVUnloaned -= RequestedLoan;
    AVRegLn += RequestedLoan;    // IHS !! Also preferred loans: implemented in lmi.
    InvariantValues().NewCashLoan[Year] = RequestedLoan;
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
// IHS !! Implemented in lmi, but not here.
}

//============================================================================
void AccountValue::SetDebugFilename(std::string const& s)
{
    DebugFilename = s;
}

// Stubs for member functions not implemented on this branch.

double AccountValue::ApportionNetMortalityReserve(double)
    {return 0.0;}
void   AccountValue::CoordinateCounters()
    {return;}
void   AccountValue::DebugPrint()
    {return;}
void   AccountValue::DebugPrintInit()
    {return;}
void   AccountValue::FinalizeLife(mcenum_run_basis)
    {return;}
void   AccountValue::FinalizeLifeAllBases()
    {return;}
double AccountValue::GetCurtateNetClaimsInforce() const
    {return 0.0;}
double AccountValue::GetCurtateNetCoiChargeInforce() const
    {return 0.0;}
double AccountValue::GetProjectedCoiChargeInforce() const
    {return 0.0;}
double AccountValue::GetSepAcctAssetsInforce() const
    {return 0.0;}
double AccountValue::IncrementBOM(int, int, double)
    {return 0.0;}
void   AccountValue::IncrementEOM(int, int, double, double)
    {return;}
void   AccountValue::IncrementEOY(int)
    {return;}
double AccountValue::InforceLivesBoy() const
    {return 0.0;}
double AccountValue::InforceLivesEoy() const
    {return 0.0;}
void   AccountValue::InitializeLife(mcenum_run_basis)
    {return;}
void   AccountValue::InitializeYear()
    {return;}
bool   AccountValue::PrecedesInforceDuration(int, int)
    {return false;}
void   AccountValue::SetClaims()
    {return;}
void   AccountValue::SetProjectedCoiCharge()
    {return;}

