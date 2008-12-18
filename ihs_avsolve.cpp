// Solves.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ihs_avsolve.cpp,v 1.36 2008-12-18 14:26:13 chicares Exp $

// All iterative illustration solves are performed in this file.
// We use Brent's algorithm because it is guaranteed to converge
// if a solution exists between the given bounds, and because of
// its favorable performance: generally, its best-case performance
// is as good as any algorithm's, and its worst-case performance
// is better than any algorithm's except Bolzano's.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "death_benefits.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "mc_enum_types_aux.hpp" // set_run_basis_from_cloven_bases()
#include "outlay.hpp"
#include "zero.hpp"

#include <algorithm>             // std::min(), std::max()
#include <functional>
#include <numeric>               // std::accumulate()

namespace
{
    // TODO ?? Shouldn't this be a typedef for a SolveHelper member?
    // As it stands, this would seem not to be reentrant.
    void (AccountValue::*solve_set_fn)(double);
}

class SolveHelper
{
    AccountValue& av;
  public:
    SolveHelper(AccountValue& a_av)
        :av(a_av)
        {
        }
    double operator()(double a_CandidateValue)
        {
        return av.SolveTest(a_CandidateValue);
        }
};

//============================================================================
// Test results of a trial value for a given input item
//
// Naively we would calculate the cash value for a given input and return
// the difference between it and the target cash value at the specified
// target duration. However, if the policy lapsed before that duration,
// that approach would return zero. That's not the best way, because it
// provides little information that the solve routine can use to refine
// the input value. Therefore, we take certain steps to make the
// objective function more well-behaved as its value approaches zero
// from either direction:
//
// 1. we prevent the policy from lapsing during a solve, by setting
// AccountValue::Solving
//
// 2. we ascertain the lowest negative cash value over the solve period,
// if any, but disregarding any duration at which a no-lapse guarantee
// is in effect
//
// 3. we ascertain the lowest (negative) loan deficit (maximum loan minus
// actual loan balance) over the solve period, if any, ignoring any
// no-lapse guarantee
//
// 4. if either 2. or 3. is negative, we return the difference between
// the target value and whichever of them is more negative; else we
// return the difference between the target value and the cash value at
// the solve target duration
//
// We also note here (although the supporting code is elsewhere) that
// when we solve for illustration-reg guaranteed premium for a GPT
// contract, we suspend guideline premium limitations. Thus, we realize
// the intention of state law by overlooking the very real possibility
// of a conflict with federal law. See NAIC Q&A 7.9:
//   http://www.naic.org/committe/modelaws/liiq&a.htm
//   "Section 7B(2) does not preclude the illustrating of premiums
//   that exceed the guideline premiums in Section 7702 of the IRC."
//
double AccountValue::SolveTest(double a_CandidateValue)
{
    (this->*solve_set_fn)(a_CandidateValue);

    mcenum_run_basis z;
    set_run_basis_from_cloven_bases
        (z
        ,SolveGenBasis_
        ,SolveSepBasis_
        );
    RunOneCell(z);

    // return least of
    //   CSV at target duration
    //   lowest negative CSV through target duration
    //   amount of loan in excess of maximum loan through target duration
    // Use UnderlyingCSV so that sales load refund doesn't prevent lapse.

    // Start only after no-lapse period, if any.
    int no_lapse_dur = std::accumulate
        (YearlyNoLapseActive.begin()
        ,YearlyNoLapseActive.end()
        ,0
        );
    double most_negative_csv = 0.0;
    if(no_lapse_dur < SolveTargetDuration_)
        {
        most_negative_csv = *std::min_element
            (VariantValues().CSVNet.begin() + no_lapse_dur
            ,VariantValues().CSVNet.begin() + SolveTargetDuration_
            );
        }

    // Assume any no-lapse provision is inoperative when overloaned.
    // TODO ?? ExcessLoan is negative; is LoanDeficit a better name?
    LMI_ASSERT(0 < SolveTargetDuration_);
    double most_negative_loan_deficit = *std::min_element
        (VariantValues().ExcessLoan.begin()
        ,VariantValues().ExcessLoan.begin() + SolveTargetDuration_
        );

    double worst_negative = std::min
        (most_negative_csv
        ,most_negative_loan_deficit
        );
    // SolveTargetDuration_ is in origin one. That's natural for loop
    // counters and iterators--it's one past the end--but indexing
    // must decrement it.
    double value = VariantValues().CSVNet[SolveTargetDuration_ - 1];
    if(worst_negative < 0.0)
        {
        value = std::min(value, worst_negative);
        }

    if(mce_solve_for_endt == SolveTarget_)
        {
        // "Solve for endowment" is deemed to mean cash surrender
        // value equals specified amount at target duration, so the
        // target value is the same for all death benefit options.
        //
        // The input specified amount mustn't be used here because
        // it wouldn't reflect dynamic adjustments.
        SolveTargetCsv_ = InvariantValues().SpecAmt[SolveTargetDuration_ - 1];
        }

    if(mce_solve_for_tax_basis == SolveTarget_)
        {
        SolveTargetCsv_ = std::accumulate
            (InvariantValues().GrossPmt.begin()
            ,InvariantValues().GrossPmt.begin() + SolveTargetDuration_
            ,0.0
            );
        }

    return value - SolveTargetCsv_;
}

