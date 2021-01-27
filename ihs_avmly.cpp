// Account value: monthiversary processing.
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

#include "pchfile.hpp"

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
#include "math_functions.hpp"
#include "miscellany.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "stratified_algorithms.hpp"
#include "stratified_charges.hpp"

#include <algorithm>                    // min(), max()
#include <cmath>                        // pow()
#include <limits>

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
        ,dblize(DBReflectingCorr + TermDB) // DB7702A
        ,dblize(OldDB) // prior_db_7702A
        ,DBReflectingCorr == DBIgnoringCorr
        // TAXATION !! This assumes the term rider can be treated as death benefit;
        // use 'TermIsDbFor7702A'.
        ,dblize(ActualSpecAmt + TermSpecAmt)
        ,dblize(OldSA) // prior_sa_7702A
        ,dblize(CashValueFor7702())
        );

    NetPmts  [Month] = C0; // TODO ?? expunge as being unnecessary
    GrossPmts[Month] = C0; // TODO ?? expunge as being unnecessary
    TxExch1035();
    // TODO ?? TAXATION !! Is this where spec amt should be increased by GPT?

    currency kludge_account_value = std::max(TotalAccountValue(), HoneymoonValue);
    if(0 == Year && 0 == Month)
        {
        // This special case was apparently intended for 1035
        // exchanges, but now seems unnecessary because this
        // assertion never fires:
        //   LMI_ASSERT(kludge_account_value == Dcv);
        kludge_account_value = round_minutiae().c(Dcv);
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
        ,dblize(AnnualTargetPrem)
        ,YearsTotLoadTgtLowestPremtax
        ,YearsTotLoadExcLowestPremtax
        ,dblize(kludge_account_value)
        );
// TAXATION !! Should round here, but need to investigate regressions.
//  max_necessary_premium = round_max_premium()(max_necessary_premium);
    // CURRENCY !! already rounded by class Irc7702A--appropriately?
    double max_non_mec_premium = Irc7702A_->MaxNonMecPremium
        (Dcv
        ,dblize(AnnualTargetPrem)
        ,YearsTotLoadTgtLowestPremtax
        ,YearsTotLoadExcLowestPremtax
        ,dblize(kludge_account_value)
        );

    // Saved for monthly detail report only. TAXATION !! Then are
    // these still needed...perhaps in order to report their values
    // prior to accepting any payment?
    NetMaxNecessaryPremium   = round_minutiae().c(Irc7702A_->DebugGetNetMaxNecPm  ());
    GrossMaxNecessaryPremium = round_minutiae().c(Irc7702A_->DebugGetGrossMaxNecPm());

    // Determine list-bill premiums only after transactions that
    // might change specamt have been processed.
    // SOMEDAY !! Should InvariantValues().ModalMinimumPremium be
    // set here for that reason?
    set_list_bill_premium();

    TxAscertainDesiredPayment();
    TxLimitPayment(max_non_mec_premium);

    // TAXATION !! This line recognizes a withdrawal for 7702A;
    // shouldn't there be similar code here for GPT?
    if(0 == Month)
        {
        Irc7702A_->UpdatePmt7702A
            (Dcv
            ,dblize(-NetWD) // TAXATION !! This should be gross, not net.
            ,false
            ,dblize(AnnualTargetPrem)
            ,YearsTotLoadTgtLowestPremtax
            ,YearsTotLoadExcLowestPremtax
            ,dblize(kludge_account_value)
            );
        }

    currency gross_1035 = C0;
    if(0 == Year && 0 == Month)
        {
        gross_1035 = External1035Amount + Internal1035Amount;
        }
    // CURRENCY !! currency is immune to catastrophic cancellation
    double necessary_premium = std::min
        // CURRENCY !! Wouldn't simple subtraction do, for currency?
        (material_difference
            (dblize(GrossPmts[Month])
            ,dblize(gross_1035)
            )
        ,max_necessary_premium
        );
    // CURRENCY !! Wouldn't simple subtraction do, for currency?
    double unnecessary_premium = material_difference
        (dblize(GrossPmts[Month])
        ,dblize(gross_1035) + necessary_premium
        );
    NecessaryPremium   = round_minutiae().c(necessary_premium  );
    UnnecessaryPremium = round_minutiae().c(unnecessary_premium);
    // It is crucial to accept necessary premium before processing a
    // material change, so that the correct DCV is used.
    TxRecognizePaymentFor7702A(NecessaryPremium, false);
    TxAcceptPayment(NecessaryPremium);
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
        ,dblize(CashValueFor7702())
        );
    LMI_ASSERT(0.0 <= Dcv);

    TxRecognizePaymentFor7702A(UnnecessaryPremium, true);
    TxAcceptPayment(UnnecessaryPremium);

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
void AccountValue::process_payment(currency payment)
{
    // Apply ee and er net payments according to database rules.
    // Net payments were already aggregated, then split between
    // necessary and unnecessary, as required for 7702A on CVAT
    // contracts. We deem net payments to have the same proportion
    // as gross payments, which we do have for ee and er separately.

    LMI_ASSERT(C0 <=   GrossPmts[Month]);
    LMI_ASSERT(C0 <= EeGrossPmts[Month]);
    LMI_ASSERT(C0 <= ErGrossPmts[Month]);

    currency gross_1035 = C0;
    if(0 == Year && 0 == Month)
        {
        gross_1035 = External1035Amount + Internal1035Amount;
        }
    currency gross_non_1035_pmts = GrossPmts[Month] - gross_1035;
    double er_proportion = 0.0;
    if(C0 != gross_non_1035_pmts)
        {
        // CURRENCY !! more efficient: currency / currency --> double
        er_proportion = ErGrossPmts[Month] / dblize(gross_non_1035_pmts);
        }

    // This is a net premium that's multiplied by a gross-premium
    // proportion, because that's the only way the proportion can be
    // deduced from input. But it remains a net premium, so the
    // net-premium rounding rule is appropriate.
    currency er_net_pmt = round_net_premium().c(er_proportion * payment);
    // This is deliberately not rounded. The crucial invariant that
    // must be preserved is that ee + er = total; but rounding both
    // addends could break that invariant, e.g.:
    //   round(2.5) + round(2.5)
    // might not produce 5.0 as desired.
    currency ee_net_pmt = payment - er_net_pmt;

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
        }

    // TODO ?? Perhaps now we should transfer funds from the separate
    // to the general account to make the latter nonnegative if needed.
}

//============================================================================
// Prorate increments to account value between separate- and general-account
// portions of unloaned account value according to input allocations.
void AccountValue::IncrementAVProportionally(currency increment)
{
    increment = round_minutiae().c(increment); // CURRENCY !! already rounded?
    currency genacct_increment = round_minutiae().c(increment * GenAcctPaymentAllocation);
    AVGenAcct += genacct_increment;
    AVSepAcct += increment - genacct_increment;
}

//============================================================================
// Apply increments to account value to the preferred account.
void AccountValue::IncrementAVPreferentially
    (currency                           increment
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
        }
}

/// Apportion all charges to be deducted from account value among
/// accounts.

void AccountValue::process_deduction(currency decrement)
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
        }
}

/// Apportion all distributions from account value among accounts.

void AccountValue::process_distribution(currency decrement)
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

