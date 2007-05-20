// Distinct and composite values for cells in a group.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: group_values.cpp,v 1.62 2007-05-20 18:36:20 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "group_values.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "configurable_settings.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "file_command.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"
#include "input.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "ledger_xsl.hpp"
#include "materially_equal.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"
#include "progress_meter.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm> // std::max()
#include <ios>
#include <iostream>
#include <iterator>  // std::back_inserter()

namespace
{
bool cell_should_be_ignored(IllusInputParms const& cell)
{
    return
            0     == value_cast<int>(cell["NumberOfIdenticalLives"].str())
        ||  "Yes" !=                 cell["IncludeInComposite"    ].str()
        ;
}

double emit_ledger
    (fs::path const& file
    ,int             index
    ,Ledger const&   ledger
    ,enum_emission   emission
    )
{
    Timer timer;
    if(emission & e_emit_pdf_to_printer)
        {
        std::string pdf_out_file = write_ledger_to_pdf
            (ledger
            ,serialized_file_path(file, index, "ill").string()
            );
        file_command()(pdf_out_file, "print");
        }
    if(emission & e_emit_test_data)
        {
        fs::ofstream ofs
            (serialized_file_path(file, index, "test")
            ,ios_out_trunc_binary()
            );
        ledger.Spew(ofs);
        }
    if(emission & e_emit_spreadsheet)
        {
        PrintFormTabDelimited
            (ledger
            ,   file.string()
            +   configurable_settings::instance().spreadsheet_file_extension()
            );
        }
    if(emission & e_emit_text_stream)
        {
        PrintLedgerFlatText(ledger, std::cout);
        }

    return timer.stop().elapsed_usec();
}
} // Unnamed namespace.

// Functors run_census_in_series and run_census_in_parallel aren't in
// an unnamed namespace because that would make it difficult to
// implement friendship.

// TODO ?? Maintain timings as part of functor state, instead of
// merely writing them to std::cerr (an odd practice anyway, intended
// only to mimic obsolete code).

class run_census_in_series
{
  public:
    explicit run_census_in_series()
        {}

    bool operator()
        (fs::path const&                     file
        ,enum_emission                       emission
        ,std::vector<IllusInputParms> const& cells
        ,Ledger&                             composite
        );
};

class run_census_in_parallel
{
  public:
    explicit run_census_in_parallel()
        {}

    bool operator()
        (fs::path const&                     file
        ,enum_emission                       emission
        ,std::vector<IllusInputParms> const& cells
        ,Ledger&                             composite
        );
};

// TODO ?? Rethink placement of try blocks. Why have them at all?
// How did they behave in the old production system? (It just rethrew.)
//
// But leave them alone for now. They turn exceptions into fatal_errors,
// which, for now at least, are handled much more gracefully. Yet perhaps
// they should be moved into run_census.

//============================================================================
bool run_census_in_series::operator()
    (fs::path const&                     file
    ,enum_emission                       emission
    ,std::vector<IllusInputParms> const& cells
    ,Ledger&                             composite
    )
{
    bool completed_normally = true;
    double usec_for_calculations = 0.0;
    double usec_for_output       = 0.0;
    Timer timer;
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (cells.size()
            ,"Calculating all cells"
            )
        );

    for(unsigned int j = 0; j < cells.size(); ++j)
        {
        try
            {
            if(cell_should_be_ignored(cells[j]))
                {
                continue;
                }
/*
// TODO ?? Rethink. Old code set debug filename to base.debug .
// TODO ?? Why use class IllusVal at all? What's the advantage over
// the account-value class?
            IllusVal IV(serialized_file_path(file, j, "MISTAKE").string());
            IV.Run(cells[j]);
            composite.PlusEq(IV.ledger());
*/
            AccountValue av(cells[j]);
            av.SetDebugFilename
                (serialized_file_path(file, j, "debug").string()
                );
            av.RunAV();
            composite.PlusEq(*av.ledger_from_av());
            usec_for_output += emit_ledger
                (file
                ,j
                ,*av.ledger_from_av()
                ,emission
                );
            }
        catch(...)
            {
            report_exception();
            }

        if(!meter->reflect_progress())
            {
            completed_normally = false;
            goto done;
            }
        }

    usec_for_output += emit_ledger
        (file
        ,-1
        ,composite
        ,emission
        );

  done:
    double total_usec = timer.stop().elapsed_usec();
    status() << Timer::elapsed_msec_str(total_usec) << std::flush;
    usec_for_calculations = total_usec - usec_for_output;
    if(e_emit_timings & emission)
        {
        std::cerr
            << "    Calculations: "
            << Timer::elapsed_msec_str(usec_for_calculations)
            << '\n'
            << "    Output:       "
            << Timer::elapsed_msec_str(usec_for_output)
            << '\n'
            ;
        }
    return completed_normally;
}

