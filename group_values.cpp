// Distinct and composite values for cells in a group.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
/// monthiversary processing. Both accumulands are zero in a lapse
/// year. These simplifying assumptions are okay because this process
/// is self correcting and therefore needs no exquisite refinements.
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
/// where C is the current COI rate defined above, G is its guaranteed
/// analogue, R is the retention rate, K is the k factor, and NAAR is
/// by convention nonnegative.
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

        // Variables to support tiering and experience rating.

        double const case_ibnr_months =
            cell_values.front().ibnr_as_months_of_mortality_charges()
            ;
        double const case_experience_rating_amortization_years =
            cell_values.front().experience_rating_amortization_years()
            ;

        double case_accum_net_mortchgs = 0.0;
        double case_accum_net_claims   = 0.0;
        double case_k_factor = cell_values.front().yare_input_.ExperienceRatingInitialKFactor;

        // Experience rating as implemented here uses either a special
        // scalar input rate, or the separate-account rate. Those
        // rates as entered might vary across cells, but there must be
        // only one rate: therefore, use the first cell's rate, and
        // extend its last element if it doesn't have enough values.

        std::vector<double> experience_reserve_rate;
        std::copy
            (cell_values.front().yare_input_.SeparateAccountRate.begin()
            ,cell_values.front().yare_input_.SeparateAccountRate.end()
            ,std::back_inserter(experience_reserve_rate)
            );
        experience_reserve_rate.resize(MaxYr, experience_reserve_rate.back());
        if(cell_values.front().yare_input_.OverrideExperienceReserveRate)
            {
            experience_reserve_rate.assign
                (experience_reserve_rate.size()
                ,cell_values.front().yare_input_.ExperienceReserveRate
                );
            }

        for(int year = first_cell_inforce_year; year < MaxYr; ++year)
            {
            double experience_reserve_annual_u =
                    1.0
                +   experience_reserve_rate[year]
                ;

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
                double assets = 0.0;

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
                    i.IncrementBOM(year, month, case_k_factor);
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

            double eoy_inforce_lives      = 0.0;
            double years_net_claims       = 0.0;
            double years_net_mortchgs     = 0.0;
            double projected_net_mortchgs = 0.0;
            for(auto& i : cell_values)
                {
                if(i.PrecedesInforceDuration(year, 11))
                    {
                    continue;
                    }
                i.SetClaims();
                i.SetProjectedCoiCharge();
                eoy_inforce_lives      += i.InforceLivesEoy();
                i.IncrementEOY(year);
                years_net_claims       += i.GetCurtateNetClaimsInforce();
                years_net_mortchgs     += i.GetCurtateNetCoiChargeInforce();
                projected_net_mortchgs += i.GetProjectedCoiChargeInforce();
                }

            // Calculate next year's k factor. Do this only for
            // current-expense bases, not as a speed optimization,
            // but rather because experience rating on other bases
            // is undefined.

            case_accum_net_claims   *= experience_reserve_annual_u;
            case_accum_net_claims   += years_net_claims;

            case_accum_net_mortchgs *= experience_reserve_annual_u;
            case_accum_net_mortchgs += years_net_mortchgs;

            // Presumably an admin system would maintain a scalar
            // reserve instead of tracking claims and mortality
            // charges separately, and accumulate it at interest more
            // frequently than once a year.
            //
            // Therefore, add inforce reserve here, to avoid crediting
            // a year's interest to it. Because only a scalar reserve
            // is captured, it must all be added to one side of the
            // reserve equation: the distinction between claims and
            // mortality charges is lost, but their difference is
            // preserved, so the resulting reserve is correct.
            //
            // The inforce reserve would reflect net claims already
            // paid as well as mortality charges already deducted for
            // any partial year. Therefore, although inforce YTD COI
            // charge is captured separately for adjusting IBNR, it
            // would be incorrect to add it here.

            if(first_cell_inforce_year == year)
                {
                case_accum_net_mortchgs += cell_values.front().yare_input_.InforceNetExperienceReserve;
                }

            // Apportion experience-rating reserve uniformly across
            // inforce lives. Previously, it had been apportioned by
            // projected mortality charges; that proved unworkable
            // when a cell lapsed, matured, or failed to have a
            // nonzero NAAR due to a corridor factor of unity. To
            // guard against such problems, the apportioned reserve
            // is summed across cells and asserted materially to
            // equal the original total reserve.

            if
                (   cell_values.front().yare_input_.UseExperienceRating
                &&  mce_gen_curr == expense_and_general_account_basis
                &&  0.0 != eoy_inforce_lives
                )
                {
                if(first_cell_inforce_year == year)
                    {
                    years_net_mortchgs += cell_values.front().yare_input_.InforceYtdNetCoiCharge;
                    }
                double case_ibnr =
                        years_net_mortchgs
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
                for(auto& i : cell_values)
                    {
                    if(i.PrecedesInforceDuration(year, 11))
                        {
                        continue;
                        }
                    case_net_mortality_reserve_checksum +=
                        i.ApportionNetMortalityReserve
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
                    alarum()
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
        (new Ledger
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
