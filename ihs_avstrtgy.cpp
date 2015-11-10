// Account value: strategy implementation.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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
#include "death_benefits.hpp"
#include "ledger_invariant.hpp"
#include "mortality_rates.hpp"

#include <algorithm>
#include <utility>

/// Set specamt according to selected strategy in a non-solve year.
///
/// Argument 'actual_year' is policy year.
///
/// Argument 'reference_year' specifies which year's premium is the
/// basis for the calculated specamt. Most often, the first-year
/// premium is used: it is usually undesirable to change specamt
/// frequently, and specamt strategies often begin in the first year.
/// The salary-based strategy, however, tracks salary changes: yearly
/// increases are common on such plans, and there's no other reason to
/// enter a non-scalar salary.
///
/// Specamt strategies ignore dumpins and 1035 exchanges. An argument
/// could be made for making adjustments for such extra premiums, but
/// the benefit doesn't seem to justify the extra complexity. The
/// argument is strongest for 7702- and 7702A-based strategies, but
/// there are other and better ways to avoid MECs and GPT problems.
/// The argument is weaker for the target strategy, for which it often
/// makes sense to ignore such extra payments; and accepting the
/// argument for some strategies but not for others would introduce
/// inconsistency in addition to complexity.
///
/// No minimum is imposed here; see PerformSpecAmtStrategy().

double AccountValue::CalculateSpecAmtFromStrategy
    (int                actual_year
    ,int                reference_year
    ,double             explicit_value
    ,mcenum_sa_strategy strategy
    ) const
{
    double annualized_pmt =
            InvariantValues().EeMode[reference_year].value()
          * InvariantValues().EePmt [reference_year]
        +   InvariantValues().ErMode[reference_year].value()
          * InvariantValues().ErPmt [reference_year]
        ;
    switch(strategy)
        {
        case mce_sa_input_scalar:
            {
            return explicit_value;
            }
        case mce_sa_maximum:
            {
            return GetModalSpecAmtMax      (annualized_pmt);
            }
        case mce_sa_target:
            {
            return GetModalSpecAmtTgt      (annualized_pmt);
            }
        case mce_sa_mep:
            {
            return GetModalSpecAmtMinNonMec(annualized_pmt);
            }
        case mce_sa_glp:
            {
            return GetModalSpecAmtGLP      (annualized_pmt);
            }
        case mce_sa_gsp:
            {
            return GetModalSpecAmtGSP      (annualized_pmt);
            }
        case mce_sa_corridor:
            {
            return GetModalSpecAmtCorridor (annualized_pmt);
            }
        case mce_sa_salary:
            {
            return GetModalSpecAmtSalary   (actual_year);
            }
        default:
            {
            fatal_error() << "Case " << strategy << " not found." << LMI_FLUSH;
            throw "Unreachable--silences a compiler diagnostic.";
            }
        }
}

/// Set specamt according to selected strategy, respecting minimum.
///
/// The actual minimum, set elsewhere, is ascertainable only during
/// monthiversary processing because, e.g., it may depend on whether
/// cash value is sufficient to keep a term rider in force.
///
/// For inforce, warn before increasing specamt to the minimum in the
/// first inforce year: either an underwriting exception was made, or
/// input is erroneous; but apply the minimum silently if specamt is
/// to be calculated from a strategy, or if a lower amount is given in
/// any other year (which doesn't represent the present state of the
/// contract, and presumably results from manual editing).

void AccountValue::PerformSpecAmtStrategy()
{
    // Store original input specamt for first inforce year, for
    // comparison below. Using DeathBfts_->specamt() here instead of
    // yare_input_.SpecifiedAmount means that the inforce warning
    // appears only once, because the former is overwritten but the
    // latter is not.
    double const inforce_specamt = DeathBfts_->specamt().at(InforceYear);
    for(int j = 0; j < BasicValues::Length; ++j)
        {
        bool t = yare_input_.TermRider && 0.0 != yare_input_.TermRiderAmount;
        double m = minimum_specified_amount(0 == j, t);
        double explicit_value = DeathBfts_->specamt()[j];
        mcenum_sa_strategy strategy = yare_input_.SpecifiedAmountStrategy[j];
        // Don't override a specamt that's being solved for.
        if
            (
                mce_solve_specamt == yare_input_.SolveType
            &&  yare_input_.SolveBeginYear <= j
            &&  j < std::min(yare_input_.SolveEndYear, BasicValues::Length)
            )
            {
            strategy = mce_sa_input_scalar;
            }
        double z = CalculateSpecAmtFromStrategy(j, 0, explicit_value, strategy);
        DeathBfts_->set_specamt(round_specamt()(std::max(m, z)), j, 1 + j);
        if
            (  j == InforceYear
            && yare_input_.EffectiveDate != yare_input_.InforceAsOfDate
            && mce_sa_input_scalar == strategy
            && inforce_specamt < m
            && !Solving
            )
            {
            warning()
                << "Inforce specified amount "
                << inforce_specamt
                << " increased to the "
                << m
                << " minimum for '"
                << yare_input_.InsuredName
                << "'."
                << std::flush
                ;
            }
        }
}

