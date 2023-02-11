// Internal Revenue Code section 7702
//
// Copyright (C) 2021, 2022, 2023 Gregory W. Chicares.
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

#include "gpt7702.hpp"

#include "assert_lmi.hpp"
#include "oecumenic_enumerations.hpp"   // oenum_glp_or_gsp
#include "round_to.hpp"

#include <algorithm>                    // max(), min()

static round_to<double> const round_max_premium(2, r_downward);

gpt7702::gpt7702
    (std::vector<double> const& qc
    ,std::vector<double> const& glp_ic
    ,std::vector<double> const& glp_ig
    ,std::vector<double> const& gsp_ic
    ,std::vector<double> const& gsp_ig
    ,gpt_vector_parms    const& charges
    )
    :cf_(qc, glp_ic, glp_ig, gsp_ic, gsp_ig, charges)
{
}

/// Set initial guideline premiums.
///
/// The parameters used here may not be readily ascertainable when the
/// constructor executes. If the specified amount is given and an
/// illustration system is to determine the payment pattern as GLP or
/// GSP, then the only common complication is that premium loads may
/// change at a target-premium breakpoint, and a closed-form algebraic
/// solution is straightforward. But if the specified amount is to be
/// determined as a function of a given premium amount, then the
/// calculation is more complicated:
///  - target premium is generally a (not necessarily simple) function
///    of specified amount, which is the unknown dependent variable;
///  - a load per dollar of specified amount might apply only up to
///    some fixed limit;
///  - the amount of a QAB such as ADB might equal specified amount,
///    but only up to some maximum determined by underwriting;
/// so that the best approach is iterative--and that requires an
/// instance of this class to be created before the specified amount
/// is determined.
///
/// To support inforce illustrations, several inforce parameters are
/// passed from an admin-system extract, representing the historical
/// GPT calculations it has performed. The full history of relevant
/// transaction could be voluminous and is generally not available;
/// without it, those parameters cannot be validated here.
///
/// Initial GLP and GSP may be wanted even for CVAT contracts, e.g. to
/// illustrate a premium pattern such as "GSP for one year, then zero"
/// for both GPT and CVAT. 'defn_life_ins' facilitates skipping GPT
/// restrictions and adjustment for CVAT contracts in such a use case.
///
/// Asserted preconditions:
///  - argument 'fractional_duration' is in [0.0, 1.0)
///  - inforce arguments are all zero if the policy is issued today
/// The values of 'inforce_.*' arguments are otherwise unrestricted.
///
/// Asserted postcondition:
///  - the guideline limit is not violated
///
/// The argument (and members) of type gpt_scalar_parms are parameter
/// objects that are forwarded to calculate_premium(), which asserts
/// appropriate preconditions for them.

void gpt7702::initialize_gpt
    (mcenum_defn_life_ins    defn_life_ins
    ,double                  fractional_duration
    ,double                  inforce_glp
    ,double                  inforce_cum_glp
    ,double                  inforce_gsp
    ,currency                inforce_cum_f1A
    ,gpt_scalar_parms const& arg_parms
    )
{
    LMI_ASSERT(0.0 <= fractional_duration && fractional_duration < 1.0);

    defn_life_ins_       = defn_life_ins;
    fractional_duration_ = fractional_duration;
    s_parms_             = arg_parms;

    if(is_issued_today())
        {
        LMI_ASSERT(0.0 == inforce_glp    );
        LMI_ASSERT(0.0 == inforce_cum_glp);
        LMI_ASSERT(0.0 == inforce_gsp    );
        LMI_ASSERT(C0  == inforce_cum_f1A);
        glp_ = cf_.calculate_premium(oe_glp, s_parms_);
        cum_glp_ = glp_;
        gsp_ = cf_.calculate_premium(oe_gsp, s_parms_);
        cum_f1A_ = C0;
        }
    else
        {
        // 7702 !! Assume that a client provides unrounded values for
        // arguments of type 'double'. If it provides rounded values,
        // they may need to be "unrounded" somehow (perhaps, e.g., by
        // substituting the next representable value toward positive
        // infinity). Alternatively, properly rounded values could be
        // passed to this function as objects of class currency.
        glp_     = inforce_glp    ;
        cum_glp_ = inforce_cum_glp;
        gsp_     = inforce_gsp    ;
        cum_f1A_ = inforce_cum_f1A;
        // Inforce data are as of the last moment of the preceding
        // day, so an illustration for an exact anniversary must
        // increment cumulative GLP.
        if(0.0 == fractional_duration_)
            {increment_boy();}
        }

    if(mce_gpt == defn_life_ins_)
        {LMI_ASSERT(cum_f1A_ <= guideline_limit());}
}

