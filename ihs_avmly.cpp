// Account value: monthiversary processing.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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
#include "contains.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "interest_rates.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "loads.hpp"
#include "materially_equal.hpp"
#include "math_functors.hpp"
#include "miscellany.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "stratified_algorithms.hpp"
#include "stratified_charges.hpp"

#include <cmath>   // std::pow()
#include <limits>
#include <stdexcept>
#include <utility>

// Each month, process all transactions in order.

// SOMEDAY !! Not yet implemented:
//   max allowable prem (without increasing NAAR)
//   increase premium during no lapse period, for solves at least
//   monthly varying corridor
//   multiple layers of coverage

// Some COLI products have M&E banded by case total assets.
//
// To determine case total assets before crediting interest on any life,
// first we have to process all transactions through the monthly deduction
// on all lives. Therefore we break monthly processing into two phases;
// calling them DR and CR for debit and credit respectively is inexact,
// but is sufficiently descriptive and has the virtue of brevity.

// Documentation of quantities that differ subtly but are all in the
// nature of 'cash value':
//
//                      displayed corr        max  max  over lapse  honeymoon
//                            CSV   DB   DCV  wd  loan  loan  test  expire
//
// unloaned account value     yes  yes   yes  yes* yes* yes*  yes   yes
// + loaned account value     yes  yes   yes  yes  yes  yes   yes   yes
// - loan balance             yes  no    no   yes  n/a  n/a   ?     no
// + refundable sales load    yes  yes   yes  no   no   no?   no    yes?
// - surrender charge         yes  if<0  if<0 if>0 if>0 if>0  if>0? yes
// + experience reserve       yes  if>0  if>0 no   no   no    yes   if>0
// - anticipated deductions   no   no    no   yes  yes  no    no    no
// honeymoon value if greater yes  ?     yes  no   no   no?   yes   n/a
//
//  * means times an optional multiplier
//  ? means maybe--depends on contract
//
//  no-lapse guarantees provide no protection against being overloaned
//
//  DCV has a corridor death benefit that follows the 'corr DB' rules;
//  the 'DCV' rules are for the phantom cash value only, and exotic
//  increments to that value are add-ons that don't accumulate from
//  one year to the next at interest
//
// Surrender charge is treated as the indivisible sum of several
// components. For example, if it's
//     .0001 * specamt (A)
//   - .01   * premium (B)
// then if (A+B) is positive, the indivisible (net) surrender charge is
// positive, and is therefore ignored in any formula that takes only
// negative surrender charge into account--even though (B) is negative.

