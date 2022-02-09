// Distinct and composite values for cells in a group.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "group_values.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "currency.hpp"
#include "emit_ledger.hpp"
#include "fenv_guard.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "ledgervalues.hpp"
#include "materially_equal.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()
#include "path_utility.hpp"
#include "progress_meter.hpp"
#include "ssize_lmi.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

// Headers required only for dtors of objects held by std::unique_ptr.
#include "death_benefits.hpp"
#include "i7702.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "premium_tax.hpp"

#include <algorithm>                    // max()
#include <iterator>                     // back_inserter()
#include <string>

namespace
{
bool cell_should_be_ignored(Input const& cell)
{
    return
            0     == value_cast<int>(cell["NumberOfIdenticalLives"].str())
        ||  "Yes" !=                 cell["IncludeInComposite"    ].str()
        ;
}

/// Number of seconds to pause between printouts.
///
/// Motivation: lmi sends illustrations to a printer in census order,
/// but end users have complained that they are printed in a different
/// order. Pausing briefly between printouts seems to forestall that
/// problem. Cf.:
///   http://www.traction-software.co.uk/batchprint/kb/KB0027.html
/// Heuristic testing suggests that ten seconds is enough, but two
/// seconds is not.

int intermission_between_printouts(mcenum_emission emission)
{
    configurable_settings const& c = configurable_settings::instance();
    int const pause = c.seconds_to_pause_between_printouts();
    return (emission & mce_emit_pdf_to_printer) ? pause : 0;
}

progress_meter::enum_display_mode progress_meter_mode(mcenum_emission emission)
{
    return (emission & mce_emit_quietly)
        ? progress_meter::e_quiet_display
        : progress_meter::e_normal_display
        ;
}
} // Unnamed namespace.

// Functors run_census_in_series and run_census_in_parallel exist as
// such only to make it easier for class AccountValue to extend its
// friendship to the latter but not the former. They aren't in an
// unnamed namespace because that would make friendship harder to
// implement.

class run_census_in_series
{
  public:
    census_run_result operator()
        (fs::path           const& file
        ,mcenum_emission           emission
        ,std::vector<Input> const& cells
        ,Ledger                  & composite
        );
};

class run_census_in_parallel
{
  public:
    census_run_result operator()
        (fs::path           const& file
        ,mcenum_emission           emission
        ,std::vector<Input> const& cells
        ,Ledger                  & composite
        );
};

census_run_result run_census_in_series::operator()
    (fs::path           const& file
    ,mcenum_emission    const  emission
    ,std::vector<Input> const& cells
    ,Ledger                  & composite
    )
{
    Timer timer;
    census_run_result result;
    std::unique_ptr<progress_meter> meter
        (create_progress_meter
            (lmi::ssize(cells)
            ,"Calculating all cells"
            ,progress_meter_mode(emission)
            )
        );

    ledger_emitter emitter(file, emission);
    result.seconds_for_output_ += emitter.initiate();

    for(int j = 0; j < lmi::ssize(cells); ++j)
        {
        if(!cell_should_be_ignored(cells[j]))
            {
            std::string const name(cells[j]["InsuredName"].str());
            IllusVal IV(serial_file_path(file, name, j, "hastur").string());
            IV.run(cells[j]);
            composite.PlusEq(*IV.ledger());
            result.seconds_for_output_ += emitter.emit_cell
                (serial_file_path(file, name, j, "hastur")
                ,*IV.ledger()
                );
            meter->dawdle(intermission_between_printouts(emission));
            }
        if(!meter->reflect_progress())
            {
            result.completed_normally_ = false;
            goto done;
            }
        }
    meter->culminate();

    result.seconds_for_output_ += emitter.emit_cell
        (serial_file_path(file, "composite", -1, "hastur")
        ,composite
        );
    result.seconds_for_output_ += emitter.finish();

  done:
    double total_seconds = timer.stop().elapsed_seconds();
    status() << Timer::elapsed_msec_str(total_seconds) << std::flush;
    result.seconds_for_calculations_ = total_seconds - result.seconds_for_output_;
    return result;
}