/// Handle an update notification from the client.
///
/// It is assumed that the client can call into this server, which
/// however cannot call back into the client. Therefore, the client
/// must periodically call this function--perhaps only once a year for
/// an illustration system that restricts relevant transactions to
/// policy anniversaries, but once a day for an admin system.
///
/// The 'fractional_duration' argument specifies what proportion of
/// the present year has elapsed. All adjustment events that occur on
/// the same date must be combined together and processed as a single
/// change; therefore, for a policy ninety-three days past its eighth
/// anniversary, an admin system would pass
///    8 as 'arg_parms.duration'
///   93/365 or 93/366, as the case may be, as 'fractional_duration'
/// while an illustration system might specify 'fractional_duration'
/// as 0.0 always (or perhaps as something like 3/12).
///
/// Return amount forced out, or zero if there is no forceout.
///
/// Asserted preconditions:
///  - argument 'fractional_duration' is in [0.0, 1.0)
///  - 'arg_parms.duration' is either the same as 's_parms_.duration',
///    or greater by one, in which case the latter is incremented here
///  - 's_parms_.duration' is incremented only on exact anniversaries
///  - the queued agenda is self-consistent
///
/// Alternative not pursued: In lieu of 'arg_parms.duration' and
/// 'fractional_duration', pass calendar dates and perform date
/// arithmetic here.
///
/// Alternative not pursued: An adjustment event occurs if either
///   spec amt and DB both changed, or
///   dbopt changed, or
///   a QAB changed (but QABs are not yet supported here),
/// and that determination could be made here instead of making it
/// the client's responsibility. That would require passing both
/// spec amt and DB here, where they would be used for no other
/// purpose; but the determination is simple, and the client already
/// has the necessary information. Furthermore, other adjustment
/// events such as a change in mortality or interest basis cannot
/// reasonably be discerned here.
///
/// Alternative not pursued: Dispense with queuing; instead, add
/// two more arguments, and enqueue them here, thus:
///   enqueue_exch_1035    (queued_exch_1035_amt);
///   enqueue_f1A_decrease (queued_f1A_decrement);
///   enqueue_adj_event    (arg_parms);
/// But that would make the client responsible for assembling those
/// arguments correctly. It is better for the client simply to send
/// notifications as the need arises, relying on the server to handle
/// them correctly--not least because the server can be unit-tested
/// far more easily than the client.

currency gpt7702::update_gpt
    (gpt_scalar_parms const& arg_parms
    ,double                  fractional_duration
    ,currency                f2A_value
    )
{
    if(mce_gpt != defn_life_ins_)
        {return C0;}

    LMI_ASSERT(0.0 <= fractional_duration && fractional_duration < 1.0);
    fractional_duration_ = fractional_duration;
    bool must_increment_duration {arg_parms.duration != s_parms_.duration};
    if(must_increment_duration)
        {
        LMI_ASSERT(0.0 == fractional_duration_);
        ++s_parms_.duration;
        LMI_ASSERT(arg_parms.duration == s_parms_.duration);
        }

    if(queued_f1A_decrease_)
        {dequeue_f1A_decrease();}
    else
        {LMI_ASSERT(C0 == queued_f1A_decrement_);}

    if(queued_exch_1035_)
        {dequeue_exch_1035();}
    else
        {LMI_ASSERT(C0 == queued_exch_1035_amt_);}

    if(queued_adj_event_)
        {dequeue_adj_event(arg_parms);}
    else
        {
        LMI_ASSERT(arg_parms.duration       == s_parms_.duration      );
        // A specamt change is an adjustment event only if DB also
        // changes; the client is responsible for determining that.
//      LMI_ASSERT(arg_parms.f3_bft         == s_parms_.f3_bft        );
//      LMI_ASSERT(arg_parms.endt_bft       == s_parms_.endt_bft      );
        // A target premium change is not an adjustment event per se.
//      LMI_ASSERT(arg_parms.target_prem    == s_parms_.target_prem   );
        LMI_ASSERT(arg_parms.chg_sa_base    == s_parms_.chg_sa_base   );
        LMI_ASSERT(arg_parms.dbopt_7702     == s_parms_.dbopt_7702    );
        LMI_ASSERT(arg_parms.qab_gio_amt    == s_parms_.qab_gio_amt   );
        LMI_ASSERT(arg_parms.qab_adb_amt    == s_parms_.qab_adb_amt   );
        LMI_ASSERT(arg_parms.qab_term_amt   == s_parms_.qab_term_amt  );
        LMI_ASSERT(arg_parms.qab_spouse_amt == s_parms_.qab_spouse_amt);
        LMI_ASSERT(arg_parms.qab_child_amt  == s_parms_.qab_child_amt );
        LMI_ASSERT(arg_parms.qab_waiver_amt == s_parms_.qab_waiver_amt);
        }

    if(must_increment_duration)
        {increment_boy();}
    return force_out(f2A_value);
}

