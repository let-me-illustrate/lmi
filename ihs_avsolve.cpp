// Solves.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// All iterative illustration solves are performed in this file.
// We use Brent's algorithm because it is guaranteed to converge
// if a solution exists between the given bounds, and because of
// its favorable performance: generally, its best-case performance
// is as good as any algorithm's, and its worst-case performance
// is better than any algorithm's except Bolzano's.

#include "pchfile.hpp"

#include "account_value.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "death_benefits.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "mc_enum_types_aux.hpp"        // set_run_basis_from_cloven_bases()
#include "miscellany.hpp"               // ios_out_app_binary()
#include "outlay.hpp"
#include "zero.hpp"

#include <algorithm>                    // min(), max()
#include <functional>
#include <numeric>                      // accumulate()

/// Helper class to provide a free function for solves.
///
/// decimal_root() wants a free function; operator() provides that.
///
/// This free function must have a signature compatible with
///   double f(double x)
/// because both 'x' and 'f(x)' are used internally by decimal_root()
/// in interpolation formulas that require floating-point numbers.
///
/// Alternatives not pursued: Writing a decimal_root() variant in
/// terms of class currency is hopeless, because interpolation does
/// not yield integral values. Writing an "adapter" to transform
///   currency f(currency x)
/// to
///   double f(double x)
/// might be a good idea if there were several use cases, but today
/// there are not.

class SolveHelper
{
  public:
    SolveHelper
        (AccountValue& av
        ,void (AccountValue::*solve_set_fn)(currency)
        )
        :av_           {av}
        ,solve_set_fn_ {solve_set_fn}
        {}
    double operator()(double a_CandidateValue)
        {
        currency candidate = av_.round_minutiae().c(a_CandidateValue);
        return dblize(av_.SolveTest(candidate, solve_set_fn_));
        }

  private:
    AccountValue& av_;
    void (AccountValue::*solve_set_fn_)(currency);
};

