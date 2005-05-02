// Distinct and composite values for cells in a group.
//
// Copyright (C) 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: group_values.cpp,v 1.9 2005-05-02 06:26:48 chicares Exp $

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
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "ledger_xsl.hpp"
#include "ledgervalues.hpp"
#include "path_utility.hpp"
#include "progress_meter.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm> // std::max()

void emit_ledger
    (fs::path                      const& file
    ,int                           index
    ,Ledger const&                 ledger
    ,e_emission_target             emission_target
    )
{
    if(emission_target & emit_to_printer)
        {
        write_ledger_to_pdf
            (ledger
            ,serialized_file_path(file, index, ".ill").string()
            );
        }
    if(emission_target & emit_to_spew_file)
        {
        fs::ofstream ofs
            (serialized_file_path(file, index, "test")
            ,   std::ios_base::in
            |   std::ios_base::binary
            |   std::ios_base::trunc
            );
        ledger.Spew(ofs);
        }
    if(emission_target & emit_to_spreadsheet)
        {
        PrintFormTabDelimited
            (ledger
            ,file.string() + configurable_settings::instance().spreadsheet_file_extension()
            );
        }
}

//============================================================================
void RunCensusInSeries::operator()
    (fs::directory_iterator       const& file
    ,std::vector<IllusInputParms> const& cells
    ,Ledger                            & composite
    )
{
// TODO ?? Rethink placement of try blocks. Why have them at all?
// How did they behave in the old production system? (It just rethrew.)

    Timer timer; // TODO ?? Combine with progress meter?
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (cells.size()
            ,"Calculating all cells"
            )
        );
    for(unsigned int j = 0; j < cells.size(); ++j)
        {
// TODO ?? Skip if cell not included in composite (IncludeInComposite).        
        try
            {
// TODO ?? Rethink. Old code set debug filename to base.debug .
// TODO ?? Why use class IllusVal at all? What's the advantage over the AV class?
            IllusVal IV(serialized_file_path(*file, j, "MISTAKE").string());
            IV.Run(cells[j]);
            composite.PlusEq(IV.ledger());
            emit_ledger
                (*file
                ,j
                ,IV.ledger()
                ,emit_to_spew_file
                );
// TODO ?? Sometimes all cells should be emitted; sometimes, only the composite.
            }
        catch(std::exception& e)
            {
            fatal_error() << "\nCaught exception: " << e.what() << LMI_FLUSH;
            }
        catch(...)
            {
            fatal_error() << "\nUnknown exception." << LMI_FLUSH;
            }
        if(!meter->reflect_progress())
            {
            return;
            }
        }

    emit_ledger
        (*file
        ,-1
        ,composite
        ,emit_to_spew_file
        );

// TODO ?? This is for calculations and output combined.
// Running in parallel permits separating those things--good idea?
    status() << timer.Stop().Report() << std::flush;
}

//============================================================================
void RunCensusInParallel::operator()
    (fs::directory_iterator       const& file
    ,std::vector<IllusInputParms> const& cells
    ,Ledger                            & composite
    )
{
    Timer timer;

// TODO ?? Rename 'AVS'.    
    std::vector<boost::shared_ptr<AccountValue> > AVS;
    std::vector<IllusInputParms>::const_iterator ip;
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
        AVS.reserve(cells.size());
        for(ip = cells.begin(); ip != cells.end(); ++ip, ++j)
            {
            // Skip any cell with zero lives.
            // TODO ?? Should that even be permitted?
            if(0 == value_cast<int>((*ip)["NumberOfIdenticalLives"].str()))
                {
                continue;
                }
            // Skip anyone not included in composite.
            if("Yes" != (*ip)["IncludeInComposite"].str())
                {
                continue;
                }

            boost::shared_ptr<AccountValue> AV(new AccountValue(*ip));
            AV->SetDebugFilename(serialized_file_path(*file, j, "debug").string());

            AVS.push_back(AV);

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
                return;
                }
            }
        }
    catch(std::exception& e)
        {
        fatal_error() << "\nCaught exception: " << e.what() << LMI_FLUSH;
        }
    catch(...)
        {
        fatal_error() << "\nUnknown exception." << LMI_FLUSH;
        }

    std::vector<boost::shared_ptr<AccountValue> >::iterator i;

    std::vector<e_run_basis> const& RunBases = composite.GetRunBases();
    for
        (std::vector<e_run_basis>::const_iterator run_basis = RunBases.begin()
        ;run_basis != RunBases.end()
        ;++run_basis
        )
    try
        {
        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            (*i)->GuessWhetherFirstYearPremiumExceedsRetaliationLimit();
            }
