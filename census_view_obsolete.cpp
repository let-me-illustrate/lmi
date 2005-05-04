// Census manager.
//
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: census_view_obsolete.cpp,v 1.1 2005-05-04 14:55:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "census_view.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "census_document.hpp"
#include "configurable_settings.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "illustration_view.hpp"
#include "input.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "materially_equal.hpp"
#include "math_functors.hpp"
#include "miscellany.hpp"
#include "progress_meter.hpp"
#include "timer.hpp"
#include "wx_new.hpp"
#include "xml_notebook.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <wx/app.h> // wxTheApp
#include <wx/icon.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/xrc/xmlres.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

void CensusView::RunAllLives(e_emission_target emission_target)
{
    // Reports based on # cells, not # cells actually used, which may
    // differ because not all are included in composite.
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (cell_parms().size()
            ,"Calculating all cells"
            )
        );
    // TODO ?? Want youngest cell instead of first cell.
IllusInputParms ihs_input0;
convert_to_ihs(ihs_input0, cell_parms()[0]);
    Ledger Composite
//        (cell_parms()[0].LedgerType()
        (ihs_input0.LedgerType()
        ,100
        ,true
        );

    Timer timer;
    was_canceled_ = false;
    for(unsigned int j = 0; j < cell_parms().size(); ++j)
        {
IllusInputParms Parms;
convert_to_ihs(Parms, cell_parms()[j]);
// TODO ?? expunge?
//        IllusInputParms Parms = cell_parms()[j];
        // Skip anyone not included in composite
        if(!Parms.IncludeInComposite)
            continue;

// TODO ?? expunge try-catch?
//        try
            {
            AccountValue AV(Parms);
            AV.SetDebugFilename(serial_filename(j, "debug"));

            AV.RunAV();

            Composite.PlusEq(AV.LedgerValues());

            EmitEveryone(emission_target, AV.LedgerValues(), j);
            }
//        catch([various exception types])
//            {
//            throw;
//            }

        was_canceled_ = !meter->reflect_progress();
        if(was_canceled_)
            {
            break;
            }
        }

    status() << timer.Stop().Report() << std::flush;

    if(!was_canceled_)
        {
        composite_ledger_ = Composite; // TODO ?? Then why do we need both?
        // Insert total specamt and payments.
//        composite_is_available_ = true;
        composite_is_available_ = false;    // TODO ?? Doesn't work.
        EmitEveryone(emission_target, composite_ledger_, -1);
        }
}

void CensusView::RunAllMonths(e_emission_target emission_target)
{
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (cell_parms().size()
            ,"Initializing all cells"
            )
        );
    was_canceled_ = false;
    Timer timer;

    // Set up array of AccountValue objects for everyone in composite.
    // We might have kept this array for the lifetime of its owner
    // ("this"). Advantage: results have already been calculated and
    // would just need to be loaded. Disadvantage: any input change
    // could invalidate one cell, a set of cells, or the whole set,
    // and a bug in code to maintain validity could cause invalid
    // results to be printed--a silent error. Conclusion: rerunning a
    // cell when needed is fast enough, so the array is local to this
    // function, and we can drop cells out as they mature or lapse.
    // If we drop them, call FinalizeLife()... !!
    std::vector
        <boost::shared_ptr
            <AccountValue
            >
        > AVS;
    std::vector<Input>::iterator ip;
// TODO ?? expunge try-catch?
//    try
        {
        int j = 0;
        int first_cell_inforce_year  = value_cast<int>((*cell_parms().begin())["InforceYear"].str());
        int first_cell_inforce_month = value_cast<int>((*cell_parms().begin())["InforceMonth"].str());
        AVS.reserve(cell_parms().size());
        for(ip = cell_parms().begin(); ip != cell_parms().end(); ++ip, ++j)
            {
            // Skip any cell with zero lives.
            // TODO ?? Should that even be permitted?
            if(0 == value_cast<int>((*ip)["NumberOfIdenticalLives"].str()))
                continue;
            // Skip anyone not included in composite.
            if("Yes" != (*ip)["IncludeInComposite"].str())
                continue;

IllusInputParms Parms;
convert_to_ihs(Parms, *ip);
            boost::shared_ptr<AccountValue> AV(new AccountValue(Parms));
            AV->SetDebugFilename    (serial_filename(j, "debug"));
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

            was_canceled_ = !meter->reflect_progress();
            if(was_canceled_)
                {
                break;
                }
            }
        }
//    catch([various exception types])
//        {
//        throw;
//        }

    if(was_canceled_)
        {
        return;
        }

    // We're through with the above progress dialog, so get rid of it.
    // Without this step, it'd remain on the screen until this function
    // returns--showing a Cancel button that would do nothing.
// TODO ?? Is this a real problem?
//    Progress.Destroy();

    std::vector
        <boost::shared_ptr
            <AccountValue
            >
        >::iterator i;

    // TODO ?? Want youngest cell instead of first cell.