void AccountValue::DecrementAVProportionally(currency decrement)
{
    decrement = round_minutiae().c(decrement); // CURRENCY !! already rounded?

#if defined USE_CURRENCY_CLASS
    if(decrement == AVGenAcct + AVSepAcct)
#else  // !defined USE_CURRENCY_CLASS
    if(materially_equal(decrement, AVGenAcct + AVSepAcct))
#endif // !defined USE_CURRENCY_CLASS
        {
        AVGenAcct = C0;
        AVSepAcct = C0;
        return;
        }

    double general_account_proportion  = 0.0;
    double separate_account_proportion = 0.0;
    // CURRENCY !! more efficient: currency / currency --> double
    double general_account_nonnegative_assets  = dblize(std::max(C0, AVGenAcct));
    double separate_account_nonnegative_assets = dblize(std::max(C0, AVSepAcct));
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
    // Disregard 'separate_account_proportion' in order to ensure
    // that the sum of the distinct decrements here equals the
    // total decrement. Keep 'separate_account_proportion' above
    // because there may still be value in the assertions.
    currency genacct_decrement = round_minutiae().c(decrement * general_account_proportion);
    AVGenAcct -= genacct_decrement;
    AVSepAcct -= decrement - genacct_decrement;
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
    (currency                           decrement
    ,oenum_increment_account_preference preferred_account
    )
{
#if defined USE_CURRENCY_CLASS
    if(decrement == AVGenAcct + AVSepAcct)
#else  // !defined USE_CURRENCY_CLASS
    if(materially_equal(decrement, AVGenAcct + AVSepAcct))
#endif // !defined USE_CURRENCY_CLASS
        {
        AVGenAcct = C0;
        AVSepAcct = C0;
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
        }
}

//============================================================================
void AccountValue::TxExch1035()
{
    if(!(0 == Year && 0 == Month))
        {
        return;
        }

    LMI_ASSERT(Year == InforceYear);
    LMI_ASSERT(Month == InforceMonth);
    LMI_ASSERT(C0 ==   GrossPmts[Month]);
    LMI_ASSERT(C0 == EeGrossPmts[Month]);
    LMI_ASSERT(C0 == ErGrossPmts[Month]);

    // Policy issue date is always a modal payment date.
    GrossPmts[Month] = External1035Amount + Internal1035Amount;

// TODO ?? TAXATION !! This looks like a good idea, but it would prevent the
// initial seven-pay premium from being set.
//    if(C0 == GrossPmts[Month])
//        {
//        return;
//        }

// TAXATION !! This condition, or a similar one, guards several GPT calls,
// but not all (ProcessAdjustableEvent(), e.g.); should this be made uniform?
// TODO ?? Perhaps this condition should be:
//    if(!SolvingForGuarPremium && Solving || mce_run_gen_curr_sep_full == RunBasis_)
    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        // Illustration-reg guaranteed premium ignores GPT limit.
        if(!SolvingForGuarPremium)
            {
            // CURRENCY !! return modified value instead of altering argument
            double z = dblize(GrossPmts[Month]);
            Irc7702_->ProcessGptPmt(Year, z);
            GrossPmts[Month] = round_gross_premium().c(z);
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
    currency amount_exempt_from_premium_tax = C0;
    if(WaivePmTxInt1035)
        {
        amount_exempt_from_premium_tax = Internal1035Amount;
        }
    currency actual_load = GetPremLoad
        (GrossPmts[Month]
        ,amount_exempt_from_premium_tax
        );

    NetPmts[Month] = GrossPmts[Month] - actual_load;

    LMI_ASSERT(C0 == AVGenAcct);
    LMI_ASSERT(C0 == AVSepAcct);
    process_payment(NetPmts[Month]);

    DBReflectingCorr = C0;
    TxSetDeathBft();
    TxSetTermAmt();
    // TODO ?? TAXATION !! Should 1035 exchanges be handled somewhere else?
    LMI_ASSERT(0.0 == Dcv);
    Irc7702A_->Update1035Exch7702A
        (Dcv
        ,dblize(NetPmts[Month])
        // TAXATION !! This assumes the term rider can be treated as death benefit;
        // use 'TermIsDbFor7702A'.
        ,dblize(ActualSpecAmt + TermSpecAmt)
//        ,DBReflectingCorr + TermDB // TAXATION !! Alternate if 7702A benefit is DB?
        );

    if(HoneymoonActive)
        {
        HoneymoonValue += std::max(C0, GrossPmts[Month]);
        }

    CumPmts += GrossPmts[Month];
    TaxBasis += round_minutiae().c
        ( yare_input_.External1035ExchangeTaxBasis
        + yare_input_.Internal1035ExchangeTaxBasis
        );

    if(mce_run_gen_curr_sep_full == RunBasis_)
        {
        // Immediately after a 1035 exchange, DCV should be
        // the 1035 amount reduced by any premium-based loads,
        // but only for the current rate basis.
        LMI_ASSERT(materially_equal(Dcv, dblize(NetPmts[Month])));

        // The initial seven-pay premium shown on the illustration
        // must be its value immediately after any 1035 exchange,
        // which by its nature occurs before a seven-pay premium
        // can be calculated.
        InvariantValues().InitSevenPayPrem = round_max_premium()
            (Irc7702A_->GetPresent7pp()
            );
        }

    assert_pmts_add_up(__FILE__, __LINE__, Month);
}

//============================================================================
currency AccountValue::CashValueFor7702() const
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
currency AccountValue::InterestCredited
    (currency principal
    ,double   monthly_rate
    ) const
{
    return round_interest_credit().c(principal * ActualMonthlyRate(monthly_rate));
}

//============================================================================
bool AccountValue::IsModalPmtDate(mcenum_mode mode) const
{
    return 0 == Month % (12 / mode);
}

/// Number of monthiversaries before next billing date, counting today.
///
/// This is intended for use only with group UL plans, so it reflects
/// the group billing mode chosen by the employer.

int AccountValue::MonthsToNextModalPmtDate() const
{
    return 1 + (11 - Month) % (12 / Outlay_->er_premium_modes()[Year]);
}

/// Determine instantaneous base-policy minimum specified amount.
///
/// Argument 'issuing_now' indicates whether the policy is being
/// issued at the present moment: i.e., this is the first month of the
/// first policy year (and therefore the policy is not in force yet).
///
/// Argument 'term_rider' indicates whether a term rider is to be
/// taken into account, as that affects the base-policy minimum.

currency AccountValue::minimum_specified_amount(bool issuing_now, bool term_rider) const
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
void AccountValue::ChangeSpecAmtBy(currency delta)
{
    delta = round_specamt().c(delta); // CURRENCY !! already rounded?
    double term_proportion = 0.0;
    currency const old_total_specamt = ActualSpecAmt + TermSpecAmt;
    // Adjust term here only if it's formally a rider.
    if(TermRiderActive && !TermIsNotRider)
        {
        switch(yare_input_.TermAdjustmentMethod)
            {
            case mce_adjust_term:
                {
                term_proportion = 1.0;
                }
                break;
            case mce_adjust_both:
                {
                // CURRENCY !! more efficient: currency / currency --> double
                term_proportion = TermSpecAmt / dblize(old_total_specamt);
                }
                break;
            case mce_adjust_base:
                {
                term_proportion = 0.0;
                }
                break;
            }

        ActualSpecAmt += round_specamt().c(delta * (1.0 - term_proportion));
        TermSpecAmt = old_total_specamt + delta - ActualSpecAmt;

        if(TermSpecAmt < C0)
            {
            // Reducing the term rider's specified amount to a value
            // less than zero is taken as a request to terminate the
            // rider altogether.
            EndTermRider(false);
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
            TermSpecAmt = round_specamt().c(TermSpecAmt); // CURRENCY !! already rounded?
            }
        }
    else
        {
        ActualSpecAmt += delta;
        }

    // If specamt would be reduced below the minimum (e.g., by a large
    // withdrawal), then force it to the minimum.
    ActualSpecAmt = std::max
        (ActualSpecAmt
        ,minimum_specified_amount(0 == Year && 0 == Month, TermRiderActive)
        );
    ActualSpecAmt = round_specamt().c(ActualSpecAmt); // CURRENCY !! already rounded?

    // Carry the new specamt forward into all future years.
    for(int j = Year; j < BasicValues::GetLength(); ++j)
        {
// TODO ?? This seems wrong. If we're changing something that doesn't
// vary across bases, why do we change it for all bases?
// TODO ?? Shouldn't this be moved to FinalizeMonth()? The problem is
// that the ledger object is used for working storage, where it should
// probably be write-only instead.
        InvariantValues().SpecAmt[j] = dblize(ActualSpecAmt);
        // Adjust term here only if it's formally a rider.
        // Otherwise, its amount should not have been changed.
        if(!TermIsNotRider)
            {
            InvariantValues().TermSpecAmt[j] = dblize(TermSpecAmt);
            }
// Term specamt is a vector in class LedgerInvariant, but a scalar in
// the input classes, e.g.:
//   yare_input_.TermRiderAmount
// as is appropriate for a 7702-integrated term rider. Another sort of
// term rider might call for vector input: SupplementalAmount is thus
// handled by ChangeSupplAmtBy().
        }
    // Reset DB whenever SA changes.
    TxSetDeathBft();
}

void AccountValue::ChangeSupplAmtBy(currency delta)
{
    delta = round_specamt().c(delta); // CURRENCY !! already rounded?
    TermSpecAmt += delta;

    // No minimum other than zero is defined.
    TermSpecAmt = std::max(TermSpecAmt, C0);
    TermSpecAmt = round_specamt().c(TermSpecAmt); // CURRENCY !! already rounded?

    // Carry the new supplemental amount forward into all future years.
    // At least for now, there is no effect on surrender charges.
    for(int j = Year; j < BasicValues::GetLength(); ++j)
        {
        InvariantValues().TermSpecAmt[j] = dblize(TermSpecAmt);
        }
    // Reset term DB whenever term SA changes. It's not obviously
    // necessary to do this here, but neither should it do any harm.
    TxSetTermAmt();
}

//============================================================================
void AccountValue::InitializeMonth()
{
    GptForceout       = C0;
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
    mcenum_dbopt const old_option = DeathBfts_->dbopt()[Year - 1];

    // Nothing to do if no option change requested.
    if(old_option == YearsDBOpt)
        {
        return;
        }

    if(!AllowChangeToDBO2 && mce_option2 == YearsDBOpt)
        {
        alarum()
            << "Change to increasing death benefit option"
            << " not allowed on this policy form."
            << LMI_FLUSH
            ;
        return;
        }

    if(NoLapseActive && mce_option1 != YearsDBOpt && NoLapseDboLvlOnly)
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
                    ChangeSpecAmtBy(std::max(C0, TotalAccountValue()));
                    }
                else if(mce_rop == old_option)
                    {
                    ChangeSpecAmtBy(std::max(C0, CumPmts));
                    }
                else if(mce_mdb == old_option)
                    {
                    // Do nothing. An argument could be made for this
                    // alternative:
                    //   TxSetDeathBft(); // update DBReflectingCorr
                    //   ChangeSpecAmtBy(std::max(C0, DBReflectingCorr));
                    // but that takes more work and is not clearly
                    // preferable.
                    }
                else
                    {
                    alarum() << "Unknown death benefit option." << LMI_FLUSH;
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
                ChangeSpecAmtBy(-std::max(C0, TotalAccountValue()));
                }
            else
                {
                // Do nothing.
                }
            break;
        case mce_rop:
            if(OptChgCanDecrSA)
                {
                ChangeSpecAmtBy(-std::max(C0, CumPmts));
                }
            else
                {
                // Do nothing.
                }
            break;
        case mce_mdb:
            {
            // Change spec amt by its additive inverse, making it 0.
            ChangeSpecAmtBy(-(ActualSpecAmt + TermSpecAmt));
            }
            break;
        }
}