/// Return outcome of a trial with a given input value.
///
/// Naively, one might run an illustration for a given input, and
/// return the difference between actual and target CSV at the
/// specified target duration. However, if the policy lapsed before
/// that duration, this naive approach would return zero. That's not
/// desirable: even if it leads to a valid answer, it provides little
/// information that the solve routine can use to refine the input
/// value. Instead, therefore, certain steps are taken to make the
/// objective function more tractable as its value approaches zero
/// from either direction:
///
/// 1. Prevent the policy from lapsing during a solve, by setting the
/// AccountValue::Solving flag.
///
/// 2. Ascertain the lowest negative CSV over the solve period, if
/// any, excluding any duration at which a no-lapse guarantee is in
/// effect.
///
/// 3. Ascertain the greatest ullage (any positive excess of requested
/// over maximum) throughout the solve period in
///  - loan, or
///  - withdrawal
/// but ignoring any no-lapse provision (which wouldn't override
/// internal limits on these amounts); and negate it for use as an
/// objective-function penalty quite like negative CSV.
///
/// 4. If either 2. or 3. is negative, return the difference between
/// whichever of them is more negative and the target value; else
/// return the difference between the target and actual values at the
/// solve target duration, the "actual" value being CSV in general,
/// but NAAR in the case of NAAR solves. (Non-MEC solves (v.i.) return
/// something altogether different.)
///
/// In all cases, solves use the same CSV as lapse processing, which
/// is not always the same as the CSV printed on an illustration. For
/// example, any sales-load refund increases the value for which the
/// contract can be surrendered, but does not prevent lapse. (NAAR
/// solves use AV rather than CSV in the second part of step 4 above.)
///
/// "Solve for endowment" is deemed to mean that CSV equals specified
/// amount at the target duration, so the target value is the same for
/// all death benefit options.
///
/// NAAR solves work the same way as CSV solves up to step 4's "else"
/// clause, where they return the difference between the target value
/// and the NAAR at the target duration. For this purpose, NAAR is
/// defined as (DB-AV) at EOY: the difference between the DB and AV
/// columns printed on an illustration, which is what agents expect.
/// This is not the same as the NAAR used in calculating COI charges,
/// which discounts the EOM DB for one month's guaranteed interest and
/// subtracts BOM AV.
///
/// Non-MEC solves use an extremely simple objective function that
/// disregards any input target value or duration. The duration is
/// implicitly the maturity date: all that matters is whether the
/// contract ever becomes a MEC. The result is naturally a boundary
/// value, so it is not interesting to compare it to any solve input.
///
/// Non-MEC solves for loan or withdrawal are untested because they
/// probably make no sense--see:
///   https://lists.nongnu.org/archive/html/lmi/2009-02/msg00028.html
/// so perhaps they should be prohibited.
///
/// Non-MEC solves seem acceptably fast despite this two-valued step
/// function. Other options considered include:
///  - Use MEC duration. This inserts a monotone segment into the
///    function in a one-sided neighborhood of the "root" sought.
///    Experiments suggest that this is a pessimization: the monotone
///    segment is rather short, and reaching a point on it causes
///    Brent's algorithm to choose a linear or quadratic interpolation
///    when bisection would actually converge faster.
///  - Minimize the difference between DCV and NSP, subject to not
///    violating the seven-pay limit. The possible improvement in
///    performance does not seem to justify the complexity: e.g.,
///    there is no obvious way to combine the two criteria into one
///    function whose "root" can be found rapidly. Such an objective
///    function would depend on how the necessary-premium exception
///    is applied, if at all; and of course it would be inappropriate
///    for GPT contracts.
/// Alternatively, MEC avoidance could have been treated not as a
/// distinct solve species but rather as a constraint optionally
/// superimposed on other solves; but that would lead to a more
/// general optimization problem.
///
/// When solving for illustration-reg guaranteed premium for a GPT
/// contract, guideline premium limitations are suspended (in code
/// found elsewhere). Thus, the intention of state law is realized by
/// overlooking the very real possibility of a conflict with federal
/// law. See NAIC Q&A 7.9:
///   http://www.naic.org/committe/modelaws/liiq&a.htm
///   "Section 7B(2) does not preclude the illustrating of premiums
///   that exceed the guideline premiums in Section 7702 of the IRC."

currency AccountValue::SolveTest
    (currency a_CandidateValue
    ,void (AccountValue::*solve_set_fn)(currency)
    )
{
    (this->*solve_set_fn)(a_CandidateValue);

    mcenum_run_basis z;
    set_run_basis_from_cloven_bases
        (z
        ,SolveGenBasis_
        ,SolveSepBasis_
        );
    RunOneCell(z);

    int no_lapse_dur = std::accumulate
        (YearlyNoLapseActive.begin()
        ,YearlyNoLapseActive.end()
        ,0
        );
    LMI_ASSERT(0 <= no_lapse_dur);
    currency most_negative_csv = C0;
    if(no_lapse_dur < SolveTargetDuration_)
        {
        // CURRENCY !! Cents in ledger will make rounding unnecessary.
        most_negative_csv = round_minutiae().c
            (*std::min_element
                (VariantValues().CSVNet.begin() + no_lapse_dur
                ,VariantValues().CSVNet.begin() + SolveTargetDuration_
                )
            );
        }

    // AccountValue::Solve() asserts that SolveTargetDuration_ lies
    // within appropriate bounds.
    currency greatest_loan_ullage = *std::max_element
        (loan_ullage_.begin()
        ,loan_ullage_.begin() + SolveTargetDuration_
        );
    currency greatest_withdrawal_ullage = *std::max_element
        (withdrawal_ullage_.begin()
        ,withdrawal_ullage_.begin() + SolveTargetDuration_
        );
    currency greatest_ullage = std::max
        (greatest_loan_ullage
        ,greatest_withdrawal_ullage
        );
    currency worst_negative = std::min
        (most_negative_csv
        ,-greatest_ullage
        );

    // SolveTargetDuration_ is in origin one. That's natural for loop
    // counters and iterators--it's one past the end--but indexing
    // must decrement it.
    // CURRENCY !! Cents in ledger will make rounding unnecessary.
    currency value = round_minutiae().c(VariantValues().CSVNet[SolveTargetDuration_ - 1]);
    if(mce_solve_for_target_naar == SolveTarget_)
        {
        // CURRENCY !! Cents in ledger will make rounding unnecessary.
        value = round_minutiae().c
            (
              VariantValues().EOYDeathBft[SolveTargetDuration_ - 1]
            - VariantValues().AcctVal    [SolveTargetDuration_ - 1]
            );
        }
    if(worst_negative < C0)
        {
        value = std::min(value, worst_negative);
        }

    if(mce_solve_for_endt == SolveTarget_)
        {
        // The input specified amount mustn't be used here because
        // it wouldn't reflect dynamic adjustments.
        SolveTargetCsv_ = base_specamt(SolveTargetDuration_ - 1);
        }

    if(mce_solve_for_tax_basis == SolveTarget_)
        {
        SolveTargetCsv_ = YearlyTaxBasis[SolveTargetDuration_ - 1];
        }

    if(mce_solve_for_non_mec == SolveTarget_)
        {
        static constexpr currency C100 = 100_cents;
        return InvariantValues().IsMec ? -C100 : C100;
        }

    return value - SolveTargetCsv_;
}

