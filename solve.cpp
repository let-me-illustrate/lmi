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
    double Negative = 0.0;

    // IHS !! Start counting only at end of no-lapse period--lmi does that already.
    for(int j = 0; j < ThatSolveTgtYear; ++j)
        {
        Negative = std::min
            (Negative
            ,ConstThat->VariantValues().CSVNet[j]
// Ideally, it'd be this:
//          ,std::min(ConstThat->VariantValues().CSVNet[j], ConstThat->loan_ullage_[j])
// but the antediluvian branch doesn't calculate ullage at all.
            );
        }

    double z = ConstThat->VariantValues().CSVNet[ThatSolveTgtYear - 1];
    if(Negative < 0.0)
        z = std::min(z, Negative);
    // IHS !! If SolveTgtYr within no-lapse period...see lmi.

    double y = 0.0;
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
            y = ThatSolveTargetValue;
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
            UpperBound = DeathBfts_->specamt()[0];
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

    root_type Solution = decimal_root
        (LowerBound
        ,UpperBound
        ,Bias
        ,Decimals
        ,SolveFn
        ,false
        ,status()
        );
    if(root_not_bracketed == Solution.second)
        {
        LMI_ASSERT(0.0 == Solution.first);
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
