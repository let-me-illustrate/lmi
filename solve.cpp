// Solves.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
#include "death_benefits.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "mc_enum_types_aux.hpp"        // set_run_basis_from_cloven_bases()
#include "outlay.hpp"
#include "round_to.hpp"
#include "zero.hpp"

#include <algorithm>                    // max(), min()

/*
IHS !! These issues have been addressed in lmi, but not here:

 - Restrict targetyear to Length.

 - Some solves happen other than in the first year; need input and calculations.

 - Solve basis: curr, guar.

 - no-lapse period.
*/

namespace
{
    AccountValue*       That;
    // IHS !! Use a struct for these?
    double              ThatSolveTargetValue;
    mcenum_solve_target ThatSolveTarget;
    int                 ThatSolveTgtYear;
    int                 ThatSolveBegYear;
    int                 ThatSolveEndYear;
    mcenum_gen_basis    ThatSolveBasis;
    bool                only_set_values;

    round_to<double> const round_to_cents(2, r_to_nearest);
} // Unnamed namespace.

//============================================================================
// This function isn't static and isn't in an unnamed namespace because
// that would make it difficult to grant it friendship.
currency SolveTest()
{
    // Separate-account basis hardcoded because separate account not supported.
    mcenum_run_basis temp;
    set_run_basis_from_cloven_bases
        (temp
        ,ThatSolveBasis
        ,mce_sep_full
        );
    That->RunOneCell(temp);
    // TRICKY !! This const reference is required for overload
    // resolution to choose the const versions of various functions.
    AccountValue const* ConstThat = const_cast<AccountValue const*>(That);

    // Return least of
    //   CSV at target duration
    //   lowest negative CSV through target duration
    //   amount of loan in excess of maximum loan through target duration
    currency Negative = C0;

    // IHS !! Start counting only at end of no-lapse period--lmi does that already.
    for(int j = 0; j < ThatSolveTgtYear; ++j)
        {
        Negative = std::min
            (Negative
            // CURRENCY !! Cents in ledger will make rounding unnecessary.
            ,round_to_cents.c(ConstThat->VariantValues().CSVNet[j])
// Ideally, it'd be this:
//          ,std::min(ConstThat->VariantValues().CSVNet[j], ConstThat->loan_ullage_[j])
// but the antediluvian branch doesn't calculate ullage at all.
            );
        }

    // CURRENCY !! Cents in ledger will make rounding unnecessary.
    currency z = round_to_cents.c(ConstThat->VariantValues().CSVNet[ThatSolveTgtYear - 1]);
    if(Negative < C0)
        z = std::min(z, Negative);
    // IHS !! If SolveTgtYr within no-lapse period...see lmi.

    currency y = C0;
    switch(ThatSolveTarget)
        {
        case mce_solve_for_endt:
            {
            // We take endowment to mean for spec amt, so it's the
            // same for options A and B.
            switch(ConstThat->DeathBfts_->dbopt()[ThatSolveTgtYear - 1])
                {
                case mce_option1:
                    {
                    y = ConstThat->base_specamt(ThatSolveTgtYear - 1);
                    }
                    break;
                case mce_option2:
                    {
                    y = ConstThat->base_specamt(ThatSolveTgtYear - 1);
                    }
                    break;
                case mce_rop: // fall through
                case mce_mdb: // fall through
                default:
                    {
                    alarum()
                        << "Case "
                        << ThatSolveTarget
                        << " not found."
                        << LMI_FLUSH
                        ;
                    }
                }
            }
            break;
        case mce_solve_for_target_csv:
            {
            y = round_to_cents.c(ThatSolveTargetValue);
            }
            break;
        case mce_solve_for_target_naar: // Fall through.
        case mce_solve_for_tax_basis:   // Fall through.
        case mce_solve_for_non_mec:
            {
            alarum() << "Not implemented.";
            }
            break;
        throw "Unreachable--silences a compiler diagnostic.";
        }

    return z - y;
}