/// Process an owner-initiated specified-amount increase or decrease.
///
/// Illustrations allow elective increases and decreases only on
/// renewal anniversaries.
///
/// Ignores multiple layers of coverage: not correct for select and
/// ultimate COI rates if select period restarts on increase.
///
/// Specamt changes are assumed not to affect the target-premium rate.

void AccountValue::TxSpecAmtChange()
{
    if(0 != Month || 0 == Year)
        {
// > This initializes DBReflectingCorr and others so that the at-issue but
// > post-1035 exchange calculation of the seven pay will have a death benefit
// > amount to base its calculations on.
#if 1
        ChangeSpecAmtBy(C0);
#else // 0
        // TODO ?? Changing specamt by zero is absurd. The following
        // commented-out alternative seems to do the same thing as
        // the line above--at least the regression test succeeds.
        // However, both ways are unclean, and both may be incorrect.
        //
        // Carry specamt forward into all future years.
        for(int j = Year; j < BasicValues::GetLength(); ++j)
            {
            ActualSpecAmt = std::max
                (ActualSpecAmt
                ,minimum_specified_amount(0 == Year && 0 == Month, TermRiderActive)
                );
            ActualSpecAmt = round_specamt().c(ActualSpecAmt); // CURRENCY !! already rounded?
            InvariantValues().SpecAmt[j] = ActualSpecAmt;
            if(!TermIsNotRider)
                {
                InvariantValues().TermSpecAmt[j] = TermSpecAmt;
                }
            }
        // Set BOM DB for 7702 and 7702A.
        TxSetDeathBft();
#endif // 0
        return;
        }

    LMI_ASSERT(0 < Year);

    if(TermIsNotRider && DeathBfts_->supplamt()[Year] != TermSpecAmt)
        {
        ChangeSupplAmtBy(DeathBfts_->supplamt()[Year] - TermSpecAmt);
        }

    currency const new_specamt = DeathBfts_->specamt()[Year];
    currency const old_specamt = DeathBfts_->specamt()[Year - 1];

    // Nothing to do if no increase or decrease requested.
    // TODO ?? new_specamt != ActualSpecAmt; the latter should be used.
    if(new_specamt == old_specamt)
        {
        return;
        }

    if
        (
            !AllowSAIncr
        &&  ActualSpecAmt < new_specamt
        )
        {
        alarum()
            << "Specified-amount increases not allowed on this policy form."
            << LMI_FLUSH
            ;
        return;
        }

    if
        (
            MaxIncrAge < Year + BasicValues::GetIssueAge()
        &&  ActualSpecAmt < new_specamt
        )
        {
        alarum()
            << "Cannot increase specified amount after age "
            << MaxIncrAge
            << " on this policy form."
            << LMI_FLUSH
            ;
        return;
        }

    ChangeSpecAmtBy(new_specamt - ActualSpecAmt);

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
    mcenum_dbopt_7702 const new_dbopt(effective_dbopt_7702(YearsDBOpt, Effective7702DboRop));
    mcenum_dbopt_7702 const old_dbopt(effective_dbopt_7702(OldDBOpt  , Effective7702DboRop));
    // TAXATION !! This may require revision if DB is treated as the 7702 benefit.
    // TAXATION !! This assumes the term rider can be treated as death benefit;
    // use 'TermIsDbFor7702'.
    bool adj_event =
#if defined USE_CURRENCY_CLASS
            (   OldSA != ActualSpecAmt + TermSpecAmt
            &&  OldDB != DBReflectingCorr + TermDB
            )
#else  // !defined USE_CURRENCY_CLASS
            (   !materially_equal(OldSA, ActualSpecAmt + TermSpecAmt)
            &&  !materially_equal(OldDB, DBReflectingCorr + TermDB)
            )
#endif // !defined USE_CURRENCY_CLASS
        ||  old_dbopt != new_dbopt
        ;
    if(adj_event)
        {
        // TODO ?? TAXATION !! Perhaps we should pass 'A' of 'A+B-C' for validation.
        // Or maybe not, because we can't match it if there was a plan change.
        Irc7702_->ProcessAdjustableEvent
            (Year
            ,dblize(DBReflectingCorr + TermDB)
            ,dblize(OldDB)
            // TAXATION !! This assumes the term rider can be treated as death benefit;
            // use 'TermIsDbFor7702'.
            ,dblize(ActualSpecAmt + TermSpecAmt)
            ,dblize(OldSA)
            ,new_dbopt
            ,old_dbopt
            ,dblize(AnnualTargetPrem)
            );
        }

    // CURRENCY !! already rounded by class Irc7702--appropriately?
    GptForceout = round_minutiae().c(Irc7702_->Forceout());
    // TODO ?? TAXATION !! On other bases, nothing is forced out, and payments aren't limited.
    process_distribution(GptForceout);
    YearsTotalGptForceout += GptForceout;

    // SOMEDAY !! Actually, forceouts reduce basis only to the extent
    // they're nontaxable. For now, tax basis is used only as a limit
    // (if requested) on withdrawals, and it is conservative for that
    // purpose to underestimate basis. TAXATION !! 7702(f)(7)(B-E).

    CumPmts     -= GptForceout;
    TaxBasis    -= GptForceout;

    if(adj_event)
        {
        Irc7702A_->InduceMaterialChange();
        }

    OldDBOpt = YearsDBOpt;
}