/// Accept payment up to limit; return the amount accepted.
///
/// The excess (if any) is "rejected" in the programming sense only,
/// and not in the accounting sense. If $100 is remitted when only $90
/// is allowed, then the entire remittance would be rejected by an
/// actual admin system. In the hypothetical world of illustrations,
/// the $100 is deemed to have been so rejected and replaced by a $90
/// remittance.
///
/// The "rejected" excess is stored in a private data member in order
/// to complete the tableau, which provides a summary of a set of
/// transactions for testing and debugging. That member deliberately
/// has no accessor; clients must use this function's return value
/// only. That member is zeroed upon entry to this function. Unlike
/// adjustment events, payments need not be combined--there can be
/// more than one in a day--so the tableau reflects only the most
/// recent "rejected" payment.

currency gpt7702::accept_payment(currency payment)
{
    if(mce_gpt != defn_life_ins_)
        {return payment;}

    rejected_pmt_ = C0;

    if(C0 == payment)
        {return C0;}

    LMI_ASSERT(C0 < payment);
    currency const allowed = std::max(C0, guideline_limit() - cum_f1A_);
    currency const accepted = std::min(allowed, payment);
    rejected_pmt_ = payment - accepted;
    LMI_ASSERT(C0 <= rejected_pmt_);
    LMI_ASSERT(accepted + rejected_pmt_ == payment);
    cum_f1A_ += accepted;
    return accepted;
}

/// Enqueue a 1035 exchange, storing the gross amount of the exchange.
///
/// Asserted preconditions:
///  - No other 1035 exchange has been queued. In the rare case that
///    several policies are exchanged for one, the client is assumed
///    to have combined them.
///  - The exchange occurs as of the issue date.
///  - Cumulative premiums paid equals zero.
///  - The exchange amount is nonnegative.
///  - The exchange amount does not exceed the guideline limit.
///
/// The exchange amount is required to be nonnegative, as negative
/// exchanges seem never to occur in practice. A 1035 exchange carries
/// over the basis, which may be advantageous even if the exchanged
/// amount is arbitrarily low or perhaps even zero.

void gpt7702::enqueue_exch_1035(currency exch_amt)
{
    if(mce_gpt != defn_life_ins_)
        {return;}

    LMI_ASSERT(!queued_exch_1035_);
    LMI_ASSERT(C0 == queued_exch_1035_amt_);
    LMI_ASSERT(is_issued_today());
    LMI_ASSERT(0  == s_parms_.duration);
    LMI_ASSERT(C0 == cum_f1A_);
    LMI_ASSERT(C0 <= exch_amt);
    LMI_ASSERT(exch_amt <= guideline_limit());
    queued_exch_1035_ = true;
    queued_exch_1035_amt_ = exch_amt;
}

/// Enqueue a decrease in premiums paid, storing the decrement.
///
/// Asserted preconditions:
///  - No other such decrease has been queued.
///  - The decrement is positive.
///  - The decrease doesn't occur on the issue date.
///
/// The contemplated purpose is to net nontaxable withdrawals against
/// premiums paid (the client being responsible for determining the
/// extent to which they're nontaxable). This function could also
/// handle exogenous events that decrease premiums paid, such as a
/// payment returned to preserve a non-MEC, but it is assumed that no
/// such payment need be returned because an admin system would refuse
/// to accept it. If it is desired to accept multiple decrements, this
/// code would need to be modified to accumulate them.

void gpt7702::enqueue_f1A_decrease(currency decrement)
{
    if(mce_gpt != defn_life_ins_)
        {return;}

    LMI_ASSERT(!queued_f1A_decrease_);
    LMI_ASSERT(C0 == queued_f1A_decrement_);
    LMI_ASSERT(C0 < decrement);
    LMI_ASSERT(!is_issued_today());
    queued_f1A_decrease_ = true;
    queued_f1A_decrement_ = decrement;
}