//============================================================================

/// Illustrations with group experience rating
///
/// Mortality profit,
///   accumulated (net mortality charges - net claims) - IBNR,
/// is amortized into future mortality charges by applying a k factor
/// to COI rates. This profit accumulates in the general account at
/// a special input gross rate that's notionally similar to a LIBOR
/// rate; optionally, the separate-account rate may be used, but the
/// reserve is nonetheless still held in the general account. This is
/// a life-insurance reserve; it does not affect a certificate's CSV
/// or 7702 corridor.
///
/// Yearly totals (without monthly interest) of monthly values of the
/// accumulands are accumulated at annual interest. Treating mortality
/// charges as though they were deducted at the end of the year is
/// consistent with curtate partial mortality, though not with normal
/// monthiversary processing. That's all right because this process is
/// self correcting and therefore needs no exquisite refinements.
///
/// The current COI rate is the tabular current COI rate times the
/// input current COI multiplier, with all other customary adjustments
/// for substandard, foreign country, etc., but with no adjustment for
/// retention or k factor--yet never to exceed the guaranteed COI rate.
///
/// The actual mortality charge deducted from the account value is
/// loaded for retention, and reflects experience through the k factor.
/// The net mortality charge is whatever remains after subtracting the
/// retention charge from the actual mortality charge.
///
///   actual mortality charge = NAAR * min(G, C * (R + K))
///   retention charge        = NAAR *        C *  R
///   net mortality charge = actual mortality charge - retention charge
///
/// where C is the current COI rate defined above, R is the retention
/// rate, K is the k factor, and NAAR is by convention nonnegative.
///
/// Database entity 'UseRawTableForRetention' optionally causes R to be
/// divided by the input current COI multiplier, removing the latter
/// from the retention calculation; in that case, retention becomes
/// zero whenever the input current COI multiplier is zero.
///
/// Net claims = partial mortality rate times (DB - AV).
///
/// IBNR (incurred but not reported reserve) is zero on the issue date;
/// on each anniversary, it becomes
///   the past twelve months' total net mortality charges, times
///   one-twelfth (to get a monthly average), times
///   the number of months given in database entity ExpRatIBNRMult.
///
/// On the date the projection begins--the issue date for new business,
/// else the inforce date--the k factor is an input scalar. On each
/// anniversary, it becomes
///   1 - (mortality profit / denominator),
/// denominator being the number of years specified in database entity
/// ExpRatAmortPeriod times a proxy for the coming year's mortality
/// charge:
///   the just-completed year's EOY (DB - AV), times
///   the about-to-begin year's COI rate times twelve, times
///   the proportion surviving into the about-to-begin year;
/// except that the k factor is set to
///   0.0 if it would otherwise be less than 0.0, or
///   1.0 if the denominator is zero.
/// Here, EOY AV reflects interest to the last day of the year, and
/// EOY DB reflects EOY AV: thus, they're the values normally printed
/// on an illustration.

