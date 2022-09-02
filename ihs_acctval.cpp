// Account value.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
#include "calendar_date.hpp"
#include "contains.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "gpt7702.hpp"
#include "i7702.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "input.hpp"                    // consummate()
#include "interest_rates.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "loads.hpp"
#include "materially_equal.hpp"
#include "miscellany.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "ssize_lmi.hpp"
#include "stratified_algorithms.hpp"

#include <algorithm>
#include <cfloat>                       // DECIMAL_DIG
#include <cmath>
#include <iomanip>                      // setprecision()
#include <ios>                          // ios_base::fixed()
#include <iterator>                     // back_inserter()
#include <limits>
#include <numeric>
#include <string>
#include <utility>

/*
We ideally want transaction functions to be reorderable.
That means each must be atomic and reentrant, with no dependencies.
To what extent is this feasible?

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
    :BasicValues           (Input::consummate(input))
    ,InputFilename         {"anonymous"}
    ,DebugFilename         {"anonymous.monthly_trace"}
    ,Debugging             {false}
    ,Solving               {mce_solve_none != BasicValues::yare_input_.SolveType}
    ,SolvingForGuarPremium {false}
    ,ItLapsed              {false}
    ,ledger_{::new Ledger(BasicValues::GetLength(), BasicValues::ledger_type(), BasicValues::nonillustrated(), BasicValues::no_can_issue(), false)}
    ,ledger_invariant_     {::new LedgerInvariant(BasicValues::GetLength())}
    ,ledger_variant_       {::new LedgerVariant  (BasicValues::GetLength())}
    ,SolveGenBasis_        {mce_gen_curr}
    ,SolveSepBasis_        {mce_sep_full}
    ,RunBasis_             {mce_run_gen_curr_sep_full}
    ,GenBasis_             {mce_gen_curr}
    ,SepBasis_             {mce_sep_full}
    ,OldDBOpt              {mce_option1}
    ,YearsDBOpt            {mce_option1}
{
    SetInitialValues();
    LMI_ASSERT(InforceYear < methuselah);
    PerformSpecAmtStrategy(enforce_minimum);
    PerformSupplAmtStrategy();
    InvariantValues().Init(this);

    // Explicitly initialize antediluvian members. It's generally
    // better to do this in the initializer-list, but here they can
    // all be kept together.
    LapseMonth               = 0;          // Antediluvian.
    LapseYear                = 0;          // Antediluvian.
    AVUnloaned               = C0;         // Antediluvian.
    pmt                      = C0;         // Antediluvian.
    pmt_mode                 = mce_annual; // Antediluvian.
    ModeIndex                = 0;          // Antediluvian.
    wd                       = C0;         // Antediluvian.
    mlyguarv                 = 0.0;        // Antediluvian.
    deathbft                 = C0;         // Antediluvian.
    haswp                    = false;      // Antediluvian.
    hasadb                   = false;      // Antediluvian.
    mlydedtonextmodalpmtdate = C0;         // Antediluvian.

    set_list_bill_year_and_month();

    OverridingEePmts    .resize(12 * BasicValues::GetLength());
    OverridingErPmts    .resize(12 * BasicValues::GetLength());

    OverridingLoan      .resize(BasicValues::GetLength());
    OverridingWD        .resize(BasicValues::GetLength());

    SurrChg_            .resize(BasicValues::GetLength());

    YearlyTaxBasis      .reserve(BasicValues::GetLength());
    YearlyNoLapseActive .reserve(BasicValues::GetLength());
    loan_ullage_        .reserve(BasicValues::GetLength());
    withdrawal_ullage_  .reserve(BasicValues::GetLength());
}

/// Specified amount (disregarding any term or "supplemental" amount).

currency AccountValue::base_specamt(int year) const
{
    return from_cents(InvariantValues().SpecAmt[year]);
}

/// Specified amount of term rider.

currency AccountValue::term_specamt(int year) const
{
    return from_cents(InvariantValues().TermSpecAmt[year]);
}

/// Specified amount for 7702 (not 7702A).

currency AccountValue::specamt_for_7702(int year) const
{
    return
                              base_specamt(year)
        + (TermIsDbFor7702  ? term_specamt(year) : C0)
        ;
}

/// Specified amount for 7702A (not 7702).

currency AccountValue::specamt_for_7702A(int year) const
{
    return
                              base_specamt(year)
        + (TermIsDbFor7702A ? term_specamt(year) : C0)
        ;
}

void AccountValue::assert_pmts_add_up(char const* file, int line, int month)
{
    // Payments, being currency amounts, should all be exact integral
    // numbers of cents, and should add up exactly.
    if(GrossPmts[month] == EeGrossPmts[month] + ErGrossPmts[month])
        {
        return;
        }

    alarum()
        << "Payments don't add up [file '" << file << "', line " << line << "]\n"
        << month << " month\n"
        << Year << " Year\n"
        << std::fixed << std::setprecision(DECIMAL_DIG)
        << EeGrossPmts[month] << " EeGrossPmts[month]\n"
        << ErGrossPmts[month] << " ErGrossPmts[month]\n"
        << GrossPmts[month] << " GrossPmts[month]\n"
        << EeGrossPmts[month] + ErGrossPmts[month] << " EeGrossPmts[month] + ErGrossPmts[month]\n"
        << LMI_FLUSH
        ;
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
/*
First run current, for solves and strategies. This determines
    payments
    specamt (increases, decreases, option changes)
    and hence surrchg
Then run other bases.

    guar and midpt values are useful only for illustrations
    for profit testing we want to avoid their overhead
    for solves we want only one run
        if solving on guar basis...does *that* basis determine pmts & specamt?
        it probably should, so that the guar columns will show what's wanted
            otherwise the solve lacks meaning
        although I wonder how other illustration systems handle this
        it should also be possible to solve on a midpt basis as well
*/

    if(contains(yare_input_.Comments, "idiosyncrasyZ"))
        {
        Debugging = true;
        DebugPrintInit();
        }

    RunAllApplicableBases();

    FinalizeLifeAllBases();
}

/// Guaranteed premium for NAIC illustration reg, section 7B(2).
///
/// Section 7B(2) requires "basic" illustrations to show "the premium
/// outlay that must be paid to guarantee coverage for the term of the
/// contract".
///
/// Section 4H(1) distinguishes "basic" from "in force" illustrations;
/// the latter category applies when the contract has been "in force
/// for one year or more". Thus, an illustration depicting values six
/// months after issue is still "basic"; in normal industry parlance,
/// it's an inforce illustration, but it's not "in force" for the
/// purpose of complying with this regulation.
///
/// Section 10C says "in force" illustrations must comply with
///   "Section 6A, 6B, 7A and 7E"
/// which excludes 7B.
///
/// Therefore, this premium must be calculated only during the first
/// year, and of course only for contracts subject to this regulation.
///
/// TODO ?? Perhaps commutation functions could be used to speed up
/// this rather expensive function.