//============================================================================
void AccountValue::SolveSetSpecAmt(double a_CandidateValue)
{
// TODO ?? Does this change the surrchg when specamt changes?
    DeathBfts_->set_specamt
        (a_CandidateValue
        ,SolveBeginYear_
        ,SolveEndYear_
        );
}

//============================================================================
void AccountValue::SolveSetEePrem(double a_CandidateValue)
{
    Outlay_->set_ee_modal_premiums(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

//============================================================================
void AccountValue::SolveSetErPrem(double a_CandidateValue)
{
    Outlay_->set_er_modal_premiums(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

//============================================================================
void AccountValue::SolveSetLoan(double a_CandidateValue)
{
    Outlay_->set_new_cash_loans(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

//============================================================================
void AccountValue::SolveSetWD(double a_CandidateValue)
{
    Outlay_->set_withdrawals(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

//============================================================================
void AccountValue::SolveSetWDThenLoan(double a_CandidateValue)
{
    LMI_ASSERT(yare_input_.WithdrawToBasisThenLoan);
    Outlay_->set_withdrawals(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

//============================================================================
double AccountValue::SolveGuarPremium()
{
    // Store original er premiums for later restoration.
    std::vector<double> stored = Outlay_->er_modal_premiums();
    // Zero out er premiums and solve for ee premiums only.
    Outlay_->set_er_modal_premiums
        (0.0
        ,0
        ,static_cast<int>(InvariantValues().EndtAge - InvariantValues().Age)
        );

    bool temp_solving     = Solving;
    Solving               = true;
    SolvingForGuarPremium = true;

    // Run the solve using guaranteed assumptions.
    double guar_premium = Solve
        (mce_solve_ee_prem
        ,0
        ,static_cast<int>(InvariantValues().EndtAge - InvariantValues().Age)
        ,mce_solve_for_endt
        ,0.0
        ,static_cast<int>(InvariantValues().EndtAge - InvariantValues().Age)
        ,mce_gen_guar
        ,mce_sep_full
        );

    // Restore original values.
    Outlay_->set_er_modal_premiums(stored);
    Solving               = temp_solving;
    SolvingForGuarPremium = false;

    return guar_premium;
}

//============================================================================
double AccountValue::Solve
    (mcenum_solve_type   a_SolveType
    ,int                 a_SolveBeginYear
    ,int                 a_SolveEndYear
    ,mcenum_solve_target a_SolveTarget
    ,double              a_SolveTargetCsv
    ,int                 a_SolveTargetYear
    ,mcenum_gen_basis    a_SolveGenBasis
    ,mcenum_sep_basis    a_SolveSepBasis
    )
{
    SolveBeginYear_      = a_SolveBeginYear;
    SolveEndYear_        = a_SolveEndYear;
    SolveTarget_         = a_SolveTarget;
    SolveTargetCsv_      = a_SolveTargetCsv;
    SolveTargetDuration_ = a_SolveTargetYear;
    SolveGenBasis_       = a_SolveGenBasis;
    SolveSepBasis_       = a_SolveSepBasis;

    LMI_ASSERT(0 <= SolveBeginYear_);
    LMI_ASSERT(     SolveBeginYear_ <= SolveEndYear_);
    LMI_ASSERT(                        SolveEndYear_ <= BasicValues::GetLength());

    LMI_ASSERT(0 < SolveTargetDuration_);
    LMI_ASSERT(    SolveTargetDuration_ <= BasicValues::GetLength());

    // Defaults: may be overridden by some cases
    // We aren't interested in negative solve results
    double           lower_bound = 0.0;
    double           upper_bound = 0.0;
    root_bias        bias        = bias_higher;
    int              decimals    = 0;

    // Many things don't plausibly exceed max input face
    for(int j = 0; j < SolveTargetDuration_; j++)
        {
        upper_bound = std::max
            (upper_bound
// TODO ?? This is probably wrong. I think there are two term amounts, only
// one of which is in Status. But I could be wrong.
            ,DeathBfts_->specamt()[j] + yare_input_.TermRiderAmount
            );
        }
    // TODO ?? Wait--initial premium may exceed input face, so
    // for now we'll bail out with this:
    upper_bound = 999999999.99;

    switch(a_SolveType)
        {
        case mce_solve_specamt:
            {
// This:
//          upper_bound  = 1000000.0 * Outlay_->GetPmts()[0];
// is not satisfactory; what would be better?
            solve_set_fn = &AccountValue::SolveSetSpecAmt;
            decimals     = round_specamt.decimals();
            // TODO ?? Respect minimum specamt?
            }
            break;
        case mce_solve_ee_prem:
            {
            solve_set_fn = &AccountValue::SolveSetEePrem;
            decimals     = round_gross_premium.decimals();
            }
            break;
        case mce_solve_er_prem:
            {
            solve_set_fn = &AccountValue::SolveSetErPrem;
            decimals     = round_gross_premium.decimals();
            }
            break;
        case mce_solve_loan:
            {
            solve_set_fn = &AccountValue::SolveSetLoan;
            decimals     = round_loan.decimals();
            }
            break;
        case mce_solve_wd:
            {
            solve_set_fn = &AccountValue::SolveSetWD;
            decimals     = round_withdrawal.decimals();
            // TODO ?? Is minimum wd respected?
            }
            break;
        case mce_solve_wd_then_loan:
            {
            solve_set_fn = &AccountValue::SolveSetWDThenLoan;
            // Withdrawals and loans might be rounded differently.
            // To obtain a level income as a mixture of loans and
            // withdrawals, both should be rounded to the less precise
            // number of decimals normally used for either.
            decimals = std::min
                (round_withdrawal.decimals()
                ,round_loan.decimals()
                );
            }
            break;
        case mce_solve_ee_prem_dur: // Fall through: not yet implemented.
        case mce_solve_er_prem_dur: // Fall through: not yet implemented.
        default:
            {
            fatal_error()
                << "Case "
                << a_SolveType
                << " not found."
                << LMI_FLUSH
                ;
            }
        }

    SolveHelper solve_helper(*this);
    root_type solution = decimal_root
        (lower_bound
        ,upper_bound
        ,bias
        ,decimals
        ,solve_helper
        ,false
        ,status()
        );

    if(root_not_bracketed == solution.second)
        {
        solution.first = 0.0;
        // Don't want this firing continually in census runs.
        if(!SolvingForGuarPremium)
            {
            warning() << "Solution not found: using zero instead." << LMI_FLUSH;
            // TODO ?? What can we do when no solution exists for guar prem?
            }
        }

    // The account and ledger values set as a side effect of solving
    // aren't necessarily what we need, for two reasons:
    //   - find_root() need not return the last iterand tested; and
    //   - the 'Solving' flag has side effects.
    // The first issue could be overcome easily enough in find_root(),
    // but the second cannot. Therefore, the final solve parameters
    // are stored now, and values are regenerated downstream.

    Solving = false;
    (this->*solve_set_fn)(solution.first);
    return solution.first;
}