/// Enqueue a potential adjustment event.
///
/// Multiple adjustment events occurring on the same day must be
/// combined together and processed as one single change. In the
/// A + B - C formula, only the respective sets of arguments to
/// calculate_premium() matter. A's are already known. B's are
/// the same as A's except that the current duration is used. C's
/// simply represent the final state resulting from all changes
/// taken together, so they're just a snapshot of the applicable
/// arguments as of the moment before the combined change is
/// processed. Therefore, if multiple events occur asynchronously,
/// it would be appropriate and correct to store a single snapshot
/// of C's arguments here, overwriting any previously stored. An
/// early version of this code did exactly that, producing a nice
/// symmetry: every /enqueue_.*()/ function took an argument and
/// stored it in a data member, and every /dequeue_.*/ function
/// used that stored member.
///
/// However, that proved to be needlessly complicated. Each of the
/// other /enqueue_.*/ functions stores only one one scalar datum,
/// whereas this one stored a gpt_scalar_parms object, and required
/// its caller(s) to pass that entire object. But update_gpt() takes
/// a gpt_scalar_parms argument, as is appropriate to ensure that no
/// adjustment is missed. It seemed that this update_gpt() argument
/// and the queued gpt_scalar_parms data member should be identical,
/// but an assertion to that effect failed when update_gpt() was
/// called in a new year with no adjustment event. Adding extra code
/// to conditionalize the assertion was unreasonable: the superfluous
/// data member imposed too much overhead for no benefit.

void gpt7702::enqueue_adj_event()
{
    if(mce_gpt != defn_life_ins_)
        {return;}

    LMI_ASSERT(!is_issued_today());
    queued_adj_event_ = true;
}

double gpt7702::raw_glp           () const
{return glp_;}

double gpt7702::raw_cum_glp       () const
{return cum_glp_;}

double gpt7702::raw_gsp           () const
{return gsp_;}

currency gpt7702::rounded_glp     () const
{return round_max_premium.c(glp_);}

currency gpt7702::rounded_cum_glp () const
{return round_max_premium.c(cum_glp_);}

currency gpt7702::rounded_gsp     () const
{return round_max_premium.c(gsp_);}

currency gpt7702::cum_f1A         () const
{return cum_f1A_;}

/// Process an adjustment event.
///
/// A = guideline premium before change
/// B = guideline premium at attained age for new f3_bft and new dbo
/// C = guideline premium at attained age for old f3_bft and old dbo
/// New guideline premium = A + B - C
///
/// As '7702.html' explains, the endowment benefit
///   "is reset to the new SA upon each adjustment event, but only
///   with respect to the seven-pay premium and the quantity B in
///   in the A + B - C formula (¶5/4); the quantities A and C use
///   the SA immediately prior to the adjustment event."
/// Because gpt_scalar_parms::endt_bft specifies the endowment
/// benefit, it is not necessary to know the specified amount here.
///
/// Similarly, because gpt_scalar_parms::f3_bft specifies the
/// 7702(f)(3) 'death benefit', the client can choose whether that
/// means death benefit (recommended) or specified amount--that choice
/// is not made here.

void gpt7702::adjust_guidelines(gpt_scalar_parms const& arg_parms)
{
    // There can be no adjustment event on the issue date.
    LMI_ASSERT(!is_issued_today());

    // 7702 !! Off-anniversary adjustments are not yet supported.
    LMI_ASSERT(0.0 == fractional_duration_);

    gpt_scalar_parms const b_parms = arg_parms;

    gpt_scalar_parms c_parms = s_parms_;
    c_parms.duration     = b_parms.duration;

    s_parms_ = arg_parms;

    double const glp_a = glp_;
    double const gsp_a = gsp_;
    double const glp_b = cf_.calculate_premium(oe_glp, b_parms);
    double const gsp_b = cf_.calculate_premium(oe_gsp, b_parms);
    double const glp_c = cf_.calculate_premium(oe_glp, c_parms);
    double const gsp_c = cf_.calculate_premium(oe_gsp, c_parms);

    glp_ = glp_a + glp_b - glp_c;
    gsp_ = gsp_a + gsp_b - gsp_c;
}