void AccountValue::PerformSupplAmtStrategy()
{
    for(int j = 0; j < BasicValues::Length; ++j)
        {
        double m = 0.0; // No minimum other than zero is defined.
        double explicit_value = DeathBfts_->supplamt()[j];
        mcenum_sa_strategy strategy = yare_input_.SupplementalAmountStrategy[j];
        double z = CalculateSpecAmtFromStrategy(j, 0, explicit_value, strategy);
        DeathBfts_->set_supplamt(round_specamt()(std::max(m, z)), j, 1 + j);
        }
}

/// Set payment according to selected strategy in a non-solve year.

double AccountValue::DoPerformPmtStrategy
    (mcenum_solve_type                       a_SolveForWhichPrem
    ,mcenum_mode                             a_CurrentMode
    ,mcenum_mode                             a_InitialMode
    ,double                                  a_TblMult
    ,std::vector<double> const&              a_PmtVector
    ,std::vector<mcenum_pmt_strategy> const& a_StrategyVector
    ) const
{
    if(SolvingForGuarPremium)
        {
        return a_PmtVector[Year];
        }

    // Don't override a premium that's being solved for.
    if
        (
            a_SolveForWhichPrem == yare_input_.SolveType
        &&  yare_input_.SolveBeginYear <= Year
        &&  Year < std::min(yare_input_.SolveEndYear, BasicValues::Length)
        )
        {
        return a_PmtVector[Year];
        }

    switch(a_StrategyVector[Year])
        {
        case mce_pmt_input_scalar:
            {
            return a_PmtVector[Year];
            }
        case mce_pmt_minimum:
            {
            if(SplitMinPrem)
                {
                if(UnsplitSplitMinPrem)
                    {
                    return
                          GetModalPremMlyDedEe(Year, a_CurrentMode, TermSpecAmt)
                        + GetModalPremMlyDedEr(Year, a_CurrentMode, ActualSpecAmt)
                        ;
                    }
                if(mce_solve_ee_prem == a_SolveForWhichPrem)
                    {
                    // Normally, ee mode is entered to match ee mode,
                    // which represents the payment mode chosen by the
                    // plan sponsor; but lmi has the extra flexibility
                    // to behave reasonably if it's not so entered.
                    return GetModalPremMlyDedEe(Year, a_CurrentMode, TermSpecAmt);
                    }
                else if(mce_solve_er_prem == a_SolveForWhichPrem)
                    {
                    return GetModalPremMlyDedEr(Year, a_CurrentMode, ActualSpecAmt);
                    }
                else
                    {
                    fatal_error()
                        << "Type "
                        << a_SolveForWhichPrem
                        << " not allowed here."
                        << LMI_FLUSH
                        ;
                    }
                }
            else
                {
                double sa = ActualSpecAmt + TermSpecAmt;
                return GetModalMinPrem(Year, a_CurrentMode, sa);
                }
            }
        case mce_pmt_target:
            {
            int const target_year = TgtPremFixedAtIssue ? 0 : Year;
            double sa = InvariantValues().SpecAmt[target_year];
            return GetModalTgtPrem(Year, a_CurrentMode, sa);
            }
        case mce_pmt_mep:
            {
            double sa =
                                      InvariantValues().SpecAmt    [0]
                + (TermIsDbFor7702A ? InvariantValues().TermSpecAmt[0] : 0.0)
                ;
            return GetModalPremMaxNonMec(0, a_InitialMode, sa);
            }
        case mce_pmt_glp:
            {
            double sa =
                                     InvariantValues().SpecAmt    [0]
                + (TermIsDbFor7702 ? InvariantValues().TermSpecAmt[0] : 0.0)
                ;
            return GetModalPremGLP(0, a_InitialMode, sa, sa);
            }
        case mce_pmt_gsp:
            {
            double sa =
                                     InvariantValues().SpecAmt    [0]
                + (TermIsDbFor7702 ? InvariantValues().TermSpecAmt[0] : 0.0)
                ;
            return GetModalPremGSP(0, a_InitialMode, sa, sa);
            }
        case mce_pmt_corridor:
            {
// TODO ?? Shouldn't this be initial specified amount?
            double sa = ActualSpecAmt + (TermIsDbFor7702 ? TermSpecAmt : 0.0);
            return GetModalPremCorridor(0, a_InitialMode, sa);
            }
        case mce_pmt_table:
            {
            return
                  ActualSpecAmt
                * MortalityRates_->GroupProxyRates()[Year]
                * a_TblMult
                / a_CurrentMode
                ;
            }
        default:
            {
            fatal_error()
                << "Case "
                << a_StrategyVector[Year]
                << " not found."
                << LMI_FLUSH
                ;
            throw "Unreachable--silences a compiler diagnostic.";
            }
        }
}

/// Set employee payment according to selected strategy.

double AccountValue::PerformEePmtStrategy() const
{
    return DoPerformPmtStrategy
        (mce_solve_ee_prem
        ,InvariantValues().EeMode[Year].value()
        ,InvariantValues().EeMode[0]   .value()
        ,yare_input_.InsuredPremiumTableFactor
        ,InvariantValues().EePmt
        ,yare_input_.PaymentStrategy
        );
}

/// Set employer payment according to selected strategy.

double AccountValue::PerformErPmtStrategy() const
{
    return DoPerformPmtStrategy
        (mce_solve_er_prem
        ,InvariantValues().ErMode[Year].value()
        ,InvariantValues().ErMode[0]   .value()
        ,yare_input_.CorporationPremiumTableFactor
        ,InvariantValues().ErPmt
        ,yare_input_.CorporationPaymentStrategy
        );
}