IllusInputParms ihs_input0;
convert_to_ihs(ihs_input0, cell_parms()[0]);

    Ledger Composite
//        (cell_parms()[0].LedgerType()
        (ihs_input0.LedgerType()
        ,100
        ,true
        );

    int year_average_age_first_exceeds_80 = 0;

if(std::string::npos != cell_parms()[0]["Comments"].str().find("idiosyncrasyZ3"))
    {
    std::ofstream ofs
        ("experience_rating"
        ,std::ios_base::out | std::ios_base::trunc
        );
    ofs
        << '\t' << "year"
        << '\t' << "1+i"
        << '\t' << "coi_rate"
        << '\t' << "q"
        << '\t' << "eoy_naar"
        << '\t' << "inforce"
        << '\t' << "coi"
        << '\t' << "cum_coi"
        << '\t' << "claims"
        << '\t' << "cum_claims"
        << '\t' << "ibnr_mos"
        << '\t' << "ibnr"
        << '\t' << "proxy_coi"
        << '\t' << "k"
        << '\n'
        ;
    }

    std::vector<e_run_basis> const& RunBases = Composite.GetRunBases();
    for
        (std::vector<e_run_basis>::const_iterator run_basis = RunBases.begin()
        ;run_basis != RunBases.end()
        ;++run_basis
        )
// TODO ?? expunge try-catch?
//    try
        {
        if(was_canceled_)
            {
            break;
            }

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

        TDatabase temp_db(ihs_input0);
        double case_ibnr_months = temp_db.Query(DB_ExpRatIBNRMult);

if(std::string::npos != cell_parms()[0]["Comments"].str().find("idiosyncrasyZ3"))
        {
        std::ofstream ofs
            ("experience_rating"
            ,std::ios_base::out | std::ios_base::ate | std::ios_base::app
            );
        ofs << *run_basis << '\n';
        }

        // Experience rating as implemented here uses either a special
        // scalar input rate, or the separate-account rate. Those
        // rates as entered might vary across cells, but there must be
        // only one rate: therefore, use the first cell's rate, and
        // extend its last element if it doesn't have enough values.

        std::vector<double> experience_reserve_rate;
        std::copy
            (ihs_input0.SepAcctRate.begin()
            ,ihs_input0.SepAcctRate.end()
            ,std::back_inserter(experience_reserve_rate)
            );
        experience_reserve_rate.resize(MaxYr, experience_reserve_rate.back());
        if(ihs_input0.OverrideExperienceReserveRate)
            {
            experience_reserve_rate.assign
                (experience_reserve_rate.size()
                ,ihs_input0.ExperienceReserveRate
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
{
////std::ofstream ofs("experience_rating", std::ios_base::out | std::ios_base::ate | std::ios_base::app);
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
////ofs << current_mortchg << " = mortchg" << std::endl;
                    }
////ofs << current_mortchg << " = total mortchg" << std::endl;

////ofs << "Month " << month << "ytd mortchg: " << case_years_net_mortchgs;
}

// TODO ?? composite is off when first cell dies.
// Assertions are designed to ignore this.
// TODO ?? Assertions would also fire if enabled for other bases than current.

                // Accumulate at interest after adding net COI.

                // Process transactions from int credit through end of month.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    if((*i)->PrecedesInforceDuration(year, month)) continue;
                    (*i)->IncrementEOM(year, month, Assets[year]);
                    }

                // Project claims using partial mortality rate.
                // Assume year's claims occur at end of last month and
                // are paid at that instant so no int adjustment required.
                //
                // We experimented with taking out 1/12 of the claims in
                // each month. But that's inconsistent with the inforce
                // factors, which change only at the end of the year.
                // So it would be quite a bit of work to offer any
                // consistent alternative to curtate partial mortality.
                double CaseMonthsClaims = 0.0;
                double current_claims = 0.0;
                if(month == 11)
                    {
                    for(i = AVS.begin(); i != AVS.end(); ++i)
                        {
                        (*i)->SetClaims();
                        CaseMonthsClaims += (*i)->GetCurtateNetClaimsInforce();
// TODO ?? AV released on death was added to the nearly-identical code in
// acctval.cpp, but not here.
                        current_claims += (*i)->GetCurtateNetClaimsInforce();
                        }

                    case_accum_net_claims *= experience_reserve_annual_u;
                    case_accum_net_claims += current_claims;

//                    case_years_net_claims *= experience_reserve_annual_u;
                    case_years_net_claims += current_claims;

                    case_accum_net_mortchgs *= experience_reserve_annual_u;
                    case_accum_net_mortchgs += current_mortchg;

//                    case_years_net_mortchgs *= experience_reserve_annual_u;
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

            // Experience rating.

            // Average age reflecting survivorship and persistency.
            // This must be done here, at the end of the year, in
            // order to reflect lapses.
            //
            // TODO ?? Instead of just stating the duration calculated
            // here, we need to do something with it, and document
            // what we do. We should also document everything here;
            // is there a reason for the development notes to give
            // more detail than inline comments?
            //
            double age_sum = 0.0;
            double average_age = 0.0;
            double lives_inforce = 0.0;
            if
                (   0 == year_average_age_first_exceeds_80
                &&  std::string::npos != cell_parms()[0]["Comments"].str().find
                        ("idiosyncrasy_average_age"
                        )
                )
                {
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    lives_inforce += (*i)->GetInforceLives();
                    age_sum += lives_inforce * ((*i)->GetIssueAge() + year);
                    }
                if(0.0 != lives_inforce)
                    {
                    average_age = age_sum / lives_inforce;
                    }
                if(80 < average_age)
                    {
                    year_average_age_first_exceeds_80 = year;
                    warning()
                        << " Average age exceeds 80 in policy year "
                        << year_average_age_first_exceeds_80
                        << LMI_FLUSH
                        ;
                    }
                }

            // Increment year; update curtate inforce factor.

            // TODO ?? Temporary. These variables are useful only
            // for testing single-life cases.
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

if(std::string::npos != cell_parms()[0]["Comments"].str().find("idiosyncrasyZ3"))
{
            std::ofstream ofs
                ("experience_rating"
                ,std::ios_base::out | std::ios_base::ate | std::ios_base::app
                );
            ofs
                << '\t' << std::setprecision( 0) << year
                << '\t' << std::setprecision(20) << experience_reserve_annual_u
                << '\t' << std::setprecision(20) << this_years_coi_rate
                << '\t' << std::setprecision(20) << this_years_part_mort_rate
                << '\t' << std::setprecision(20) << eoy_naar
                << '\t' << std::setprecision(20) << case_years_net_mortchgs
                << '\t' << std::setprecision(20) << case_accum_net_mortchgs
                << '\t' << std::setprecision(20) << case_years_net_claims
                << '\t' << std::setprecision(20) << case_accum_net_claims
                << '\t' << std::setprecision(20) << case_ibnr_months
                << '\t' << std::setprecision(20) << case_ibnr
                << '\t' << std::setprecision(20) << projected_net_mortchgs
                << '\t' << std::setprecision(20) << case_k_factor
                << '\n' << std::flush
                ;
}

            was_canceled_ = !meter->reflect_progress();
            if(was_canceled_)
                {
                break;
                }
            } // End for year.

        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            (*i)->FinalizeLife(*run_basis);
            }

        } // End for...try.
