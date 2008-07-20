// Solves.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: solve.cpp,v 1.14 2008-07-20 02:41:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include "alert.hpp"
#include "death_benefits.hpp"
#include "inputs.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "mc_enum_types_aux.hpp"
#include "outlay.hpp"
#include "zero.hpp"

#include <algorithm>     // std::min(), std::max()

/*
TODO ?? Restrict targetyear to Length.

TODO ?? Some solves happen other than in the first year; need input
and calculations.

TODO ?? Multiple solves may be incompatible.

TODO ?? Solve basis: curr, guar.

TODO ?? no lapse period.
*/

namespace
{
    AccountValue*       That;
    // TODO ?? Use a struct for these?
    double              ThatSolveTargetCSV;
    mcenum_solve_target ThatSolveTarget;
    int                 ThatSolveTgtYear;
    int                 ThatSolveBegYear;
    int                 ThatSolveEndYear;
    mcenum_gen_basis    ThatSolveBasis;
    bool                only_set_values;
}

//============================================================================
// This function isn't static and isn't in an unnamed namespace because
// that would make it difficult to grant it friendship.
double SolveTest()
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
    double Negative = 0.0;

    // TODO ?? Start counting only at end of no-lapse period?
    for(int j = 0; j < ThatSolveTgtYear; j++)
        {
        Negative = std::min
            (Negative
            ,std::min(ConstThat->VariantValues().CSVNet[j], ConstThat->VariantValues().ExcessLoan[j])
            );
        }

    double z = ConstThat->VariantValues().CSVNet[ThatSolveTgtYear - 1];
    if(Negative < 0.0)
        z = std::min(z, Negative);
    // TODO ?? If SolveTgtYr within no-lapse period...

    double y = 0.0;
    switch(ThatSolveTarget)
        {
        case mce_solve_for_endt:
            {
            // We take endowment to mean for spec amt, so it's the
            // same for options A and B.
            switch(ConstThat->InvariantValues().DBOpt[ThatSolveTgtYear - 1].value())
                {
                case mce_option1:
                    {
                    y = ConstThat->InvariantValues().SpecAmt[ThatSolveTgtYear - 1];
                    }
                    break;
                case mce_option2:
                    {
                    y = ConstThat->InvariantValues().SpecAmt[ThatSolveTgtYear - 1];
                    }
                    break;
                case mce_rop:
                    {
                    // TODO ?? What should be done here?
                    }
// TODO ?? fall through...    break;
                default:
                    {
                    fatal_error()
                        << "Case "
                        << ThatSolveTarget
                        << " not found."
                        << LMI_FLUSH
                        ;
                    }
                }
            }
            break;
        case mce_solve_for_target:
            {
            y = ThatSolveTargetCSV;
            }
            break;
        throw "Unreachable--silences a compiler diagnostic.";
        }

    return z - y;
}

