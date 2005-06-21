// A custom interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: custom_io_0.cpp,v 1.6 2005-06-21 05:27:48 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "custom_io_0.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "inputillus.hpp"
#include "inputstatus.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "name_value_pairs.hpp"
#include "platform_dependent.hpp" // access()
#include "value_cast.hpp"

#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// TODO ?? Eventually rename all this stuff. Explain that this file is
// 'custom_io_0.cpp' because there'll be other customizations.

//==============================================================================
bool DoesSpecialInputFileExist()
{
    return 0 == access
        (configurable_settings::instance().custom_input_filename().c_str()
        ,F_OK
        );
}

//==============================================================================
bool SetSpecialInput(IllusInputParms& ip, char const* overridden_filename)
{
    // Set global flag to liberalize input restrictions slightly.
    global_settings::instance().set_custom_io_0(true);
    std::string filename =
        overridden_filename
        ? overridden_filename
        : configurable_settings::instance().custom_input_filename()
        ;
    if(0 != access(filename.c_str(), F_OK))
        {
        fatal_error()
            << "File '"
            << filename
            << "' is required but could not be found."
            << LMI_FLUSH
            ;
        }

    name_value_pairs n_v_pairs(filename);

    // Turn off solves, overriding a general default that's not
    // appropriate here.
    ip.SolveType = enum_solve_type(e_solve_none);

    // The list is not complete; other items may be required eventually.
    ip.InforceYear              = static_cast<int>(n_v_pairs.numeric_value("InforceYear"));
    ip.InforceMonth             = static_cast<int>(n_v_pairs.numeric_value("InforceMonth"));
    ip.InforceAVGenAcct         = n_v_pairs.numeric_value("InforceAVGenAcct");
    ip.InforceAVSepAcct         = n_v_pairs.numeric_value("InforceAVSepAcct");
    ip.InforceAVRegLn           = n_v_pairs.numeric_value("InforceAVRegLn");
    ip.InforceAVPrfLn           = n_v_pairs.numeric_value("InforceAVPrfLn");
    ip.InforceCumNoLapsePrem    = n_v_pairs.numeric_value("InforceCumNoLapsePrem");
    ip.InforceCumPmts           = n_v_pairs.numeric_value("InforceCumPmts");

// TRICKY !! Other input methods distinguish the insured's first, middle,
// and last names. This method uses a single field to meet customer
// requirements. Combining that single field with the middle and last
// names works as long as we initialize the others to a nonempty string.
    ip.InsdFirstName            = n_v_pairs.string_value("ApplicantName");
// Not yet used, but might be wanted someday:
//  n_v_pairs.string_value("ApplicantDOB"); // ApplicantDOB=01/01/1968
    ip.Status[0].IssueAge       = static_cast<int>(n_v_pairs.numeric_value("ApplicantIssueAge"));
    ip.Status[0].RetAge         = 100;

    std::string gender          = n_v_pairs.string_value("ApplicantGender");
    if("F" == gender)
        {
        ip.Status[0].Gender = e_female;
        }
    else if("M" == gender)
        {
        ip.Status[0].Gender = e_male;
        }
    else if("U" == gender)
        {
        ip.Status[0].Gender = e_unisex;
        }
    else
        {
        fatal_error()
            << "ApplicantGender is '"
            << gender
            << "', but it must be 'F', 'M', or 'U'."
            << LMI_FLUSH
            ;
        }

    std::string tobacco         = n_v_pairs.string_value("ApplicantTobacco");
    if("Y" == tobacco)
        {
        ip.Status[0].Smoking = e_smoker;
        }
    else if("N" == tobacco)
        {
        ip.Status[0].Smoking = e_nonsmoker;
        }
    else if("U" == tobacco)
        {
        ip.Status[0].Smoking = e_unismoke;
        }
    else
        {
        fatal_error()
            << "ApplicantTobacco is '"
            << tobacco
            << "', but it must be 'Y', 'N', or 'U'."
            << LMI_FLUSH
            ;
        }

    ip.InsdState                = n_v_pairs.string_value("ApplicantState");
    ip.SponsorState = ip.InsdState;

// Not yet used, but might be wanted someday:
// PaymentsPerYear=1

    ip.ProductName              = n_v_pairs.string_value("ProductCode");

    if(ip.Status[0].Class != e_standard)
        {
        fatal_error()
            << "Internal error: not initialized to standard rate class."
            << LMI_FLUSH
            ;
        }

    std::string undw            = n_v_pairs.string_value("ProductOption");
    if("P" == undw)
        {
        ip.Status[0].Class = e_preferred;
        ip.GroupUWType = e_medical;
        }
    else if("F" == undw)
        {
        ip.GroupUWType = e_medical;
        }
    else if("S" == undw)
        {
        ip.GroupUWType = e_simplifiedissue;
        }
    else if("G" == undw)
        {
        ip.GroupUWType = e_guaranteedissue;
        }
    else
        {
        fatal_error()
            << "ProductOption is '"
            << undw
            << "', but it must be 'P', 'F', 'S', or 'G'."
            << LMI_FLUSH
            ;
        }

    std::string dbopt           = n_v_pairs.string_value("DeathBenefitOption");
    if("L" == dbopt)
        {
        ip.DeathBenefitOption = "a";
        }
    else if("I" == dbopt)
        {
        ip.DeathBenefitOption = "b";
        }
    else if("ROP" == dbopt)
        {
        ip.DeathBenefitOption = "rop";
        }
    else
        {
        fatal_error()
            << "DeathBenefitOption is '"
            << dbopt
            << "', but it must be 'L', 'I', or 'ROP'."
            << LMI_FLUSH
            ;
        }

    // For single-premium cases, the specified amount would normally
    // be calculated this way:
    //   ip.SAStrategy               = e_sacorridor;
    // but the customer wants to enter the specified amount explicitly.
    ip.SpecifiedAmount = n_v_pairs.string_value("FaceAmt");

    // Assume single premium.
    ip.IndvPayment = n_v_pairs.string_value("PremiumAmt") + ";0";

// Not yet used, but might be wanted someday:
//ExchangeAmt=0
//PremiumYears=01 [single premium assumed for now]
//Revised=N
//Mortality=C

// Table ratings: not yet used, but might be wanted someday:
// ApplicantRating=
// ApplicantThruAge=
    ip.Status[0].SubstdTable    = n_v_pairs.string_value("ApplicantRating");

    double permanent_flat         = n_v_pairs.numeric_value("PermFlatExtraAmt");
    double temporary_flat         = n_v_pairs.numeric_value("TempFlatExtraAmt");
    double temporary_flat_max_age = n_v_pairs.numeric_value("TempFlatExtraThruAge");
    if(ip.Status[0].IssueAge < temporary_flat_max_age)
        {
        ip.FlatExtra =
              value_cast<std::string>(permanent_flat + temporary_flat)
            + "[0, @"
            + value_cast<std::string>(temporary_flat_max_age)
            + "); "
            + value_cast<std::string>(permanent_flat)
            ;
        }

    if
        (
            e_table_none != ip.Status[0].SubstdTable
        )
        {
        ip.Status[0].Class = e_rated;
        }

    TDatabase database(ip);

    double first_year_general_account_rate =
            0.01
        *   n_v_pairs.numeric_value("InterestRateFirstYr")
        ;
    double renewal_year_general_account_rate =
            0.01
        *   n_v_pairs.numeric_value("InterestRateOngoing")
        ;

    // Respect the 'ongoing' interest field if anything is entered
    // there; otherwise, set it to
    //   first-year field + (current renewal rate - current first-year rate)
    //
    // The customer's front end enables its 'ongoing' field iff the
    // wire date precedes the rate effective date. If it's disabled,
    // we derive the value as above. If it's enabled, then we should
    // get 'ongoing' input; if we happen not to, we'll just apply the
    // current difference between first and renewal, which should be
    // conservative in the case of the product we're designing this
    // for. That product's credited rates vary only by first versus
    // renewal year, as asserted below; the customer's interface
    // doesn't implement any other kind of variation.

    if(0.0 == renewal_year_general_account_rate)
        {
        std::vector<double> credited_rate;
        database.Query(credited_rate, DB_MaxGenAcctRate);
        LMI_ASSERT
            (each_equal
                (1 + credited_rate.begin()
                ,credited_rate.end()
                ,credited_rate[1]
                )
            );
        renewal_year_general_account_rate =
                first_year_general_account_rate
            +   credited_rate[1]
            -   credited_rate[0]
            ;
        }

    std::ostringstream oss;
    oss
        << first_year_general_account_rate
        << ';'
        << renewal_year_general_account_rate
        ;
    ip.GenAcctIntRate = oss.str();

// TRICKY !! Other input methods distinguish the agent's first, middle,
// and last names. This method uses a single field to meet customer
// requirements. Combining that single field with the middle and last
// names works only  as long as we initialize the latter to a nonempty
// string, which we do as a temporary workaround elsewhere; when that's
// resolved, revisit this.
    ip.AgentFirstName           = n_v_pairs.string_value("AgentName");
    ip.AgentLastName            = "";
    ip.AgentAddr1               = n_v_pairs.string_value("AgentAddress");
    ip.AgentCity                = n_v_pairs.string_value("AgentCity");
    ip.AgentState               = n_v_pairs.string_value("AgentState");
    ip.AgentZipCode             = n_v_pairs.string_value("AgentZip");
    ip.AgentPhone               = n_v_pairs.string_value("AgentPhone");
    ip.AgentID                  = n_v_pairs.string_value("AgentLicense");
// Not yet used, but might be wanted someday:
// AgentCompanyName
// AgentLicense

    double separate_account_rate =
            0.01
        *   n_v_pairs.numeric_value("InterestRateSepAcctFirstYr")
        ;

    ip.SepAcctIntRate = value_cast<std::string>(separate_account_rate);

    // TRICKY !! We need to consider the unconverted string: if it's empty,
    // it should be ignored, and must not be incorrectly converted to
    // zero. Yet one might actually wish to set the multiplier to zero;
    // that would be indicated by non-empty input evaluating to zero.
    std::string coi_mult        = n_v_pairs.string_value("COIMult");
    if("" != coi_mult)
        {
        ip.OverrideCOIMultiplier= "Yes";
        ip.CountryCOIMultiplier = coi_mult;
        }

    ip.propagate_changes_from_base_and_finalize();

    ip.ResetAllFunds(database.Query(DB_AllowGenAcct));

    // "AutoClose": "Y" or "N". Either way, read the custom input file
    // and write the custom output file. Then:
    //   if "Y", then exit;
    //   else, leave the GUI active.
    // Ignored for command-line regression testing.
    return "Y" == n_v_pairs.string_value("AutoClose");
}