census_run_result run_census_in_parallel::operator()
    (fs::path           const& file
    ,mcenum_emission    const  emission
    ,std::vector<Input> const& cells
    ,Ledger                  & composite
    )
{
    Timer timer;
    census_run_result result;
    std::unique_ptr<progress_meter> meter
        (create_progress_meter
            (lmi::ssize(cells)
            ,"Initializing all cells"
            ,progress_meter_mode(emission)
            )
        );

    ledger_emitter emitter(file, emission);

    std::vector<AccountValue> cell_values;
    std::vector<mcenum_run_basis> const& RunBases = composite.GetRunBases();

    int const first_cell_inforce_year  = value_cast<int>((*cells.begin())["InforceYear"].str());
    int const first_cell_inforce_month = value_cast<int>((*cells.begin())["InforceMonth"].str());
    cell_values.reserve(cells.size());
    int j = 0;
    for(auto const& ip : cells)
        {
        // This condition need be written only once, here, because
        // subsequently 'cell_values' (which reflects the condition)
        // is iterated across instead of 'cells'.
        if(!cell_should_be_ignored(cells[j]))
            {
            { // Begin fenv_guard scope.
            fenv_guard fg;
            cell_values.emplace_back(ip);
            AccountValue& av = cell_values.back();

            std::string const name(cells[j]["InsuredName"].str());
            // Indexing: here, j is an index into cells, not cell_values.
            av.SetDebugFilename
                (serial_file_path(file, name, j, "hastur").string()
                );

            if(contains(av.yare_input_.Comments, "idiosyncrasyZ"))
                {
                av.Debugging = true;
                av.DebugPrintInit();
                }

            if
                (   first_cell_inforce_year  != av.yare_input_.InforceYear
                ||  first_cell_inforce_month != av.yare_input_.InforceMonth
                )
                {
                alarum()
                    << "Running census by month untested for inforce"
                    << " with inforce duration varying across cells."
                    << LMI_FLUSH
                    ;
                }

            if(mce_solve_none != av.yare_input_.SolveType)
                {
                alarum()
                    << "Running census by month: solves not permitted."
                    << LMI_FLUSH
                    ;
                }
            } // End fenv_guard scope.
            }

        if(!meter->reflect_progress())
            {
            result.completed_normally_ = false;
            goto done;
            }

        ++j;
        } // End for.
    meter->culminate();
    if(cell_values.empty())
        {
        // Make sure it's safe to dereference cell_values[0] later.
        alarum()
            << "No cell with any lives was included in the composite."
            << LMI_FLUSH
            ;
        }

    for(auto const& run_basis : RunBases)
        {
        // It seems somewhat anomalous to create and update a GUI
        // progress meter inside this critical calculation section,
        // because it is not entirely inconceivable that doing so
        // might affect the floating-point control word. However,
        // rogue msw dlls that improperly alter the control word
        // seem to do so when they are initially loaded, and any
        // such dll would already have been loaded to support the
        // progress meter used earlier in this function.
        { // Begin fenv_guard scope.
        fenv_guard fg;
        mcenum_gen_basis expense_and_general_account_basis;
        mcenum_sep_basis separate_account_basis;
        set_cloven_bases_from_run_basis
            (run_basis
            ,expense_and_general_account_basis
            ,separate_account_basis
            );

        // Calculate duration when the youngest life matures.
        int MaxYr = 0;
        for(auto& i : cell_values)
            {
            i.InitializeLife(run_basis);
            MaxYr = std::max(MaxYr, i.GetLength());
            }

        meter = create_progress_meter
            (MaxYr - first_cell_inforce_year
            ,mc_str(run_basis)
            ,progress_meter_mode(emission)
            );

        for(int year = first_cell_inforce_year; year < MaxYr; ++year)
            {
            for(auto& i : cell_values)
                {
                // A cell must be initialized at the beginning of any
                // partial inforce year in which it's illustrated.
                if(i.PrecedesInforceDuration(year, 11))
                    {
                    continue;
                    }
                i.Year = year;
                i.CoordinateCounters();
                i.InitializeYear();
                }

            // Process one month at a time for all cells.
            int const inforce_month =
                first_cell_inforce_year == year
                    ? first_cell_inforce_month
                    : 0
                    ;
            for(int month = inforce_month; month < 12; ++month)
                {
                currency assets = C0;

                // Get total case assets prior to interest crediting because
                // those assets may determine the M&E charge.

                // Process transactions through monthly deduction.
                for(auto& i : cell_values)
                    {
                    if(i.PrecedesInforceDuration(year, month))
                        {
                        continue;
                        }
                    i.Month = month;
                    i.CoordinateCounters();
                    i.IncrementBOM(year, month);
                    assets += i.GetSepAcctAssetsInforce();
                    }

                // Process transactions from int credit through end of month.
                for(auto& i : cell_values)
                    {
                    if(i.PrecedesInforceDuration(year, month))
                        {
                        continue;
                        }
                    i.IncrementEOM(year, month, assets, i.CumPmts);
                    }
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

            for(auto& i : cell_values)
                {
                if(i.PrecedesInforceDuration(year, 11))
                    {
                    continue;
                    }
                i.SetClaims();
                i.IncrementEOY(year);
                }

            if(!meter->reflect_progress())
                {
                result.completed_normally_ = false;
                goto done;
                }
            } // End for year.
        meter->culminate();

        for(auto& i : cell_values)
            {
            i.FinalizeLife(run_basis);
            }

        } // End fenv_guard scope.
        } // End for.

    meter = create_progress_meter
        (lmi::ssize(cell_values)
        ,"Finalizing all cells"
        ,progress_meter_mode(emission)
        );
    for(auto& i : cell_values)
        {
        fenv_guard fg;
        i.FinalizeLifeAllBases();
        composite.PlusEq(*i.ledger_from_av());
        if(!meter->reflect_progress())
            {
            result.completed_normally_ = false;
            goto done;
            }
        }
    meter->culminate();

    result.seconds_for_output_ += emitter.initiate();

    meter = create_progress_meter
        (lmi::ssize(cell_values)
        ,"Writing output for all cells"
        ,progress_meter_mode(emission)
        );
    j = 0;
    for(auto const& i : cell_values)
        {
        // Indexing: here, j is an index into cell_values, not cells.
        std::string const name(cells[j]["InsuredName"].str());
        result.seconds_for_output_ += emitter.emit_cell
            (serial_file_path(file, name, j, "hastur")
            ,*i.ledger_from_av()
            );
        meter->dawdle(intermission_between_printouts(emission));
        if(!meter->reflect_progress())
            {
            result.completed_normally_ = false;
            goto done;
            }
        ++j;
        }
    meter->culminate();

    result.seconds_for_output_ += emitter.emit_cell
        (serial_file_path(file, "composite", -1, "hastur")
        ,composite
        );
    result.seconds_for_output_ += emitter.finish();

  done:
    double total_seconds = timer.stop().elapsed_seconds();
    status() << Timer::elapsed_msec_str(total_seconds) << std::flush;
    result.seconds_for_calculations_ = total_seconds - result.seconds_for_output_;
    return result;
}

census_run_result run_census::operator()
    (fs::path           const& file
    ,mcenum_emission    const  emission
    ,std::vector<Input> const& cells
    )
{
    census_run_result result;

    int composite_length = 0;
    for(auto const& i : cells)
        {
        if(!cell_should_be_ignored(i))
            {
            composite_length = std::max(composite_length, i.years_to_maturity());
            }
        }
    // If cell_should_be_ignored() is true for all cells, composite
    // length is appropriately zero.
    composite_.reset
        (::new Ledger
            (composite_length
            ,cells[0].ledger_type()
            ,false
            ,false
            ,true
            )
        );

    // Use the first cell's run order for the entire census, ignoring
    // any conflicting run order for any other cell--which would have
    // been prevented upstream by assert_consistent_run_order().
    switch(yare_input(cells[0]).RunOrder)
        {
        case mce_life_by_life:
            {
            result = run_census_in_series()
                (file
                ,emission
                ,cells
                ,*composite_
                );
            }
            break;
        case mce_month_by_month:
            {
            result = run_census_in_parallel()
                (file
                ,emission
                ,cells
                ,*composite_
                );
            }
            break;
        }

    // Indicate cancellation on the statusbar. This may be of little
    // importance to end users, but is quite helpful for testing.
    //
    // It might seem like a good idea to write this statusbar message
    // in progress_meter::culminate(), but that function is bypassed
    // upon cancellation in this translation unit; and writing it in
    // ~progress_meter() seems to be a poor idea because it may throw.
    //
    if(!result.completed_normally_)
        {
        status() << "Cancelled." << std::flush;
        }
    return result;
}

std::shared_ptr<Ledger const> run_census::composite() const
{
    LMI_ASSERT(composite_.get());
    return composite_;
}
