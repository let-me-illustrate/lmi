// Solves.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ihs_avsolve.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

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

#include "ihs_acctval.hpp"

#include "alert.hpp"
#include "ihs_deathbft.hpp"
#include "ihs_ldginvar.hpp"
#include "ihs_ldgvar.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "outlay.hpp"
#include "zero.hpp"

#include <algorithm>    // std::min(), std::max()
#include <functional>
#include <numeric>      // std::accumulate()

/*
?? Limitations

Restrict targetyear to Length

Multiple solves may be incompatible

no lapse period not yet reflected?

*/

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
void AccountValue::SolveSetTargetValueAndDuration
    (e_solve_target const&  a_SolveTarget
    ,double                 a_SolveTgtCSV
    ,int                    a_SolveTgtYear
    )
{
    // TODO ?? EffectiveSolveTargetYear is in origin one. OK for loop counters,
    // bad for indexing. Should we change it to origin zero?

    // TODO ?? EffectiveSolveTargetYear and SolveTargetValue are copies of
    //    SolveTgtYear and SolveTgtCSV -- however I see instances where
    //    one of these copies gets changed during the Solve routine.
// if solve for endt
//      EffectiveSolveTargetYear = BasicValues::GetLength();
// and always these:
//  std::max(EffectiveSolveTargetYear, BasicValues::GetLength())
//  std::max(EffectiveSolveTargetYear, 1)
    SolveTargetValue            = a_SolveTgtCSV;
    EffectiveSolveTargetYear    = a_SolveTgtYear;

    if(e_solve_for_endt == a_SolveTarget)
        {
        // We take endowment to mean for spec amt at normal maturity,
        // so the target value is the same for all DB options
        EffectiveSolveTargetYear = BasicValues::GetLength();
        // We can't just get spec amt from the original input, because
        // a solve for spec amt can change it dynamically.
        SolveTargetValue = InvariantValues().SpecAmt.back();
        }

    // TODO ?? This assertion seems odd because 0 is a valid input value on
    // the screen. Did it get changed to origin 1 in class InputParms?
    LMI_ASSERT
        (   EffectiveSolveTargetYear
        >=  1
        );
}

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

    e_run_basis temp;
    set_run_basis_from_separate_bases
        (temp
        ,e_basis(SolveBasis)
        ,e_sep_acct_basis(SolveSABasis)
        );
    PerformRun(temp);

    // return least of
    //   CSV at target duration
    //   lowest negative CSV through target duration
    //   amount of loan in excess of maximum loan through target duration
    // Use UnderlyingCSV so that sales load refund doesn't prevent lapse.

    // Start only after no-lapse period, if any.
    // TODO ?? Would it be better to use find() and distance()?
    int no_lapse_dur = std::accumulate
        (YearlyNoLapseActive.begin()
        ,YearlyNoLapseActive.end()
        ,0
        );
    double most_negative_csv = 0.0;
    std::vector<double> const& csv(VariantValues().CSVNet);

    // If [no-lapse dur, solve target dur) is a valid range, then use it
    if(no_lapse_dur < (EffectiveSolveTargetYear - 1))
        {
        std::vector<double>::const_iterator
            csv_begin = csv.begin();
        std::advance(csv_begin, no_lapse_dur);
        // Stop at EffectiveSolveTargetYear
        std::vector<double>::const_iterator
            csv_end = csv.begin();
        std::advance(csv_end, EffectiveSolveTargetYear - 1);

        // Paranoid check that solvetgtyr is within no-lapse period.
        std::vector<double>::difference_type dist = csv_end - csv_begin;
        LMI_ASSERT(0 < dist);

        most_negative_csv = *std::min_element
            (csv_begin
            ,csv_end
            );
        }

    // Start at year 0--assume no-lapse provision doesn't allow overloan.
    // TODO ?? Assert that.
    // TODO ?? If ExcessLoan is negative, then its name is confusing
    double most_negative_loan_deficit = 0.0;
    if(0 < (EffectiveSolveTargetYear - 1))  // TODO ?? "-1" ?
        {
        // Stop at EffectiveSolveTargetYear
        std::vector<double>::iterator
            excess_loan_end = VariantValues().ExcessLoan.begin();
        std::advance(excess_loan_end, EffectiveSolveTargetYear);
        most_negative_loan_deficit = *std::min_element
            (VariantValues().ExcessLoan.begin()
            ,excess_loan_end
        );
        }

    double worst_negative = std::min
        (most_negative_csv
        ,most_negative_loan_deficit
        );
    // Need to index it: back() doesn't work if solve target is not last dur
    double value = csv[EffectiveSolveTargetYear - 1];
    if(worst_negative < 0.0)
        {
        value = std::min(value, worst_negative);
        }
    return value - SolveTargetValue;
}

//============================================================================
void AccountValue::SolveSetSpecAmt(double a_CandidateValue)
{
// TODO ?? Does this change the surrchg when specamt changes?
    DeathBfts->SetSpecAmt
        (a_CandidateValue
        ,SolveBegYear
        ,SolveEndYear
        );
}

//============================================================================
void AccountValue::SolveSetEePrem(double a_CandidateValue)
{
    Outlay_->set_ee_modal_premiums(a_CandidateValue, SolveBegYear, SolveEndYear);
}

//============================================================================
void AccountValue::SolveSetErPrem(double a_CandidateValue)
{
    Outlay_->set_er_modal_premiums(a_CandidateValue, SolveBegYear, SolveEndYear);
}

