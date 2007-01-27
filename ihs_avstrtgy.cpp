// Account value: strategy implementation.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: ihs_avstrtgy.cpp,v 1.8 2007-01-27 00:00:51 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include "alert.hpp"
#include "death_benefits.hpp"
#include "ihs_rnddata.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
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
    switch(Input_->VectorSpecifiedAmountStrategy[actual_year])
        {
        case e_sasalary:
            {
            // This ignores yearly-varying salary.
            double y;
            y = Input_->Salary[actual_year] * Input_->SalarySAPct;
            if(0.0 != Input_->SalarySACap)
                {
                y = std::min(y, Input_->SalarySACap.value());
                }
            y -= Input_->SalarySAOffset;
            z = y;
            }
            break;
        case e_sainputscalar:
            {
            z = DeathBfts_->specamt()[actual_year];
            }
            break;
        case e_sainputvector:
            {
            // TODO ?? It's somewhat frightening that this warning is
            // never displayed, even though the test deck contains
            // cases with yearly varying spec amt.
            fatal_error()
                << "Varying specified amount not implemented."
                << " Specified amount set to scalar input value."
                << LMI_FLUSH
                ;
            z = DeathBfts_->specamt()[actual_year];
            }
            break;
        case e_samaximum:
            {
            z = GetModalSpecAmtMax
                (InvariantValues().EeMode[reference_year]
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year]
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case e_satarget:
            {
            z = GetModalSpecAmtTgt
                (InvariantValues().EeMode[reference_year]
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year]
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
// TODO ?? The following strategies (at least) should recognize dumpins.
        case e_samep:
            {
            z = GetModalSpecAmtMinNonMec
                (InvariantValues().EeMode[reference_year]
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year]
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case e_sacorridor:
            {
            z = GetModalSpecAmtCorridor
                (InvariantValues().EeMode[reference_year]
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year]
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case e_saglp:
            {
            z = GetModalSpecAmtGLP
                (InvariantValues().EeMode[reference_year]
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year]
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        case e_sagsp:
            {
            z = GetModalSpecAmtGSP
                (InvariantValues().EeMode[reference_year]
                ,InvariantValues().EePmt [reference_year]
                ,InvariantValues().ErMode[reference_year]
                ,InvariantValues().ErPmt [reference_year]
                );
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input_->VectorSpecifiedAmountStrategy[actual_year]
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    return z;
}

//============================================================================
void AccountValue::NewPerformSpecAmtStrategy()
{
    for
        (int j = 0; j < Input_->YearsToMaturity(); ++j)
        {
        double z = round_specamt(CalculateSpecAmtFromStrategy(j, 0));
        DeathBfts_->set_specamt(z, j, 1 + j);
        }
}

//============================================================================
// Sets spec amt according to selected strategy, in every year
// TODO ?? Despite the name, this 'Old-' function is still used, but
// the 'New-' variant apparently is not.
void AccountValue::OldPerformSpecAmtStrategy()
{
    NewPerformSpecAmtStrategy();
    if(e_obsolete_same_as_initial == Input_->PostRetType)
        {
        return;
        }

    // TODO ?? Needs work for post-retirement strategy.
    double SA = CalculateSpecAmtFromStrategy(0, 0);

    // Done if no strategy to apply
    if
        (
//            e_sasalary      == Input_->SAStrategy
//        ||  e_sainputscalar == Input_->SAStrategy
        e_sainputscalar == Input_->SAStrategy
        )
        {
        return;
        }

    SA = round_specamt(SA);

    int RetDur = Input_->Status[0].YearsToRetirement();

    // I think we really need to do this here, because
    // DeathBenefits::SpecAmt is used downstream
//  DeathBfts_->set_specamt(SA, 0, BasicValues::Length);
    DeathBfts_->set_specamt(SA, 0, RetDur);
// TODO ?? Error if post-ret is same as pre-ret strategy.

    double postret_spec_amt = 0.0;
    switch(Input_->PostRetType)
        {
        case e_obsolete_same_as_initial:
            {
            postret_spec_amt = SA;
            }
            break;
        case e_obsolete_scalar:
            {
            return;
            }
//          break;
        case e_obsolete_percent_of_initial:
            {
            postret_spec_amt = SA * Input_->PostRetPct;
            }
            break;
        case e_obsolete_varying:
            {
            return;
//            throw std::logic_error
//                (
//                "Internal error: 'impossible' case e_obsolete_varying in "
//                __FILE__
//                );
            }
        default:
            {
            fatal_error()
                << "Case '"
                << Input_->PostRetType
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    DeathBfts_->set_specamt(postret_spec_amt, RetDur, BasicValues::GetLength());
}

//============================================================================
// Sets payment according to selected strategy, in each non-solve year
double AccountValue::DoPerformPmtStrategy
    (e_solve_type const&                a_SolveForWhichPrem
    ,e_mode const&                      a_CurrentMode
    ,e_mode const&                      a_InitialMode
    ,double                             a_TblMult
    ,std::vector<double> const&         a_PmtVector
    ,std::vector<e_pmt_strategy> const& a_StrategyVector
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
            a_SolveForWhichPrem == Input_->SolveType
        &&  Input_->SolveBegYear <= Year
        &&  Year < std::min(Input_->SolveEndYear.value(), BasicValues::Length)
        )
        {
        return a_PmtVector[Year];
        }

    switch(static_cast<enum_pmt_strategy>(a_StrategyVector[Year]))
        {
        case e_pmtinputscalar:
            {
            return a_PmtVector[Year];
            }
        case e_pmtinputvector:
            {
            return a_PmtVector[Year];
            }
        case e_pmtminimum:
            {
            return GetModalMinPrem
                (Year
                ,a_CurrentMode
                ,ActualSpecAmt
                );
            }
        case e_pmttarget:
            {
            return GetModalTgtPrem
                (Year
                ,a_CurrentMode
                ,ActualSpecAmt
                );
            }
        case e_pmtmep:
            {
// TODO ?? This assumes that the term rider continues to at least age 95.
// We ought to have a database flag for that.
            return GetModalPremMaxNonMec
                (0
                ,a_InitialMode
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                );
            }
        case e_pmtglp:
            {
            return GetModalPremGLP
                (0
                ,a_InitialMode
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                );
            }
        case e_pmtgsp:
            {
            return GetModalPremGSP
                (0
                ,a_InitialMode
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                ,InvariantValues().SpecAmt[0] + InvariantValues().TermSpecAmt[0]
                );
            }
        case e_pmttable:
            {
            return
                ActualSpecAmt
                * MortalityRates_->TableYRates()[Year]
                * (12.0 / a_CurrentMode)
                * a_TblMult;
            }
        case e_pmtcorridor:
            {
// TODO ?? This assumes that the term rider continues to at least age 95.
// We ought to have a database flag for that.
            return GetModalPremCorridor
                (0
                ,a_InitialMode
                ,ActualSpecAmt
// TODO ?? This may be wanted for an 'integrated' term rider.
//                ,ActualSpecAmt + TermSpecAmt
                );
            }
        default:
            {
            fatal_error()
                << "Case '"
                << a_StrategyVector[Year]
                << "' not found."
                << LMI_FLUSH
                ;
            return 0; // Bogus return--actually unreachable.
            }
        }
}

//============================================================================
double AccountValue::PerformEePmtStrategy() const
{
    return DoPerformPmtStrategy
        (e_solve_type(e_solve_ee_prem)
        ,InvariantValues().EeMode[Year]
        ,InvariantValues().EeMode[0]
        ,Input_->EePremTableMult
        ,InvariantValues().EePmt
        ,Input_->VectorIndvPaymentStrategy
        );
}

//============================================================================
double AccountValue::PerformErPmtStrategy() const
{
    return DoPerformPmtStrategy
        (e_solve_type(e_solve_er_prem)
        ,InvariantValues().ErMode[Year]
        ,InvariantValues().ErMode[0]
        ,Input_->ErPremTableMult
        ,InvariantValues().ErPmt
        ,Input_->VectorCorpPaymentStrategy
        );
}

