// Account value.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "account_value.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "input.hpp"                    // consummate()
#include "interest_rates.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "loads.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"

#include <algorithm>                    // max(), min()
#include <cmath>                        // pow()
#include <numeric>                      // accumulate()

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
        }
    throw "Unreachable--silences a compiler diagnostic.";
    }
} // Unnamed namespace.

/*
IHS !!
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
    :BasicValues       (Input::consummate(input))
    ,InputFilename     {"anonymous"}
    ,DebugFilename     {"anonymous.monthly_trace"}
    ,ledger_{::new Ledger(BasicValues::GetLength(), BasicValues::ledger_type(), BasicValues::nonillustrated(), BasicValues::no_can_issue(), false)}
    ,ledger_invariant_ {::new LedgerInvariant(BasicValues::GetLength())}
    ,ledger_variant_   {::new LedgerVariant  (BasicValues::GetLength())}
    ,RunBasis_         {mce_run_gen_curr_sep_full}
    ,GenBasis_         {mce_gen_curr}
    ,SepBasis_         {mce_sep_full}
    ,pmt_mode          {mce_annual}
    ,OldDBOpt          {mce_option1}
    ,YearsDBOpt        {mce_option1}
    ,stored_pmts       {Outlay_->ee_modal_premiums()}
{
    GrossPmts  .resize(12);
    NetPmts    .resize(12);
}

/// Specified amount.

currency AccountValue::base_specamt(int year) const
{
    // CURRENCY !! Cents in ledger will make rounding unnecessary.
    return round_specamt().c(InvariantValues().SpecAmt[year] / 100.0);
}

//============================================================================
std::shared_ptr<Ledger const> AccountValue::ledger_from_av() const
{
    LMI_ASSERT(ledger_.get());
    return ledger_;
}

//============================================================================
void AccountValue::RunAV()
{
    InvariantValues().Init(this);
    OverridingPmts = stored_pmts;
    Solving = mce_solve_none != yare_input_.SolveType;
    RunAllApplicableBases();
}

//============================================================================
void AccountValue::RunOneBasis(mcenum_run_basis TheBasis)
{
    if(Solving)
        {
        // IHS !! Isn't this unreachable?
        throw std::logic_error("This line had seemed to be unreachable.");
//        LMI_ASSERT(TheBasis corresponds to yare_input_.SolveExpenseGeneralAccountBasis);
        }
    else
        {
        RunOneCell(TheBasis);
        }
}

//============================================================================
// If not solving
//   if running one basis
//     just do that basis
//   if running all bases
//     run all bases
//
void AccountValue::RunAllApplicableBases()
{
    // set pmts, specamt, surrchg

    // Separate-account basis hardcoded because separate account not supported.
    mcenum_run_basis run_basis;
    set_run_basis_from_cloven_bases
        (run_basis
        ,yare_input_.SolveExpenseGeneralAccountBasis
        ,mce_sep_full
        );

    if(Solving)
        {
        Solve();
        OverridingPmts = stored_pmts;
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
}

//============================================================================
void AccountValue::RunOneCell(mcenum_run_basis TheBasis)
{
    if(Solving)
        {
        stored_pmts = Outlay_->ee_modal_premiums();
        // IHS !! This seems wasteful. Track down the reason for doing it.
        InvariantValues().Init(this);
        }

    RunBasis_ = TheBasis;
    set_cloven_bases_from_run_basis(RunBasis_, GenBasis_, SepBasis_);

    VariantValues().Init(*this, GenBasis_, SepBasis_);

    Debugging       = false;

    InforceYear      = yare_input_.InforceYear;
    InforceMonth     = yare_input_.InforceMonth;
    InforceAVGenAcct = round_minutiae().c(yare_input_.InforceGeneralAccountValue);

    ItLapsed         = false;
    LapseMonth       = 0;
    LapseYear        = 0;

    TaxBasis         = C0;

    MaxLoan          = C0;
    RegLnBal         = C0;
    PrfLnBal         = C0;
    AVRegLn          = C0;
    AVPrfLn          = C0;

    // 'InforceAVGenAcct' is unloaned only; this branch wasn't
    // designed to allow inforce loans.
    AVUnloaned = InforceAVGenAcct;

    PerformSpecAmtStrategy();

    for(Year = InforceYear; Year < BasicValues::GetLength(); ++Year)
        {
        if(!ItLapsed)
            {
            DoYear(TheBasis, Year, (Year == InforceYear) ? InforceMonth : 0);
            }
        }
}

//============================================================================
void AccountValue::DoYear
    (mcenum_run_basis a_TheBasis
    ,int              a_Year
    ,int              a_InforceMonth
    )
{
    Year = a_Year; // IHS !! expunge?

    RunBasis_ = a_TheBasis;
    set_cloven_bases_from_run_basis(RunBasis_, GenBasis_, SepBasis_);

// IHS !! Solve...() should reset not inputs but...?

    // IHS !! These variables are set in current run and used in
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

    pmt                   = stored_pmts[Year];
    YearsPremLoadTgt      = Loads_->target_premium_load(GenBasis_)[Year];
    YearsMonthlyPolicyFee = Loads_->monthly_policy_fee (GenBasis_)[Year];
    ActualSpecAmt         = base_specamt(Year);

    // These variables are set for each pass independently.
    pmt_mode        = Outlay_->ee_premium_modes()[Year];
    ModeIndex       = get_mode_index(pmt_mode);
    RequestedLoan   = Outlay_->new_cash_loans()[Year];
    wd              = Outlay_->withdrawals()[Year];
    mlyguarv        =
            1.0
        /   (1.0 + InterestRates_->GenAcctNetRate
                (mce_gen_guar
                ,mce_monthly_rate
                )[Year]
            );
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

    GrossPmts  .assign(12, C0);
    NetPmts    .assign(12, C0);

    // IHS !! Strategy here?

    for(Month = a_InforceMonth; Month < 12; ++Month)
        {
        DoMonth();
        if(ItLapsed)
            {
            return;
            }
        }

    VariantValues().AcctVal[Year] = centize(AVUnloaned + AVRegLn + AVPrfLn);
    VariantValues().CSVNet[Year] = VariantValues().AcctVal[Year] - VariantValues().SurrChg[Year];
    // Update death benefit: "deathbft" currently holds benefit as of the
    //   beginning of month 12, but we want it as of the end of that month,
    //   in case the corridor or option 2 drove it up during the last month.
    TxSetDeathBft();
    VariantValues().EOYDeathBft[Year] = centize(deathbft);

    // IHS !! Change one of these names, which differ only in the terminal 's'.
    InvariantValues().GrossPmt[Year] += centize(std::accumulate(GrossPmts.begin(), GrossPmts.end(), C0));
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
    currency SA = C0;
    switch(yare_input_.SpecifiedAmountStrategy[0])
        {
        case mce_sa_input_scalar:
            {
            SA = base_specamt(0);
            }
            break;
        case mce_sa_maximum:
            {
            SA = GetModalMaxSpecAmt(Outlay_->ee_premium_modes()[0], stored_pmts[0]);
            }
            break;
        case mce_sa_target:
            {
            SA = GetModalTgtSpecAmt(Outlay_->ee_premium_modes()[0], stored_pmts[0]);
            }
            break;
        case mce_sa_mep:
            {
            alarum()
                << "Modified endowment premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            SA = base_specamt(0);
            }
            break;
        case mce_sa_glp:
            {
            alarum()
                << "Guideline level premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            SA = base_specamt(0);
            }
            break;
        case mce_sa_gsp:      // fall through
        case mce_sa_corridor: // fall through
        case mce_sa_salary:   // fall through
        default:
            {
            alarum()
                << "Case "
                << yare_input_.SpecifiedAmountStrategy[0]
                << " not found."
                << LMI_FLUSH
                ;
            }
        }

    for(int j = 0; j < BasicValues::GetLength(); ++j)
        {
        InvariantValues().SpecAmt[j] = centize(SA);
        }
}

/// Change death benefit option.

void AccountValue::TxOptionChange()
{
    // Illustrations allow option changes only on anniversary,
    // but not on the zeroth anniversary.
    if(0 != Month || 0 == Year)
        {
        return;
        }

    // It's OK to index by [Year - 1] because of the early return
    // above in the 0 == Year case.

    // Nothing to do if no option change requested.
    if(DeathBfts_->dbopt()[Year - 1] == YearsDBOpt)
        {
        return;
        }

    // Nothing to do unless AV is positive.
    currency AV = AVUnloaned + AVRegLn + AVPrfLn;
    if(AV <= C0)
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
        case mce_rop: // fall through
        case mce_mdb: // fall through
        default:
            {
            alarum() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }
    // AV normally rounded to cents, but specamt perhaps to dollars.
    ActualSpecAmt = round_specamt().c(ActualSpecAmt);

    // Carry the new spec amt forward into all future years.
    for(int j = Year; j < BasicValues::GetLength(); ++j)
        {
        InvariantValues().SpecAmt[j] = centize(ActualSpecAmt);
        }
}

/// Process an owner-initiated specified-amount increase or decrease.
///
/// Ignores multiple layers of coverage: not correct for select and
/// ultimate COI rates if select period restarts on increase.
///
/// Specamt changes are assumed not to affect the target-premium rate.

void AccountValue::TxSpecAmtChange()
{
    // Illustrations allow increases and decreases only on anniversary,
    // but not on the zeroth anniversary.
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
    for(int j = Year; j < BasicValues::GetLength(); ++j)
        {
        InvariantValues().SpecAmt[j] = centize(ActualSpecAmt);
        }
}

//============================================================================
// Set payment according to selected strategy, in each non-solve year.
void AccountValue::PerformPmtStrategy(currency* a_Pmt)
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
            *a_Pmt = stored_pmts[Year];
            }
            break;
        case mce_pmt_minimum:
            {
            *a_Pmt = GetModalMinPrem
                (Year
                ,pmt_mode
                ,ActualSpecAmt
                );
            }
            break;
        case mce_pmt_target:
            {
            *a_Pmt = GetModalTgtPrem
                (Year
                ,pmt_mode
                ,ActualSpecAmt
                );
            }
            break;
        case mce_pmt_mep:
            {
            alarum()
                << "Modified endowment premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            *a_Pmt = stored_pmts[Year];
            }
            break;
        case mce_pmt_glp:
            {
            alarum()
                << "Guideline level premium not implemented."
                << " Payment set to scalar input value."
                << LMI_FLUSH
                ;
            *a_Pmt = stored_pmts[Year];
            }
            break;
        case mce_pmt_gsp:      // fall through
        case mce_pmt_corridor: // fall through
        case mce_pmt_table:    // fall through
        default:
            {
            alarum()
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
//   mode is not monthly; IHS !! is this something we need to do here?
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
        currency TotalDumpin =
              Outlay_->dumpin()
            + Outlay_->external_1035_amount()
            + Outlay_->internal_1035_amount()
            ;
        GrossPmts[Month] += TotalDumpin;
        }

    // IHS !! Test maximum premium. Round it with Rounding.RoundMAXP .
//            (DB-AV)/YearsCorridorFactor - AV

    // Subtract premium load from gross premium yielding net premium.
    NetPmts[Month] = round_net_premium().c
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
    if(C0 <= RequestedLoan)
        {
        return;
        }

    AVUnloaned -= RequestedLoan;
    AVRegLn += RequestedLoan;    // IHS !! Preferred loans--see lmi.
    InvariantValues().NewCashLoan[Year] = centize(RequestedLoan);
}

/// Set account value before monthly deductions.

void AccountValue::TxSetBOMAV()
{
    // IHS !! Seems bogus: the lmi implementation does much more.
    AVUnloaned -= YearsMonthlyPolicyFee;
}

/// Set death benefit reflecting corridor and death benefit option.

void AccountValue::TxSetDeathBft()
{
    // Total account value is unloaned plus loaned.
    currency AV = AVUnloaned + AVRegLn + AVPrfLn;
    currency corr = round_death_benefit().c(YearsCorridorFactor * AV);

    // Set death benefit reflecting corridor and death benefit option.
    switch(YearsDBOpt)
        {
        case mce_option1:
            {
            // Option 1: specamt, or corridor times AV if greater.
            deathbft = std::max(ActualSpecAmt, corr);
            }
            break;
        case mce_option2:
            // Option 2: specamt plus AV, or corridor times AV if greater.
            // Negative AV doesn't decrease death benefit.
            deathbft = std::max(ActualSpecAmt + std::max(C0, AV), corr);
            break;
        case mce_rop: // fall through
        case mce_mdb: // fall through
        default:
            {
            alarum() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }

    deathbft = round_death_benefit().c(deathbft);

    // SOMEDAY !! Accumulate average death benefit for profit testing here.
}

/// Calculate mortality charge.

void AccountValue::TxSetCoiCharge()
{
    TxSetDeathBft();

    // Negative AV doesn't increase NAAR.
    NAAR = round_naar().c(deathbft * mlyguarv - dblize(AVUnloaned + AVRegLn + AVPrfLn));

    CoiCharge = round_coi_charge().c(NAAR * YearsCoiRate0);
}

/// Calculate rider charges.

void AccountValue::TxSetRiderDed()
{
    WpCharge = C0;
    if(haswp)
        {
        WpCharge = round_rider_charges().c
            (   YearsWpRate
            *   (CoiCharge + YearsMonthlyPolicyFee + AdbCharge)
            );
        }

    AdbCharge = C0;
    if(hasadb)
        {
        AdbCharge = round_rider_charges().c
            ( YearsAdbRate
            // IHS !! Icky manifest constant--lmi uses a database entity.
            * std::min(500'000'00_cents, ActualSpecAmt)
            );
        }
}

/// Subtract monthly deductions from unloaned account value.

void AccountValue::TxDoMlyDed()
{
    AVUnloaned -=             CoiCharge + AdbCharge + WpCharge;
    MlyDed = YearsMonthlyPolicyFee + CoiCharge + AdbCharge + WpCharge;
    mlydedtonextmodalpmtdate = MlyDed * MonthsToNextModalPmtDate();
}

/// Credit interest on account value.

void AccountValue::TxCreditInt()
{
    // Accrue interest on unloaned and loaned account value separately,
    //   but do not charge interest on negative account value.
    if(C0 < AVUnloaned)
        {
        // IHS !! Each interest increment is rounded separately in lmi.
        AVUnloaned += round_interest_credit().c(AVUnloaned * YearsGenAcctIntRate);
        }
    // Loaned account value cannot be negative.
    LMI_ASSERT(C0 <= AVRegLn + AVPrfLn);
}

/// Accrue loan interest, and calculate interest credit on loaned AV.

void AccountValue::TxLoanInt()
{
    // Nothing to do if there's no loan outstanding.
    if(C0 == RegLnBal && C0 == PrfLnBal)
        {
        return;
        }

    // We may want to display credited interest separately.
    // IHS !! Each interest increment is rounded separately in lmi.
    RegLnIntCred = round_interest_credit().c(AVRegLn * YearsRegLnIntCredRate);
    PrfLnIntCred = round_interest_credit().c(AVPrfLn * YearsPrfLnIntCredRate);

    AVRegLn += RegLnIntCred;
    AVPrfLn += PrfLnIntCred;

    currency RegLnIntAccrued = round_interest_credit().c(RegLnBal * YearsRegLnIntDueRate);
    currency PrfLnIntAccrued = round_interest_credit().c(PrfLnBal * YearsPrfLnIntDueRate);

    RegLnBal += RegLnIntAccrued;
    PrfLnBal += PrfLnIntAccrued;
}

/// Take a withdrawal.

void AccountValue::TxTakeWD()
{
// IHS !! Min AV after WD not implemented here, though max WD calculation
// may take care of it. It is implemented in lmi.

    // Illustrations allow withdrawals only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no withdrawal requested.
    if(C0 == wd)
        {
        return;
        }

    // For solves, we may wish to ignore min and max.

    // Impose minimum amount (if nonzero) on withdrawals.
    if(wd < MinWD)
        {
        wd = C0;
        }

    // Impose maximum amount.
    // If maximum exceeded, limit it.
    // IHS !! Max WD and max loan formulas treat loan interest differently:
    //   max WD on a loaned policy: cannot become overloaned until next
    //     modal premium date;
    //   max loan: cannot become overloaned until end of policy year.
    // However, lmi provides a variety of implementations instead of
    // only one.
    currency max_wd =
          AVUnloaned
        + (AVRegLn  + AVPrfLn)
        - (RegLnBal + PrfLnBal)
        - mlydedtonextmodalpmtdate;
    if(max_wd < wd)
        {
        wd = max_wd;
        }

    AVUnloaned -= wd;

    switch(YearsDBOpt)
        {
        case mce_option1:
            {
            // Spec amt reduced for option 1 even if in corridor?
            //   --taken care of by max WD formula
            // If WD causes spec amt < min spec amt, do we:
            //   set spec amt = min spec amt?
            //   reduce the WD?
            //   lapse the policy?
// IHS !!            ActualSpecAmt = std::min(ActualSpecAmt, deathbft - wd);
            ActualSpecAmt -= wd;
            ActualSpecAmt = std::max(ActualSpecAmt, MinSpecAmt);
            ActualSpecAmt = round_specamt().c(ActualSpecAmt);
            // IHS !! If WD causes AV < min AV, do we:
            //   reduce the WD?
            //   lapse the policy?
            // Maybe it can't happen because of max WD defn?

            // Carry the new spec amt forward into all future years.
            for(int j = Year; j < BasicValues::GetLength(); ++j)
                {
                InvariantValues().SpecAmt[j] = centize(ActualSpecAmt);
                }
            }
            break;
        case mce_option2:
            ;
            break;
        case mce_rop: // fall through
        case mce_mdb: // fall through
        default:
            {
            alarum() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }

    // Deduct withdrawal fee.
    wd -= std::min(WDFee, round_withdrawal().c(wd * WDFeeRate));
    // IHS !! This treats input WD as gross; it probably should be net. But compare lmi.

    InvariantValues().NetWD[Year] = centize(wd);
// IHS !!    TaxBasis -= wd; // Withdrawals are subtracted from basis in lmi.
}

/// Take a new cash loan, limiting it to respect the maximum loan.

void AccountValue::TxTakeLoan()
{
    // Illustrations allow loans only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Nothing to do if no loan requested.
    if(RequestedLoan <= C0)
        {
        return;
        }

    // Impose maximum amount.
    // If maximum exceeded...limit it.
    // IHS !! For solves, the lmi branch uses an 'ullage' concept.
    double max_loan =
          AVUnloaned * 0.9 // IHS !! Icky manifest constant--lmi uses a database entity.
        // - surrchg
        + dblize(AVRegLn + AVPrfLn)
        - RegLnBal * (std::pow((1.0 + YearsRegLnIntDueRate), 12 - Month) - 1.0)
        - PrfLnBal * (std::pow((1.0 + YearsPrfLnIntDueRate), 12 - Month) - 1.0)
        - dblize(mlydedtonextmodalpmtdate)
        ;
    // Interest adjustment: d upper n where n is # months remaining in year.
    // Witholding this keeps policy from becoming overloaned before year end.
    double IntAdj = std::pow((1.0 + YearsRegLnIntDueRate), 12 - Month);
    IntAdj = (IntAdj - 1.0) / IntAdj;
    max_loan *= 1.0 - IntAdj;
    max_loan = std::max(0.0, max_loan);
    MaxLoan = round_loan().c(max_loan);

    // IHS !! Preferred loan calculations would go here: implemented in lmi.

    // Update loan AV, loan balance.

    if(MaxLoan < RequestedLoan)
        {
        RequestedLoan = MaxLoan;
        }

    AVUnloaned -= RequestedLoan;
    AVRegLn += RequestedLoan;    // IHS !! Also preferred loans: implemented in lmi.
    InvariantValues().NewCashLoan[Year] = centize(RequestedLoan);
}

/// Test for lapse.

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
            (AVUnloaned + AVRegLn + AVPrfLn < C0)
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
    InputFilename = s;
    DebugFilename = s;
}

// Stubs for member functions not implemented on this branch.

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
currency AccountValue::GetSepAcctAssetsInforce() const
    {return C0;}
currency AccountValue::IncrementBOM(int, int)
    {return C0;}
void   AccountValue::IncrementEOM(int, int, currency, currency)
    {return;}
void   AccountValue::IncrementEOY(int)
    {return;}
double AccountValue::InforceLivesBoy() const
    {return 0.0;}
void   AccountValue::InitializeLife(mcenum_run_basis)
    {return;}
void   AccountValue::InitializeYear()
    {return;}
bool   AccountValue::PrecedesInforceDuration(int, int)
    {return false;}
void   AccountValue::SetClaims()
    {return;}