bool run_census_in_parallel::operator()
    (fs::path const&                     file
    ,enum_emission                       emission
    ,std::vector<IllusInputParms> const& cells
    ,Ledger&                             composite
    )
{
    bool completed_normally = true;
    double usec_for_calculations = 0.0;
    double usec_for_output       = 0.0;
    Timer timer;

    std::vector<IllusInputParms>::const_iterator ip;
    std::vector<boost::shared_ptr<AccountValue> > cell_values;
    std::vector<boost::shared_ptr<AccountValue> >::iterator i;
    std::vector<e_run_basis> const& RunBases = composite.GetRunBases();
    try
        {
        boost::shared_ptr<progress_meter> meter
            (create_progress_meter
                (cells.size()
                ,"Initializing all cells"
                )
            );
        int j = 0;
        int first_cell_inforce_year  = value_cast<int>((*cells.begin())["InforceYear"].str());
        int first_cell_inforce_month = value_cast<int>((*cells.begin())["InforceMonth"].str());
        cell_values.reserve(cells.size());
        for(ip = cells.begin(); ip != cells.end(); ++ip, ++j)
            {
            if(cell_should_be_ignored(cells[j]))
                {
                continue;
                }

            boost::shared_ptr<AccountValue> av(new AccountValue(*ip));
            av->SetDebugFilename
                (serialized_file_path(file, j, "debug").string()
                );

            cell_values.push_back(av);

            if(std::string::npos != av->Input_->Comments.find("idiosyncrasyZ"))
                {
                av->Debugging = true;
                av->DebugPrintInit();
                }

            if
                (   first_cell_inforce_year  != value_cast<int>((*ip)["InforceYear"].str())
                ||  first_cell_inforce_month != value_cast<int>((*ip)["InforceMonth"].str())
                )
                {
                fatal_error()
                    << "Running census by month untested for inforce"
                    << " with inforce duration varying across cells."
                    << LMI_FLUSH
                    ;
                }

            if("SolveNone" != (*ip)["SolveType"].str())
                {
                fatal_error()
                    << "Running census by month: solves not permitted."
                    << LMI_FLUSH
                    ;
                }

            if(!meter->reflect_progress())
                {
                completed_normally = false;
                goto done;
                }
            }
        if(0 == cell_values.size())
            {
            // Make sure it's safe to dereference cell_values[0] later.
            fatal_error()
                << "No cell with any lives was included in the composite."
                << LMI_FLUSH
                ;
            }
        }
    catch(...)
        {
        report_exception();
        }

    for
        (std::vector<e_run_basis>::const_iterator run_basis = RunBases.begin()
        ;run_basis != RunBases.end()
        ;++run_basis
        )
    try
        {
        for(i = cell_values.begin(); i != cell_values.end(); ++i)
            {
            (*i)->GuessWhetherFirstYearPremiumExceedsRetaliationLimit();
            }
  restart:
        e_basis          expense_and_general_account_basis;
        e_sep_acct_basis separate_account_basis;
        set_separate_bases_from_run_basis
            (*run_basis
            ,expense_and_general_account_basis
            ,separate_account_basis
            );

        // Calculate duration when the youngest life matures.
        int MaxYr = 0;
        for(i = cell_values.begin(); i != cell_values.end(); ++i)
            {
            (*i)->InitializeLife(*run_basis);
            MaxYr = std::max(MaxYr, (*i)->GetLength());
            }

        boost::shared_ptr<progress_meter> meter
            (create_progress_meter
                (MaxYr
                ,run_basis->str().c_str()
                )
            );

        // Variables to support tiering and experience rating.

        double const case_ibnr_months =
            cell_values.front()->ibnr_as_months_of_mortality_charges()
            ;
        double const case_experience_rating_amortization_years =
            cell_values.front()->experience_rating_amortization_years()
            ;

        double case_accum_net_mortchgs = 0.0;
        double case_accum_net_claims   = 0.0;
        double case_k_factor = cells[0].ExperienceRatingInitialKFactor;

        // Experience rating as implemented here uses either a special
        // scalar input rate, or the separate-account rate. Those
        // rates as entered might vary across cells, but there must be
        // only one rate: therefore, use the first cell's rate, and
        // extend its last element if it doesn't have enough values.

        std::vector<double> experience_reserve_rate;
        std::copy
            (cells[0].SepAcctRate.begin()
            ,cells[0].SepAcctRate.end()
            ,std::back_inserter(experience_reserve_rate)
            );
        experience_reserve_rate.resize(MaxYr, experience_reserve_rate.back());
        if(cells[0].OverrideExperienceReserveRate)
            {
            experience_reserve_rate.assign
                (experience_reserve_rate.size()
                ,cells[0].ExperienceReserveRate
                );
            }

        // TODO ?? We don't start at InforceYear, because issue years may
        // differ between cells and we have not coded support for that yet.
        for(int year = 0; year < MaxYr; ++year)
            {
            double projected_net_mortchgs  = 0.0;
            double ytd_net_mortchgs        = 0.0;

            double experience_reserve_annual_u =
                    1.0
                +   experience_reserve_rate[year]
                ;

            for(i = cell_values.begin(); i != cell_values.end(); ++i)
                {
                if((*i)->PrecedesInforceDuration(year, 0))
                    {
                    continue;
                    }
                (*i)->Year = year;
                (*i)->CoordinateCounters();
                (*i)->InitializeYear();
                }

            // Process one month at a time for all cells.
            for(int month = 0; month < 12; ++month)
                {
                double assets = 0.0;

                // Get total case assets prior to interest crediting because
                // those assets may determine the M&E charge.

                // Process transactions through monthly deduction.
                for(i = cell_values.begin(); i != cell_values.end(); ++i)
                    {
                    if((*i)->PrecedesInforceDuration(year, month))
                        {
                        continue;
                        }
                    (*i)->Month = month;
                    (*i)->CoordinateCounters();
                    (*i)->IncrementBOM(year, month, case_k_factor);

                    assets += (*i)->GetSepAcctAssetsInforce();
                    ytd_net_mortchgs += (*i)->GetNetCoiChargeInforce();
                    }

                // Process transactions from int credit through end of month.
                for(i = cell_values.begin(); i != cell_values.end(); ++i)
                    {
                    if((*i)->PrecedesInforceDuration(year, month))
                        {
                        continue;
                        }
                    (*i)->IncrementEOM(year, month, assets, (*i)->CumPmts);
                    }
                }

            bool need_to_restart = false;
            for(i = cell_values.begin(); i != cell_values.end(); ++i)
                {
                if(!(*i)->TestWhetherFirstYearPremiumExceededRetaliationLimit())
                    {
                    need_to_restart = true;
                    }
                }
            if(need_to_restart)
                {
                // To satisfy the popular 'zero-tolerance' attitude toward
                // the goto statement, we could instead reinitialize
                // everything explicitly and decrement the loop counter,
                // but that would be more unnatural.
                for(i = cell_values.begin(); i != cell_values.end(); ++i)
                    {
                    (*i)->DebugRestart
                        ("First-year premium did not meet retaliation limit"
                        " for at least one cell in the group."
                        );
                    }
                goto restart;
                }

            // Perform end of year calculations.

            // Project claims using the partial-mortality rate:
            // it's curtate, so the whole year's claims occur at
            // the end of the last month and no interest
            // adjustment is required.
            //
            // An off-anniversary inforce case generates a full
            // year's claims, which is consistent with curtate
            // mortality.

            double ytd_net_claims = 0.0;
            double eoy_inforce_lives = 0.0;
            for(i = cell_values.begin(); i != cell_values.end(); ++i)
                {
                if((*i)->PrecedesInforceDuration(year, 11))
                    {
                    continue;
                    }
                (*i)->SetClaims();
                (*i)->SetProjectedCoiCharge();
                eoy_inforce_lives += (*i)->InforceLivesEoy();
                (*i)->IncrementEOY(year);
                ytd_net_claims += (*i)->GetCurtateNetClaimsInforce();
                projected_net_mortchgs += (*i)->GetProjectedCoiChargeInforce();
                }

            // Calculate next year's k factor. Do this only for
            // current-expense bases, not as a speed optimization,
            // but rather because experience rating on other bases
            // is undefined.

            case_accum_net_claims *= experience_reserve_annual_u;
            case_accum_net_claims += ytd_net_claims;

            case_accum_net_mortchgs *= experience_reserve_annual_u;
            case_accum_net_mortchgs += ytd_net_mortchgs;

            // Apportion experience-rating reserve uniformly across
            // inforce lives. Previously, it had been apportioned by
            // projected mortality charges; that proved unworkable
            // when a cell lapsed, matured, or failed to have a
            // nonzero NAAR due to a corridor factor of unity. To
            // guard against such problems, the apportioned reserve
            // is summed across cells and asserted materially to
            // equal the original total reserve.

            if
                (   cells[0].UseExperienceRating
                &&  e_currbasis == expense_and_general_account_basis
                &&  0.0 != eoy_inforce_lives
                )
                {
                double case_ibnr =
                        ytd_net_mortchgs
                    *   case_ibnr_months
                    /   12.0
                    ;
                double case_net_mortality_reserve =
                        case_accum_net_mortchgs
                    -   case_accum_net_claims
                    -   case_ibnr
                    ;

                // Current net mortality charge can actually be zero,
                // e.g., when the corridor factor is unity.
                double denominator =
                        case_experience_rating_amortization_years
                    *   projected_net_mortchgs
                    ;
                if(0.0 == denominator)
                    {
                    case_k_factor = 1.0;
                    }
                else
                    {
                    case_k_factor = std::max
                        (0.0
                        ,1.0 - case_net_mortality_reserve / denominator
                        );
                    }

                double case_net_mortality_reserve_checksum = 0.0;
                for(i = cell_values.begin(); i != cell_values.end(); ++i)
                    {
                    if((*i)->PrecedesInforceDuration(year, 11))
                        {
                        continue;
                        }
                    case_net_mortality_reserve_checksum +=
                        (*i)->ApportionNetMortalityReserve
                            (   case_net_mortality_reserve
                            /   eoy_inforce_lives
                            );
                    }
                if
                    (!materially_equal
                        (case_net_mortality_reserve
                        ,case_net_mortality_reserve_checksum
                        )
                    )
                    {
                    fatal_error()
                        << "\nExperience-rating reserve discrepancy in year "
                        << year
                        << ": "
                        << case_net_mortality_reserve
                        << " != "
                        << case_net_mortality_reserve_checksum
                        << LMI_FLUSH
                        ;
                    }
                }

            if(!meter->reflect_progress())
                {
                completed_normally = false;
                goto done;
                }
            } // End for year.

        for(i = cell_values.begin(); i != cell_values.end(); ++i)
            {
            (*i)->FinalizeLife(*run_basis);
            }

        } // End for...try.
    catch(...)
        {
        report_exception();
        }

    for(i = cell_values.begin(); i != cell_values.end(); ++i)
        {
        (*i)->FinalizeLifeAllBases();
        composite.PlusEq(*(*i)->ledger_from_av());
        }

    {
    int j = 0;
    for(i = cell_values.begin(); i != cell_values.end(); ++i, ++j)
        {
        usec_for_output += emit_ledger
            (file
            ,j
            ,*(*i)->ledger_from_av()
            ,emission
            );
        }
    }

    usec_for_output += emit_ledger
        (file
        ,-1
        ,composite
        ,emission
        );

  done:
    double total_usec = timer.stop().elapsed_usec();
    status() << Timer::elapsed_msec_str(total_usec) << std::flush;
    usec_for_calculations = total_usec - usec_for_output;
    if(e_emit_timings & emission)
        {
        std::cerr
            << "    Calculations: "
            << Timer::elapsed_msec_str(usec_for_calculations)
            << '\n'
            << "    Output:       "
            << Timer::elapsed_msec_str(usec_for_output)
            << '\n'
            ;
        }
    return completed_normally;
}