void AccountValue::SetGuarPrem()
{
    GuarPremium = C0;
    if
        (  BasicValues::IsSubjectToIllustrationReg()
        && 0 == InforceYear
        && !database().query<bool>(DB_OmitGuarPremSolve)
        )
        {
        GuarPremium = SolveGuarPremium();
        }
    ledger_->SetGuarPremium(dblize(GuarPremium));
}

//============================================================================
void AccountValue::RunOneBasis(mcenum_run_basis a_Basis)
{
    if
        (  !BasicValues::IsSubjectToIllustrationReg()
        && mce_run_gen_mdpt_sep_full == a_Basis
        )
        {
        alarum()
            << "Midpoint basis defined only for illustration-reg ledger."
            << LMI_FLUSH
            ;
        }

    if(Solving)
        {
// Apparently this should never be done because Solve() is called in
//   RunAllApplicableBases() .
// TODO ?? Isn't this unreachable?
//      LMI_ASSERT(a_Basis corresponds to yare_input_.SolveExpenseGeneralAccountBasis);
        }
    else
        {
        RunOneCell(a_Basis);
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
    // TODO ?? Normally, running on the current basis determines the
    // overriding values for all components of outlay--e.g., premiums,
    // forceouts, loans, and withdrawals. For a solve on any basis
    // other than current, the overriding values could be determined
    // in two ways:
    //
    // (1) on the current basis--but then the solve won't be right;
    //
    // (2) on the solve basis: but if it lapses on that basis, then
    // overriding values were not determined for later durations,
    // yet such values may be needed e.g. for the current basis.

    if(Solving)
        {
        // TODO ?? This conditional tests the value of SolveGenBasis_,
        // which hasn't yet been assigned any contextual value. The
        // test is invalid anyway: the types don't match. Furthermore,
        // the purpose of the statement it controls is not pellucid;
        // it seems safer to execute it than not.
//        if(mce_run_gen_curr_sep_full != SolveGenBasis_)
            {
            RunOneBasis(mce_run_gen_curr_sep_full);
            }

        Solve
            (yare_input_.SolveType
            ,yare_input_.SolveBeginYear
            ,yare_input_.SolveEndYear
            ,yare_input_.SolveTarget
            ,round_minutiae().c(yare_input_.SolveTargetValue)
            ,yare_input_.SolveTargetYear
            ,yare_input_.SolveExpenseGeneralAccountBasis
            ,yare_input_.SolveSeparateAccountBasis
            );
        Solving = false;
        // TODO ?? Here we might save overriding parameters determined
        // on the solve basis.
        }
    // Run all bases, current first.
    for(auto const& b : ledger_->GetRunBases())
        {
        RunOneBasis(b);
        }
}

//============================================================================
/// This implementation seems slightly unnatural because it strives
/// for similarity with run_census_in_parallel::operator(). For
/// instance, 'Year' and 'Month' aren't used directly as loop
/// counters, and the loop has no early-exit condition like
///   if(ItLapsed) break;
/// which isn't necessary anyway because all the functions it calls
/// contain such a condition.

void AccountValue::RunOneCell(mcenum_run_basis a_Basis)
{
    InitializeLife(a_Basis);

    for(int year = InforceYear; year < BasicValues::GetLength(); ++year)
        {
        Year = year;
        CoordinateCounters();
        InitializeYear();

        int inforce_month = (Year == InforceYear) ? InforceMonth : 0;
        for(int month = inforce_month; month < 12; ++month)
            {
            Month = month;
            CoordinateCounters();
            IncrementBOM(year, month);
            IncrementEOM
                (year
                ,month
                ,SepAcctValueAfterDeduction
                ,CumPmts
                );
            }

        SetClaims();
        IncrementEOY(year);
        }

    FinalizeLife(a_Basis);
}

//============================================================================
void AccountValue::InitializeLife(mcenum_run_basis a_Basis)
{
    RunBasis_ = a_Basis;
    set_cloven_bases_from_run_basis(RunBasis_, GenBasis_, SepBasis_);

    // Many values set by SetInitialValues() are never subsequently
    // changed. SOMEDAY !! Factor those out into a function that can
    // be called OAOO.
    SetInitialValues();

    if
        (  Solving
        && !SolvingForGuarPremium
        && (  mce_solve_ee_prem == yare_input_.SolveType
           || mce_solve_er_prem == yare_input_.SolveType
           )
        && mce_run_gen_curr_sep_full == RunBasis_
        && (  contains(yare_input_.SpecifiedAmountStrategy, mce_sa_maximum)
           || contains(yare_input_.SpecifiedAmountStrategy, mce_sa_target)
           || contains(yare_input_.SpecifiedAmountStrategy, mce_sa_mep)
           || contains(yare_input_.SpecifiedAmountStrategy, mce_sa_glp)
           || contains(yare_input_.SpecifiedAmountStrategy, mce_sa_gsp)
           || contains(yare_input_.SpecifiedAmountStrategy, mce_sa_corridor)
           )
        )
        {
        PerformSpecAmtStrategy(ignore_minimum);
        PerformSupplAmtStrategy();
        }

    VariantValues().Init(*this, GenBasis_, SepBasis_);
    InvariantValues().ReInit(this);

    // Default initial values assume that the policy never lapses or
    // becomes a MEC, so that the lapse and MEC durations are the last
    // possible month.
    // TODO ?? TAXATION !! Last possible month? Why? In that month, it
    // doesn't quite lapse, and it's certainly wrong to assume it
    // becomes a MEC then.
    LMI_ASSERT(11                       == VariantValues().LapseMonth);
    LMI_ASSERT(BasicValues::GetLength() == VariantValues().LapseYear );
    LMI_ASSERT(false                    == InvariantValues().IsMec   );
    LMI_ASSERT(11                       == InvariantValues().MecMonth);
    LMI_ASSERT(BasicValues::GetLength() == InvariantValues().MecYear );

    OldDBOpt = DeathBfts_->dbopt()[0];
    // TAXATION !! 'OldSA' and 'OldDB' need to be distinguished for 7702 and 7702A,
    // with inclusion of term dependent on 'TermIsDbFor7702' and 'TermIsDbFor7702A'.
    OldSA = base_specamt(0) + term_specamt(0);
    // TODO ?? TAXATION !! Shouldn't we increase initial SA if contract in corridor at issue?
    OldDB = OldSA;

    SurrChg_.assign(BasicValues::GetLength(), C0);

    // TAXATION !! Input::InforceAnnualTargetPremium should be used here.
    currency annual_target_premium = GetModalTgtPrem
        (0
        ,mce_annual
        ,base_specamt(0)
        );
    currency sa = specamt_for_7702(0);

    // Base for specified-amount load, for GPT only.
    //
    // Unlike 'SpecAmtLoadBase', this ignores the corridor. Reasons:
    //  - initial payment is not yet known--e.g., it may be determined
    //    by a strategy or a solve, perhaps with a non-MEC limit; and
    //  - it shouldn't matter, because a GPT contract should be issued
    //    at a spec amt no lower than its initial corridor DB.
    // Initial guideline premiums are calculated based on the (f)(3)
    // benefit before any payment or 1035 exchange is accepted, which
    // by definition cannot exceed the spec amt. If that's less than
    // the initial corridor DB, then the guideline limit is already
    // lower than it needs to be, regardless of any spec amt load;
    // in that case, a warning is given downstream.
    gpt_chg_sa_base_ =
        (yare_input_.EffectiveDate == yare_input_.InforceAsOfDate)
        ? term_specamt(0) + base_specamt(0)
        : round_specamt().c(yare_input_.InforceSpecAmtLoadBase)
        ;
    gpt_chg_sa_base_ = std::min(gpt_chg_sa_base_, SpecAmtLoadLimit);

    // Normally 'f3_bft' is death benefit, not specified amount; but
    // on the issue date, it is defined as specified amount.
    gpt_scalar_parms s_parms =
        {.duration       = yare_input_.InforceYear
        ,.f3_bft         = dblize(specamt_for_7702(0))
        ,.endt_bft       = dblize(specamt_for_7702(0))
        ,.target_prem    = dblize(annual_target_premium)
        ,.chg_sa_base    = dblize(gpt_chg_sa_base_)
        ,.dbopt_7702     = effective_dbopt_7702(DeathBfts_->dbopt()[0], Effective7702DboRop)
        };
    gpt7702_->initialize_gpt
        (yare_input_.DefinitionOfLifeInsurance // defn_life_ins
        ,yare_input_.InforceMonth / 12.0       // fractional_duration
        ,yare_input_.InforceGlp                // inforce_glp
        ,yare_input_.InforceCumulativeGlp      // inforce_cum_glp
        ,yare_input_.InforceGsp                // inforce_gsp
        ,round_minutiae().c(yare_input_.InforceCumulativeGptPremiumsPaid) // inforce_cum_f1A
        ,s_parms
        );

    // It is at best superfluous to do this for every basis.
    // TAXATION !! Don't do that then.
    Irc7702_->Initialize7702
        (dblize(sa)
        ,dblize(sa)
        ,effective_dbopt_7702(DeathBfts_->dbopt()[0], Effective7702DboRop)
        ,dblize(annual_target_premium)
        );

    LMI_ASSERT(materially_equal(gpt7702_->raw_glp(), Irc7702_->glp()));
    LMI_ASSERT(materially_equal(gpt7702_->raw_gsp(), Irc7702_->gsp()));

    InvariantValues().InitGLP = centize(gpt7702_->rounded_glp());
    InvariantValues().InitGSP = centize(gpt7702_->rounded_gsp());

    // This is notionally called once per *current*-basis run
    // and actually called once per run, with calculations suppressed
    // for all other bases. TODO ?? TAXATION !! How should we handle MEC-avoid
    // solves on bases other than current?

    // INPUT !! This should depend only on 'yare_input_.InforceIsMec',
    // which probably should be forced to 'false' for new business.
    bool inforce_is_mec =
           yare_input_.EffectiveDate != yare_input_.InforceAsOfDate
        && yare_input_.InforceIsMec
        ;
    InvariantValues().InforceIsMec = inforce_is_mec;
    bool mec_1035 =
              yare_input_.External1035ExchangeFromMec
           && C0 != Outlay_->external_1035_amount()
        ||    yare_input_.Internal1035ExchangeFromMec
           && C0 != Outlay_->internal_1035_amount()
        ;
    bool is_already_a_mec = inforce_is_mec || mec_1035;
    if(is_already_a_mec)
        {
        InvariantValues().IsMec    = true;
        InvariantValues().MecYear  = 0;
        InvariantValues().MecMonth = 0;
        }
    std::vector<double> pmts_7702a;
    std::vector<double> bfts_7702a;
    if(yare_input_.EffectiveDate == yare_input_.InforceAsOfDate)
        {
        // No need to initialize 'pmts_7702a' in this case.
        bfts_7702a.push_back(dblize(specamt_for_7702A(0)));
        }
    else
        {
        int length_7702a = std::min(7, BasicValues::GetLength());
        // Premium history starts at contract year zero.
        std::copy_n
            (yare_input_.Inforce7702AAmountsPaidHistory.begin()
            ,length_7702a
            ,std::back_inserter(pmts_7702a)
            );
        // Specamt history is irrelevant except for LDB.
        bfts_7702a = std::vector<double>(length_7702a, yare_input_.InforceLeastDeathBenefit);
        }
    double lowest_death_benefit = yare_input_.InforceLeastDeathBenefit;
    if(yare_input_.EffectiveDate == yare_input_.InforceAsOfDate)
        {
        lowest_death_benefit = bfts_7702a.front(); // TAXATION !! See above--use input LDB instead.
        }
    Irc7702A_->Initialize7702A
        (mce_run_gen_curr_sep_full != RunBasis_
        ,is_already_a_mec
        ,IssueAge
        ,BasicValues::EndtAge
        ,InforceYear
        ,InforceMonth
        ,yare_input_.InforceContractYear
        ,yare_input_.InforceContractMonth
        ,yare_input_.InforceAvBeforeLastMc
        ,lowest_death_benefit // TAXATION !! See above--use input LDB instead.
        ,pmts_7702a
        ,bfts_7702a
        );
}

/// Post results to ledger for a single basis.
///
/// However, exit early, posting nothing, when performing a solve
/// for NAIC illustration reg guaranteed premium.

void AccountValue::FinalizeLife(mcenum_run_basis a_Basis)
{
    LMI_ASSERT(RunBasis_ == a_Basis);

    DebugEndBasis();

    if(SolvingForGuarPremium)
        {
        return;
        }

    if(mce_run_gen_curr_sep_full == RunBasis_)
        {
        ledger_->SetLedgerInvariant(InvariantValues());
        }
    ledger_->SetOneLedgerVariant(a_Basis, VariantValues());
}

//============================================================================
void AccountValue::FinalizeLifeAllBases()
{
    ledger_->ZeroInforceAfterLapse();
    SetGuarPrem();
}

//============================================================================
void AccountValue::SetInitialValues()
{
    InforceYear           = yare_input_.InforceYear                     ;
    InforceMonth          = yare_input_.InforceMonth                    ;
    InforceAVGenAcct      = round_minutiae().c(yare_input_.InforceGeneralAccountValue      );
    InforceAVSepAcct      = round_minutiae().c(yare_input_.InforceSeparateAccountValue     );
    InforceAVRegLn        = round_minutiae().c(yare_input_.InforceRegularLoanValue         );
    InforceAVPrfLn        = round_minutiae().c(yare_input_.InforcePreferredLoanValue       );
    InforceRegLnBal       = round_minutiae().c(yare_input_.InforceRegularLoanBalance       );
    InforcePrfLnBal       = round_minutiae().c(yare_input_.InforcePreferredLoanBalance     );
    InforceCumNoLapsePrem = round_minutiae().c(yare_input_.InforceCumulativeNoLapsePremium );
    InforceCumPmts        = round_minutiae().c(yare_input_.InforceCumulativeNoLapsePayments);
    InforceTaxBasis       = round_minutiae().c(yare_input_.InforceTaxBasis                 );

    Year                  = InforceYear;
    Month                 = InforceMonth;
    daily_interest_accounting = contains
        (yare_input_.Comments
        ,"idiosyncrasy_daily_interest_accounting"
        );
    CoordinateCounters();

    DB7702A               = C0;  // TODO ?? TAXATION !! This seems silly.

    AVRegLn               = InforceAVRegLn;
    AVPrfLn               = InforceAVPrfLn;
    RegLnBal              = InforceRegLnBal;
    PrfLnBal              = InforcePrfLnBal;
    AVGenAcct             = InforceAVGenAcct;
    AVSepAcct             = InforceAVSepAcct;

    // WX PORT !! When fund selection is finally ported from ihs,
    // this workaround should not be needed; until then, it sidesteps
    // spurious errors in product_test().
    double const sa_allocation =  premium_allocation_to_sepacct(yare_input_);
    bool const override_allocation =
           !database().query<bool>(DB_AllowGenAcct)
        && global_settings::instance().regression_testing()
        ;
//  SepAcctPaymentAllocation = premium_allocation_to_sepacct(yare_input_);
    SepAcctPaymentAllocation = override_allocation ? 1.0 : sa_allocation ;
    GenAcctPaymentAllocation = 1.0 - SepAcctPaymentAllocation;

    if(!database().query<bool>(DB_AllowGenAcct) && 0.0 != GenAcctPaymentAllocation)
        {
        alarum()
            << "No general account is allowed for this product, but "
            << GenAcctPaymentAllocation
            << " is allocated to the general account."
            << LMI_FLUSH
            ;
        }

    if(!database().query<bool>(DB_AllowSepAcct) && 0.0 != SepAcctPaymentAllocation)
        {
        alarum()
            << "No separate account is allowed for this product, but "
            << SepAcctPaymentAllocation
            << " is allocated to the separate account."
            << LMI_FLUSH
            ;
        }

    MaxLoan                     = C0;

    GenAcctIntCred              = C0;
    SepAcctIntCred              = C0;
    RegLnIntCred                = C0;
    PrfLnIntCred                = C0;

    MaxWD                       = C0;
    GrossWD                     = C0;
    NetWD                       = C0;

    CumPmts                     = InforceCumPmts;
    TaxBasis                    = InforceTaxBasis;
    YearlyTaxBasis.assign(BasicValues::GetLength(), C0);
    MlyNoLapsePrem              = C0;
    CumNoLapsePrem              = InforceCumNoLapsePrem;

    // Initialize all elements of this vector to 'false'. Then, when
    // the no-lapse criteria fail to be met, future values are right.
    YearlyNoLapseActive.assign(BasicValues::GetLength(), false);
    loan_ullage_       .assign(BasicValues::GetLength(), C0);
    withdrawal_ullage_ .assign(BasicValues::GetLength(), C0);
    NoLapseActive               = true;
    if(NoLapseDboLvlOnly && mce_option1 != DeathBfts_->dbopt()[0])
        {
        NoLapseActive           = false;
        }
    if(NoLapseUnratedOnly && is_policy_rated(yare_input_))
        {
        NoLapseActive           = false;
        }

    database().query_into(DB_SplitMinPrem       , SplitMinPrem);
    database().query_into(DB_UnsplitSplitMinPrem, UnsplitSplitMinPrem);

    database().query_into(DB_TermCanLapse       , TermCanLapse);
    TermRiderActive             = true;
    TermDB                      = C0;

    ItLapsed                    = false;

    Dcv                         = round_minutiae().c(yare_input_.InforceDcv);
    DcvDeathBft                 = C0;
    DcvNaar                     = C0;
    DcvCoiCharge                = C0;
    DcvTermCharge               = C0;
    DcvWpCharge                 = C0;

    HoneymoonActive             = false;
    // Identity element for std::max(), disregarding -INF and NaN.
    // CURRENCY !! alternatively, use -INF
    HoneymoonValue = -from_cents(std::numeric_limits<currency::data_type>::max());
    if(mce_gen_curr == GenBasis_)
        {
        HoneymoonActive = yare_input_.HoneymoonEndorsement;
        if(0 != Year || 0 != Month)
            {
            HoneymoonActive =
                   HoneymoonActive
                && 0.0 < yare_input_.InforceHoneymoonValue
                ;
            }
        if(HoneymoonActive)
            {
            HoneymoonValue = round_minutiae().c(yare_input_.InforceHoneymoonValue);
            }
        }

    gpt_chg_sa_base_            = C0;

    CoiCharge                   = C0;
    RiderCharges                = C0;
    MlyDed                      = C0;
    CumulativeSalesLoad         = round_minutiae().c(yare_input_.InforceCumulativeSalesLoad);

    Dumpin             = Outlay_->dumpin();
    External1035Amount = Outlay_->external_1035_amount();
    Internal1035Amount = Outlay_->internal_1035_amount();

    database().query_into(DB_EePremMethod      , ee_premium_allocation_method  );
    database().query_into(DB_EePremAcct        , ee_premium_preferred_account  );
    database().query_into(DB_ErPremMethod      , er_premium_allocation_method  );
    database().query_into(DB_ErPremAcct        , er_premium_preferred_account  );
    database().query_into(DB_DeductionMethod   , deduction_method              );
    database().query_into(DB_DeductionAcct     , deduction_preferred_account   );
    database().query_into(DB_DistributionMethod, distribution_method           );
    database().query_into(DB_DistributionAcct  , distribution_preferred_account);

    // If any account preference is the separate account, then a
    // separate account must be available.
    if
        (    oe_prefer_separate_account == ee_premium_preferred_account
        ||   oe_prefer_separate_account == er_premium_preferred_account
        ||   oe_prefer_separate_account == deduction_preferred_account
        ||   oe_prefer_separate_account == distribution_preferred_account
        )
        {
        LMI_ASSERT(database().query<bool>(DB_AllowSepAcct));
        }
    // If any account preference for premium is the general account,
    // then payment into the separate account must be permitted; but
    // even a product that doesn't permit that might have a general
    // account, e.g. for loans or deductions.
    if
        (    oe_prefer_separate_account == ee_premium_preferred_account
        ||   oe_prefer_separate_account == er_premium_preferred_account
        )
        {
        LMI_ASSERT(database().query<bool>(DB_AllowSepAcct));
        }
}

//============================================================================
// Process monthly transactions up to but excluding interest credit
currency AccountValue::IncrementBOM
    (int    year
    ,int    month
    )
{
    if(ItLapsed || BasicValues::GetLength() <= year)
        {
        // Return value is total assets. After the policy has lapsed or
        // matured, there are no assets.
        return C0;
        }

    // Paranoid check.
    LMI_ASSERT(year == Year);
    LMI_ASSERT(month == Month);
    LMI_ASSERT(MonthsSinceIssue == Month + 12 * Year);
    if(daily_interest_accounting)
        {
        LMI_ASSERT(365 <= days_in_policy_year && days_in_policy_year <= 366);
        LMI_ASSERT(28 <= days_in_policy_month && days_in_policy_month <= 31);
        }
    if
        (   year != Year
        ||  month != Month
        ||  MonthsSinceIssue != Month + 12 * Year
        ||  daily_interest_accounting && !(365 <= days_in_policy_year && days_in_policy_year <= 366)
        ||  daily_interest_accounting && !(28 <= days_in_policy_month && days_in_policy_month <= 31)
        )
        {
        alarum()
            << "Expected year = "  << Year
            << "; actual year is  " << year << ".\n"
            << "Expected month = " << Month
            << "; actual month is " << month << ".\n"
            << "Expected MonthsSinceIssue = " << (Month + 12 * Year)
            << "; actual value is " << MonthsSinceIssue << ".\n"
            << "Days in policy year = " << days_in_policy_year << ".\n"
            << "Days in policy month = " << days_in_policy_month << ".\n"
            << LMI_FLUSH
            ;
        }

    DoMonthDR();
    return TotalAccountValue();
}

//============================================================================
// Credit interest and process all subsequent monthly transactions
void AccountValue::IncrementEOM
    (int      year
    ,int      month
    ,currency assets_post_bom
    ,currency cum_pmts_post_bom
    )
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

    // Paranoid check.
    LMI_ASSERT(year == Year);
    LMI_ASSERT(month == Month);
    LMI_ASSERT(MonthsSinceIssue == Month + 12 * Year);
    if(daily_interest_accounting)
        {
        LMI_ASSERT(365 <= days_in_policy_year && days_in_policy_year <= 366);
        LMI_ASSERT(28 <= days_in_policy_month && days_in_policy_month <= 31);
        }

    // Save arguments, constraining their values to be nonnegative,
    // for calculating banded and tiered quantities.
    AssetsPostBom  = std::max(C0, assets_post_bom  );
    CumPmtsPostBom = std::max(C0, cum_pmts_post_bom);

    DoMonthCR();
}