//============================================================================
// Monthly transactions up through monthly deduction.
void AccountValue::DoMonthDR()
{
    if(ItLapsed)
        {
        return;
        }

    InitializeMonth();
    TxCapitalizeLoan();

    TxOptionChange();

    TxSpecAmtChange();
    TxTakeWD();

    TxTestGPT();
    // TODO ?? TAXATION !! Doesn't this mean dumpins and 1035s get ignored?
    LMI_ASSERT(0.0 <= Dcv);
    // TAXATION !! Is it really useful to comment the arguments here?
    Irc7702A_->UpdateBft7702A
        (Dcv
        ,DBReflectingCorr + TermDB // DB7702A
        ,OldDB // prior_db_7702A
        ,DBReflectingCorr == DBIgnoringCorr
        // TAXATION !! This assumes the term rider can be treated as death benefit;
        // use 'TermIsDbFor7702A'.
        ,ActualSpecAmt + TermSpecAmt
        ,OldSA // prior_sa_7702A
        ,CashValueFor7702()
        );

    NetPmts[Month]   = 0.0; // TODO ?? expunge as being unnecessary
    GrossPmts[Month] = 0.0; // TODO ?? expunge as being unnecessary
    TxExch1035();
    // TODO ?? TAXATION !! Is this where spec amt should be increased by GPT?

    double kludge_account_value = std::max(TotalAccountValue(), HoneymoonValue);
    if(0 == Year && 0 == Month)
        {
        // This special case was apparently intended for 1035
        // exchanges, but now seems unnecessary because this
        // assertion never fires:
        //   LMI_ASSERT(kludge_account_value == Dcv);
        kludge_account_value = Dcv;
        }
    kludge_account_value = std::max
        (HoneymoonValue
        ,   kludge_account_value
          + GetRefundableSalesLoad()
//          + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        );
// TODO ?? TAXATION !! Use CashValueFor7702() instead?
    double max_necessary_premium = Irc7702A_->MaxNecessaryPremium
        (Dcv
        ,AnnualTargetPrem
        ,YearsTotLoadTgtLowestPremtax
        ,YearsTotLoadExcLowestPremtax
        ,kludge_account_value
        );
    double max_non_mec_premium = Irc7702A_->MaxNonMecPremium
        (Dcv
        ,AnnualTargetPrem
        ,YearsTotLoadTgtLowestPremtax
        ,YearsTotLoadExcLowestPremtax
        ,kludge_account_value
        );

    // Saved for monthly detail report only. TAXATION !! Then are
    // these still needed...perhaps in order to report their values
    // prior to accepting any payment?
    NetMaxNecessaryPremium   = Irc7702A_->DebugGetNetMaxNecPm  ();
    GrossMaxNecessaryPremium = Irc7702A_->DebugGetGrossMaxNecPm();

    TxAscertainDesiredPayment();
    TxLimitPayment(max_non_mec_premium);

    // TAXATION !! This line recognizes a withdrawal for 7702A;
    // shouldn't there be similar code here for GPT?
    if(0 == Month)
        {
        Irc7702A_->UpdatePmt7702A
            (Dcv
            ,-NetWD // TAXATION !! This should be gross, not net.
            ,false
            ,AnnualTargetPrem
            ,YearsTotLoadTgtLowestPremtax
            ,YearsTotLoadExcLowestPremtax
            ,kludge_account_value
            );
        }

    double gross_1035 = 0.0;
    if(0 == Year && 0 == Month)
        {
        gross_1035 = External1035Amount + Internal1035Amount;
        }
    double necessary_premium = std::min
        (GrossPmts[Month] - gross_1035
        ,max_necessary_premium
        );
    double unnecessary_premium = material_difference
        (GrossPmts[Month]
        ,gross_1035 + necessary_premium
        );

    // It is crucial to accept necessary premium before processing a
    // material change, so that the correct DCV is used.
    TxRecognizePaymentFor7702A(necessary_premium, false);
    TxAcceptPayment(necessary_premium);
    if(0.0 < unnecessary_premium)
        {
        Irc7702A_->InduceMaterialChange();
        }

    // Process any queued material change for this day. Illustrations
    // process all BOM transactions as if on the month's first day.
    // Material changes occurring on the same day (e.g. unnecessary
    // premium triggering a corridor DB increase, depending on the 7702A
    // interpretation chosen) are queued to be processed together.
    Irc7702A_->RedressMatChg
        (Dcv // Potentially modified.
        ,unnecessary_premium
        ,necessary_premium
        ,CashValueFor7702()
        );
    LMI_ASSERT(0.0 <= Dcv);

    UnnecessaryPremium = unnecessary_premium;
    TxRecognizePaymentFor7702A(unnecessary_premium, true);
    TxAcceptPayment(unnecessary_premium);

    TxTakeLoan();
    TxLoanRepay();
    TxSetBOMAV();
    TxTestHoneymoonForExpiration();
    TxSetDeathBft();
    TxSetTermAmt();
    TxSetCoiCharge();
    TxSetRiderDed();
    TxDoMlyDed();
}

//============================================================================
// Monthly transactions that follow monthly deduction.
void AccountValue::DoMonthCR()
{
    TxTakeSepAcctLoad();
    TxLoanInt();
    TxCreditInt();
    TxTestLapse();
    FinalizeMonth();
    TxDebug();
}

//============================================================================
// Apportion all payments among accounts.
void AccountValue::process_payment(double payment)
{
    // Apply ee and er net payments according to database rules.
    // Net payments were already aggregated, then split between
    // necessary and unnecessary, as required for 7702A on CVAT
    // contracts. We deem net payments to have the same proportion
    // as gross payments, which we do have for ee and er separately.

    HOPEFULLY(0.0 <= GrossPmts[Month]);
    HOPEFULLY(0.0 <= EeGrossPmts[Month]);
    HOPEFULLY(0.0 <= ErGrossPmts[Month]);

    double net_pmt = payment;

    double gross_1035 = 0.0;
    if(0 == Year && 0 == Month)
        {
        gross_1035 = External1035Amount + Internal1035Amount;
        }
    double gross_non_1035_pmts = GrossPmts[Month] - gross_1035;
    double er_ratio = 0.0;
    if(0.0 != gross_non_1035_pmts)
        {
        er_ratio = ErGrossPmts[Month] / gross_non_1035_pmts;
        }
    double er_net_pmt = er_ratio * net_pmt;
    double ee_net_pmt = net_pmt - er_net_pmt;

    switch(ee_premium_allocation_method)
        {
        case oe_input_allocation:
            {
            IncrementAVProportionally(ee_net_pmt);
            }
            break;
        case oe_override_allocation:
            {
            IncrementAVPreferentially(ee_net_pmt, ee_premium_preferred_account);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << ee_premium_allocation_method
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    switch(er_premium_allocation_method)
        {
        case oe_input_allocation:
            {
            IncrementAVProportionally(er_net_pmt);
            }
            break;
        case oe_override_allocation:
            {
            IncrementAVPreferentially(er_net_pmt, er_premium_preferred_account);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << er_premium_allocation_method
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // TODO ?? Perhaps now we should transfer funds from the separate
    // to the general account to make the latter nonnegative if needed.
}

//============================================================================
// Prorate increments to account value between separate- and general-account
// portions of unloaned account value according to input allocations.
void AccountValue::IncrementAVProportionally(double increment)
{
    AVGenAcct += increment * GenAcctPaymentAllocation;
    AVSepAcct += increment * SepAcctPaymentAllocation;
}

//============================================================================
// Apply increments to account value to the preferred account.
void AccountValue::IncrementAVPreferentially
    (double                             increment
    ,oenum_increment_account_preference preferred_account
    )
{
    switch(preferred_account)
        {
        case oe_prefer_general_account:
            {
            AVGenAcct += increment;
            }
            break;
        case oe_prefer_separate_account:
            {
            AVSepAcct += increment;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << preferred_account
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
}

/// Apportion all charges to be deducted from account value among
/// accounts.

void AccountValue::process_deduction(double decrement)
{
    switch(deduction_method)
        {
        case oe_proportional:
            {
            DecrementAVProportionally(decrement);
            }
            break;
        case oe_progressive:
            {
            DecrementAVProgressively(decrement, deduction_preferred_account);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << deduction_method
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
}

/// Apportion all distributions from account value among accounts.

void AccountValue::process_distribution(double decrement)
{
    switch(distribution_method)
        {
        case oe_proportional:
            {
            DecrementAVProportionally(decrement);
            }
            break;
        case oe_progressive:
            {
            DecrementAVProgressively(decrement, distribution_preferred_account);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << distribution_method
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
}

/// Prorate decrements to account value between separate- and general-
/// account portions of unloaned account value according to their
/// balances. Treat a negative balance in either account as zero for
/// proration, because its absolute value is irrelevant. If neither
/// balance is positive, then prorate decrement by input fund
/// allocations.
///
/// If a decrement is materially equal to unloaned account value, then
/// force both separate- and general-account values to exactly zero.
/// Otherwise, unloaned account value might have a minuscule negative
/// value due to catastrophic cancellation, improperly causing lapse.

void AccountValue::DecrementAVProportionally(double decrement)
{
    if(materially_equal(decrement, AVGenAcct + AVSepAcct))
        {
        AVGenAcct = 0.0;
        AVSepAcct = 0.0;
        return;
        }

    double general_account_proportion  = 0.0;
    double separate_account_proportion = 0.0;
    stifle_warning_for_unused_value(general_account_proportion );
    stifle_warning_for_unused_value(separate_account_proportion);
    double general_account_nonnegative_assets  = std::max(0.0, AVGenAcct);
    double separate_account_nonnegative_assets = std::max(0.0, AVSepAcct);
    if
        (  0.0 == general_account_nonnegative_assets
        && 0.0 == separate_account_nonnegative_assets
        )
        {
        general_account_proportion  = GenAcctPaymentAllocation;
        separate_account_proportion = SepAcctPaymentAllocation;
        }
    else
        {
        general_account_proportion =
              general_account_nonnegative_assets
            / ( general_account_nonnegative_assets
              + separate_account_nonnegative_assets
              )
            ;
        LMI_ASSERT
            (                         0.0 <= general_account_proportion
            && general_account_proportion <= 1.0
            );
        separate_account_proportion = 1.0 - general_account_proportion;
        }
    LMI_ASSERT
        (materially_equal
            (general_account_proportion + separate_account_proportion
            ,1.0
            )
        );
    AVGenAcct -= decrement * general_account_proportion;
    AVSepAcct -= decrement * separate_account_proportion;
}

/// Apportion decrements to account value between separate- and
/// general-account portions of unloaned account value, applying them
/// to the preferred account to the extent possible without making
/// that account negative.
///
/// If a decrement is materially equal to unloaned account value, then
/// force both separate- and general-account values to exactly zero.
/// Otherwise, unloaned account value might have a minuscule negative
/// value due to catastrophic cancellation, improperly causing lapse.

void AccountValue::DecrementAVProgressively
    (double                             decrement
    ,oenum_increment_account_preference preferred_account
    )
{
    if(materially_equal(decrement, AVGenAcct + AVSepAcct))
        {
        AVGenAcct = 0.0;
        AVSepAcct = 0.0;
        return;
        }

    switch(preferred_account)
        {
        case oe_prefer_general_account:
            {
            AVGenAcct -= progressively_reduce(AVGenAcct, AVSepAcct, decrement);
            }
            break;
        case oe_prefer_separate_account:
            {
            AVGenAcct -= progressively_reduce(AVSepAcct, AVGenAcct, decrement);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << preferred_account
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
}

//============================================================================
void AccountValue::TxExch1035()
{
    if(!(0 == Year && 0 == Month))
        {
        return;
        }

    HOPEFULLY(Year == InforceYear);
    HOPEFULLY(Month == InforceMonth);
    HOPEFULLY(0.0 == GrossPmts[Month]);
    HOPEFULLY(0.0 == EeGrossPmts[Month]);
    HOPEFULLY(0.0 == ErGrossPmts[Month]);

    // Policy issue date is always a modal payment date.
    GrossPmts[Month] = External1035Amount + Internal1035Amount;

// TODO ?? TAXATION !! This looks like a good idea, but it would prevent the
// initial seven-pay premium from being set.
//    if(0.0 == GrossPmts[Month])
//        {
//        return;
//        }

// TAXATION !! This condition, or a similar one, guards several GPT calls,
// but not all (ProcessAdjustableEvent(), e.g.); should this be made uniform?
// TODO ?? Perhaps this condition should be:
//    if(!SolvingForGuarPremium && Solving || mce_run_gen_curr_sep_full == RunBasis_)
    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        if(!SolvingForGuarPremium)
            {
            double fake_cum_pmt = 0.0; // TODO ?? TAXATION !! Needs work.
            Irc7702_->ProcessGptPmt(Year, GrossPmts[Month], fake_cum_pmt);
            }
        // Limit external 1035 first, then internal, as necessary to avoid
        // exceeding the guideline limit. This is what the customer would
        // normally want, because an internal exchange might be free of
        // premium tax.
        progressively_limit(External1035Amount, Internal1035Amount, GrossPmts[Month]);

        OverridingExternal1035Amount = External1035Amount;
        OverridingInternal1035Amount = Internal1035Amount;
        }
    else
        {
        External1035Amount = OverridingExternal1035Amount;
        Internal1035Amount = OverridingInternal1035Amount;
        GrossPmts[Month] = External1035Amount + Internal1035Amount;
        }

    // TODO ?? Assume for now that all 1035 exchanges represent ee payments.
    EeGrossPmts[Month] = GrossPmts[Month];
    double amount_exempt_from_premium_tax = 0.0;
    if(WaivePmTxInt1035)
        {
        amount_exempt_from_premium_tax = Internal1035Amount;
        }
    double actual_load = GetPremLoad
        (GrossPmts[Month]
        ,amount_exempt_from_premium_tax
        );

    NetPmts[Month] = GrossPmts[Month] - actual_load;

    HOPEFULLY(0.0 == AVGenAcct);
    HOPEFULLY(0.0 == AVSepAcct);
    process_payment(NetPmts[Month]);

    DBReflectingCorr = 0.0;
    TxSetDeathBft();
    TxSetTermAmt();
    // TODO ?? TAXATION !! Should 1035 exchanges be handled somewhere else?
    HOPEFULLY(0.0 == Dcv);
    Irc7702A_->Update1035Exch7702A
        (Dcv
        ,NetPmts[Month]
        // TAXATION !! This assumes the term rider can be treated as death benefit;
        // use 'TermIsDbFor7702A'.
        ,ActualSpecAmt + TermSpecAmt
//        ,DBReflectingCorr + TermDB // TAXATION !! Alternate if 7702A benefit is DB?
        );

    if(HoneymoonActive)
        {
        HoneymoonValue += std::max(0.0, GrossPmts[Month]);
        }

    CumPmts += GrossPmts[Month];
    TaxBasis +=
          yare_input_.External1035ExchangeBasis
        + yare_input_.Internal1035ExchangeBasis
        ;

    if(mce_run_gen_curr_sep_full == RunBasis_)
        {
        // Immediately after a 1035 exchange, DCV should be
        // the 1035 amount reduced by any premium-based loads,
        // but only for the current rate basis.
        HOPEFULLY(materially_equal(Dcv, NetPmts[Month]));

        // The initial seven-pay premium shown on the illustration
        // must be its value immediately after any 1035 exchange,
        // which by its nature occurs before a seven-pay premium
        // can be calculated.
        InvariantValues().InitSevenPayPrem = round_max_premium()
            (Irc7702A_->GetPresent7pp()
            );
        }

    HOPEFULLY
        (materially_equal
            (GrossPmts[Month]
            ,EeGrossPmts[Month] + ErGrossPmts[Month]
            )
        );
}

//============================================================================
double AccountValue::CashValueFor7702() const
{
    return std::max
        (HoneymoonValue
        ,   TotalAccountValue()
          + GetRefundableSalesLoad()
//          + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        );
}

//============================================================================
// Actual monthly rate reflecting optional daily interest accounting.
double AccountValue::ActualMonthlyRate(double monthly_rate) const
{
    if(daily_interest_accounting)
        {
        LMI_ASSERT(   0 != days_in_policy_year);
        LMI_ASSERT(-1.0 <= monthly_rate);
        return -1.0 + std::pow
            (1.0 + monthly_rate
            ,12.0 * days_in_policy_month / days_in_policy_year
            );
        }
    else
        {
        return monthly_rate;
        }
}

//============================================================================
// Rounded interest increment.
double AccountValue::InterestCredited
    (double principal
    ,double monthly_rate
    ) const
{
    return round_interest_credit()(principal * ActualMonthlyRate(monthly_rate));
}

//============================================================================
bool AccountValue::IsModalPmtDate(mcenum_mode mode) const
{
    return 0 == Month % (12 / mode);
}

//============================================================================
int AccountValue::MonthsToNextModalPmtDate() const
{
    // TODO ?? Answer is in terms of *ee* mode only, but it seems
    // wrong to ignore *er* mode.
    return 1 + (11 - Month) % (12 / InvariantValues().EeMode[Year].value());
}

/// Determine instantaneous base-policy minimum specified amount.
///
/// Argument 'issuing_now' indicates whether the policy is being
/// issued at the present moment: i.e., this is the first month of the
/// first policy year (and therefore the policy is not in force yet).
///
/// Argument 'term_rider' indicates whether a term rider is to be
/// taken into account, as that affects the base-policy minimum.

double AccountValue::minimum_specified_amount(bool issuing_now, bool term_rider) const
{
    return
          issuing_now
        ? (term_rider ? MinIssBaseSpecAmt  : MinIssSpecAmt )
        : (term_rider ? MinRenlBaseSpecAmt : MinRenlSpecAmt)
        ;
}

//============================================================================
// All changes to SA must be handled here.
// Proportionately reduce base and term SA if term rider present.
// Make sure ActualSpecAmt is never less than minimum specamt.
void AccountValue::ChangeSpecAmtBy(double delta)
{
    double ProportionAppliedToTerm = 0.0;
    double prior_specamt = ActualSpecAmt;
    if(TermRiderActive)
        {
        switch(yare_input_.TermAdjustmentMethod)
            {
            case mce_adjust_term:
                {
                ProportionAppliedToTerm = 1.0;
                }
                break;
            case mce_adjust_both:
                {
                ProportionAppliedToTerm =
                    TermSpecAmt
                    / (ActualSpecAmt + TermSpecAmt);
                }
                break;
            case mce_adjust_base:
                {
                ProportionAppliedToTerm = 0.0;
                }
                break;
            default:
                {
                fatal_error()
                    << "Case "
                    << yare_input_.TermAdjustmentMethod
                    << " not found."
                    << LMI_FLUSH
                    ;
                }
            }

        if(!AllowTerm || !yare_input_.TermRider || !TermRiderActive)
            {
            TermSpecAmt = 0.0;
            ProportionAppliedToTerm = 0.0;
            TermRiderActive = false;
            }

        ActualSpecAmt += delta * (1.0 - ProportionAppliedToTerm);
        TermSpecAmt += delta * ProportionAppliedToTerm;
        if(TermSpecAmt < 0.0)
            {
            EndTermRider();
            }
        else
            {
            // TODO ?? Only the mce_adjust_base method seems to work
            // correctly. More care must be given to rounding and to
            // minimums, and the order of adjustment (and term-rider
            // removal) in Input::make_term_rider_consistent() as well
            // as here.
            if(TermRiderActive)
                {
                TermSpecAmt =
                      std::max(TermSpecAmt + ActualSpecAmt, MinRenlSpecAmt)
                    - ActualSpecAmt
                    ;
                }
            TermSpecAmt = round_specamt()(TermSpecAmt);
            }
        }
    else
        {
        ActualSpecAmt += delta;
        }

    // If the minimum isn't met, then force it.
    ActualSpecAmt = std::max
        (ActualSpecAmt
        ,minimum_specified_amount(0 == Year && 0 == Month, TermRiderActive)
        );
    ActualSpecAmt = round_specamt()(ActualSpecAmt);
    AddSurrChgLayer(Year, std::max(0.0, ActualSpecAmt - prior_specamt));

    // Carry the new specamt forward into all future years.
    for(int j = Year; j < BasicValues::GetLength(); j++)
        {
// TODO ?? This seems wrong. If we're changing something that doesn't
// vary across bases, why do we change it for all bases?
// TODO ?? Shouldn't this be moved to FinalizeMonth()? The problem is
// that the ledger object is used for working storage, where it should
// probably be write-only instead.
        InvariantValues().SpecAmt[j] = ActualSpecAmt;
        InvariantValues().TermSpecAmt[j] = TermSpecAmt;
// Term specamt is a vector in class LedgerInvariant, but a scalar in
// the input classes, e.g.:
//   yare_input_.TermRiderAmount
// as is appropriate for a 7702-integrated term rider. Another sort of
// term rider might call for vector input.
        }
    // Reset DB whenever SA changes.
    TxSetDeathBft();
}

//============================================================================
// The spec amt used as the basis for surrender charges is not
// always the current spec amt, but rather the original spec amt
// adjusted for withdrawals only. This function simply decreases
// this special spec amt by the same amount as the normal spec amt
// decreases, and only in the event of a withdrawal, which is
// constrained to be nonnegative by constraining the additive
// adjustment to be nonpositive. Other approaches are possible.
void AccountValue::ChangeSurrChgSpecAmtBy(double delta)
{
    HOPEFULLY(delta <= 0.0);
    SurrChgSpecAmt += delta;
    SurrChgSpecAmt = std::max(0.0, SurrChgSpecAmt);
    // TODO ?? 'SurrChgSpecAmt' isn't used yet.

    // SOMEDAY !! Recalculation of GDB premium is not yet implemented.
    // It is fairly common to let withdrawals affect it. If this is
    // the best place to do that, then perhaps this function should
    // be renamed, since it wouldn't merely change 'SurrChgSpecAmt'.
}

//============================================================================
void AccountValue::InitializeMonth()
{
    GptForceout       = 0.0;
    premium_load_     = 0.0;
    sales_load_       = 0.0;
    premium_tax_load_ = 0.0;
    dac_tax_load_     = 0.0;

    // Set BOM DB for 7702 and 7702A.
    TxSetDeathBft();
    TxSetTermAmt();

    // TAXATION !! 'OldSA' and 'OldDB' need to be distinguished for 7702 and 7702A,
    // with inclusion of term dependent on 'TermIsDbFor7702' and 'TermIsDbFor7702A'.
    OldSA = ActualSpecAmt + TermSpecAmt;
    OldDB = DBReflectingCorr + TermDB;

    // TODO ?? CVAT only? TAXATION !! Consider whether and how 7702A
    // functions should be guarded generally, with due regard to the
    // definitional test chosen. This function probably should be
    // called for GPT as well as CVAT.
    Irc7702A_->UpdateBOM7702A(Month);
}

//============================================================================
// Death benefit option change.
// Assume surrender charge is not affected by this transaction.
// Assume target premium rate is not affected by this transaction.
// Assume change to option 2 mustn't decrease spec amt below minimum.
void AccountValue::TxOptionChange()
{
    // Illustrations allow option changes only on anniversary,
    // but not on the zeroth anniversary.
    if(0 != Month || 0 == Year)
        {
        return;
        }

    // It's OK to index by [Year - 1] because above we return early
    // if 0 == Year.
    mcenum_dbopt const old_option = DeathBfts_->dbopt()[Year - 1];

    // Nothing to do if no option change requested.
    if(old_option == YearsDBOpt)
        {
        return;
        }

    if(!AllowChangeToDBO2 && mce_option2 == YearsDBOpt)
        {
        fatal_error()
            << "Change to increasing death benefit option"
            << " not allowed on this policy form."
            << LMI_FLUSH
            ;
        return;
        }

    if(NoLapseActive &&  mce_option1 != YearsDBOpt &&  NoLapseOpt1Only)
        {
        NoLapseActive = false;
        }

    // An option change still needs to be processed even if AV <= 0.0,
    // e.g. because a dbopt change to or from ROP should have an
    // effect even if the contract remains in force only by virtue
    // of a no-lapse provision. But specamt changes to or from option
    // 2 should reflect AV only to the extent it's positive.

    // Change specified amount, keeping amount at risk invariant.
    switch(YearsDBOpt)
        {
        case mce_option1:
            {
            if(OptChgCanIncrSA)
                {
                if(mce_option2 == old_option)
                    {
                    ChangeSpecAmtBy(std::max(0.0, TotalAccountValue()));
                    }
                else if(mce_rop == old_option)
                    {
                    ChangeSpecAmtBy(std::max(0.0, CumPmts));
                    }
                else
                    {
                    fatal_error() << "Unknown death benefit option." << LMI_FLUSH;
                    }
                }
            else
                {
                // Do nothing.
                }
            }
            break;
        // TODO ?? Changes to option 1 take into account the old dbopt, but
        // changes to other options defectively do not.
        case mce_option2:
            if(OptChgCanDecrSA)
                {
                ChangeSpecAmtBy(-std::max(0.0, TotalAccountValue()));
                }
            else
                {
                // Do nothing.
                }
            break;
        case mce_rop:
            if(OptChgCanDecrSA)
                {
                ChangeSpecAmtBy(-std::max(0.0, CumPmts));
                }
            else
                {
                // Do nothing.
                }
            break;
        default:
            {
            fatal_error() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }
}

//============================================================================
// Specified amount change: increase or decrease.
// Ignores multiple layers of coverage: not correct for select and
// ultimate COI rates if select period restarts on increase.
// Assumes target premium rate is not affected by increases or decreases.
// TODO ?? Is this the right place to change target premium?
void AccountValue::TxSpecAmtChange()
{
    // Illustrations allow increases and decreases only on anniversary,
    // but not on the zeroth anniversary.
    if(0 != Month || 0 == Year)
        {
        // What needful thing does this accomplish?
        // Set option 2 death benefit, e.g.?

// > This initializes DBReflectingCorr and others so that the at-issue but
// > post-1035 exchange calculation of the seven pay will have a death benefit
// > amount to base its calculations on.
//
// TODO ?? We should find a better way. The original intention of this block
// was to return immediately without doing anything at all except on renewal
// anniversaries, because elective spec amt changes are not allowed at the
// issue date or in any off-anniversary month. The line added to change the
// specified amount by zero is executed almost every month. The call is costly
// and very probably superfluous. It is probably one reason why calculations
// are so slow when the term rider is present. ChangeSpecAmtBy() was designed
// to be called only when the spec amt changes; calling it with an argument
// of zero is a rather bizarre concept.

        ChangeSpecAmtBy(0.0);
        return;
        }

    LMI_ASSERT(0 < Year);
    double const old_specamt = DeathBfts_->specamt()[Year - 1];

    // Nothing to do if no increase or decrease requested.
    // TODO ?? YearsSpecAmt != ActualSpecAmt; the latter should be used.
    if(YearsSpecAmt == old_specamt)
        {
        return;
        }

    if
        (
            !AllowSAIncr
        &&  ActualSpecAmt < YearsSpecAmt
        )
        {
        fatal_error()
            << "Specified-amount increases not allowed on this policy form."
            << LMI_FLUSH
            ;
        return;
        }

    if
        (
            MaxIncrAge < Year + BasicValues::GetIssueAge()
        &&  ActualSpecAmt < YearsSpecAmt
        )
        {
        fatal_error()
            << "Cannot increase specified amount after age "
            << MaxIncrAge
            << " on this policy form."
            << LMI_FLUSH
            ;
        return;
        }

    ChangeSpecAmtBy(YearsSpecAmt - ActualSpecAmt);

    // TODO ?? TAXATION !! Should 7702 or 7702A processing be done here?
}

//============================================================================
void AccountValue::TxTestGPT()
{
    if(mce_gpt != DefnLifeIns_ || mce_run_gen_curr_sep_full != RunBasis_)
        {
        return;
        }

    // Adjustable events are not restricted to anniversary, even for
    // illustrations: for instance, payment of premium with an ROP
    // DB option.
    //
    // Illustrations allow no adjustable events at issue.
    // TODO ?? TAXATION !! If this assumption is not valid, then OldSA, OldDB, and
    // OldDBOpt need to be initialized more carefully. It's not valid as long as
    // withdrawals are not forbidden in the first year.
    if(0 == Year && 0 == Month)
        {
        return;
        }

    // Guideline premium must be determined before premium is processed.
    // Before doing that, we have to determine DB; usually, that will
    // have been done already, because ChangeSpecAmtBy() is called for
    // any transaction that changes specamt, and that function calls
    // TxSetDeathBft()--but not in the case that dbopt changes and
    // specamt does not. It's difficult to see this at a glance, and
    // to prevent any later change from invalidating this analysis, so
    // instead of calling TxSetDeathBft() in that (dbopt change) case,
    // we unconditionally call TxSetDeathBft() here.
    //
    // Furthermore, we need to know the term specified amount, but
    // that is normally not determined until later.
    //
    // The specs need to be updated (section 5.9). Premium payments
    // change the ROP DB (unless we're in the corridor), but this
    // effect cannot be combined with other adjustable events because
    // the premium in question must first be tested against the
    // guideline premium limit. We should probably ignore any effect
    // of ROP premium unless a forceout is required. TODO ?? TAXATION !! Confirm this.
    //
    TxSetDeathBft();
    TxSetTermAmt();

    // Nothing to do if no adjustable event.
    //
    // A change in DB alone is not an adjustable event.
    // A change in specamt is an adjustable event iff DB also changes.
    // If there is an adjustable event, then DB (not specamt) is used
    // in calculating the adjustment.
    //
    mcenum_dbopt_7702 const new_dbopt(effective_dbopt_7702(YearsDBOpt, Equiv7702DBO3));
    mcenum_dbopt_7702 const old_dbopt(effective_dbopt_7702(OldDBOpt  , Equiv7702DBO3));
    // TAXATION !! This may require revision if DB is treated as the 7702 benefit.
    // TAXATION !! This assumes the term rider can be treated as death benefit;
    // use 'TermIsDbFor7702'.
    bool adj_event =
            (
                !materially_equal(OldSA, ActualSpecAmt + TermSpecAmt)
            &&  !materially_equal(OldDB, DBReflectingCorr + TermDB)
            )
        ||  old_dbopt != new_dbopt
        ;
    if(adj_event)
        {
        // TODO ?? TAXATION !! Perhaps we should pass 'A' of 'A+B-C' for validation.
        // Or maybe not, because we can't match it if there was a plan change.
        Irc7702_->ProcessAdjustableEvent
            (Year
            ,DBReflectingCorr + TermDB
            ,OldDB
            // TAXATION !! This assumes the term rider can be treated as death benefit;
            // use 'TermIsDbFor7702'.
            ,ActualSpecAmt + TermSpecAmt
            ,OldSA
            ,new_dbopt
            ,old_dbopt
            ,AnnualTargetPrem
            );
        }

    GptForceout = Irc7702_->Forceout();
    // TODO ?? TAXATION !! On other bases, nothing is forced out, and payments aren't limited.
    process_distribution(GptForceout);
    YearsTotalGptForceout += GptForceout;

    // SOMEDAY !! Actually, forceouts reduce basis only to the extent
    // they're nontaxable. For now, tax basis is used only as a limit
    // (if requested) on withdrawals, and it is conservative for that
    // purpose to underestimate basis.

    CumPmts     -= GptForceout;
    TaxBasis    -= GptForceout;

    if(adj_event)
        {
        Irc7702A_->InduceMaterialChange();
        }

    // TODO ?? TAXATION !! GPT--perform only if current basis?
    OldDBOpt = YearsDBOpt;
}

//============================================================================
// All payments must be made here.
// Process premium payment reflecting premium load.
// TODO ?? TAXATION !! Contains hooks for guideline premium test; they need to be
//   fleshed out.
// Ignores strategies such as pay guideline premium, which are handled
//   in PerformE[er]PmtStrategy().
// Ignores no-lapse periods and other death benefit guarantees.
//
// SOMEDAY !! Some systems force monthly premium to be integral cents even
// though actual mode is not monthly; is that something we need to do here?

/*
Decide whether we need to do anything
    no pmt this month due to mode
    zero pmt
Perform strategy
Test 7702, 7702A
    apportion limited prem across ee, er,...dumpin?
    need to limit pmt here, but other events e.g. WD affect limits
    pmts must be the same on all bases
        7702A effect varies by basis
        does GPT effect also vary by basis?
            e.g. when opt change produces different spec amts *
Handle dumpins
What to store?
Deduct load (varies by basis)
Apportion across gen, sep accounts
Update cum pmts, tax basis, DCV
*/

//============================================================================
void AccountValue::TxAscertainDesiredPayment()
{
    // Do nothing if this is not a modal payment date.
    // TODO ?? There has to be a better criterion for early termination.
    bool ee_pay_this_month  = IsModalPmtDate(InvariantValues().EeMode[Year].value());
    bool er_pay_this_month  = IsModalPmtDate(InvariantValues().ErMode[Year].value());

    if(!ee_pay_this_month && !er_pay_this_month)
        {
        return;
        }

    //  ForceOut = 0.0;
    //  double GuidelinePremLimit = 0.0;

    // Pay premium--current basis determines premium for all bases.

    HOPEFULLY(materially_equal(GrossPmts[Month], EeGrossPmts[Month] + ErGrossPmts[Month]));

    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        if(ee_pay_this_month)
            {
            double eepmt = PerformEePmtStrategy();
            // Don't enforce the GPT premium limit when solving for
            // illustration-reg guaranteed premium.
            if(!SolvingForGuarPremium)
                {
                double fake_cum_pmt = 0.0; // TODO ?? TAXATION !! Needs work.
                Irc7702_->ProcessGptPmt(Year, eepmt, fake_cum_pmt);
                }
            EeGrossPmts[Month] += eepmt;
            GrossPmts[Month] += eepmt;
            }
        if(er_pay_this_month)
            {
            double erpmt = PerformErPmtStrategy();
            // Don't enforce the GPT premium limit when solving for
            // illustration-reg guaranteed premium.
            if(!SolvingForGuarPremium)
                {
                double fake_cum_pmt = 0.0; // TODO ?? TAXATION !! Needs work.
                Irc7702_->ProcessGptPmt(Year, erpmt, fake_cum_pmt);
                }
            ErGrossPmts[Month] += erpmt;
            GrossPmts[Month] += erpmt;
            }
        }

    HOPEFULLY(materially_equal(GrossPmts[Month], EeGrossPmts[Month] + ErGrossPmts[Month]));
    HOPEFULLY(GrossPmts[Month] < 1.0e100);

    // Guideline limits are imposed in this order:
    //   1035 exchanges
    //   employee vector-input premium
    //   employer vector-input premium
    //   dumpin
    // This order is pretty much arbitrary, except that 1035 exchanges
    // really must be processed first. An argument could be made for
    // changing the order of employee and employer premiums. An
    // argument could be made for grouping dumpin with employee
    // premiums, at least as long as we treat dumpin as employee
    // premium. Even though dumpin and 1035 exchanges are similar in
    // that both are single payments notionally made at issue, it is
    // not necessary to group them together: 1035 exchanges have a
    // unique nature that requires them to be recognized before any
    // premium is paid, and dumpins do not share that nature.

    if(0 == Year && 0 == Month && (Solving || mce_run_gen_curr_sep_full == RunBasis_))
        {
        // Don't enforce the GPT premium limit when solving for
        // illustration-reg guaranteed premium.
        if(!SolvingForGuarPremium)
            {
            double fake_cum_pmt = 0.0; // TODO ?? TAXATION !! Needs work.
            Irc7702_->ProcessGptPmt(Year, Dumpin, fake_cum_pmt);
            }
        EeGrossPmts[Month] += Dumpin;
        GrossPmts[Month] += Dumpin;
        }
    HOPEFULLY(materially_equal(GrossPmts[Month], EeGrossPmts[Month] + ErGrossPmts[Month]));
}

//============================================================================
// TAXATION !! Should this be called for gpt? or, if it's called,
// should it assert that it has no effect?
void AccountValue::TxLimitPayment(double a_maxpmt)
{
// Subtract premium load from gross premium yielding net premium.

    // This is needed only for current-basis or solve-basis runs.
    // Otherwise we're doing too much work, and maybe doing things
    // we shouldn't.
// TODO ?? TAXATION !! Clean this up, and put GPT limit here, on prem net of WD.

    HOPEFULLY(materially_equal(GrossPmts[Month], EeGrossPmts[Month] + ErGrossPmts[Month]));

    if(mce_reduce_prem == yare_input_.AvoidMecMethod && !Irc7702A_->IsMecAlready())
        {
        double gross_1035 = 0.0;
        if(0 == Year && 0 == Month)
            {
            gross_1035 = External1035Amount + Internal1035Amount;
            }
        double gross_pmt_without_1035 = GrossPmts[Month] - gross_1035;
        gross_pmt_without_1035 = std::min(gross_pmt_without_1035, a_maxpmt);
        // TODO ?? For now at least, reduce employee premium first.
        progressively_limit
            (EeGrossPmts[Month]
            ,ErGrossPmts[Month]
            ,gross_pmt_without_1035
            );
        // Non-1035 dumpin is already treated as part of employee
        // payments; 1035 amounts, which stand outside the non-MEC
        // premium limit, have to be added back in.
        EeGrossPmts[Month] += gross_1035;
        GrossPmts[Month] = EeGrossPmts[Month] + ErGrossPmts[Month];
        }

    HOPEFULLY(materially_equal(GrossPmts[Month], EeGrossPmts[Month] + ErGrossPmts[Month]));

    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        OverridingEePmts[MonthsSinceIssue] = EeGrossPmts[Month];
        OverridingErPmts[MonthsSinceIssue] = ErGrossPmts[Month];
        }
    else
        {
        EeGrossPmts[Month] = OverridingEePmts[MonthsSinceIssue];
        ErGrossPmts[Month] = OverridingErPmts[MonthsSinceIssue];
        GrossPmts[Month] = EeGrossPmts[Month] + ErGrossPmts[Month];
        }

    HOPEFULLY
        (materially_equal
            (GrossPmts[Month]
            ,EeGrossPmts[Month] + ErGrossPmts[Month]
            )
        );
}

//============================================================================
void AccountValue::TxRecognizePaymentFor7702A
    (double a_pmt
    ,bool   a_this_payment_is_unnecessary
    )
{
    if(0.0 == a_pmt)
        {
        return;
        }

    // 1035 exchanges are handled in a separate transaction.

    // Policy issue date is always a modal payment date.

    double kludge_account_value = std::max(TotalAccountValue(), HoneymoonValue);
    if(0 == Year && 0 == Month)
        {
        kludge_account_value = Dcv;
        }
    kludge_account_value = std::max
        (HoneymoonValue
        ,   kludge_account_value
          + GetRefundableSalesLoad()
//          + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        );
    LMI_ASSERT(0.0 <= Dcv);

    // TODO ?? TAXATION !! Not correct yet--need to test pmt less deductible WD; and
    // shouldn't we deduct the *gross* WD? [Yes, if any fee is part of the
    // WD, which it normally is.]
    double amount_paid_7702A = a_pmt;
    Irc7702A_->UpdatePmt7702A
        (Dcv
        ,amount_paid_7702A
        ,a_this_payment_is_unnecessary
        ,AnnualTargetPrem
        ,YearsTotLoadTgtLowestPremtax
        ,YearsTotLoadExcLowestPremtax
        ,kludge_account_value
        );
}

//============================================================================
void AccountValue::TxAcceptPayment(double a_pmt)
{
    if(0.0 == a_pmt)
        {
        return;
        }

    HOPEFULLY(0.0 <= a_pmt);
    // Internal 1035 exchanges may be exempt from premium tax; they're
    // handled elsewhere, so here the exempt amount is always zero.
    double actual_load = GetPremLoad(a_pmt, 0.0);
    double net_pmt = a_pmt - actual_load;
    HOPEFULLY(0.0 <= net_pmt);
    NetPmts[Month] += net_pmt;

    // If a payment on an ROP contract is treated as an adjustable event,
    // then it should be processed here. The present implementation does
    // not treat it that way; the Blue Book (page 654) says that
    // automatic increases are not adjustable events, and the SA increase
    // due to a payment in this case might be considered automatic by IRS.

    process_payment(net_pmt);

    Dcv += std::max(0.0, net_pmt);
    LMI_ASSERT(0.0 <= Dcv);

    if(HoneymoonActive)
        {
        HoneymoonValue += std::max(0.0, a_pmt);
        }

    CumPmts += a_pmt;
    // TODO ?? This thing isn't really the tax basis as it should be
    // because we subtract all WDs without regard to taxability:
    // if WDs exceed basis, 7702A calculations are incorrect.
    // TAXATION !! Fix this. Nontaxable WDs are handled much like
    // GPT forceouts. Generally, WDs are nontaxable up to basis for
    // non-MECs, except for 7702(f)(7)(B-E).
    TaxBasis += a_pmt;
    // TODO ?? TAXATION !! Save ee and er bases separately e.g. for split dollar;
    // call them EeTaxBasis and ErTaxBasis.
}

/// Determine premium load.
///
/// The total load has several components:
///  - nonrefundable premium load
///  - refundable sales load
///  - premium-tax load
///  - DAC-tax load
/// which are applied separately (with due regard to variation by
/// target versus excess), added together, and then rounded.
///
/// Alternatively, the load factors may be totalled, and their sum
/// applied to the target and excess portions of the payment. This may
/// yield a slightly different result due to intermediate rounding,
/// which could be important for matching a particular admin system.
/// Therefore, both calculations are performed, and their results are
/// asserted to be materially equal--but only when the alternative
/// calculation doesn't require too many adjustments, in particular
/// when tiered premium tax is passed through as a load.

double AccountValue::GetPremLoad
    (double a_pmt
    ,double a_portion_exempt_from_premium_tax
    )
{
    double excess_portion;
    // All excess.
    if(0.0 == UnusedTargetPrem)
        {
        excess_portion = a_pmt;
        }
    // Part target, part excess.
    else if(UnusedTargetPrem < a_pmt)
        {
        excess_portion = a_pmt - UnusedTargetPrem;
        UnusedTargetPrem = 0.0;
        }
    // All target.
    else
        {
        excess_portion = 0.0;
        UnusedTargetPrem -= a_pmt;
        }
    double target_portion = a_pmt - excess_portion;

    premium_load_ =
            target_portion * YearsPremLoadTgt
        +   excess_portion * YearsPremLoadExc
        ;

    sales_load_ =
            target_portion * YearsSalesLoadTgt
        +   excess_portion * YearsSalesLoadExc
        ;
    LMI_ASSERT(0.0 <= sales_load_);
    CumulativeSalesLoad += sales_load_;

    premium_tax_load_ = PremiumTax_->calculate_load
        (a_pmt - a_portion_exempt_from_premium_tax
        ,*StratifiedCharges_
        );

    dac_tax_load_ = YearsDacTaxLoadRate * a_pmt;
    YearsTotalDacTaxLoad += dac_tax_load_;

    double sum_of_separate_loads =
          premium_load_
        + sales_load_
        + premium_tax_load_
        + dac_tax_load_
        ;
    LMI_ASSERT(0.0 <= sum_of_separate_loads);

    double total_load =
          target_portion * YearsTotLoadTgt
        + excess_portion * YearsTotLoadExc
        - a_portion_exempt_from_premium_tax * PremiumTax_->load_rate()
        ;
    LMI_ASSERT
        (   PremiumTax_->is_tiered()
        ||  materially_equal(total_load, sum_of_separate_loads)
        );

    return round_net_premium()(sum_of_separate_loads);
}

//============================================================================
double AccountValue::GetRefundableSalesLoad() const
{
    return CumulativeSalesLoad * YearsSalesLoadRefundRate;
}

//============================================================================
// TODO ?? This is untested, and probably isn't right.
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

// TODO ?? ActualLoan should be eliminated. It's used only in two functions,
// one that takes a loan, and one that repays a loan.

    // TODO ?? This idiom seems too cute. And it can return -0.0 .
    // Maximum repayment is total debt.
    ActualLoan = -std::min(-RequestedLoan, RegLnBal);

    process_distribution(ActualLoan);
    AVRegLn  += ActualLoan;
    RegLnBal += ActualLoan;
// TODO ?? First repay regular loan, then apply excess to preferred.
//  AVPrfLn  += ActualLoan;
//  PrfLnBal += ActualLoan;

// This seems wrong. If we're changing something that's invariant among
// bases, why do we change it for each basis?
// TODO ?? Shouldn't this be moved to FinalizeMonth()?
    InvariantValues().NewCashLoan[Year] = ActualLoan;
    // TODO ?? Consider changing loan_ullage_[Year] here.
}

//============================================================================
// Set account value before monthly deductions.
void AccountValue::TxSetBOMAV()
{
    // Subtract monthly policy fee and per K charge from account value.

    // These assignments must happen every month.

    // Set base for per K load at issue. Other approaches could be imagined.
    if(Year == InforceYear && Month == InforceMonth)
        {
        if(!yare_input_.TermRider)
            {
            HOPEFULLY(0.0 == InvariantValues().TermSpecAmt[0]);
            }
        double z;
        if(0 == Year && 0 == Month)
            {
            z = InvariantValues().TermSpecAmt[0] + InvariantValues().SpecAmt[0];
            }
        else
            {
            // TAXATION !! We could at least change this in xrc help elements:
            // USER !! User documentation should explain that this
            // includes any 7702-integrated term rider.
            //
            // Some products have loads that depend on the initial
            // specified amount, which probably includes term; if it
            // doesn't, then a new input field would need to be added.
            //
            // TAXATION !! Is this really desirable? INPUT !! Should we
            // instead capture specamt as of issue date for this purpose?
            z = yare_input_.SpecamtHistory.front();
            }
        SpecAmtLoadBase = std::max(z, NetPmts[Month] * YearsCorridorFactor);
        SpecAmtLoadBase = std::min(SpecAmtLoadBase, SpecAmtLoadLimit);
        }

    MonthsPolicyFees = YearsMonthlyPolicyFee;
    if(0 == Month)
        {
        MonthsPolicyFees    += YearsAnnualPolicyFee;
        }
    YearsTotalPolicyFee += MonthsPolicyFees;

    SpecAmtLoad = YearsSpecAmtLoadRate * SpecAmtLoadBase;
    YearsTotalSpecAmtLoad += SpecAmtLoad;

    process_deduction(MonthsPolicyFees + SpecAmtLoad);

    Dcv -= MonthsPolicyFees + SpecAmtLoad;
    Dcv = std::max(0.0, Dcv);
}

//============================================================================
// Set death benefit reflecting corridor and death benefit option.
void AccountValue::TxSetDeathBft(bool force_eoy_behavior)
{
    // TODO ?? TAXATION !! Should 7702 or 7702A processing be done here?
    // If so, then this code may be useful:
//    double prior_db_7702A = DB7702A;
//    double prior_sa_7702A = ActualSpecAmt;

    // Total account value is unloaned plus loaned.
    // TODO ?? Should we use CSV here?
    double AV = TotalAccountValue();
// TAXATION !! Revisit this--it affects 'DB7702A':
// > TxSetDeathBft() needs to be called every time a new solve-spec amt
// > is applied to determine the death benefit. But you don't really want to
// > add the sales load (actually a percent of the sales load) to the AV
// > each time. TxSetDeathBft() also gets called during the initial seven
// > pay calc whether or not there is a 1035 exchange. Since it is called
// > from within DoYear() and needs to be called from outside of DoYear(0)
// > even a non-solve will end up double counting the refund of PPL's.

    switch(YearsDBOpt)
        {
        case mce_option1:
            {
            DBIgnoringCorr = ActualSpecAmt;
            DB7702A        = ActualSpecAmt;
            }
            break;
        case mce_option2:
            {
            // Negative AV doesn't decrease death benefit.
            DBIgnoringCorr = ActualSpecAmt + std::max(0.0, AV);
            DB7702A        = ActualSpecAmt;
            }
            break;
        case mce_rop:
            {
            // SA + sum of premiums less withdrawals, but not < SA.
            DBIgnoringCorr = ActualSpecAmt + std::max(0.0, CumPmts);
            DB7702A        = ActualSpecAmt + std::max(0.0, CumPmts);
            }
            break;
        default:
            {
            fatal_error() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }

    // Surrender charges are generally ignored here, but any negative
    // surrender charge must be subtracted, increasing the account value.
    double cash_value_for_corridor =
          TotalAccountValue()
        - std::min(0.0, SurrChg())
        + GetRefundableSalesLoad()
//        + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        ;

    if
        (   force_eoy_behavior
        ||  contains(yare_input_.Comments, "idiosyncrasyV")
        )
        {
        // The corridor death benefit ought always to reflect the
        // honeymoon value. It always does if "idiosyncrasyV" is
        // specified; otherwise, to match account values (but not
        // death benefits) with an incorrect admin system, the
        // honeymoon value is respected only if end-of-year behavior
        // is forced--thus, year-end death benefit is correct, but
        // the effect of the honeymoon value on corridor calculations
        // (and hence mortality charges) is ignored for monthiversary
        // processing. The idea is that in the event of death the
        // faulty admin system's death-benefit calculation is ignored
        // and the correct death benefit is calculated manually.
        //
        cash_value_for_corridor = std::max
            (cash_value_for_corridor
            ,HoneymoonValue
            );
        }
    DBReflectingCorr = std::max
        (DBIgnoringCorr
        ,YearsCorridorFactor * cash_value_for_corridor
        );
    DBReflectingCorr = round_death_benefit()(DBReflectingCorr);
    // This overrides the value assigned above. There's more than one
    // way to interpret 7702A "death benefit"; this is just one.
    // TAXATION !! DATABASE !! Offer a choice of interpretations.
    DB7702A = DBReflectingCorr + TermDB;

    DcvDeathBft = std::max
        (DBIgnoringCorr
        ,   (
                YearsCorridorFactor
            *   (   Dcv
                +   GetRefundableSalesLoad()
//                +   std::max(0.0, ExpRatReserve) // This would be added if it existed.
                )
            )
        );
    // TODO ?? TAXATION !! Should 7702 or 7702A processing be done here?
    // If so, then this code may be useful:
/*
    // Try moving this here...
    Irc7702A_->UpdateBft7702A(...);
    LMI_ASSERT(0.0 <= Dcv);
*/

    // SOMEDAY !! Accumulate average death benefit for profit testing here.
}

//============================================================================
void AccountValue::TxSetTermAmt()
{
    if(!TermRiderActive)
        {
        return;
        }
    if(!AllowTerm || !yare_input_.TermRider)
        {
        TermRiderActive = false;
        return;
        }

    // TODO ?? Assumes that term rider lasts exactly as long as no-lapse guarantee.
    // DATABASE !! Similar entities should be established for term.
    if
        (  (BasicValues::NoLapseMinDur <= Year)
        && (BasicValues::NoLapseMinAge <= Year + BasicValues::GetIssueAge())
        )
        {
        EndTermRider();
        return;
        }

    TermDB = std::max(0.0, TermSpecAmt + DBIgnoringCorr - DBReflectingCorr);
    TermDB = round_death_benefit()(TermDB);
}

//============================================================================
// Terminate the term rider
void AccountValue::EndTermRider()
{
    if(!TermRiderActive)
        {
        return;
        }
    // TODO ?? Not yet implemented.
    // If insufficient AV for termchg, then term rider terminates.
    // In that case, assume that the owner has an opportunity to pay
    // the term charge in cash, and that term coverage continues by
    // grace until the next monthiversary even if it is not so paid.
    // Thus, termination of the term rider is not an adjustable event
    // until that next monthiversary when DB actually changes. But
    // illustrations show no such gratis continuation of the term
    // benefit, since they depict death benefits as at the end of
    // the anniversary month only.

    TermRiderActive = false;
    ChangeSpecAmtBy(TermSpecAmt);
    TermSpecAmt = 0.0;
    TermDB = 0.0;
    // Carry the new term spec amt forward into all future years.
    for(int j = Year; j < BasicValues::GetLength(); j++)
        {
        InvariantValues().TermSpecAmt[j] = TermSpecAmt;
        }
}

//============================================================================
// Calculate mortality charge.
void AccountValue::TxSetCoiCharge()
{
    // Net amount at risk is the death benefit discounted one month
    // at the guaranteed interest rate, minus account value iff
    // nonnegative (a negative account value mustn't increase NAAR);
    // but never less than zero. The account value could be negative
    // because of pending lapse on an admin system (whose inforce
    // values might be used in an illustration), or because of a
    // no-lapse guarantee. The subtraction could yield a value less
    // than zero because the corridor factor can be as low as unity,
    // but it's constrained to be nonnegative to prevent increasing
    // the account value by deducting a negative mortality charge.
    NAAR = material_difference
        (DBReflectingCorr * DBDiscountRate[Year]
        ,std::max(0.0, TotalAccountValue())
        );
    NAAR = std::max(0.0, round_naar()(NAAR));

// TODO ?? This doesn't work. We need to reconsider the basic transactions.
//  double naar_forceout = std::max(0.0, NAAR - MaxNAAR);
//  process_distribution(naar_forceout);
// TAXATION !! Should this be handled at the same time as GPT forceouts?

    DcvNaar = material_difference
        (std::max(DcvDeathBft, DBIgnoringCorr) * DBDiscountRate[Year]
        ,std::max(0.0, Dcv)
        );
    // DCV need not be rounded.
    DcvNaar = std::max(0.0, DcvNaar);

    double retention_charge = 0.0;
    double coi_rate = GetBandedCoiRates(GenBasis_, ActualSpecAmt)[Year];
    ActualCoiRate = coi_rate;

    // COI retention is a percentage of tabular rather than current
    // COI charge. It's a risk charge, so it follows mortality
    // expectations rather than fluctuations.
    //
    // TODO ?? For now, however, the COI retention calculated here
    // improvidently reflects the current COI multiplier, so end users
    // need to divide input CoiRetentionRate by that multiplier.

    if
        (   yare_input_.UseExperienceRating
        &&  mce_gen_curr == GenBasis_
        )
        {
        ActualCoiRate = round_coi_rate()
            (std::min
                (GetBandedCoiRates(mce_gen_guar, ActualSpecAmt)[Year]
                ,coi_rate * (case_k_factor + CoiRetentionRate)
                )
            );
        double retention_rate = round_coi_rate()(coi_rate * CoiRetentionRate);
        retention_charge = round_coi_charge()(NAAR * retention_rate);
        }

    CoiCharge    = round_coi_charge()(NAAR * ActualCoiRate);
    NetCoiCharge = CoiCharge - retention_charge;
    YearsTotalCoiCharge += CoiCharge;

    // DCV need not be rounded.
    DcvCoiCharge = DcvNaar * (YearsDcvCoiRate + CoiRetentionRate);
}

//============================================================================
// Calculate rider charges.
void AccountValue::TxSetRiderDed()
{
    AdbCharge = 0.0;
    if(yare_input_.AccidentalDeathBenefit)
        {
        AdbCharge = YearsAdbRate * std::min(ActualSpecAmt, AdbLimit);
        }

    SpouseRiderCharge = 0.0;
    if(yare_input_.SpouseRider)
        {
        SpouseRiderCharge = YearsSpouseRiderRate * yare_input_.SpouseRiderAmount;
        }
    ChildRiderCharge = 0.0;
    if(yare_input_.ChildRider)
        {
        ChildRiderCharge = YearsChildRiderRate * yare_input_.ChildRiderAmount;
        }

    TermCharge = 0.0;
    DcvTermCharge = 0.0;
    if(TermRiderActive && yare_input_.TermRider)
        {
        TermCharge    = YearsTermRate   * TermDB * DBDiscountRate[Year];
        DcvTermCharge = YearsDcvCoiRate * TermDB * DBDiscountRate[Year];
        }

    WpCharge = 0.0;
    DcvWpCharge = 0.0;
    if(yare_input_.WaiverOfPremiumBenefit)
        {
        switch(WaiverChargeMethod)
            {
            case oe_waiver_times_naar:
                {
                WpCharge = YearsWpRate * std::min(ActualSpecAmt, WpLimit);
                DcvWpCharge = WpCharge;
                }
                break;
            case oe_waiver_times_deductions:
                {
                // TODO ?? Should the separate-account load be waived?
                WpCharge =
                    YearsWpRate
                    *   (
                            CoiCharge
                        +   MonthsPolicyFees
                        +   SpecAmtLoad
                        +   AdbCharge
                        +   SpouseRiderCharge
                        +   ChildRiderCharge
                        +   TermCharge
                        );
                DcvWpCharge =
                    YearsWpRate
                    *   (
                            DcvCoiCharge
                        +   MonthsPolicyFees
                        +   SpecAmtLoad
                        +   AdbCharge
                        +   SpouseRiderCharge
                        +   ChildRiderCharge
                        +   DcvTermCharge
                        );
                }
                break;
            default:
                {
                fatal_error()
                    << "Case '"
                    << WaiverChargeMethod
                    << "' not found."
                    << LMI_FLUSH
                    ;
                }
            }
        }
}

//============================================================================
// Subtract monthly deductions from unloaned account value.
void AccountValue::TxDoMlyDed()
{
    // Subtract mortality and rider deductions from unloaned account value.
    // Policy fee was already subtracted in NAAR calculation.
    if(TermRiderActive && (AVGenAcct + AVSepAcct - CoiCharge) < TermCharge)
        {
        EndTermRider();
        TermCharge = 0.0;
        }

    // 'Simple' riders are the same for AV and DCV.
    double simple_rider_charges =
            AdbCharge
        +   SpouseRiderCharge
        +   ChildRiderCharge
        ;

    double dcv_mly_ded =
            DcvCoiCharge
        +   simple_rider_charges
        +   DcvTermCharge
        +   DcvWpCharge
        ;

    RiderCharges = simple_rider_charges + TermCharge + WpCharge;
    YearsTotalRiderCharges += RiderCharges;
    MlyDed = CoiCharge + RiderCharges;

    process_deduction(MlyDed);
    Dcv -= dcv_mly_ded;
    Dcv = std::max(0.0, Dcv);

    // Policy and issue fees and the specified-amount load are really
    // part of the monthly deduction, yet they must be kept distinct
    // so that they can be deducted before the mortality charge is
    // determined.
    MlyDed += MonthsPolicyFees + SpecAmtLoad;

    YearsTotalNetCoiCharge += NetCoiCharge;

    SepAcctValueAfterDeduction = AVSepAcct;
}

//============================================================================
void AccountValue::TxTestHoneymoonForExpiration()
{
    if(!HoneymoonActive)
        {
        return;
        }
    // This value includes separate account value and loan balance.
    // To our knowledge, honeymoon provisions aren't offered with
    // those features, but they should be reflected if present.
    // Loan balance should not be subtracted from this value: if it
    // were, then the owner could retain the honeymoon forever by
    // maintaining a maximum loan.
    //
    // Honeymoon provisions are probably inconsistent with experience
    // rating. If those features occur together, then presumably the
    // experience rating reserve would affect the cash surrender value
    // but not the honeymoon value.
    //
    double csv_ignoring_loan =
          TotalAccountValue()
        - SurrChg()
        + GetRefundableSalesLoad()
//        + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        ;

    // It may seem 'obvious' that 'HoneymoonValue' can never be
    // zero when 'HoneymoonActive' is true. But maybe someday
    // someone will change things so that becomes possible.
    // At any rate, 'HoneymoonValue' can be zero if no premium
    // is paid, and premium solves use zero as a lower bound.
    // And it doesn't make sense for the honeymoon provision to
    // keep the contract in force if 'HoneymoonValue' is -10000
    // and CSV is -20000.
    if(HoneymoonValue <= 0.0 || HoneymoonValue < csv_ignoring_loan)
        {
        HoneymoonActive = false;
        HoneymoonValue  = -std::numeric_limits<double>::max();
        }
}

/// Subtract separate account load after monthly deductions: it is not
/// regarded as part of monthly deductions per se.
///
/// When the sepacct load depends on each month's case total assets, the
/// interest rate is no longer an annual invariant. Set it monthly here.

void AccountValue::TxTakeSepAcctLoad()
{
    if(SepAcctLoadIsDynamic)
        {
        double stratified_load = StratifiedCharges_->stratified_sepacct_load
            (GenBasis_
            ,AssetsPostBom
            ,CumPmtsPostBom
            ,Database_->Query(DB_DynSepAcctLoadLimit)
            );

        double tiered_comp = 0.0;
        if(oe_asset_charge_load == Database_->Query(DB_AssetChargeType))
            {
            tiered_comp = StratifiedCharges_->tiered_asset_based_compensation(AssetsPostBom);
            }
        if(0.0 != tiered_comp)
            {
            fatal_error()
                << "Tiered asset-based compensation unimplemented."
                << LMI_FLUSH
                ;
            }

        // TODO ?? Consider changing Loads::separate_account_load() to
        // return an annual rate, and converting it to monthly here in
        // every case, instead of converting a monthly rate to annual,
        // adding some adjustments, and converting the result back to
        // monthly in this special case.

        YearsSepAcctLoadRate = Loads_->separate_account_load(GenBasis_)[Year];
        YearsSepAcctLoadRate = i_from_i_upper_12_over_12<double>()(YearsSepAcctLoadRate);
        YearsSepAcctLoadRate += stratified_load;
        YearsSepAcctLoadRate += tiered_comp;
        YearsSepAcctLoadRate = i_upper_12_over_12_from_i<double>()(YearsSepAcctLoadRate);
        round_interest_rate()(YearsSepAcctLoadRate);
        }

    SepAcctLoad = YearsSepAcctLoadRate * AVSepAcct;
    process_deduction(SepAcctLoad);
    YearsTotalSepAcctLoad += SepAcctLoad;
    Dcv -= SepAcctLoad;
    Dcv = std::max(0.0, Dcv);
}

//============================================================================
// When the M&E charge depends on each month's case total assets, the
// interest rate is no longer an annual invariant. Set it monthly here.
void AccountValue::ApplyDynamicMandE(double assets)
{
    if(!MandEIsDynamic)
        {
        return;
        }

    // Calculate M&E dynamically for current expense basis only
    switch(GenBasis_)
        {
        case mce_gen_curr:
            {
            // do nothing here; what follows will be correct
            }
            break;
        case mce_gen_guar:
            {
            // guaranteed M&E is not dynamic
            return;
            }
        case mce_gen_mdpt:
            {
            fatal_error()
                << "Dynamic M&E not supported with midpoint expense basis."
                << LMI_FLUSH
                ;
            }
            break;
        default:
            {
            fatal_error()
                << "Case "
                << GenBasis_
                << " not found."
                << LMI_FLUSH
                ;
            }
        }

// TODO ?? Implement tiered comp and tiered management fee.

    // Annual separate-account rates.

    double m_and_e_rate = StratifiedCharges_->tiered_m_and_e(GenBasis_, assets);
    double imf_rate = StratifiedCharges_->tiered_investment_management_fee(assets);
    if(0.0 != imf_rate)
        {
        fatal_error()
            << "Tiered investment management fee unimplemented."
            << LMI_FLUSH
            ;
        }
    double asset_comp_rate =
        (oe_asset_charge_spread == Database_->Query(DB_AssetChargeType))
            ? StratifiedCharges_->tiered_asset_based_compensation(assets)
            : 0.0
            ;
    if(0.0 != asset_comp_rate)
        {
        fatal_error()
            << "Tiered asset-based compensation unimplemented."
            << LMI_FLUSH
            ;
        }

    // TODO ?? Unused for the moment. At least the first must be
    // implemented in order for reports to show components separately.
    double stable_value_rate = 0.0;
    double gross_rate = 0.0;

    InterestRates_->DynamicMlySepAcctRate
        (GenBasis_
        ,SepBasis_
        ,Year
        ,gross_rate
        ,m_and_e_rate
        ,imf_rate
        ,asset_comp_rate
        ,stable_value_rate
        );
    YearsSepAcctIntRate     = InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;
}

/// Credit interest on account value.
///
/// No interest is "credited" to a negative balance in either the
/// general or the separate account.
///
/// Loan interest is calculated elsewhere, but credited here.
///
/// For the separate account, the only reality is unit values, but a
/// notional shadow calculation pretends that a gross rate is credited
/// and then charges are deducted from it. There is no corresponding
/// notion for the general account or for loans that makes as much
/// "sense" for sales presentations.

void AccountValue::TxCreditInt()
{
    ApplyDynamicMandE(AssetsPostBom);

    double notional_sep_acct_charge = 0.0;

    // SOMEDAY !! This should be done in the interest-rate class.
    double gross_sep_acct_rate = i_upper_12_over_12_from_i<double>()
        (InterestRates_->SepAcctGrossRate(SepBasis_)[Year]
        );
    gross_sep_acct_rate = round_interest_rate()(gross_sep_acct_rate);
    if(mce_gen_mdpt == GenBasis_)
        {
        gross_sep_acct_rate = 0.0;
        }

    if(0.0 < AVSepAcct)
        {
        SepAcctIntCred = InterestCredited(AVSepAcct, YearsSepAcctIntRate);
        double gross   = InterestCredited(AVSepAcct, gross_sep_acct_rate);
        notional_sep_acct_charge = gross - SepAcctIntCred;
        // Guard against catastrophic cancellation. Testing the
        // absolute values of the addends for material equality is not
        // sufficient, because the interest increment has already been
        // rounded.
        double result = AVSepAcct + SepAcctIntCred;
        if(result < 0.0 && 0.0 <= AVSepAcct)
            {
            AVSepAcct = 0.0;
            }
        else
            {
            AVSepAcct = result;
            }
        }
    else
        {
        SepAcctIntCred = 0.0;
        }

    if(0.0 < AVGenAcct)
        {
        double effective_general_account_interest_factor = YearsGenAcctIntRate;
        if
            (  yare_input_.HoneymoonEndorsement
            && !HoneymoonActive
            && mce_gen_curr == GenBasis_
            )
            {
            effective_general_account_interest_factor =
                YearsPostHoneymoonGenAcctIntRate
                ;
            }
        GenAcctIntCred = InterestCredited
            (AVGenAcct
            ,effective_general_account_interest_factor
            );
        AVGenAcct += GenAcctIntCred;
        }
    else
        {
        GenAcctIntCred = 0.0;
        }

    LMI_ASSERT(0.0 <= Dcv);
    if(0.0 < Dcv)
        {
        Dcv *= 1.0 + YearsDcvIntRate;
        }

    if(HoneymoonActive)
        {
        HoneymoonValue += InterestCredited
            (HoneymoonValue
            ,YearsHoneymoonValueRate
            );
        }

    // Loaned account value must not be negative.
    LMI_ASSERT(0.0 <= AVRegLn && 0.0 <= AVPrfLn);

    double z = RegLnIntCred + PrfLnIntCred + SepAcctIntCred + GenAcctIntCred;
    YearsTotalNetIntCredited   += z;
    YearsTotalGrossIntCredited += z + notional_sep_acct_charge;
}

/// Accrue loan interest and calculate interest credit on loaned account value.

void AccountValue::TxLoanInt()
{
    // Nothing to do if there's no loan outstanding.
    if(0.0 == RegLnBal && 0.0 == PrfLnBal)
        {
        return;
        }

    // We may want to display credited interest separately.
    // Each interest increment is rounded separately.
    RegLnIntCred = InterestCredited(AVRegLn, YearsRegLnIntCredRate);
    PrfLnIntCred = InterestCredited(AVPrfLn, YearsPrfLnIntCredRate);

    AVRegLn += RegLnIntCred;
    AVPrfLn += PrfLnIntCred;

    double RegLnIntAccrued = InterestCredited(RegLnBal, YearsRegLnIntDueRate);
    double PrfLnIntAccrued = InterestCredited(PrfLnBal, YearsPrfLnIntDueRate);

    RegLnBal += RegLnIntAccrued;
    PrfLnBal += PrfLnIntAccrued;

    YearsTotalLoanIntAccrued += RegLnIntAccrued + PrfLnIntAccrued;
}

//============================================================================
// Allowance for deductions withheld from max loan or wd formula.
//
// Both 'mce_to_next_anniversary' and 'mce_to_next_modal_pmt_date'
// aim to keep the contract from lapsing on the next anniversary or
// the next modal payment date respectively. For example, in the first
// month, with annual mode, both anticipate a deduction thirteen times
// the last monthly deduction calculated. Thirteen, including both
// endpoints: the first, because the current month's deduction has not
// yet been taken when this function is called; and the second, in
// order to keep the contract from lapsing when the anniversary
// deduction is taken. Of course, there is no guarantee that such
// goals will be achieved: only a multiple of the most recent monthly
// deduction is withheld, so no matter how exquisitely precise the
// multiplier may be, the most recent deduction may be lower than
// actual future deductions--particularly in the month of issue, when
// it is zero.
//
double AccountValue::anticipated_deduction
    (mcenum_anticipated_deduction method)
{
    switch(method)
        {
        case mce_twelve_times_last:
            {
            return 12.0 * MlyDed;
            }
        case mce_eighteen_times_last:
            {
            return 18.0 * MlyDed;
            }
        case mce_to_next_anniversary:
            {
            return MlyDed * (13 - Month);
            }
        case mce_to_next_modal_pmt_date:
            {
            return MlyDed * (1 + MonthsToNextModalPmtDate());
            }
        default:
            {
            fatal_error()
                << "Case "
                << method
                << " not found."
                << LMI_FLUSH
                ;
            throw "Unreachable--silences a compiler diagnostic.";
            }
        }
}

/// Calculate maximum permissible withdrawal.
///
/// If the (dynamic) maximum withdrawal would be less than the
/// (fixed) minimum, then the maximum becomes zero.
///
/// Anticipated monthly deductions reduce the maximum withdrawal.
/// They may be calculated in various ways, none of which necessarily
/// prevents the contract from lapsing before the end of the current
/// policy year.
///
/// Some contracts make only a portion of account value eligible for
/// withdrawal, say 80% or 90%. Some apply such a multiple only to
/// separate-account value--a refinement not yet implemented.
/// DATABASE !! Add a database item to restrict the multiple to the
/// separate account only.

void AccountValue::SetMaxWD()
{
    MaxWD =
          (AVGenAcct + AVSepAcct) * MaxWDAVMult
        + (AVRegLn  + AVPrfLn)
        - (RegLnBal + PrfLnBal)
        - anticipated_deduction(MaxWDDed_)
        - std::max(0.0, SurrChg())
        ;
    if(MaxWD < MinWD)
        {
        MaxWD = 0.0;
        }
    MaxWD = std::max(0.0, MaxWD);
}

//============================================================================
void AccountValue::TxTakeWD()
{
    // Illustrations allow withdrawals only on anniversary.
    if(0 != Month)
        {
        return;
        }

    GrossWD = 0.0;
    RequestedWD = Outlay_->withdrawals()[Year];

    if(Debugging || 0.0 != RequestedWD)
        {
        SetMaxWD();
        }

    NetWD = 0.0;

    // Nothing more to do if no withdrawal requested.
    if(0.0 == RequestedWD)
        {
        withdrawal_ullage_[Year] = 0.0;
// This seems wrong. If we're changing something that's invariant among
// bases, why do we change it for each basis?
// TODO ?? Shouldn't this be moved to FinalizeMonth()?
        InvariantValues().NetWD[Year] = 0.0;
        return;
        }

    // TODO ?? Maximum withdrawal--is it gross or net?
    // If maximum exceeded...limit it, rather than letting it lapse, on
    // the current basis--but on other bases, let it lapse

    if(Solving)
        {
        withdrawal_ullage_[Year] = std::max(0.0, RequestedWD - MaxWD);
        }

    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        NetWD = std::min(RequestedWD, MaxWD);
        OverridingWD[Year] = NetWD;
        }
    else
    // TODO ?? If it's going to lapse, call the lapse function--don't
    // manipulate the state variables directly.
        {
        NetWD = OverridingWD[Year];
        if(MaxWD < NetWD)
            {
            VariantValues().LapseMonth = Month;
            VariantValues().LapseYear = Year;
            ItLapsed = true;
            }
        }

    // Impose minimum amount on withdrawals.
    if(RequestedWD < MinWD)
        {
        withdrawal_ullage_[Year] = 0.0;
        NetWD = 0.0;
        }
// TODO ?? If WD zero, skip some steps? Cannot simply return in this case
// because user may prefer to shift to loans.

    // If in solve period and solve is for WD to basis then loan,
    // need to do this in yearly loop: e.g. there could be a forceout
    // order dependency: after prem pmt, before loan.
    if
        (
        yare_input_.WithdrawToBasisThenLoan
//      && yare_input_.SolveBeginYear <= Year && Year < yare_input_.SolveEndYear
// TODO ?? What about guar prem solve?
        )
        {
        // WD below min: switch to loan.
        //
        // Solve objective fn now not monotone; may introduce multiple roots.
        // Even after the entire basis has been withdrawn, we still
        // take withdrawals if payments since have increased the basis.
        // TODO ?? Should RequestedWD be constrained by MaxWD and MinWD here?
        if(0.0 == TaxBasis || std::min(TaxBasis, RequestedWD) < MinWD) // All loan
            {
            withdrawal_ullage_[Year] = 0.0;
            NetWD = 0.0;
            }
        else if(NetWD < TaxBasis) // All WD
            {
            }
        else // Part loan, part WD
            {
            NetWD = TaxBasis;
            }
// The code below switches to loans even if the basis has not yet been
// recovered through withdrawals. Is this a good thing, or a problem
// we should fix? TODO ?? Investigate.
        RequestedLoan += RequestedWD - NetWD;
        }

    if(NetWD <= 0.0)
        {
// TODO ?? What should this be?
//      withdrawal_ullage_[Year] = ?
//
// This seems wrong. If we're changing something that's invariant among
// bases, why do we change it for each basis?
// TODO ?? Shouldn't this be moved to FinalizeMonth()?
        InvariantValues().NetWD[Year] = 0.0;
        return;
        }

    // Gross up by WD fee and partial surrender charge.
    //
    // Partial surrenders preserve the proportions
    //   surrchg:av and csv:av
    // except perhaps for any partial surrender fee, which is
    // customarily some function like
    //   min(X, Y * Z), where
    //     X might be something like $20,
    //     Y might be nominally a percent or two, and
    //     Z is some measure of the partial surrender.
    // Specifications rarely define the fee 'Y*Z' very well.
    // Given a policy with
    //   3000 account value
    //   1000 surrender charge
    //   2000 cash surrender value
    // if half the policy is to be surrendered
    //   1500 gross withdrawal
    //    500 partial-surrender charge
    //   1000 net withdrawal
    // and Y is one percent, then the fee might be
    //   1500 * .01
    //   1500 * 1 / (1-.01)
    //   1000 * .01
    //   1000 * 1 / (1-.01)
    // and the fee might increase the partial surrender (and thus
    // bear its own partial surrender charge) or merely decrease
    // the proceeds.
    //
    // Of course, limiting the fee to something like $10 is a nice
    // marketing feature, and it's easy to calculate if the gross
    // withdrawal is known. But in practice customers state the
    // amount of the check they want to receive, and a fee that's
    // the lesser of two quantities doesn't have a simple linear
    // inverse function.
    //
    // Instead of worrying about these refinements, which happen
    // not to matter in our paid practice, we simply calculate
    // the fee based on the requested proceeds and add that
    // to the partial surrender amount.

    double av = TotalAccountValue();
    double csv = av - SurrChg_[Year];
    HOPEFULLY(0.0 <= SurrChg_[Year]);
    if(csv <= 0.0)
        {
// TODO ?? What should this be?
//      withdrawal_ullage_[Year] = ?
//
// This seems wrong. If we're changing something that's invariant among
// bases, why do we change it for each basis?
// TODO ?? Shouldn't this be moved to FinalizeMonth()?
        InvariantValues().NetWD[Year] = 0.0;
        return;
        }

    GrossWD = NetWD + std::min(WDFee, NetWD * WDFeeRate);

    // Free partial surrenders: for instance, the first 20% of account
    // value might be withdrawn each policy year free of surrender
    // charge. This would become more complicated if we maintained
    // distinct surrender-charge layers.

    double surrchg_proportion = SurrChg_[Year] / csv;
    double non_free_wd = GrossWD;
    if(0.0 != FreeWDProportion[Year])
        {
        // The free partial surrender amount is determined annually,
        // on anniversary and before the anniversary deduction but
        // after any loan has been capitalized.
        LMI_ASSERT(AVRegLn == RegLnBal);
        LMI_ASSERT(AVPrfLn == PrfLnBal);
        LMI_ASSERT(av == AVGenAcct + AVSepAcct);
        double free_wd = FreeWDProportion[Year] * av;
        non_free_wd = std::max(0.0, GrossWD - free_wd);
        }
    double partial_surrchg = non_free_wd * surrchg_proportion;
    GrossWD += partial_surrchg;

    process_distribution(GrossWD);
    Dcv -= GrossWD;
    Dcv = std::max(0.0, Dcv);

    double original_specamt = ActualSpecAmt;
    switch(YearsDBOpt)
        {
        // If DBOpt 1, SA = std::min(SA, DB - WD); if opt 2, no change.
        case mce_option1:
            {
            // Spec amt reduced for option 1 even if in corridor?
            //   --taken care of by max WD formula
            // If WD causes spec amt < min spec amt, do we:
            //   set spec amt = min spec amt?
            //   reduce the WD?
            //   lapse the policy?
            // I'm confused by
            //   specamt (after withdrawal) = min[specamt(before withdrawal);'face'(m)-W]
            // Do you really want 'face' here rather than specamt? --Yes
            if(WDCanDecrSADBO1)
                {
                ChangeSpecAmtBy(-GrossWD);
                // Min AV after WD not directly implemented.
                // If WD causes AV < min AV, do we:
                //   reduce the WD?
                //   lapse the policy?
                // Looks like it can't happen anyway cuz of max WD defn
                //      min AV after WD: debt +
                //          months remaining to end of modal term *
                //          most recent mly deds
                if(original_specamt != ActualSpecAmt)
                    {
                    ChangeSurrChgSpecAmtBy(-GrossWD);
                    }
                }
            else
                {
                // Do nothing.
                }
            }
            break;
        case mce_option2:
            {
            if(WDCanDecrSADBO2)
                {
                ChangeSpecAmtBy(-GrossWD);
                if(original_specamt != ActualSpecAmt)
                    {
                    ChangeSurrChgSpecAmtBy(-GrossWD);
                    }
                }
            else
                {
                // Do nothing.
                }
            }
            break;
        case mce_rop:
            {
            if(WDCanDecrSADBO3)
                {
                ChangeSpecAmtBy(-GrossWD);
                if(original_specamt != ActualSpecAmt)
                    {
                    ChangeSurrChgSpecAmtBy(-GrossWD);
                    }
                }
            else
                {
                // Do nothing.
                }
            }
            break;
        default:
            {
            fatal_error() << "Case " << YearsDBOpt << " not found." << LMI_FLUSH;
            }
        }

    CumPmts     -= NetWD;
    TaxBasis    -= NetWD; // TODO ?? TAXATION !! This should be gross, not net; how about the line above and the line below?
    CumWD       += NetWD;
    // TAXATION !! What about 7702A "amounts paid"?

    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        if(!SolvingForGuarPremium)
            {
            double fake_cum_pmt = 0.0; // TODO ?? Needs work.
            double premiums_paid_increment = -GrossWD;
            Irc7702_->ProcessGptPmt(Year, premiums_paid_increment, fake_cum_pmt);
            }
        }

// This seems wrong. If we're changing something that's invariant among
// bases, why do we change it for each basis?
// TODO ?? Shouldn't this be moved to FinalizeMonth()?
    InvariantValues().NetWD[Year] = NetWD;

    ReduceSurrChg(Year, partial_surrchg);
}

//============================================================================
// Calculate maximum permissible total loan (not increment).
void AccountValue::SetMaxLoan()
{
    MaxLoan =
          (AVGenAcct + AVSepAcct) * MaxLoanAVMult
        + (AVRegLn + AVPrfLn)
        - anticipated_deduction(MaxLoanDed_)
        - std::max(0.0, SurrChg())
        ;

    // Illustrations generally permit loans only on anniversary.
    double reg_loan_factor = InterestRates_->RegLnDueRate
        (GenBasis_
        ,mce_annual_rate
        )
        [Year]
        ;
    double prf_loan_factor = InterestRates_->PrfLnDueRate
        (GenBasis_
        ,mce_annual_rate
        )
        [Year]
        ;
    if(0 != Month)
        {
        fatal_error()
            << "Off-anniversary loans untested."
            << LMI_FLUSH
            ;
        reg_loan_factor =
                std::pow(1.0 + YearsRegLnIntDueRate, 12 - Month)
            -   1.0
            ;
        prf_loan_factor =
                std::pow(1.0 + YearsPrfLnIntDueRate, 12 - Month)
            -   1.0
            ;
        }

    // Withholding anticipated interest, with the adjustment
    //   d upper n, where n is # months remaining in year,
    // attempts to keep the contract from becoming overloaned before
    // the end of the policy year--but does not guarantee that, e.g.
    // because the specified amount may change between anniversaries,
    // even on illustrations.
    MaxLoan -=
          RegLnBal * reg_loan_factor
        + PrfLnBal * prf_loan_factor
        ;

    // TODO ?? This adjustment isn't quite right because it uses only
    // the regular-loan interest factor. Is it conservative under the
    // plausible but unasserted assumption that that factor is more
    // liberal than the preferred-loan factor?
    //
    MaxLoan *= 1.0 - (reg_loan_factor) / (1.0 + reg_loan_factor);

    MaxLoan = round_loan()(MaxLoan);

    // I do not think we want a MaxLoan < current level of indebtedness.
    MaxLoan = std::max((AVRegLn + AVPrfLn), MaxLoan);
    // TODO ?? Yet I do not think we want to ratify something that looks broken!
}

//============================================================================
// Take a new cash loan, limiting it to respect the maximum loan.
void AccountValue::TxTakeLoan()
{
    // Illustrations allow loans only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Even if no loan is requested, the maximum loan is still shown
    // in a monthly trace.
    if(Debugging || 0.0 <= RequestedLoan)
        {
        SetMaxLoan();
        }

    // Nothing more to do if no loan requested.
    if(RequestedLoan <= 0.0)
        {
        loan_ullage_[Year] = 0.0;
        return;
        }

    // SOMEDAY !! Preferred loan calculations not yet implemented.
    LMI_ASSERT(0.0 == AVPrfLn);

    double max_loan_increment = MaxLoan - (AVRegLn + AVPrfLn);

    // When performing a solve, let it become overloaned--otherwise
    // we'd introduce a discontinuity in the function for which we
    // seek a root, and that generally makes the solve slower.
    if(Solving)
        {
        ActualLoan = RequestedLoan;
        loan_ullage_[Year] = std::max(0.0, RequestedLoan - max_loan_increment);
        }
    else
        {
        ActualLoan = std::min(max_loan_increment, RequestedLoan);
        ActualLoan = std::max(ActualLoan, 0.0);
        // TODO ?? Shouldn't this happen in FinalizeMonth()?
        InvariantValues().NewCashLoan[Year] = ActualLoan;
        }

    {
// TODO ?? Perhaps this condition should be:
//   Solving || mce_run_gen_curr_sep_full == RunBasis_
    if(mce_run_gen_curr_sep_full == RunBasis_)
        {
        OverridingLoan[Year] = ActualLoan;
        }
    else
    // TODO ?? If it's going to lapse, call the lapse function--don't
    // manipulate the state variables directly.
        {
        //
        ActualLoan = OverridingLoan[Year];
        if(MaxLoan < ActualLoan)
            {
            VariantValues().LapseMonth = Month;
            VariantValues().LapseYear = Year;
            ItLapsed = true;
            }
        }
    }

    // Transfer new cash loan from the appropriate unloaned account(s).
    process_distribution(ActualLoan);

    // SOMEDAY !! Also handle preferred loan.
    AVRegLn += ActualLoan;
    RegLnBal += ActualLoan;
}

//============================================================================
// On anniversary, capitalize loan and set loaned AV equal to loan balance.
void AccountValue::TxCapitalizeLoan()
{
    // Capitalized loans only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Transfer amount of loan interest capitalized from unloaned AV.
    // This is a 'distribution' for monthly processing in that it is
    // handled the same as any other loan; of course, it's not a
    // 'distribution' in the tax sense because no cash is received.
    process_distribution(RegLnBal + PrfLnBal - AVRegLn - AVPrfLn);
    AVRegLn = RegLnBal;
    AVPrfLn = PrfLnBal;
}

//============================================================================
// Test for lapse.
void AccountValue::TxTestLapse()
{
    // The refundable load cannot prevent a lapse that would otherwise
    // occur, because it is refunded only after termination. The same
    // principle applies to a negative surrender charge.
    //
    // The experience rating reserve can prevent a lapse, because it
    // is an actual balance-sheet item that is actually held in the
    // certificate.

    double lapse_test_csv =
          TotalAccountValue()
        - (RegLnBal + PrfLnBal)
//        + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        ;
    if(!LapseIgnoresSurrChg)
        {
        lapse_test_csv -= std::max(0.0, SurrChg());
        }
    lapse_test_csv = std::max(lapse_test_csv, HoneymoonValue);

    // Perform no-lapse test.
    if(NoLapseActive && ! NoLapseAlwaysActive)
        {
        CumNoLapsePrem += MlyNoLapsePrem + RiderCharges;
        if
            (       NoLapseMinAge <= Year + BasicValues::GetIssueAge()
                &&  NoLapseMinDur <= Year
            ||      CumPmts < CumNoLapsePrem
                &&  !materially_equal(CumPmts, CumNoLapsePrem)
            )
            {
            NoLapseActive = false;
            }
        }
    YearlyNoLapseActive[Year] = NoLapseActive;

// SOMEDAY !! Handle these GDB issues:
//   Recalculate GDB on face change--need 7PP.
//   Recapture monthly deductions due if in GDB period.

    // If we're doing a solve, don't let it lapse--otherwise lapse would
    // introduce a discontinuity in the function for which we seek a root.
    if(Solving)
        {
        return;
        }

    if(HoneymoonActive)
        {
        return;
        }

    // Otherwise if CSV is negative or if overloaned, then lapse the policy.
    else if
        (
            (!NoLapseActive && lapse_test_csv < 0.0)
        // Lapse if overloaned regardless of guar DB.
        // CSV includes a positive loan (that can offset a negative AV);
        // however, we still need to test for NoLapseActive.
        ||  (!NoLapseActive && (AVGenAcct + AVSepAcct) < 0.0)
        // Test for nonnegative unloaned account value.
        // We are aware that some companies test against loan balance:
// TODO ?? Would the explicit test
//      ||  (MaxLoan < RegLnBal + PrfLnBal)
// below be better? No. Testing against MaxLoan only when it's calculated
// (on anniversary) is not sufficient, because the preceding-anniversary
// MaxLoan goes stale with the passage of time even if other things
// remain the same, and also because MaxLoan becomes invalid if the
// specamt changes off anniversary.
        // If there is interest in that alternative, we can offer
        // that behavior as an option controlled by a database flag.
        //
        // TODO ?? At this time there is no test for overloan, at least
        // not in any year when no new cash loan is taken.
        )
        {
        VariantValues().LapseMonth = Month;
        VariantValues().LapseYear = Year;
        ItLapsed = true;

        // We need to set VariantValues().CSVPayable here
        // (else it would print as a negative number)
        // TODO ?? Can't this be done elsewhere?
        VariantValues().CSVNet[Year] = 0.0;
        }
    else
        {
        if(NoLapseActive && lapse_test_csv < 0.0)
            {
            AVGenAcct = 0.0;
            AVSepAcct = 0.0;
            // TODO ?? Can't this be done elsewhere?
            VariantValues().CSVNet[Year] = 0.0;
            }
        else if(!HoneymoonActive && !Solving && lapse_test_csv < 0.0)
            {
            fatal_error()
                << "Unloaned value not positive,"
                << " no-lapse guarantee not active,"
                << " and honeymoon not active, yet policy did not lapse."
                << LMI_FLUSH
                ;
            }
        }
}

//============================================================================
void AccountValue::FinalizeMonth()
{
    if(mce_run_gen_curr_sep_full == RunBasis_)
        {
        if(0 == Year && 0 == Month)
            {
            InvariantValues().External1035Amount = External1035Amount;
            InvariantValues().Internal1035Amount = Internal1035Amount;
            InvariantValues().Dumpin = Dumpin;
            }

        // We could also capture MEC status on other bases here.
        if(true == Irc7702A_->UpdateEOM7702A())
            {
            if(!InvariantValues().IsMec)
                {
                InvariantValues().IsMec = true;
                InvariantValues().MecYear = Year;
                InvariantValues().MecMonth = Month;
                }
            }
        }
}

//============================================================================
void AccountValue::TxDebug()
{
    DebugPrint();
}