restart:
        // Initialize each cell.
        // Calculate duration when the youngest one ends.
        int MaxYr = 0;
        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            (*i)->InitializeLife(*run_basis);
            MaxYr = std::max(MaxYr, (*i)->GetLength());
            }

    // TODO ?? WANT MONTHLY, NOT YEARLY? Why store it at all?
    // Perhaps use it for individual-cell solves?
        std::vector<double> Assets(MaxYr, 0.0);

        boost::shared_ptr<progress_meter> meter
            (create_progress_meter
                (MaxYr
                ,run_basis->str().c_str()
                )
            );

        // Experience rating mortality reserve.
        double case_accum_net_mortchgs = 0.0;
        double case_accum_net_claims   = 0.0;
        double case_k_factor           = 0.0;

        // TODO ?? Inelegant and probably inefficient.
        TDatabase temp_db(cells[0]);
        double case_ibnr_months = temp_db.Query(DB_ExpRatIBNRMult);

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
            double case_years_net_claims = 0.0;
            double case_years_net_mortchgs = 0.0;
            double projected_net_mortchgs  = 0.0;

            double experience_reserve_annual_u = 1.0 + experience_reserve_rate[year];

            double current_mortchg = 0.0;

            // Process one month at a time for all cells.
            for(int month = 0; month < 12; ++month)
                {
                // Initialize year's assets to zero.
                // TODO ?? Uh--it already is, yearly...but this is monthly.
                // TODO ?? Perhaps we'll want a vector of monthly assets.
                Assets[year] = 0.0;

                // Get total case assets prior to interest crediting because
                // those assets determine the M&E charge.

                // Process transactions through monthly deduction.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    (*i)->Year = year;
                    (*i)->Month = month;
                    (*i)->CoordinateCounters();
                    if((*i)->PrecedesInforceDuration(year, month)) continue;
                    (*i)->IncrementBOM(year, month, case_k_factor);

                    // Add assets and COI charges to case totals.
                    Assets[year] += (*i)->GetSepAcctAssetsInforce();

                    current_mortchg += (*i)->GetLastCOIChargeInforce();
                    }

                // Process transactions from int credit through end of month.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    if((*i)->PrecedesInforceDuration(year, month)) continue;
                    (*i)->IncrementEOM(year, month, Assets[year]);
                    }

                // Project claims using the partial-mortality rate,
                // which is curtate, so the whole year's claims occur
                // at the end of last month and no interest adjustment
                // is required.
                //
                double current_claims = 0.0;
                if(month == 11)
                    {
                    for(i = AVS.begin(); i != AVS.end(); ++i)
                        {
                        (*i)->SetClaims();
// TODO ?? AV released on death was added to the nearly-identical code in
// the account-value class, but not here.
                        current_claims += (*i)->GetCurtateNetClaimsInforce();
                        }

                    case_accum_net_claims *= experience_reserve_annual_u;
                    case_accum_net_claims += current_claims;

                    case_years_net_claims += current_claims;

                    case_accum_net_mortchgs *= experience_reserve_annual_u;
                    case_accum_net_mortchgs += current_mortchg;

                    case_years_net_mortchgs += current_mortchg;
                    }
                }

            bool need_to_restart = false;
            for(i = AVS.begin(); i != AVS.end(); ++i)
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
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    (*i)->DebugRestart
                        ("First-year premium did not meet retaliation limit"
                        " for at least one cell in the group."
                        );
                    }
                goto restart;
                }

            // Perform end of year calculations.

            // Increment year; update curtate inforce factor.

            // TODO ?? Temporary. Were these variables useful only
            // for testing single-life cases?
            double this_years_coi_rate       = 0.0;
            double this_years_part_mort_rate = 0.0;
            double eoy_naar                  = 0.0;
            for(i = AVS.begin(); i != AVS.end(); ++i)
                {
                if((*i)->PrecedesInforceDuration(year, 11)) continue;
                projected_net_mortchgs += (*i)->GetInforceProjectedCoiCharge
                    (this_years_coi_rate
                    ,this_years_part_mort_rate
                    ,eoy_naar
                    );
                (*i)->IncrementEOY(year);
                }

            // Calculate next year's k factor.
            // TODO ?? Only for current-mortality basis?

            double case_ibnr =
                    case_years_net_mortchgs
                *   case_ibnr_months
                /   12.0
                ;
            double case_net_mortality_reserve =
                    case_accum_net_mortchgs
                -   case_accum_net_claims
                -   case_ibnr
                ;

            // Current COI charges can actually be zero, e.g. when the
            // corridor factor is unity.
            if(0.0 == projected_net_mortchgs)
                {
                case_k_factor = 0.0;
                }
            else
                {
                case_k_factor = -
                        case_net_mortality_reserve
// TODO ?? '4.0' is an arbitrary factor that belongs in the database.
                    /   (4.0 * projected_net_mortchgs)
                    ;
                case_k_factor = std::max(-1.0, case_k_factor);
                }

            for(i = AVS.begin(); i != AVS.end(); ++i)
                {
                (*i)->ApportionNetMortalityReserve
                    (case_net_mortality_reserve
                    ,case_years_net_mortchgs
                    );
                }

            if(!meter->reflect_progress())
                {
                return;
                }
            } // End for year.

        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            (*i)->FinalizeLife(*run_basis);
            }

        } // End for...try.
    catch(std::exception& e)
        {
        fatal_error() << "\nCaught exception: " << e.what() << LMI_FLUSH;
        }
    catch(...)
        {
        fatal_error() << "\nUnknown exception." << LMI_FLUSH;
        }

    for(i = AVS.begin(); i != AVS.end(); ++i)
        {
        (*i)->FinalizeLifeAllBases();
        composite.PlusEq((*i)->LedgerValues());
        }

    status() << timer.Stop().Report() << std::flush;

    int j = 0;
    for(i = AVS.begin(); i != AVS.end(); ++i, ++j)
        {
        emit_ledger
            (*file
            ,j
            ,(*i)->LedgerValues()
            ,emit_to_spew_file
            );
        }

        emit_ledger
            (*file
            ,-1
            ,composite
            ,emit_to_spew_file
            );
}