//============================================================================
void AccountValue::IncrementEOY(int year)
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

    // Paranoid check.
    LMI_ASSERT(year == Year);

    FinalizeYear();
}

//============================================================================
bool AccountValue::PrecedesInforceDuration(int year, int month)
{
    return year < InforceYear || (year == InforceYear && month < InforceMonth);
}

//============================================================================
void AccountValue::InitializeYear()
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

// TODO ?? Solve...() should reset not inputs but...something else?
    SetAnnualInvariants();

    PremiumTax_->start_new_year();
    // Skip this in an incomplete initial inforce year.
    // TAXATION !! Premium tax should perhaps be handled similarly.
    // TAXATION !! For 7702A, some rates change on policy anniversary
    // because age does, but cum 7pp changes on contract anniversary.
    if(Year != InforceYear || 0 == InforceMonth)
        {
        Irc7702_ ->UpdateBOY7702();
        Irc7702A_->UpdateBOY7702A(Year);
        }

    MonthsPolicyFees            = C0;
    SpecAmtLoad                 = C0;

    AssetsPostBom               = C0;
    CumPmtsPostBom              = C0;
    SepAcctLoad                 = C0;

    YearsTotalCoiCharge         = C0;
    YearsTotalRiderCharges      = C0;
    YearsAVRelOnDeath           = 0.0;
    YearsLoanRepaidOnDeath      = 0.0;
    YearsGrossClaims            = 0.0;
    YearsDeathProceeds          = 0.0;
    YearsNetClaims              = 0.0;
    YearsTotalNetIntCredited    = C0;
    YearsTotalGrossIntCredited  = C0;
    YearsTotalLoanIntAccrued    = C0;
    YearsTotalPolicyFee         = C0;
    YearsTotalDacTaxLoad        = 0.0;
    YearsTotalSpecAmtLoad       = C0;
    YearsTotalSepAcctLoad       = C0;
    YearsTotalGptForceout       = C0;

    DacTaxRsv                   = 0.0;

    RequestedLoan               = Outlay_->new_cash_loans()[Year];
    // ActualLoan is set in TxTakeLoan() and TxLoanRepay(). A local
    // variable in each function might have sufficed, except that this
    // quantity is used in the optional monthly detail report. Its
    // value depends on the maximum loan, so it cannot be known here.
    ActualLoan                  = C0;

    GrossPmts   .assign(12, C0);
    EeGrossPmts .assign(12, C0);
    ErGrossPmts .assign(12, C0);
    NetPmts     .assign(12, C0);

    InitializeSpecAmt();
}