//============================================================================
void AccountValue::SolveSetLoan(double a_CandidateValue)
{
    Outlay_->set_new_cash_loans(a_CandidateValue, SolveBegYear, SolveEndYear);
}

//============================================================================
void AccountValue::SolveSetWD(double a_CandidateValue)
{
    Outlay_->set_withdrawals(a_CandidateValue, SolveBegYear, SolveEndYear);
}

//============================================================================
void AccountValue::SolveSetWDThenLoan(double a_CandidateValue)
{
    Outlay_->set_withdrawals(a_CandidateValue, SolveBegYear, SolveEndYear);
    // TODO ?? EGREGIOUS_DEFECT Do this in the input class instead.
    const_cast<InputParms*>(Input)->WDToBasisThenLoan = "Yes";
}

//============================================================================
double AccountValue::SolveGuarPremium()
{
    // Make a copy of the current objects which will be used by Solve() .
    Outlay temp_prem(*BasicValues::Outlay_);
    // Zero out corp pmts and solve for ee pmts only.
    Outlay_->set_er_modal_premiums
        (0.0
        ,0
        ,static_cast<int>(LedgerInvariant->EndtAge - LedgerInvariant->Age)
        );

    bool temp_solving       = Solving;
    Solving                 = true;
    SolvingForGuarPremium   = true;

    // Run the solve using guaranteed assumptions.
    double guar_premium = Solve
        (e_solve_type(e_solve_ee_prem)
        ,0
        ,static_cast<int>(LedgerInvariant->EndtAge - LedgerInvariant->Age)
        ,e_solve_target(e_solve_for_endt)
        ,0
        ,static_cast<int>(LedgerInvariant->EndtAge - LedgerInvariant->Age)
        ,e_basis(e_guarbasis)
        ,e_sep_acct_basis(e_sep_acct_full)
        );

    // Restore original values of saved objects.
    *BasicValues::Outlay_   = temp_prem;
    Solving                 = temp_solving;
    SolvingForGuarPremium   = false;

    return guar_premium;
}

//============================================================================
// TODO ?? Can these member variables
//   EffectiveSolveTargetYear
//   SolveTargetValue
// be dispensed with?
double AccountValue::Solve
    (e_solve_type const&     a_SolveType
    ,int                     a_SolveBegYear
    ,int                     a_SolveEndYear
    ,e_solve_target const&   a_SolveTarget
    ,int                     a_SolveTgtCSV
    ,int                     a_SolveTgtYear
    ,e_basis const&          a_SolveBasis
    ,e_sep_acct_basis const& a_SolveSABasis
    )
{
// TODO ?? We left these as class members for now:
// SolveBegYear SolveEndYear SolveBasis SolveSABasis
    SolveBegYear    = a_SolveBegYear;
    SolveEndYear    = a_SolveEndYear;
    SolveBasis      = a_SolveBasis;
    SolveSABasis    = a_SolveSABasis;

    SolveSetTargetValueAndDuration
        (a_SolveTarget
        ,a_SolveTgtCSV
        ,a_SolveTgtYear
        );

//  LMI_ASSERT(0 <= SolveBegYear);
    LMI_ASSERT(SolveBegYear <= SolveEndYear);
    LMI_ASSERT(SolveEndYear <= BasicValues::GetLength());

    // Defaults: may be overridden by some cases
    // We aren't interested in negative solve results
    double           lower_bound = 0.0;
    double           upper_bound = 0.0;
    root_bias        bias        = bias_higher;
    int              decimals    = 0;

    // Many things don't plausibly exceed max input face
    for(int j = 0; j < EffectiveSolveTargetYear; j++)
        {
        upper_bound = std::max
            (upper_bound
// TODO ?? This is probably wrong. I think there are two term amounts, only
// one of which is in Status. But I could be wrong.
            ,DeathBfts->GetSpecAmt()[j] + Input->Status[0].TermAmt
            );
        }
    // TODO ?? Wait--initial premium may exceed input face, so
    // for now we'll bail out with this:
    upper_bound = 999999999.99;

    switch(a_SolveType)
        {
        case e_solve_specamt:
            {
// This:
//          upper_bound  = 1000000.0 * Outlay_->GetPmts()[0];
// is not satisfactory; what would be better?
            solve_set_fn = &AccountValue::SolveSetSpecAmt;
            decimals     = round_specamt.decimals();
            // TODO ?? Respect minimum specamt?
            }
            break;
        case e_solve_ee_prem:
            {
            solve_set_fn = &AccountValue::SolveSetEePrem;
            decimals     = round_gross_premium.decimals();
            }
            break;
        case e_solve_er_prem:
            {
            solve_set_fn = &AccountValue::SolveSetErPrem;
            decimals     = round_gross_premium.decimals();
            }
            break;
        case e_solve_loan:
            {
            solve_set_fn = &AccountValue::SolveSetLoan;
            decimals     = round_loan.decimals();
            }
            break;
        case e_solve_wd:
            {
            solve_set_fn = &AccountValue::SolveSetWD;
            decimals     = round_withdrawal.decimals();
            // TODO ?? Is minimum wd respected?
            }
            break;
        case e_solve_wd_then_loan:
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
        case e_solve_ee_prem_dur: // Fall through: not yet implemented.
        case e_solve_er_prem_dur: // Fall through: not yet implemented.
        default:
            {
            fatal_error()
                << "Case '"
                << a_SolveType
                << "' not found."
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

