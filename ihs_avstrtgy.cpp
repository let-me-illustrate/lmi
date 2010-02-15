// Account value: strategy implementation.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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
#include "ihs_rnddata.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "mortality_rates.hpp"

#include <algorithm>
#include <utility>

// Set premium or spec amount according to fixed relationships
// e.g. if prem is target for given spec amt, then given either
// we can determine for the other.

// TODO ?? Known defects
//
// Premium and specamt are either literal dollar amounts, or enums indicating
// strategy, encoded by multiplying the enum value by 1.0E100. This is most
// regrettable. Bugs probably lurk here.
//
// There are two functions for specamt strategy; apparently only the one
// called "Old" is used.
//
// Why is VariantValues used? Where it is used, wouldn't it be clearer
// just to get the initial values from BasicValues or InputParms?

//============================================================================
double AccountValue::CalculateSpecAmtFromStrategy
    (int actual_year
    ,int reference_year
    ) const
{
    double z = 0.0;
    switch(yare_input_.SpecifiedAmountStrategy[actual_year])
        {
        case mce_sa_salary:
            {
            // This ignores yearly-varying salary.
            double y;
            y = yare_input_.ProjectedSalary[actual_year] * yare_input_.SalarySpecifiedAmountFactor;
            if(0.0 != yare_input_.SalarySpecifiedAmountCap)
                {
                y = std::min(y, yare_input_.SalarySpecifiedAmountCap);
                }
            y -= yare_input_.SalarySpecifiedAmountOffset;
            z = y;
            }
            break;
        case mce_sa_input_scalar:
            {
            z = DeathBfts_->specamt()[actual_year];
            }
            break;
        case mce_sa_maximum:
            {
            z = GetModalSpecAmtMax
                (InvariantValues().EeMode[reference_year].value()
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year].value()
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case mce_sa_target:
            {
            z = GetModalSpecAmtTgt
                (InvariantValues().EeMode[reference_year].value()
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year].value()
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
// TODO ?? The following strategies (at least) should recognize dumpins.
        case mce_sa_mep:
            {
            z = GetModalSpecAmtMinNonMec
                (InvariantValues().EeMode[reference_year].value()
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year].value()
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case mce_sa_corridor:
            {
            z = GetModalSpecAmtCorridor
                (InvariantValues().EeMode[reference_year].value()
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year].value()
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case mce_sa_glp:
            {
            z = GetModalSpecAmtGLP
                (InvariantValues().EeMode[reference_year].value()
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year].value()
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case mce_sa_gsp:
            {
            z = GetModalSpecAmtGSP
                (InvariantValues().EeMode[reference_year].value()
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year].value()
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        default:
            {
            fatal_error()
                << "Case "
                << yare_input_.SpecifiedAmountStrategy[actual_year]
                << " not found."
                << LMI_FLUSH
                ;
            }
        }
    return z;
}

/// Set spec amt according to selected strategy, in every year.

void AccountValue::PerformSpecAmtStrategy()
{
    for
        (int j = 0; j < BasicValues::Length; ++j)
        {
        double z = round_specamt(CalculateSpecAmtFromStrategy(j, 0));
        DeathBfts_->set_specamt(z, j, 1 + j);
        }
}

//============================================================================
// Sets payment according to selected strategy, in each non-solve year
double AccountValue::DoPerformPmtStrategy
    (mcenum_solve_type                       a_SolveForWhichPrem
    ,mcenum_mode                             a_CurrentMode
    ,mcenum_mode                             a_InitialMode
    ,double                                  a_TblMult
    ,std::vector<double> const&              a_PmtVector
    ,std::vector<mcenum_pmt_strategy> const& a_StrategyVector
    ) const
{
    // TODO ?? What happens if a corporation payment is specified?
    if(SolvingForGuarPremium)
        {
        return a_PmtVector[Year];
        }

    // Don't override premium during premium solve period.
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
            return GetModalMinPrem
                (Year
                ,a_CurrentMode
                ,ActualSpecAmt + TermSpecAmt
                );
            }
        case mce_pmt_target:
            {
// TODO ?? Shouldn't a modalized version of something like
//   InitialTargetPremium
//   AnnualTargetPrem
// be used instead, at least in the
//       if(Database_->Query(DB_TgtPmFixedAtIssue))
// case?
            return GetModalTgtPrem
                (Year
                ,a_CurrentMode
                ,ActualSpecAmt
                );
            }
        case mce_pmt_mep:
            {
// TODO ?? This assumes that the term rider continues to at least age 95.
// We ought to have a database flag for that.
            return GetModalPremMaxNonMec
                (0
                ,a_InitialMode
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                );
            }
        case mce_pmt_glp:
            {
            return GetModalPremGLP
                (0
                ,a_InitialMode
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                );
            }
        case mce_pmt_gsp:
            {
            return GetModalPremGSP
                (0
                ,a_InitialMode
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                );
            }
        case mce_pmt_table:
            {
            return
                ActualSpecAmt
                * MortalityRates_->TableYRates()[Year]
                * (12.0 / a_CurrentMode)
                * a_TblMult;
            }
        case mce_pmt_corridor:
            {
// TODO ?? This assumes that the term rider continues to at least age 95.
// We ought to have a database flag for that.
            return GetModalPremCorridor
                (0
                ,a_InitialMode
// TODO ?? Shouldn't this be initial specified amount?
                ,ActualSpecAmt
// TODO ?? This may be wanted for an 'integrated' term rider.
//                ,ActualSpecAmt + TermSpecAmt
                );
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

//============================================================================
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

//============================================================================
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