//============================================================================
void AccountValue::InitializeSpecAmt()
{
    // TODO ?? These variables are set in current run and used in guar and midpt.
    ActualSpecAmt = base_specamt(Year);
    TermSpecAmt   = term_specamt(Year);

    set_modal_min_premium();

    // No-lapse premium generally changes whenever specamt changes for
    // any reason (e.g., elective increases or decreases, DBO changes,
    // and withdrawals). Target premium may change similarly, or may
    // be fixed immutably at issue. For illustrations, these premium
    // changes occur only on anniversary, because triggering events
    // are allowed only on anniversary.
    //
    // Target premium is annual by its nature: commission is earned
    // immediately if the full target is paid on anniversary.
    // Conversely, no-lapse premium by its nature is on the most
    // frequent mode (monthly for lmi), because no-lapse guarantees
    // are offered for all modes.
    //
    // Arguably the "supplemental" specamt should be included in the
    // target or even the minimum calculation in the TermIsNotRider
    // case; but that's used only with one family of exotic products
    // for which these quantities don't matter anyway.
    //
    int const target_year = TgtPremFixedAtIssue ? 0 : Year;
    MlyNoLapsePrem = GetModalMinPrem
        (target_year
        ,mce_monthly
        ,base_specamt(target_year)
        );
    UnusedTargetPrem = GetModalTgtPrem
        (target_year
        ,mce_annual
        ,base_specamt(target_year)
        );
    AnnualTargetPrem = UnusedTargetPrem;

    if(0 == Year)
        {
        InvariantValues().InitTgtPrem = centize(AnnualTargetPrem);
        }

    // TODO ?? Perform specamt strategy here?
}