//============================================================================
inline static double SolveSpecAmt(double CandidateValue)
{
// IHS !! Change surrchg when SA changes?
    That->SolveSetSpecAmt(round_to_cents.c(CandidateValue), ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : dblize(SolveTest());
}

//============================================================================
inline static double SolvePrem(double CandidateValue)
{
    That->SolveSetPmts(round_to_cents.c(CandidateValue), ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : dblize(SolveTest());
}

//============================================================================
inline static double SolveLoan(double CandidateValue)
{
    That->SolveSetLoans(round_to_cents.c(CandidateValue), ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : dblize(SolveTest());
}

//============================================================================
inline static double SolveWD(double CandidateValue)
{
    That->SolveSetWDs(round_to_cents.c(CandidateValue), ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : dblize(SolveTest());
}

//============================================================================
void AccountValue::SolveSetPmts
    (currency a_Pmt
    ,int      ThatSolveBegYear
    ,int      ThatSolveEndYear
    )
{
    Outlay_->set_ee_modal_premiums(a_Pmt, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetSpecAmt
    (currency a_Bft
    ,int      ThatSolveBegYear
    ,int      ThatSolveEndYear
    )
{
    DeathBfts_->set_specamt(a_Bft, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetLoans
    (currency a_Loan
    ,int      ThatSolveBegYear
    ,int      ThatSolveEndYear
    )
{
    Outlay_->set_new_cash_loans(a_Loan, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetWDs
    (currency a_WD
    ,int      ThatSolveBegYear
    ,int      ThatSolveEndYear
    )
{
    Outlay_->set_withdrawals(a_WD, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetLoanThenWD
    (currency // Amt
    ,int      // ThatSolveBegYear
    ,int      // ThatSolveEndYear
    )
{
    // IHS !! Implemented in lmi.
}

//============================================================================
currency AccountValue::Solve()
{
    That = this;
    ThatSolveTargetValue = yare_input_.SolveTargetValue;
    ThatSolveTarget      = yare_input_.SolveTarget;
    ThatSolveBasis       = yare_input_.SolveExpenseGeneralAccountBasis;
    only_set_values = !Solving;

    // We mustn't solve for a target at a duration beyond the end.
    ThatSolveTgtYear = yare_input_.SolveTargetYear;
    ThatSolveTgtYear = std::min(ThatSolveTgtYear, BasicValues::GetLength());
    // ... or before the beginning
    ThatSolveTgtYear = std::max(ThatSolveTgtYear, 1);
    // We should do the same for these I guess...IHS !! but here?
    ThatSolveBegYear = yare_input_.SolveBeginYear;
    ThatSolveEndYear = yare_input_.SolveEndYear;

    if(mce_solve_for_endt == ThatSolveTarget)
        {
        // We take endowment to mean at normal maturity.
        ThatSolveTgtYear = BasicValues::GetLength();
        ThatSolveEndYear = BasicValues::GetLength();
        }

    double(*SolveFn)(double)     = nullptr;
    double           LowerBound  = 0.0;
    double           UpperBound  = 0.0;
    root_bias        Bias        = bias_higher;
    int              Decimals    = 0;

    switch(yare_input_.SolveType)
        {
        case mce_solve_specamt:
            {
            // We aren't interested in negative specified amounts.
            LowerBound = 0.0;
            // IHS !! Not satisfactory; lmi tries a bit harder.
            UpperBound = 1000000.0 * Outlay_->ee_modal_premiums()[0];
            Decimals   = 0;
            SolveFn    = SolveSpecAmt;
            // IHS !! Respect minimum premium?
            }
            break;
        case mce_solve_ee_prem:
            {
            // We aren't interested in negative premiums.
            LowerBound = 0.0;
            // If solved premium exceeds specified amount, there's a problem.
            // IHS !! Better to use the maximum SA, not the first SA?
            UpperBound = dblize(DeathBfts_->specamt()[0]);
            Decimals   = 2;
            SolveFn    = SolvePrem;
            }
            break;
        case mce_solve_loan:
            {
            // We aren't interested in negative loans.
            LowerBound = 0.0;
            // IHS !! Not satisfactory.
            UpperBound = 1000000.0 * Outlay_->ee_modal_premiums()[0];
            Decimals   = 2;
            SolveFn    = SolveLoan;
            }
            break;
        case mce_solve_wd:
            {
            // We aren't interested in negative withdrawals.
            LowerBound = 0.0;
            // IHS !! Not satisfactory.
            UpperBound = 1000000.0 * Outlay_->ee_modal_premiums()[0];
            Decimals   = 2;
            SolveFn    = SolveWD;
            }
            break;
        case mce_solve_none:    // fall through
        case mce_solve_er_prem: // fall through
        default:
            {
            alarum()
                << "Case "
                << yare_input_.SolveType
                << " not found."
                << LMI_FLUSH
                ;
            }
        }

    root_type const solution = decimal_root
        (SolveFn
        ,LowerBound
        ,UpperBound
        ,Bias
        ,Decimals
        ,status()
        );
    currency const solution_cents = round_to_cents.c(solution.root);

    Solving = false;

    // For the final run, we just want the side effect of setting
    // solved-for values from the final iterand; we don't want to
    // generate or analyze account values. This global variable is a
    // kludge, but so is 'That'; a function object is wanted instead.
    only_set_values = !Solving;

    // The account and ledger values set as a side effect of solving
    // aren't generally the same as those shown on the illustration
    // because the 'Solving' flag has side effects. Therefore, the
    // final solve parameters are stored by calling 'SolveFn'
    // now, and actual values are freshly generated downstream.
    SolveFn(dblize(solution_cents));

    switch(solution.validity)
        {
        case root_is_valid:
            {} // Do nothing.
            break;
        case root_not_bracketed:
            {
            LMI_ASSERT(C0 == solution_cents);
            warning() << "solution not found. Using zero instead." << LMI_FLUSH;
            }
            break;
        case improper_bounds:
            {
            alarum() << "Improper bounds." << LMI_FLUSH;
            }
            break;
        }
    return solution_cents;
}