/// Update cumulative guideline level premium on anniversary.
///
/// This implementation is correct for an illustration system that
/// restricts all changes that might constitute adjustment events to
/// policy anniversaries only. For an admin system, the effect of
/// adjustment events would be prorated.
///
/// The accumulation of GLP here is the reason why guideline-premium
/// data members are of type double rather than currency. If, say,
/// GLP is $50.00999, then after twenty years the sum is $1000.19
/// after rounding, as opposed to only $1000.00 if GLP were rounded.
/// Both the benefit and the cost may seem immaterial, but there are
/// two strong reasons for preferring the more precise calculation:
///  - This reference implementation may be used to validate another
///    system; the GPT is a bright-line test, and it would be wrong to
///    deem the other system incorrect just because it is precise.
///  - Retaining all available precision likewise facilitates testing
///    this code against manual spreadsheet calculations--agreement to
///    ten significant digits, say, is a more powerful witness to
///    accuracy than agreement to four.

void gpt7702::increment_boy()
{
    cum_glp_ += glp_;
}

/// Dequeue a 1035 exchange.
///
/// Add the exchanged amount to cumulative premiums paid.
///
/// Asserted preconditions:
///  - The exchange occurs as of the issue date.
///  - Cumulative premiums paid equals zero.
///  - The exchange amount is nonnegative.
///  - The exchange amount does not exceed the guideline limit.

void gpt7702::dequeue_exch_1035()
{
    LMI_ASSERT(is_issued_today());
    LMI_ASSERT(0  == s_parms_.duration);
    LMI_ASSERT(C0 == cum_f1A_);
    LMI_ASSERT(C0 <= queued_exch_1035_amt_);
    LMI_ASSERT(queued_exch_1035_amt_ <= guideline_limit());
    cum_f1A_ += queued_exch_1035_amt_;
    queued_exch_1035_ = false;
    queued_exch_1035_amt_ = C0;
}

/// Dequeue a decrease in premiums paid.
///
/// Subtract the decrement from cumulative premiums paid.
///
/// Asserted preconditions:
///  - The decrease doesn't occur on the issue date.
///  - The decrement is positive.

void gpt7702::dequeue_f1A_decrease()
{
    LMI_ASSERT(!is_issued_today());
    LMI_ASSERT(C0 < queued_f1A_decrement_);
    cum_f1A_ -= queued_f1A_decrement_;
    queued_f1A_decrease_ = false;
    queued_f1A_decrement_ = C0;
}

/// Dequeue a potential adjustment event.
///
/// Delegate the real work to adjust_guidelines().

void gpt7702::dequeue_adj_event(gpt_scalar_parms const& arg_parms)
{
    LMI_ASSERT(!is_issued_today());
    adjust_guidelines(arg_parms);
    queued_adj_event_ = false;
}

/// Force money out to the extent necessary and possible.
///
/// If 7702(f)(1)(A) 'premiums paid' exceeds the guideline limit, any
/// excess is forced out of the contract. If that excess is greater
/// than the 7702(f)(2)(A) value (akin, but not identical, to account
/// value), then the entire 7702(f)(2)(A) value is forced out.
///
/// (In that case, 7702(f)(6) does offer the option of maintaining the
/// policy in force with a strictly zero 7702(f)(2)(A) value by making
/// bare minimum payments in excess of the guideline limit. If that
/// option is elected, it is assumed that the client illustration or
/// admin system enforces the 7702(f)(6) limit. If it is not possible
/// to force out the entire 7702(f)(2)(A) value (e.g., because it
/// includes a refundable sales load or an experience-rating reserve
/// that is available only on full surrender), then the policy lapses.
///
/// 7702(f)(1)(A) prescribes that 'premiums paid' is decreased only by
/// the amount "with respect to which there is a distribution": i.e.,
/// limited to the available 7702(f)(2)(A) value, which is an argument
/// to this function.
///
/// The amount forced out is stored in a private data member in order
/// to complete the tableau, which provides a summary of a set of
/// transactions for testing and debugging. That member deliberately
/// has no accessor; clients must use this function's return value
/// only. That member is zeroed upon entry to this function.

currency gpt7702::force_out(currency f2A_value)
{
    forceout_amount_ = C0;

    if(cum_f1A_ <= guideline_limit())
        {return C0;}

    forceout_amount_ = std::min(f2A_value, cum_f1A_ - guideline_limit());
    cum_f1A_ -= forceout_amount_;
    return forceout_amount_;
}

bool gpt7702::is_issued_today() const
{
    return 0 == s_parms_.duration && 0.0 == fractional_duration_;
}

currency gpt7702::guideline_limit() const
{
    return round_max_premium.c(std::max(cum_glp_, gsp_)); // r_downward
}