/// Set payments, reflecting strategies and imposing GPT limit.
///
/// Payments are set here on the current-full-expense basis (or on the
/// solve basis if different), and reused on other bases.
///
/// 1035 rollovers are handled elsewhere.
///
/// Guideline limits are imposed in the order in which payments are
/// applied:
///   1035 exchanges
///   employee vector-input premium
///   employer vector-input premium
///   dumpin
/// This order is pretty much arbitrary, except that 1035 exchanges
/// really must be processed first. An argument could be made for
/// changing the order of employee and employer premiums. An
/// argument could be made for grouping dumpin with employee
/// premiums, at least as long as we treat dumpin as employee
/// premium. Even though dumpin and 1035 exchanges are similar in
/// that both are single payments notionally made at issue, it is
/// not necessary to group them together: 1035 exchanges have a
/// unique nature that requires them to be recognized before any
/// premium is paid, and dumpins do not share that nature.

void AccountValue::TxAscertainDesiredPayment()
{
// SOMEDAY !! Some systems force monthly premium to be integral cents even
// though actual mode is not monthly; is that something we need to do here?
//
// TAXATION !! Resolve these issues:
//   pmts must be the same on all bases; however:
//     7702A effect varies by basis
//     does GPT effect also vary by basis?
//       e.g. when opt change produces different spec amts

    bool const ee_pay_this_month = IsModalPmtDate(Outlay_->ee_premium_modes()[Year]);
    bool const er_pay_this_month = IsModalPmtDate(Outlay_->er_premium_modes()[Year]);
    // Month zero must be a modal payment date for both ee and er.
    LMI_ASSERT((ee_pay_this_month && er_pay_this_month) || (0 != Month));

    // Do nothing if this is not a modal payment date, or if the
    // present basis reuses payments previously set on another basis.
    bool const nothing_to_pay = !(ee_pay_this_month || er_pay_this_month);
    bool const nothing_to_set = !(Solving || mce_run_gen_curr_sep_full == RunBasis_);
    if(nothing_to_pay || nothing_to_set)
        {
        return;
        }

    assert_pmts_add_up(__FILE__, __LINE__, Month);

    currency eepmt = C0;
    if(ee_pay_this_month)
        {
        eepmt = PerformEePmtStrategy();
        // Illustration-reg guaranteed premium ignores GPT limit.
        if(!SolvingForGuarPremium)
            {
            // CURRENCY !! return modified value instead of altering argument
            double z = dblize(eepmt);
            Irc7702_->ProcessGptPmt(Year, z);
            eepmt = round_gross_premium().c(z);
            }
        EeGrossPmts[Month] += eepmt;
        GrossPmts  [Month] += eepmt;
        }

    currency erpmt = C0;
    if(er_pay_this_month)
        {
        erpmt = PerformErPmtStrategy();
        // Illustration-reg guaranteed premium ignores GPT limit.
        if(!SolvingForGuarPremium)
            {
            // CURRENCY !! return modified value instead of altering argument
            double z = dblize(erpmt);
            Irc7702_->ProcessGptPmt(Year, z);
            erpmt = round_gross_premium().c(z);
            }
        ErGrossPmts[Month] += erpmt;
        GrossPmts  [Month] += erpmt;
        }

    assert_pmts_add_up(__FILE__, __LINE__, Month);
    // CURRENCY !! is this useful?
    // bignum: the largest integer convertible to and from double
    // such that incrementing it by one loses that property.
    LMI_ASSERT(GrossPmts[Month] < from_cents(1LL << 53));

    if(0 == Year && 0 == Month)
        {
        // Illustration-reg guaranteed premium ignores GPT limit.
        if(!SolvingForGuarPremium)
            {
            // CURRENCY !! return modified value instead of altering argument
            double z = dblize(Dumpin);
            Irc7702_->ProcessGptPmt(Year, z);
            Dumpin = round_gross_premium().c(z);
            }
        EeGrossPmts[Month] += Dumpin;
        GrossPmts  [Month] += Dumpin;
        }

    assert_pmts_add_up(__FILE__, __LINE__, Month);
}

/// Limit payment (e.g., to the non-MEC maximum).
///
/// The limit argument is of type double because the taxation code may
/// return DBL_MAX. CURRENCY !! Can currency work with such values?
///
/// TAXATION !! Should this be called for gpt? or, if it's called,
/// should it assert that it has no effect?