//    catch(xmsg& x) // TODO ?? or let it leak through to application?
//        {
//        ::MessageBox(0, x.why().c_str(), "Exception", MB_OK);
//        }
//    catch([various exception types])
//        {
//        throw;
//        }

    for(i = AVS.begin(); i != AVS.end(); ++i)
        {
        (*i)->FinalizeLifeAllBases();
        Composite.PlusEq((*i)->LedgerValues());
        }

    status() << timer.Stop().Report() << std::flush;

    if(!was_canceled_)
        {
        int j = 0;
        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            EmitEveryone(emission_target, (*i)->LedgerValues(), ++j);
            }

        composite_ledger_ = Composite;
        // Insert total specamt and payments.
//        composite_is_available_ = true;
        composite_is_available_ = false; // TODO ?? Doesn't work.

        EmitEveryone(emission_target, composite_ledger_, -1);
        }
}

void CensusView::EmitEveryone
    (e_emission_target emission_target
    ,Ledger const& a_Values
    ,int           a_idx
    )
{
    if(emission_target & emit_to_printer)
        {
        PrintAnonymousIllustration(a_Values, a_idx);
        }
/*
    if(emission_target & to_tab_delim)
        {
        // This is no longer reachable by any path through the code.
        // We leave it here so that someday we can offer finer
        // control over census output, perhaps through a GUI.
        composite_is_available_ = true; // TODO ?? Doesn't work.
        SaveRegressionTestFile(a_Values, a_idx);
        composite_is_available_ = false; // TODO ?? Doesn't work.
        }
*/        
    if(emission_target & emit_to_spew_file)
        {
        composite_is_available_ = true; // TODO ?? Doesn't work.
        // Optionally generate tab delimited regression-test output too.
        if(global_settings::instance().regression_test_full)
            {
            SaveRegressionTestFile(a_Values, a_idx);
            }
        SaveCRCToFile(a_Values, a_idx);
        if(a_Values.GetIsComposite())
            {
            // For composite, emit all the vectors in Ledger.
            std::ofstream os
                (GetCRCFilename().c_str()
                ,  std::ios_base::out
                 | std::ios_base::ate
                 | std::ios_base::app
                );
            a_Values.Spew(os);
            }
        composite_is_available_ = false; // TODO ?? Doesn't work.
        }
    if(emission_target & emit_to_spreadsheet)
        {
        composite_is_available_ = true; // TODO ?? Doesn't work.
        SaveSpreadsheetFile(a_Values, a_idx);
        composite_is_available_ = false; // TODO ?? Doesn't work.
        }
}