/// Set duration at which list-bill premium is to be determined.
///
/// The year and month of determination correspond to the first
/// monthiversary on or after the list-bill date. If there is no
/// such monthiversary, then harmless default values are used.

void AccountValue::set_list_bill_year_and_month()
{
    auto const& cert_date = yare_input_.EffectiveDate;
    auto const& bill_date = yare_input_.ListBillDate;
    if(bill_date < cert_date) return;

    auto const z = years_and_months_since(cert_date, bill_date, false);
    list_bill_year_   = z.first;
    list_bill_month_  = z.second;
}

void AccountValue::set_list_bill_premium()
{
    bool const the_time_is_now =
           Year  == list_bill_year_
        && Month == list_bill_month_
        && mce_run_gen_curr_sep_full == RunBasis_
        ;
    if(!the_time_is_now) return;

    if(!SplitMinPrem)
        {
        auto const z = GetListBillPremMlyDed
            (Year
            ,Outlay_->er_premium_modes()[Year]
            ,base_specamt(Year)
            );
        InvariantValues().ListBillPremium   = centize(z);
        InvariantValues().ErListBillPremium = centize(z);
        }
    else
        {
        auto const z = GetListBillPremMlyDedEx
            (Year
            ,Outlay_->er_premium_modes()[Year]
            ,base_specamt(Year)
            ,term_specamt(Year)
            );
        InvariantValues().EeListBillPremium = centize(z.first);
        InvariantValues().ErListBillPremium = centize(z.second);
        InvariantValues().ListBillPremium   = centize(z.first + z.second);
        }
}