//==============================================================================
// Assumptions:
//   values are all as of EOY
//   "interest earned" is net interest credited, net of any spread
//   "mortality cost" is sum of actual COIs deducted throughout the year
//   "load" is premium load including any sales load and premium-based
//      loads for premium tax and dac tax, but excluding policy fee
//   "minimum premium" is a required premium as is typical of interest
//      sensitive whole life, and should be zero for flexible premium
//      universal life
//   "surrender cost" is account value minus cash surrender value; if
//      there is any refund in the early years, this value can be negative
//
void PrintFormSpecial
    (Ledger const& ledger_values
    ,char const*   overridden_filename
    )
{
    std::string filename =
        overridden_filename
        ? overridden_filename
        : configurable_settings::instance().custom_output_filename()
        ;
    std::ofstream os
        (filename.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    if(!os.good())
        {
        hobsons_choice() << "Error initializing output file." << LMI_FLUSH;
        }

    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();

    os
        << "CashValu,SurrValu,DeathBen,IntEarned,"
        << "MortCost,Load,MinPrem,SurrCost,PremAmt,IntRate\n"
        ;

    std::vector<double> surr_chg(Curr_.AcctVal);
    std::transform
        (surr_chg.begin()
        ,surr_chg.end()
        ,Curr_.CSVNet.begin()
        ,surr_chg.begin()
        ,std::minus<double>()
        );

    std::vector<double> prem_load(Invar.GrossPmt);
    std::transform
        (prem_load.begin()
        ,prem_load.end()
        ,Curr_.NetPmt.begin()
        ,prem_load.begin()
        ,std::minus<double>()
        );

    os.setf(std::ios_base::fixed, std::ios_base::floatfield);

    int max_duration = static_cast<int>(Invar.EndtAge - Invar.Age);
    for(int j = 0; j < max_duration; j++)
        {
        // Customer requirement: show interest rate in bp.
        double gen_acct_int_rate_bp = 10000.0 * Curr_.AnnGAIntRate[j];
        os
            << std::setprecision(0)
            << Curr_.AcctVal        [j] << ','
            << Curr_.CSVNet         [j] << ','
            << Curr_.EOYDeathBft    [j] << ','
            << Curr_.NetIntCredited [j] << ','
            << Curr_.COICharge      [j] << ','
            << 0 << ',' // 'MinPrem' always zero.
            << prem_load            [j] << ','
            << surr_chg             [j] << ','
            << Invar.GrossPmt       [j] << ','
            << gen_acct_int_rate_bp
            << '\n'
            ;
        }
    if(!os.good())
        {
        hobsons_choice() << "Error writing output file." << LMI_FLUSH;
        }
}