//============================================================================
bool run_census::operator()
    (fs::path const&                     file
    ,enum_emission                       emission
    ,std::vector<IllusInputParms> const& cells
    )
{
    bool completed_normally = true;

    composite_.reset
        (new Ledger
            (cells[0].LedgerType()
            ,100
            ,true
            )
        );

    enum_run_order order = cells[0].RunOrder;
    switch(order)
        {
        case e_life_by_life:
            {
            completed_normally = run_census_in_series()
                (file
                ,emission
                ,cells
                ,*composite_
                );
            }
            break;
        case e_month_by_month:
            {
            completed_normally = run_census_in_parallel()
                (file
                ,emission
                ,cells
                ,*composite_
                );
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << order
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    return completed_normally;
}

//============================================================================
boost::shared_ptr<Ledger const> run_census::composite()
{
    return composite_;
}

//============================================================================
// The run order depends on the first cell's parameters and ignores
// any conflicting input for any individual cell. It might be cleaner
// to offer this field (and certain others) only at the case level.
//
void run_census::assert_consistency
    (IllusInputParms const& case_default
    ,IllusInputParms const& cell
    )
{
    if(case_default.RunOrder != cell.RunOrder)
        {
        fatal_error()
            << "Case-default run order '"
            << case_default.RunOrder
            << "' differs from first cell's run order '"
            << cell.RunOrder
            << "'. Make them consistent before running illustrations."
            << LMI_FLUSH
            ;
        }
}

void run_census::assert_consistency
    (Input const& case_default
    ,Input const& cell
    )
{
    if(case_default["RunOrder"] != cell["RunOrder"])
        {
        fatal_error()
            << "Case-default run order '"
            << case_default["RunOrder"]
            << "' differs from first cell's run order '"
            << cell["RunOrder"]
            << "'. Make them consistent before running illustrations."
            << LMI_FLUSH
            ;
        }
}