/// Calculate "modal minimum" premiums designed for group plans.
///
/// They are intended roughly to approximate the minimum payment
/// (at a modal frequency chosen by the employer) necessary to
/// prevent lapse if no other premium is paid.
///
/// Some products apportion them explicitly between ee and er. For
/// those that don't, convention deems the er to pay it all.
///
/// These values are calculated for all products, but probably should
/// be restricted to group plans that actually use them on reports.

void AccountValue::set_modal_min_premium()
{
    if(!SplitMinPrem)
        {
        auto const z = GetModalMinPrem
            (Year
            ,Outlay_->er_premium_modes()[Year]
            ,base_specamt(Year)
            );
        InvariantValues().ModalMinimumPremium[Year]   = centize(z);
        InvariantValues().ErModalMinimumPremium[Year] = centize(z);
        }
    else
        {
        auto const z = GetModalPremMlyDedEx
            (Year
            ,Outlay_->er_premium_modes()[Year]
            ,base_specamt(Year)
            ,term_specamt(Year)
            );
        InvariantValues().EeModalMinimumPremium[Year] = centize(z.first);
        InvariantValues().ErModalMinimumPremium[Year] = centize(z.second);
        InvariantValues().ModalMinimumPremium[Year]   = centize(z.first + z.second);
        }
}

/// Surrender charge.
///
/// The "cash value enhancement" components should be implemented as
/// such, rather than as negative surrender charges.
///
/// This ought to be called annually, OAOO, at the top level, and
/// stored in the variant or invariant ledger depending on whether
/// surrender charges are to vary by basis. All other direct calls
/// to this function should be replaced by constant values looked up
/// in a ledger, or perhaps in SurrChg_ if that vector is ever given
/// any nonzero value.
///
/// SOMEDAY !! Table support and UL model reg formulas should be added.

currency AccountValue::SurrChg() const
{
    LMI_ASSERT(C0 <= SurrChg_[Year]);
    // For the nonce, CSVBoost() is netted against surrender charge.
    // This class's implementation should be revised to distinguish
    // these additive and subtractive components of CSV.
    return SurrChg_[Year] - CSVBoost();
}

/// Cash value augmentation--like a negative surrender charge.
///
/// INPUT !! Probably the input field should be expunged.

currency AccountValue::CSVBoost() const
{
    if(!AllowCashValueEnh) return C0;
    double const z =
          CashValueEnhMult[Year]
        + yare_input_.CashValueEnhancementRate[Year]
        ;
    LMI_ASSERT(0.0 <= z);
    return round_minutiae().c(z * std::max(C0, TotalAccountValue()));
}

//============================================================================
/// Amounts such as claims and account value released on death are
/// multiplied by the beginning-of-year inforce factor when a
/// composite is produced; it would be incorrect to multiply them by
/// the inforce factor here because individual-cell ledgers do not
/// reflect partial mortality. This calculation assumes that partial
/// mortality is curtate.
///
/// It is assumed that the death benefit is sufficient to repay the
/// loan and loan interest--or, at least, that no company would
/// attempt to recover any excess.
///
/// material_difference() is used to subtract amounts that may be
/// materially equal--and, for consistency, in parallel cases where
/// the difference is unlikely to be nearly zero.
///
/// TODO ?? This function is designed to be called at the end of a
/// year, when 'DBReflectingCorr' probably equals the death benefit as
/// of the beginning of the twelfth month, but its end-of-year value
/// (as of the end of the twelfth month) is needed--so the death
/// benefit is updated explicitly. This seems inelegant, and is an
/// obstacle to showing amounts affected by partial mortality on the
/// monthly-detail report.

void AccountValue::SetClaims()
{
    if(!yare_input_.UsePartialMortality || ItLapsed || BasicValues::GetLength() <= Year)
        {
        return;
        }

    TxSetDeathBft();
    TxSetTermAmt();

    YearsGrossClaims       = partial_mortality_qx()[Year] * DBReflectingCorr;
    YearsAVRelOnDeath      = partial_mortality_qx()[Year] * TotalAccountValue();
    YearsLoanRepaidOnDeath = partial_mortality_qx()[Year] * (RegLnBal + PrfLnBal);
    YearsDeathProceeds = material_difference
        (YearsGrossClaims
        ,YearsLoanRepaidOnDeath
        );
    YearsNetClaims = material_difference
        (YearsGrossClaims
        ,YearsAVRelOnDeath
        );
}

/// Post year-end results to ledger.
///
/// This function is called only if the contract is in force at the
/// end of the year. One might alternatively post a partial year's
/// results in the year of lapse; perhaps the greatest benefit would
/// be a more comprehensive composite. Apparently, in 2008, the most
/// common industry practice is:
///  - for composites, truncate each cell as at the beginning of its
///    lapse year;
///  - for individual illustrations, either print nothing for the year
///    of lapse, or print the whole annualized planned premium, with
///    an asterisk pointing to a footnote.
/// If composites were changed to bring lapsing contracts into an
/// account-value rollforward, it would be necessary to show only
/// payments made prior to lapse, and to limit each charge to the
/// available account value; but that would introduce an inconsistency
/// with prevailing practice. It is not unreasonable to follow common
/// practice by defining a composite as the weighted sum of individual
/// illustrations, which show completed years only.