//============================================================================
inline static double SolveSpecAmt(double CandidateValue)
{
// TODO ?? Change surrchg when SA changes?
    That->SolveSetSpecAmt(CandidateValue, ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : SolveTest();
}

//============================================================================
inline static double SolvePrem(double CandidateValue)
{
    That->SolveSetPmts(CandidateValue, ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : SolveTest();
}

//============================================================================
inline static double SolveLoan(double CandidateValue)
{
    That->SolveSetLoans(CandidateValue, ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : SolveTest();
}

//============================================================================
inline static double SolveWD(double CandidateValue)
{
    That->SolveSetWDs(CandidateValue, ThatSolveBegYear, ThatSolveEndYear);
    return only_set_values ? 0.0 : SolveTest();
}

//============================================================================
// TODO ?? Never used.
inline static double SolveWDThenLoan(double /* CandidateValue */)
{
    return 0.0;
}

//============================================================================
void AccountValue::SolveSetPmts
    (double a_Pmt
    ,int    ThatSolveBegYear
    ,int    ThatSolveEndYear
    )
{
    Outlay_->set_ee_modal_premiums(a_Pmt, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetSpecAmt
    (double a_Bft
    ,int    ThatSolveBegYear
    ,int    ThatSolveEndYear
    )
{
    DeathBfts_->set_specamt(a_Bft, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetLoans
    (double a_Loan
    ,int    ThatSolveBegYear
    ,int    ThatSolveEndYear
    )
{
    Outlay_->set_new_cash_loans(a_Loan, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetWDs
    (double a_WD
    ,int    ThatSolveBegYear
    ,int    ThatSolveEndYear
    )
{
    Outlay_->set_withdrawals(a_WD, ThatSolveBegYear, ThatSolveEndYear);
}

//============================================================================
void AccountValue::SolveSetLoanThenWD
    (double // Amt
    ,int    // ThatSolveBegYear
    ,int    // ThatSolveEndYear
    )
{
    // TODO ?? Needs an implementation.
}

//============================================================================
double AccountValue::Solve()
{
    That = this;
    ThatSolveTargetCSV  = Input_->SolveTgtCSV.value();
    ThatSolveTarget     = yare_input_.SolveTarget;
    ThatSolveBasis      = yare_input_.SolveBasis;
    only_set_values = !Solving;

    // We mustn't solve for a target at a duration beyond the end.
    ThatSolveTgtYear = Input_->SolveTgtYear.value();
    ThatSolveTgtYear = std::min(ThatSolveTgtYear, BasicValues::GetLength());
    // ... or before the beginning
    ThatSolveTgtYear = std::max(ThatSolveTgtYear, 1);
    // We should do the same for these I guess...TODO ?? but here?
    ThatSolveBegYear = Input_->SolveBegYear.value();
    ThatSolveEndYear = Input_->SolveEndYear.value();

    if(mce_solve_for_endt == ThatSolveTarget)
        {
        // We take endowment to mean at normal maturity.
        ThatSolveTgtYear = BasicValues::GetLength();
        ThatSolveEndYear = BasicValues::GetLength();
        }

    double(*SolveFn)(double)     = 0;
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
            // TODO ?? Not satisfactory.
            UpperBound = 1000000.0 * Outlay_->ee_modal_premiums()[0];
            Decimals   = 0;
            SolveFn    = SolveSpecAmt;
            // TODO ?? Respect minimum premium?
            }
            break;
        case mce_solve_ee_prem:
            {
            // We aren't interested in negative premiums.
            LowerBound = 0.0;
            // If solved premium exceeds specified amount, there's a problem.
            // TODO ?? Better to use the maximum SA, not the first SA?
            UpperBound = DeathBfts_->specamt()[0];
            Decimals   = 2;
            SolveFn    = SolvePrem;
            }
            break;
        case mce_solve_loan:
            {
            // We aren't interested in negative loans.
            LowerBound = 0.0;
            // TODO ?? Not satisfactory.
            UpperBound = 1000000.0 * Outlay_->ee_modal_premiums()[0];
            Decimals   = 2;
            SolveFn    = SolveLoan;
            }
            break;
        case mce_solve_wd:
            {
            // We aren't interested in negative withdrawals.
            LowerBound = 0.0;
            // TODO ?? Not satisfactory.
            UpperBound = 1000000.0 * Outlay_->ee_modal_premiums()[0];
            Decimals   = 2;
            SolveFn    = SolveWD;
            }
            break;
        default:
            {
            fatal_error()
                << "Case "
                << yare_input_.SolveType
                << " not found."
                << LMI_FLUSH
                ;
            }
        }

    root_type Solution = decimal_root
        (LowerBound
        ,UpperBound
        ,Bias
        ,Decimals
        ,SolveFn
        );
    if(root_not_bracketed == Solution.second)
        {
        Solution.first = 0.0;
        warning() << "Solution not found. Using zero instead." << LMI_FLUSH;
        }

    // The account and ledger values set as a side effect of solving
    // aren't necessarily what we need, for two reasons:
    //   - find_root() need not return the last iterand tested; and
    //   - the 'Solving' flag has side effects.
    // The first issue could be overcome easily enough in find_root(),
    // but the second cannot. Therefore, the final solve parameters
    // are stored now, and values are regenerated downstream.

    Solving = false;

    // For the final run, we just want the side effect of setting
    // solved-for values from the final iterand; we don't want to
    // generate or analyze account values. This global variable is a
    // kludge, but so is 'That'; a function object is wanted instead.
    only_set_values = !Solving;
    double actual_solution = Solution.first;

    SolveFn(actual_solution);
    return actual_solution;
}