void AccountValue::TxLimitPayment(double a_maxpmt)
{
// Subtract premium load from gross premium yielding net premium.

    // This is needed only for current-basis or solve-basis runs.
    // Otherwise we're doing too much work, and maybe doing things
    // we shouldn't.
// TODO ?? TAXATION !! Clean this up, and put GPT limit here, on prem net of WD.

    assert_pmts_add_up(__FILE__, __LINE__, Month);

    if(mce_reduce_prem == yare_input_.AvoidMecMethod && !Irc7702A_->IsMecAlready())
        {
        currency gross_1035 = C0;
        if(0 == Year && 0 == Month)
            {
            gross_1035 = External1035Amount + Internal1035Amount;
            }
        currency gross_pmt_without_1035 = GrossPmts[Month] - gross_1035;
        // CURRENCY !! support infinities?
//      gross_pmt_without_1035 = std::min(gross_pmt_without_1035, a_maxpmt);
        gross_pmt_without_1035 = round_gross_premium().c(std::min(dblize(gross_pmt_without_1035), a_maxpmt));
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

    assert_pmts_add_up(__FILE__, __LINE__, Month);

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

    assert_pmts_add_up(__FILE__, __LINE__, Month);
}

//============================================================================
void AccountValue::TxRecognizePaymentFor7702A
    (currency a_pmt
    ,bool     a_this_payment_is_unnecessary
    )
{
    if(C0 == a_pmt)
        {
        return;
        }

    // 1035 exchanges are handled in a separate transaction.

    // Policy issue date is always a modal payment date.

    currency kludge_account_value = std::max(TotalAccountValue(), HoneymoonValue);
    if(0 == Year && 0 == Month)
        {
        kludge_account_value = round_minutiae().c(Dcv);
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
    currency amount_paid_7702A = a_pmt;
    Irc7702A_->UpdatePmt7702A
        (Dcv
        ,dblize(amount_paid_7702A)
        ,a_this_payment_is_unnecessary
        ,dblize(AnnualTargetPrem)
        ,YearsTotLoadTgtLowestPremtax
        ,YearsTotLoadExcLowestPremtax
        ,dblize(kludge_account_value)
        );
}

//============================================================================
void AccountValue::TxAcceptPayment(currency a_pmt)
{
    if(C0 == a_pmt)
        {
        return;
        }

    LMI_ASSERT(C0 <= a_pmt);
    // Internal 1035 exchanges may be exempt from premium tax; they're
    // handled elsewhere, so here the exempt amount is always zero.
    currency actual_load = GetPremLoad(a_pmt, C0);
    currency net_pmt = a_pmt - actual_load;
    LMI_ASSERT(C0 <= net_pmt);
    NetPmts[Month] += net_pmt;

    // If a payment on an ROP contract is treated as an adjustable event,
    // then it should be processed here. The present implementation does
    // not treat it that way; the Blue Book (page 654) says that
    // automatic increases are not adjustable events, and the SA increase
    // due to a payment in this case might be considered automatic by IRS.

    process_payment(net_pmt);

    Dcv += std::max(0.0, dblize(net_pmt));
    LMI_ASSERT(0.0 <= Dcv);

    if(HoneymoonActive)
        {
        HoneymoonValue += std::max(C0, a_pmt);
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

currency AccountValue::GetPremLoad
    (currency a_pmt
    ,currency a_portion_exempt_from_premium_tax
    )
{
    currency excess_portion;
    // All excess.
    if(C0 == UnusedTargetPrem)
        {
        excess_portion = a_pmt;
        }
    // Part target, part excess.
    else if(UnusedTargetPrem < a_pmt)
        {
        excess_portion = a_pmt - UnusedTargetPrem;
        UnusedTargetPrem = C0;
        }
    // All target.
    else
        {
        excess_portion = C0;
        UnusedTargetPrem -= a_pmt;
        }
    currency target_portion = a_pmt - excess_portion;

    premium_load_ =
            target_portion * YearsPremLoadTgt
        +   excess_portion * YearsPremLoadExc
        ;

    sales_load_ =
            target_portion * YearsSalesLoadTgt
        +   excess_portion * YearsSalesLoadExc
        ;
    LMI_ASSERT(0.0 <= sales_load_);
    CumulativeSalesLoad += round_net_premium().c(sales_load_);

    premium_tax_load_ = PremiumTax_->calculate_load
        (dblize(a_pmt - a_portion_exempt_from_premium_tax)
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

    return round_net_premium().c(sum_of_separate_loads);
}

//============================================================================
currency AccountValue::GetRefundableSalesLoad() const
{
    return round_net_premium().c(CumulativeSalesLoad * YearsSalesLoadRefundRate);
#if 0
    // CURRENCY !! Assertions such as these are desirable, but adding
    // them now would cause regression artifacts.
    LMI_ASSERT(C0 <= CumulativeSalesLoad);
    LMI_ASSERT(C0 <= YearsSalesLoadRefundRate);
    double const z = CumulativeSalesLoad * YearsSalesLoadRefundRate;
    LMI_ASSERT(C0 <= z);
    return z;
#endif // 0
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
    if(C0 <= RequestedLoan)
        {
        return;
        }

    // TODO ?? This idiom seems too cute. And it can return -0.0 . See:
    //   https://lists.nongnu.org/archive/html/lmi/2020-09/msg00005.html
    // Maximum repayment is total debt.
    ActualLoan = -std::min(-RequestedLoan, RegLnBal + PrfLnBal);

    process_distribution(ActualLoan);
    LMI_ASSERT(C0 == progressively_reduce(AVRegLn , AVPrfLn , -ActualLoan));
    LMI_ASSERT(C0 == progressively_reduce(RegLnBal, PrfLnBal, -ActualLoan));

// This seems wrong. If we're changing something that's invariant among
// bases, why do we change it for each basis?
// TODO ?? Shouldn't this be moved to FinalizeMonth()?
    InvariantValues().NewCashLoan[Year] = dblize(ActualLoan);
    // TODO ?? Consider changing loan_ullage_[Year] here.
}

/// Set account value before monthly deductions.

void AccountValue::TxSetBOMAV()
{
    // Subtract monthly policy fee and per K charge from account value.

    // Set base for specified-amount load. Usually, this represents
    // the cost of underwriting, so the amount is determined at issue
    // and includes term rider.
    if(Year == InforceYear && Month == InforceMonth)
        {
        if(!yare_input_.TermRider && !TermIsNotRider)
            {
            LMI_ASSERT(C0 == term_specamt(0));
            }
        LMI_ASSERT(yare_input_.InforceSpecAmtLoadBase <= SpecAmtLoadLimit);
        SpecAmtLoadBase =
            (0 == Year && 0 == Month)
            ? std::max
                (term_specamt(0) + base_specamt(0)
                ,round_specamt().c(NetPmts[0] * YearsCorridorFactor)
                )
            : round_specamt().c(yare_input_.InforceSpecAmtLoadBase)
            ;
        // CURRENCY !! support infinities?
        SpecAmtLoadBase = round_specamt().c(std::min(dblize(SpecAmtLoadBase), SpecAmtLoadLimit));
        }

    // These assignments must happen every month.

    MonthsPolicyFees = YearsMonthlyPolicyFee;
    if(0 == Month)
        {
        MonthsPolicyFees    += YearsAnnualPolicyFee;
        }
    YearsTotalPolicyFee += MonthsPolicyFees;

    SpecAmtLoad = round_minutiae().c(YearsSpecAmtLoadRate * SpecAmtLoadBase);
    YearsTotalSpecAmtLoad += SpecAmtLoad;

    process_deduction(MonthsPolicyFees + SpecAmtLoad);

    Dcv -= dblize(MonthsPolicyFees + SpecAmtLoad);
    Dcv = std::max(0.0, Dcv);
}

/// Set death benefit reflecting corridor and death benefit option.
///
/// These variables are updated here:
///   DBIgnoringCorr
///   DBReflectingCorr
///   DB7702A
///   DcvDeathBft
/// so this function is called before any of them is needed.
///
/// Option 2 reflects the total account value: general as well as
/// separate account, and loaned as well as unloaned. Notionally, it
/// keeps NAAR level; therefore, it reflects AV rather than CSV.
///
/// TAXATION !! Revisit this--it affects 'DB7702A':
/// TxSetDeathBft() needs to be called every time a new solve-spec amt
/// is applied to determine the death benefit. But you don't really want to
/// add the sales load (actually a percent of the sales load) to the AV
/// each time. TxSetDeathBft() also gets called during the initial seven
/// pay calc whether or not there is a 1035 exchange. Since it is called
/// from within DoYear() and needs to be called from outside of DoYear(0)
/// even a non-solve will end up double counting the refund of PPL's.
///
/// TODO ?? TAXATION !! Should 7702 or 7702A processing be done here?
/// If so, then this code may be useful:
///    currency prior_db_7702A = DB7702A;
///    currency prior_sa_7702A = ActualSpecAmt;
/// toward the beginning, and:
///    Irc7702A_->UpdateBft7702A(...);
///    LMI_ASSERT(0.0 <= Dcv);
/// toward the end.

void AccountValue::TxSetDeathBft()
{
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
            DBIgnoringCorr = ActualSpecAmt + std::max(C0, TotalAccountValue());
            DB7702A        = ActualSpecAmt;
            }
            break;
        case mce_rop:
            {
            // SA + sum of premiums less withdrawals, but not < SA;
            // i.e., ignore 'CumPmts' if it is less than zero, as it
            // easily can be, e.g., if WDs are not limited to basis.
            DBIgnoringCorr = ActualSpecAmt + std::max(C0, CumPmts);
            DB7702A        = ActualSpecAmt + std::max(C0, CumPmts);
            }
            break;
        case mce_mdb:
            {
            // Specamt is a floor under DB (and therefore zero here)
            // because this option defines the DB as the minimum
            // required by the corridor (but not less than zero).
            DBIgnoringCorr = C0;
            DB7702A        = C0;
            }
            break;
        }
    LMI_ASSERT(C0 <= DBIgnoringCorr);

    // Surrender charges are generally ignored here, but any negative
    // surrender charge must be subtracted, increasing the account value.
    currency cash_value_for_corridor =
          TotalAccountValue()
        - std::min(C0, SurrChg())
        + GetRefundableSalesLoad()
//        + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        ;

    cash_value_for_corridor = std::max
        (cash_value_for_corridor
        ,HoneymoonValue
        );

    DBReflectingCorr = std::max
        (DBIgnoringCorr
        ,round_death_benefit().c(YearsCorridorFactor * std::max(C0, cash_value_for_corridor))
        );
    LMI_ASSERT(C0 <= DBReflectingCorr);
    // This overrides the value assigned above. There's more than one
    // way to interpret 7702A "death benefit"; this is just one.
    // TAXATION !! Use DB_Irc7702BftIsSpecAmt
    DB7702A = DBReflectingCorr + TermDB;

    DcvDeathBft = std::max
        (   dblize(DBIgnoringCorr)
        ,   (
                YearsCorridorFactor
            *   (   Dcv
                -   dblize(std::min(C0, SurrChg()))
                +   dblize(GetRefundableSalesLoad())
//                +   std::max(0.0, ExpRatReserve) // This would be added if it existed.
                )
            )
        );

    // SOMEDAY !! Accumulate average death benefit for profit testing here.
}

//============================================================================
void AccountValue::TxSetTermAmt()
{
    if(!TermRiderActive)
        {
        return;
        }
    // If term is not formally a rider, then it's always active for
    // the entire illustrated duration. Its amount may be reduced to
    // zero, but might later be increased; at any rate, illustrations
    // do not "remove" it.
    if(!yare_input_.TermRider && !TermIsNotRider)
        {
        TermRiderActive = false;
        return;
        }
    if
        (  (TermForcedConvDur <= Year)
        && (TermForcedConvAge <= Year + BasicValues::GetIssueAge())
        )
        {
        EndTermRider(true);
        return;
        }

    TermDB = std::max(C0, TermSpecAmt + DBIgnoringCorr - DBReflectingCorr);
    TermDB = round_death_benefit().c(TermDB); // CURRENCY !! already rounded?
}

/// Terminate the term rider, optionally converting it to base.
///
/// Conversion may be required in a state that imposes an age limit
/// (e.g., attained age seventy) on term riders.
///
/// For at least one supported product, the term rider terminates if
/// the AV is insufficient to pay the term charge. In that case:
///  - the base policy might in principle continue for some time; and
///  - the admin system would keep the term coverage in force until
/// the next monthiversary by grace; therefore, it would not recognize
/// a material change or adjustment event until the following month.
/// Illustrations recognize the tax event immediately: they show no
/// gratis continuation of benefits in the hope that charges may later
/// be recouped.

void AccountValue::EndTermRider(bool convert)
{
    if(!TermRiderActive)
        {
        return;
        }

    TermRiderActive = false;
    if(convert)
        {
        ChangeSpecAmtBy(TermSpecAmt);
        }
    TermSpecAmt = C0;
    TermDB      = C0;
    // Carry the new term spec amt forward into all future years.
    for(int j = Year; j < BasicValues::GetLength(); ++j)
        {
        InvariantValues().TermSpecAmt[j] = dblize(TermSpecAmt);
        }
}

/// Calculate mortality charge.

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
#if defined USE_CURRENCY_CLASS
    NAAR = round_naar().c
        (  DBReflectingCorr * DBDiscountRate[Year]
        - dblize(std::max(C0, TotalAccountValue()))
        );
    NAAR = std::max(C0, NAAR);
#else  // !defined USE_CURRENCY_CLASS
    NAAR = material_difference
        (DBReflectingCorr * DBDiscountRate[Year]
        ,std::max(0.0, TotalAccountValue())
        );
    NAAR = std::max(0.0, round_naar()(NAAR));
#endif // !defined USE_CURRENCY_CLASS

// TODO ?? This doesn't work. We need to reconsider the basic transactions.
//  currency naar_forceout = std::max(0.0, NAAR - MaxNAAR);
//  process_distribution(naar_forceout);
// TAXATION !! Should this be handled at the same time as GPT forceouts?

    DcvNaar = material_difference
        (std::max(DcvDeathBft, dblize(DBIgnoringCorr)) * DBDiscountRate[Year]
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
        retention_charge = NAAR * retention_rate;
        }

    CoiCharge    = round_coi_charge().c(NAAR * ActualCoiRate);
    NetCoiCharge = CoiCharge - round_coi_charge().c(retention_charge);
    YearsTotalCoiCharge += CoiCharge;

    // DCV need not be rounded.
    DcvCoiCharge = DcvNaar * (YearsDcvCoiRate + CoiRetentionRate);
}

/// Calculate rider charges.

void AccountValue::TxSetRiderDed()
{
    AdbCharge = C0;
    if(yare_input_.AccidentalDeathBenefit)
        {
        AdbCharge = round_rider_charges().c
            (YearsAdbRate * std::min(dblize(ActualSpecAmt), AdbLimit)
            );
        }

    SpouseRiderCharge = C0;
    if(yare_input_.SpouseRider)
        {
        SpouseRiderCharge = round_rider_charges().c
            (YearsSpouseRiderRate * yare_input_.SpouseRiderAmount
            );
        }

    ChildRiderCharge = C0;
    if(yare_input_.ChildRider)
        {
        ChildRiderCharge = round_rider_charges().c
            (YearsChildRiderRate * yare_input_.ChildRiderAmount
            );
        }

    TermCharge    = C0;
    DcvTermCharge = 0.0;
    if(TermRiderActive)
        {
        TermCharge    = round_rider_charges().c
            (YearsTermRate   * TermDB * DBDiscountRate[Year]
            );
        // TAXATION !! Integrated term: s/TermDB/TermSpecAmt/ because
        // it can't go into the corridor under tax assumptions.
        // TAXATION !! Use a distinct discount rate for taxation? Or
        // the policy's rate, as used for DcvNaar?
        DcvTermCharge = YearsDcvCoiRate * TermDB * DBDiscountRate[Year];
        }

    WpCharge    = C0;
    DcvWpCharge = 0.0;
    if(yare_input_.WaiverOfPremiumBenefit)
        {
        switch(WaiverChargeMethod)
            {
            case oe_waiver_times_specamt:
                {
                WpCharge = round_rider_charges().c
                    (YearsWpRate * std::min(dblize(ActualSpecAmt), WpLimit)
                    );
                DcvWpCharge = dblize(WpCharge);
                }
                break;
            case oe_waiver_times_deductions:
                {
                // Premium load and M&E charges are not waived.
                // The amount waived is subject to no maximum.
                WpCharge = round_rider_charges().c
                    (
                    YearsWpRate
                    *   (
                          CoiCharge
                        + MonthsPolicyFees
                        + SpecAmtLoad
                        + AdbCharge
                        + SpouseRiderCharge
                        + ChildRiderCharge
                        + TermCharge
                        )
                    );
                DcvWpCharge =
                    YearsWpRate
                    *   (
                          DcvCoiCharge
                        + dblize
                            ( MonthsPolicyFees
                            + SpecAmtLoad
                            + AdbCharge
                            + SpouseRiderCharge
                            + ChildRiderCharge
                            )
                        + DcvTermCharge
                        );
                }
                break;
            }
        }
}

/// Subtract monthly deductions from unloaned account value.

void AccountValue::TxDoMlyDed()
{
    if(TermRiderActive && TermCanLapse && (AVGenAcct + AVSepAcct - CoiCharge) < TermCharge)
        {
        EndTermRider(false);
        TermCharge = C0;
        }

    // 'Simple' riders are the same for AV and DCV.
    currency simple_rider_charges =
            AdbCharge
        +   SpouseRiderCharge
        +   ChildRiderCharge
        ;

    double dcv_mly_ded =
            DcvCoiCharge
        +   dblize(simple_rider_charges)
        +   DcvTermCharge
        +   DcvWpCharge
        ;

    // Round total rider charges, even if each individual charge was
    // not rounded, so that deductions can be integral cents.
    RiderCharges = round_minutiae().c(simple_rider_charges + TermCharge + WpCharge);
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

    YearsTotalNetCoiCharge += dblize(NetCoiCharge);

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
    currency csv_ignoring_loan =
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
    if(HoneymoonValue <= C0 || HoneymoonValue < csv_ignoring_loan)
        {
        HoneymoonActive = false;
#if defined USE_CURRENCY_CLASS
        // CURRENCY !! support infinities?
        HoneymoonValue = -from_cents(std::numeric_limits<currency::data_type>::max());
#else  // !defined USE_CURRENCY_CLASS
        HoneymoonValue = -std::numeric_limits<double>::max();
#endif // !defined USE_CURRENCY_CLASS
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
        // CURRENCY !! should class stratified_charges use currency?
        double stratified_load = StratifiedCharges_->stratified_sepacct_load
            (GenBasis_
            ,dblize(AssetsPostBom)
            ,dblize(CumPmtsPostBom)
            ,database().query<double>(DB_DynSepAcctLoadLimit)
            );

        double tiered_comp = 0.0;
        if(oe_asset_charge_load == database().query<oenum_asset_charge_type>(DB_AssetChargeType))
            {
            tiered_comp = StratifiedCharges_->tiered_asset_based_compensation(dblize(AssetsPostBom));
            }
        if(0.0 != tiered_comp)
            {
            alarum()
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
        YearsSepAcctLoadRate = round_interest_rate()(YearsSepAcctLoadRate);
        }

    SepAcctLoad = round_interest_credit().c(YearsSepAcctLoadRate * AVSepAcct);
    // CURRENCY !! Does this seem right? Mightn't it take a sepacct load from the genacct?
    process_deduction(SepAcctLoad);
    YearsTotalSepAcctLoad += SepAcctLoad;
    Dcv -= dblize(SepAcctLoad);
    Dcv = std::max(0.0, Dcv);
}

//============================================================================
// When the M&E charge depends on each month's case total assets, the
// interest rate is no longer an annual invariant. Set it monthly here.
void AccountValue::ApplyDynamicMandE(currency assets)
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
            alarum()
                << "Dynamic M&E not supported with midpoint expense basis."
                << LMI_FLUSH
                ;
            }
            break;
        }

// TODO ?? Implement tiered comp and tiered management fee.

    // Annual separate-account rates.

    double m_and_e_rate = StratifiedCharges_->tiered_m_and_e(GenBasis_, dblize(assets));
    double imf_rate = StratifiedCharges_->tiered_investment_management_fee(dblize(assets));
    if(0.0 != imf_rate)
        {
        alarum()
            << "Tiered investment management fee unimplemented."
            << LMI_FLUSH
            ;
        }
    double asset_comp_rate =
        (oe_asset_charge_spread == database().query<oenum_asset_charge_type>(DB_AssetChargeType))
            ? StratifiedCharges_->tiered_asset_based_compensation(dblize(assets))
            : 0.0
            ;
    if(0.0 != asset_comp_rate)
        {
        alarum()
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

    currency notional_sep_acct_charge = C0;

    // SOMEDAY !! This should be done in the interest-rate class.
    double gross_sep_acct_rate = i_upper_12_over_12_from_i<double>()
        (InterestRates_->SepAcctGrossRate(SepBasis_)[Year]
        );
    gross_sep_acct_rate = round_interest_rate()(gross_sep_acct_rate);
    if(mce_gen_mdpt == GenBasis_)
        {
        gross_sep_acct_rate = 0.0;
        }

    if(C0 < AVSepAcct)
        {
        SepAcctIntCred = InterestCredited(AVSepAcct, YearsSepAcctIntRate);
        currency gross = InterestCredited(AVSepAcct, gross_sep_acct_rate);
        notional_sep_acct_charge = gross - SepAcctIntCred;
#if defined USE_CURRENCY_CLASS
        currency result = AVSepAcct + SepAcctIntCred;
        // CURRENCY !! rethink this weird logic
        if(result < C0 && C0 <= AVSepAcct)
            {
            AVSepAcct = C0;
            }
        else
            {
            AVSepAcct = result;
            }
#else  // !defined USE_CURRENCY_CLASS
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
#endif // !defined USE_CURRENCY_CLASS
        }
    else
        {
        SepAcctIntCred = C0;
        }

    if(C0 < AVGenAcct)
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
        GenAcctIntCred = C0;
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
    LMI_ASSERT(C0 <= AVRegLn && C0 <= AVPrfLn);

    currency z = RegLnIntCred + PrfLnIntCred + SepAcctIntCred + GenAcctIntCred;
    YearsTotalNetIntCredited   += z;
    YearsTotalGrossIntCredited += z + notional_sep_acct_charge;
}

/// Accrue loan interest, and calculate interest credit on loaned AV.

void AccountValue::TxLoanInt()
{
    // Reinitialize to zero before potential early exit, to sweep away
    // any leftover values (e.g., after a loan has been paid off).
    RegLnIntCred = C0;
    PrfLnIntCred = C0;

    // Nothing more to do if there's no loan outstanding.
    if(C0 == RegLnBal && C0 == PrfLnBal)
        {
        return;
        }

    // We may want to display credited interest separately.
    // Each interest increment is rounded separately.
    RegLnIntCred = InterestCredited(AVRegLn, YearsRegLnIntCredRate);
    PrfLnIntCred = InterestCredited(AVPrfLn, YearsPrfLnIntCredRate);

    AVRegLn += RegLnIntCred;
    AVPrfLn += PrfLnIntCred;

    currency RegLnIntAccrued = InterestCredited(RegLnBal, YearsRegLnIntDueRate);
    currency PrfLnIntAccrued = InterestCredited(PrfLnBal, YearsPrfLnIntDueRate);

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
currency AccountValue::anticipated_deduction
    (mcenum_anticipated_deduction method)
{
    switch(method)
        {
        case mce_twelve_times_last:
            {
            return 12 * MlyDed;
            }
        case mce_eighteen_times_last:
            {
            return 18 * MlyDed;
            }
        case mce_to_next_anniversary:
            {
            return MlyDed * (13 - Month);
            }
        case mce_to_next_modal_pmt_date:
            {
            return MlyDed * (1 + MonthsToNextModalPmtDate());
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
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
/// withdrawal, say 80% or 90%. Some apply different proportions to
/// general- and separate-account values.

void AccountValue::SetMaxWD()
{
    MaxWD =
          round_withdrawal().c
            ( AVGenAcct * MaxWdGenAcctValMult
            + AVSepAcct * MaxWdSepAcctValMult
            )
        + (AVRegLn  + AVPrfLn)
        - (RegLnBal + PrfLnBal)
        - anticipated_deduction(MaxWDDed_)
        - std::max(C0, SurrChg())
        ;
    if(MaxWD < MinWD)
        {
        MaxWD = C0;
        }
    MaxWD = std::max(C0, MaxWD);
}

/// Take a withdrawal.

void AccountValue::TxTakeWD()
{
    // Illustrations allow withdrawals only on anniversary.
    if(0 != Month)
        {
        return;
        }

    GrossWD = C0;
    RequestedWD = Outlay_->withdrawals()[Year];

    if(Debugging || C0 != RequestedWD)
        {
        SetMaxWD();
        }

    NetWD = C0;

    // Nothing more to do if no withdrawal requested.
    if(C0 == RequestedWD)
        {
        withdrawal_ullage_[Year] = C0;
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
        withdrawal_ullage_[Year] = std::max(C0, RequestedWD - MaxWD);
        }

    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        NetWD = round_withdrawal().c(std::min(RequestedWD, MaxWD)); // CURRENCY !! already rounded?
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
        withdrawal_ullage_[Year] = C0;
        NetWD = C0;
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
        if(C0 == TaxBasis || std::min(TaxBasis, RequestedWD) < MinWD) // All loan
            {
            withdrawal_ullage_[Year] = C0;
            NetWD = C0;
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

    if(NetWD <= C0)
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

    currency av = TotalAccountValue();
    currency csv = av - SurrChg_[Year];
    LMI_ASSERT(C0 <= SurrChg_[Year]);
    if(csv <= C0)
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

    GrossWD = NetWD + std::min(WDFee, round_withdrawal().c(NetWD * WDFeeRate));

    // Free partial surrenders: for instance, the first 20% of account
    // value might be withdrawn each policy year free of surrender
    // charge. This would become more complicated if we maintained
    // distinct surrender-charge layers.

    // CURRENCY !! more efficient: currency / currency --> double
    double surrchg_proportion = SurrChg_[Year] / dblize(csv);
    currency non_free_wd = GrossWD;
    if(0.0 != FreeWDProportion[Year])
        {
        // The free partial surrender amount is determined annually,
        // on anniversary and before the anniversary deduction but
        // after any loan has been capitalized.
        LMI_ASSERT(AVRegLn == RegLnBal);
        LMI_ASSERT(AVPrfLn == PrfLnBal);
        LMI_ASSERT(av == AVGenAcct + AVSepAcct);
        currency free_wd = round_withdrawal().c(FreeWDProportion[Year] * av);
        non_free_wd = std::max(C0, GrossWD - free_wd);
        }
    double partial_surrchg = non_free_wd * surrchg_proportion;
    GrossWD += round_withdrawal().c(partial_surrchg);

    process_distribution(GrossWD);
    Dcv -= dblize(GrossWD);
    Dcv = std::max(0.0, Dcv);

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
            if(WdDecrSpecAmtDboLvl)
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
                }
            else
                {
                // Do nothing.
                }
            }
            break;
        case mce_option2:
            {
            if(WdDecrSpecAmtDboInc)
                {
                ChangeSpecAmtBy(-GrossWD);
                }
            else
                {
                // Do nothing.
                }
            }
            break;
        case mce_rop:
            {
            if(WdDecrSpecAmtDboRop)
                {
                ChangeSpecAmtBy(-GrossWD);
                }
            else
                {
                // Do nothing.
                }
            }
            break;
        case mce_mdb:
            {
            // Do nothing. Every other DBO conditionally decreases the
            // specamt, but the mce_mdb specamt is zero by definition
            // and cannot be decreased.
            }
            break;
        }

    CumPmts     -= NetWD;
    TaxBasis    -= NetWD; // TODO ?? TAXATION !! This should be gross, not net; how about the line above and the line below?
    CumWD       += NetWD;
    // TAXATION !! What about 7702A "amounts paid"? --That's handled
    // in DoMonthDR() instead. Probably both should be handled in the
    // same place.

    if(Solving || mce_run_gen_curr_sep_full == RunBasis_)
        {
        // Illustration-reg guaranteed premium ignores GPT limit.
        if(!SolvingForGuarPremium)
            {
            // TODO ?? TAXATION !! What if reference argument
            // 'premiums_paid_increment' is modified?
            double premiums_paid_increment = -dblize(GrossWD);
            Irc7702_->ProcessGptPmt(Year, premiums_paid_increment);
            }
        }

// This seems wrong. If we're changing something that's invariant among
// bases, why do we change it for each basis?
// TODO ?? Shouldn't this be moved to FinalizeMonth()?
    InvariantValues().NetWD[Year] = dblize(NetWD);
}

//============================================================================
// Calculate maximum permissible total loan (not increment).
void AccountValue::SetMaxLoan()
{
    double max_loan =
          (AVGenAcct + AVSepAcct) * MaxLoanAVMult
        + dblize
            ( (AVRegLn + AVPrfLn)
            - anticipated_deduction(MaxLoanDed_)
            - std::max(C0, SurrChg())
            )
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
        alarum() << "Off-anniversary loans untested." << LMI_FLUSH;
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
    max_loan -=
          RegLnBal * reg_loan_factor
        + PrfLnBal * prf_loan_factor
        ;

    // TODO ?? This adjustment isn't quite right because it uses only
    // the regular-loan interest factor. Is it conservative under the
    // plausible but unasserted assumption that that factor is more
    // liberal than the preferred-loan factor?
    //
    max_loan *= 1.0 - (reg_loan_factor) / (1.0 + reg_loan_factor);

    MaxLoan = round_loan().c(max_loan);

    // I do not think we want a MaxLoan < current level of indebtedness.
    MaxLoan = std::max((AVRegLn + AVPrfLn), MaxLoan);
    // TODO ?? Yet I do not think we want to ratify something that looks broken!
}

/// Take a new cash loan, limiting it to respect the maximum loan.

void AccountValue::TxTakeLoan()
{
    // Illustrations allow loans only on anniversary.
    if(0 != Month)
        {
        return;
        }

    // Even if no loan is requested, the maximum loan is still shown
    // in a monthly trace.
    if(Debugging || C0 <= RequestedLoan)
        {
        SetMaxLoan();
        }

    // Nothing more to do if no loan requested.
    if(RequestedLoan <= C0)
        {
        loan_ullage_[Year] = C0;
        return;
        }

    currency max_loan_increment = MaxLoan - (AVRegLn + AVPrfLn);

    // When performing a solve, let it become overloaned--otherwise
    // we'd introduce a discontinuity in the function for which we
    // seek a root, and that generally makes the solve slower.
    if(Solving)
        {
        ActualLoan = RequestedLoan;
        loan_ullage_[Year] = std::max(C0, RequestedLoan - max_loan_increment);
        }
    else
        {
        ActualLoan = std::min(max_loan_increment, RequestedLoan);
        ActualLoan = std::max(ActualLoan, C0);
        // TODO ?? Shouldn't this happen in FinalizeMonth()?
        InvariantValues().NewCashLoan[Year] = dblize(ActualLoan);
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

    if(!AllowPrefLoan || Year < FirstPrefLoanYear)
        {
        AVRegLn  += ActualLoan;
        RegLnBal += ActualLoan;
        }
    else
        {
        AVPrfLn  += ActualLoan;
        PrfLnBal += ActualLoan;
        }
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

/// Test for lapse.

void AccountValue::TxTestLapse()
{
    // The refundable load cannot prevent a lapse that would otherwise
    // occur, because it is refunded only after termination. The same
    // principle applies to a negative surrender charge.
    //
    // The experience rating reserve can prevent a lapse, because it
    // is an actual balance-sheet item that is actually held in the
    // certificate.

    currency lapse_test_csv =
          TotalAccountValue()
        - (RegLnBal + PrfLnBal)
//        + std::max(0.0, ExpRatReserve) // This would be added if it existed.
        ;
    if(!LapseIgnoresSurrChg)
        {
        lapse_test_csv -= std::max(C0, SurrChg());
        }
    lapse_test_csv = std::max(lapse_test_csv, HoneymoonValue);

    // Perform no-lapse test.
    if(NoLapseActive && !NoLapseAlwaysActive)
        {
        CumNoLapsePrem += MlyNoLapsePrem + RiderCharges;
        if
            (       NoLapseMinAge <= Year + BasicValues::GetIssueAge()
                &&  NoLapseMinDur <= Year
            ||      CumPmts < CumNoLapsePrem
#if defined USE_CURRENCY_CLASS
            // x<y --> x<>y for x,y integral
#else  // !defined USE_CURRENCY_CLASS
                &&  !materially_equal(CumPmts, CumNoLapsePrem)
#endif // !defined USE_CURRENCY_CLASS
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
            (!NoLapseActive && lapse_test_csv < C0)
        // Lapse if overloaned regardless of guar DB.
        // CSV includes a positive loan (that can offset a negative AV);
        // however, we still need to test for NoLapseActive.
        ||  (!NoLapseActive && (AVGenAcct + AVSepAcct) < C0)
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
        if(NoLapseActive && lapse_test_csv < C0)
            {
            AVGenAcct = C0;
            AVSepAcct = C0;
            // TODO ?? Can't this be done elsewhere?
            VariantValues().CSVNet[Year] = 0.0;
            }
        else if(!HoneymoonActive && !Solving && lapse_test_csv < C0)
            {
            alarum()
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
            InvariantValues().External1035Amount = dblize(External1035Amount);
            InvariantValues().Internal1035Amount = dblize(Internal1035Amount);
            InvariantValues().Dumpin = dblize(Dumpin);
            }

        // TAXATION !! We could also capture MEC status on other bases here.
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