void AccountValue::FinalizeYear()
{
    VariantValues().TotalLoanBalance[Year] = centize(RegLnBal + PrfLnBal);

    currency total_av = TotalAccountValue();
    currency surr_chg = SurrChg();
    currency csv_net =
          total_av
        - (RegLnBal + PrfLnBal)
        ;

    // While performing a solve, ignore any sales-load refund, because
    // it wouldn't prevent the contract from lapsing.
    if(!Solving)
        {
        csv_net += GetRefundableSalesLoad();
        }

    // While performing a solve, ignore any positive surrender charge
    // that cannot cause the contract to lapse.
    if(Solving && C0 < surr_chg && LapseIgnoresSurrChg)
        {
        // Do nothing.
        }
    else
        {
        csv_net -= surr_chg;
        }

    csv_net = std::max(csv_net, HoneymoonValue);

    if(!Solving)
        {
        csv_net = std::max(csv_net, C0);
        }

    if(Solving)
        {
        YearlyTaxBasis[Year] = TaxBasis;
        }

    // 7702(f)(2)(A)
    currency cv_7702 =
          total_av
        + GetRefundableSalesLoad()
        ;
    // Increase by negative surrender charge. If some components of
    // the surrender charge are negative while others are positive,
    // consider only their sum here, instead of considering the sign
    // of each component individually.
    if(surr_chg < C0)
        {
        cv_7702 -= surr_chg;
        }
    cv_7702 = std::max(cv_7702, HoneymoonValue);

    VariantValues().AcctVal     [Year] = centize(total_av);
    VariantValues().AVGenAcct   [Year] = centize(AVGenAcct + AVRegLn + AVPrfLn);
    VariantValues().AVSepAcct   [Year] = centize(AVSepAcct);
    VariantValues().DacTaxRsv   [Year] = 100.0 * DacTaxRsv;
    VariantValues().CSVNet      [Year] = centize(csv_net);
    VariantValues().CV7702      [Year] = centize(cv_7702);

    InvariantValues().Dcv       [Year] = centize(Dcv);

    // Update death benefit. 'DBReflectingCorr' currently equals the
    // death benefit as of the beginning of the twelfth month, but its
    // end-of-year value (as of the end of the twelfth month) is
    // needed.

    TxSetDeathBft();
    TxSetTermAmt();
    // post values to LedgerVariant
    InvariantValues().TermSpecAmt   [Year] = centize(TermSpecAmt);
    VariantValues().TermPurchased   [Year] = centize(TermDB);
    // Add term rider DB
    VariantValues().BaseDeathBft    [Year] = centize(DBReflectingCorr);
    VariantValues().EOYDeathBft     [Year] = centize(DBReflectingCorr + TermDB);

    // Monthly deduction detail

    VariantValues().COICharge         [Year] = centize(YearsTotalCoiCharge)   ;
    VariantValues().RiderCharges      [Year] = centize(YearsTotalRiderCharges);
    VariantValues().AVRelOnDeath      [Year] = 100.0 * YearsAVRelOnDeath      ;
    VariantValues().ClaimsPaid        [Year] = 100.0 * YearsGrossClaims       ;
    VariantValues().DeathProceedsPaid [Year] = 100.0 * YearsDeathProceeds     ;
    VariantValues().NetClaims         [Year] = 100.0 * YearsNetClaims         ;
    VariantValues().NetIntCredited    [Year] = centize(YearsTotalNetIntCredited);
    VariantValues().GrossIntCredited  [Year] = centize(YearsTotalGrossIntCredited);
    VariantValues().LoanIntAccrued    [Year] = centize(YearsTotalLoanIntAccrued);
    VariantValues().PolicyFee         [Year] = centize(YearsTotalPolicyFee)   ;
    VariantValues().DacTaxLoad        [Year] = 100.0 * YearsTotalDacTaxLoad   ;
    VariantValues().SpecAmtLoad       [Year] = centize(YearsTotalSpecAmtLoad) ;
    VariantValues().PremTaxLoad       [Year] = 100.0 * PremiumTax_->ytd_load();

    currency notional_sep_acct_charge =
          YearsTotalSepAcctLoad
        + YearsTotalGrossIntCredited
        - YearsTotalNetIntCredited
        ;
    VariantValues().SepAcctCharges    [Year] = centize(notional_sep_acct_charge);

    // Record dynamic interest rate in ledger object.
    //
    // TODO ?? Actually, 'YearsSepAcctIntRate' holds the dynamic rate
    // for the current month--here, the year's last month. It would be
    // more accurate to use an average across twelve months.
    //
    // TODO ?? The annual rate is calculated in the interest-rate
    // class's dynamic callback function; it would be better to pass
    // it here, instead of invoking a costly transcendental function
    // that loses accuracy.
    //
    // TODO ?? The rates this code records are static, not dynamic.
    //
    if(MandEIsDynamic)
        {
        VariantValues().RecordDynamicSepAcctRate
            (InterestRates_->SepAcctNetRate
                (SepBasis_
                ,GenBasis_
                ,mce_annual_rate
                )
                [Year]
            ,Year
            );
        }

    VariantValues().NetPmt[Year] = centize(std::accumulate
        (NetPmts.begin()
        ,NetPmts.end()
        ,-YearsTotalGptForceout
        ));

    if(mce_run_gen_curr_sep_full == RunBasis_)
        {
        // This is just a temporary kludge. Apparently /Init*/ members
        // exist only as an XSL-FO legacy, and the whole
        //   $git grep -h '\<Init[A-Z]' ledger_invariant.hpp
        // family should be eliminated.
        InvariantValues().InitBaseSpecAmt = InvariantValues().SpecAmt[0];
        InvariantValues().InitTermSpecAmt = InvariantValues().TermSpecAmt[0];

        InvariantValues().GrossPmt  [Year]  = 0.0;
        InvariantValues().EeGrossPmt[Year]  = 0.0;
        InvariantValues().ErGrossPmt[Year]  = 0.0;

        // TODO ?? TAXATION !! This is a temporary workaround until we do it right.
        // Forceouts should be a distinct component, passed separately
        // to ledger values. Probably we should treat 1035 exchanges
        // and NAAR 'forceouts' the same way.
        InvariantValues().GrossPmt  [Year]  -= centize(YearsTotalGptForceout);
        InvariantValues().EeGrossPmt[Year]  -= centize(YearsTotalGptForceout);

        for(int j = 0; j < 12; ++j)
            {
            assert_pmts_add_up(__FILE__, __LINE__, j);
            InvariantValues().GrossPmt  [Year]  += centize(GrossPmts  [j]);
            InvariantValues().EeGrossPmt[Year]  += centize(EeGrossPmts[j]);
            InvariantValues().ErGrossPmt[Year]  += centize(ErGrossPmts[j]);
            }
        if(0 == Year)
            {
            InvariantValues().InitPrem = InvariantValues().GrossPmt[Year];
            }
        LMI_ASSERT
            (materially_equal
                (   InvariantValues().GrossPmt  [Year]
                ,   InvariantValues().EeGrossPmt[Year]
                +   InvariantValues().ErGrossPmt[Year]
                )
            );
        InvariantValues().Outlay[Year] =
                InvariantValues().GrossPmt   [Year]
            -   InvariantValues().NetWD      [Year]
            -   InvariantValues().NewCashLoan[Year]
            ;

        InvariantValues().GptForceout[Year] = centize(YearsTotalGptForceout);

// SOMEDAY !! Not yet implemented.
//        InvariantValues().NaarForceout[Year] = InvariantValues().ErGrossPmt[Year];
        }
}