/// Set specified amount for a solve iteration.
///
/// Anything dependent on specified amount (e.g., surrender charges,
/// which are currently unsupported) should be changed accordingly.

void AccountValue::SolveSetSpecAmt(currency a_CandidateValue)
{
    DeathBfts_->set_specamt
        (a_CandidateValue
        ,SolveBeginYear_
        ,SolveEndYear_
        );
}

void AccountValue::SolveSetEePrem(currency a_CandidateValue)
{
    Outlay_->set_ee_modal_premiums(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

void AccountValue::SolveSetErPrem(currency a_CandidateValue)
{
    Outlay_->set_er_modal_premiums(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

void AccountValue::SolveSetLoan(currency a_CandidateValue)
{
    Outlay_->set_new_cash_loans(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

void AccountValue::SolveSetWD(currency a_CandidateValue)
{
    Outlay_->set_withdrawals(a_CandidateValue, SolveBeginYear_, SolveEndYear_);
}

currency AccountValue::SolveGuarPremium()
{
    // Store original er premiums for later restoration.
    std::vector<currency> stored = Outlay_->er_modal_premiums();
    // Zero out er premiums and solve for ee premiums only.
    Outlay_->set_er_modal_premiums(C0, 0, BasicValues::GetLength());

    bool temp_solving     = Solving;
    Solving               = true;
    SolvingForGuarPremium = true;

    // Run the solve using guaranteed assumptions.
    currency guar_premium = Solve
        (mce_solve_ee_prem
        ,0
        ,BasicValues::GetLength()
        ,mce_solve_for_endt
        ,C0
        ,BasicValues::GetLength()
        ,mce_gen_guar
        ,mce_sep_full
        );

    // Restore original values.
    Outlay_->set_er_modal_premiums(stored);
    Solving               = temp_solving;
    SolvingForGuarPremium = false;

    return guar_premium;
}

currency AccountValue::Solve
    (mcenum_solve_type   a_SolveType
    ,int                 a_SolveBeginYear
    ,int                 a_SolveEndYear
    ,mcenum_solve_target a_SolveTarget
    ,currency            a_SolveTargetCsv
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

    // Default bounds.
    //
    // These are 'const' to discourage replacing them when narrower
    // bounds can be determined in context (as is often the case for
    // mce_solve_specamt and mce_solve_wd). The objective function,
    // AccountValue::SolveTest(), must already embody such knowledge,
    // and returns a value that is carefully crafted to facilitate
    // solves--in particular, for iterations that violate certain
    // product rules (e.g., see "ullage" above). Imposing a product
    // minimum here can vitiate such optimizations; it is unlikely to
    // make solves faster (finding a zero of x^2-1e8 in (0,1e9] is
    // not materially harder than in [500,1e9], e.g.); it is certain
    // to entail non-negligible coding and maintenance costs; and it
    // introduces new opportunities for mistkaes.
    //
    // Solve results are constrained to be nonnegative.
    double const lower_bound = 0.0;
    // No amount solved for can plausibly reach one billion dollars.
    double const upper_bound = 999999999.99;

    root_bias bias =
        mce_solve_for_tax_basis == SolveTarget_
        ? bias_lower
        : bias_higher
        ;
    int decimals = 0;

    void (AccountValue::*solve_set_fn)(currency) {nullptr};
    switch(a_SolveType)
        {
        case mce_solve_none:
            {
            alarum() << "Unreachable--cannot solve for nothing." << LMI_FLUSH;
            }
            break;
        case mce_solve_specamt:
            {
            solve_set_fn = &AccountValue::SolveSetSpecAmt;
            decimals     = round_specamt().decimals();
            }
            break;
        case mce_solve_ee_prem:
            {
            solve_set_fn = &AccountValue::SolveSetEePrem;
            decimals     = round_gross_premium().decimals();
            }
            break;
        case mce_solve_er_prem:
            {
            solve_set_fn = &AccountValue::SolveSetErPrem;
            decimals     = round_gross_premium().decimals();
            }
            break;
        case mce_solve_loan:
            {
            solve_set_fn = &AccountValue::SolveSetLoan;
            decimals     = round_loan().decimals();
            }
            break;
        case mce_solve_wd:
            {
            solve_set_fn = &AccountValue::SolveSetWD;
            decimals     = round_withdrawal().decimals();
            if(yare_input_.WithdrawToBasisThenLoan)
                {
                // Withdrawals and loans might be rounded differently.
                // To obtain a level income as a mixture of loans and
                // withdrawals, both should be rounded to the less
                // precise number of decimals normally used for either.
                decimals = std::min
                    (round_withdrawal().decimals()
                    ,round_loan      ().decimals()
                    );
                }
            }
            break;
        }
    LMI_ASSERT(nullptr != solve_set_fn);

    std::ostream os_trace(status().rdbuf());
    std::ofstream ofs_trace;
    if(contains(yare_input_.Comments, "idiosyncrasyT") && !SolvingForGuarPremium)
        {
        ofs_trace.open("trace.txt", ios_out_app_binary());
        os_trace.rdbuf(ofs_trace.rdbuf());
        }

    SolveHelper solve_helper(*this, solve_set_fn);
    root_type const solution = decimal_root
        (lower_bound
        ,upper_bound
        ,bias
        ,decimals
        ,solve_helper
        ,os_trace
        );
    currency const solution_cents = round_minutiae().c(solution.first);

    Solving = false;

    // The account and ledger values set as a side effect of solving
    // aren't generally the same as those shown on the illustration
    // because the 'Solving' flag has side effects. Therefore, the
    // final solve parameters are stored by calling 'solve_set_fn'
    // now, and actual values are freshly generated downstream.
    (this->*solve_set_fn)(solution_cents);

    switch(solution.second)
        {
        case root_is_valid:
            {} // Do nothing.
            break;
        case root_not_bracketed:
            {
            LMI_ASSERT(0.0 == solution.first);
            // Don't want this firing continually in census runs.
            if(!SolvingForGuarPremium)
                {
                warning() << "Solution not found: using zero instead." << LMI_FLUSH;
                // TODO ?? What can we do when no solution exists for guar prem?
                }
            }
            break;
        }
    return solution_cents;
}