//============================================================================
void AccountValue::SetAnnualInvariants()
{
    YearsCorridorFactor     = GetCorridorFactor()[Year];
    YearsDBOpt              = DeathBfts_->dbopt()[Year];
    YearsMonthlyPolicyFee   = Loads_->monthly_policy_fee(GenBasis_)[Year];
    YearsAnnualPolicyFee    = Loads_->annual_policy_fee (GenBasis_)[Year];

    YearsGenAcctIntRate     = InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;
    YearsSepAcctIntRate     = InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;
    YearsSepAcctGrossRate = 0.0;
    if(database().query<bool>(DB_AllowSepAcct) && mce_gen_mdpt != GenBasis_)
        {
        YearsSepAcctGrossRate   = InterestRates_->SepAcctGrossRate
            (SepBasis_
            ,mce_monthly_rate
            )
            [Year]
            ;
        }

    YearsDcvIntRate         = i7702_->ic_usual()[Year];
    YearsHoneymoonValueRate = InterestRates_->HoneymoonValueRate
        (GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;
    YearsPostHoneymoonGenAcctIntRate = InterestRates_->PostHoneymoonGenAcctRate
        (GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;

    YearsRegLnIntCredRate   = InterestRates_ ->RegLnCredRate
        (GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;
    YearsPrfLnIntCredRate   = InterestRates_ ->PrfLnCredRate
        (GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;
    YearsRegLnIntDueRate    = InterestRates_ ->RegLnDueRate
        (GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;
    YearsPrfLnIntDueRate    = InterestRates_ ->PrfLnDueRate
        (GenBasis_
        ,mce_monthly_rate
        )
        [Year]
        ;

    YearsCoiRate0           = MortalityRates_->MonthlyCoiRatesBand0(GenBasis_)[Year];
    YearsCoiRate1           = MortalityRates_->MonthlyCoiRatesBand1(GenBasis_)[Year];
    YearsCoiRate2           = MortalityRates_->MonthlyCoiRatesBand2(GenBasis_)[Year];
    YearsDcvCoiRate         = GetMlyDcvqc                          ()         [Year];
    YearsAdbRate            = MortalityRates_->AdbRates            ()         [Year];
    YearsTermRate           = MortalityRates_->MonthlyTermCoiRates (GenBasis_)[Year];
    YearsWpRate             = MortalityRates_->WpRates             ()         [Year];
    YearsSpouseRiderRate    = MortalityRates_->SpouseRiderRates    (GenBasis_)[Year];
    YearsChildRiderRate     = MortalityRates_->ChildRiderRates     ()         [Year];

    YearsTotLoadTgt         = Loads_->target_total_load     (GenBasis_)[Year];
    YearsTotLoadExc         = Loads_->excess_total_load     (GenBasis_)[Year];
    // TAXATION !! This 'lowest_premium_tax' approach needs to be
    // reworked: there should be an option (at least) to use the
    // current tax rates.
    YearsTotLoadTgtLowestPremtax = Loads_->target_premium_load_minimum_premium_tax()[Year];
    YearsTotLoadExcLowestPremtax = Loads_->excess_premium_load_minimum_premium_tax()[Year];
    YearsPremLoadTgt         = Loads_->target_premium_load    (GenBasis_)[Year];
    YearsPremLoadExc         = Loads_->excess_premium_load    (GenBasis_)[Year];
    YearsSalesLoadTgt        = Loads_->target_sales_load      (GenBasis_)[Year];
    YearsSalesLoadExc        = Loads_->excess_sales_load      (GenBasis_)[Year];
    YearsSpecAmtLoadRate     = Loads_->specified_amount_load  (GenBasis_)[Year];
    YearsSepAcctLoadRate     = Loads_->separate_account_load  (GenBasis_)[Year];
    YearsSalesLoadRefundRate = Loads_->refundable_sales_load_proportion()[Year];
    YearsDacTaxLoadRate      = Loads_->dac_tax_load                    ()[Year];
}

/// Separate-account assets, after deductions, times survivorship.
///
/// Returns a currency value because assets are ordinarily thought of
/// as currency, and because this function is used only where currency
/// is wanted--even though it's multiplied by a survivorship factor.

currency AccountValue::GetSepAcctAssetsInforce() const
{
    if(ItLapsed || BasicValues::GetLength() <= Year)
        {
        return C0;
        }

    return round_minutiae().c(SepAcctValueAfterDeduction * partial_mortality_lx()[Year]);
}

/// Beginning of year inforce lives, reflecting lapses and survivorship.

double AccountValue::InforceLivesBoy() const
{
    bool const b {ItLapsed || BasicValues::GetLength() <= Year};
    return b ? 0.0 : partial_mortality_lx().at(Year);
    // If EOY rather than BOY is wanted, add one to 'Year':
//  return b ? 0.0 : partial_mortality_lx().at(1 + Year);
}

//============================================================================
void AccountValue::CoordinateCounters()
{
    MonthsSinceIssue = Month + 12 * Year;

    if(daily_interest_accounting)
        {
        calendar_date current_anniversary = add_years
            (yare_input_.EffectiveDate
            ,Year
            ,true
            );
        calendar_date next_anniversary = add_years
            (yare_input_.EffectiveDate
            ,1 + Year
            ,true
            );
        // This alternative
        //   days_in_policy_year = current_anniversary.days_in_year();
        // is not used: a policy issued in a leap year after its leap
        // day is unaffected by that leap day in the approximate
        // algorithm used here.
        days_in_policy_year =
                next_anniversary.julian_day_number()
            -   current_anniversary.julian_day_number()
            ;
        LMI_ASSERT(365 <= days_in_policy_year && days_in_policy_year <= 366);

        calendar_date current_monthiversary = add_years_and_months
            (yare_input_.EffectiveDate
            ,Year
            ,Month
            ,true
            );
        calendar_date next_monthiversary = add_years_and_months
            (yare_input_.EffectiveDate
            ,Year
            ,1 + Month
            ,true
            );
        days_in_policy_month =
                next_monthiversary.julian_day_number()
            -   current_monthiversary.julian_day_number()
            ;

        LMI_ASSERT(28 <= days_in_policy_month && days_in_policy_month <= 31);

//        LMI_ASSERT
//            (   days_in_policy_month
//            ==  current_monthiversary.days_in_month()
//            );
/*
// This would be wrong for ill-conditioned effective dates.
// Example:
//   effective date 2000-03-29
//   tenth    monthiversary 2001-01-29
//   eleventh monthiversary 2001-02-28 (not the twenty-ninth)
        if(days_in_policy_month !=  current_monthiversary.days_in_month())
            {
            alarum()
                << Year << " Year; " << Month << " Month\n"
                << current_monthiversary.year() << '-'
                << current_monthiversary.month() << '-'
                << current_monthiversary.day() << " calendar date BOM\n"
                << days_in_policy_month << " days_in_policy_month does not equal\n"
                << current_monthiversary.days_in_month() << " current_monthiversary.days_in_month()\n"
                << LMI_FLUSH
                ;
            }
*/
        }
}
