// Custom interface number one.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "custom_io_1.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "et_vector.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "name_value_pairs.hpp"
#include "platform_dependent.hpp"       // access()
#include "value_cast.hpp"
#include "yare_input.hpp"

#include <fstream>
#include <vector>

bool custom_io_1_file_exists()
{
    return 0 == access
        (configurable_settings::instance().custom_input_1_filename().c_str()
        ,F_OK
        );
}

namespace
{
// Empty for the moment.
} // Unnamed namespace.

/// Read custom input for a particular customer.

bool custom_io_1_read(Input& z, std::string const& filename)
{
    std::string actual_filename =
        !filename.empty()
        ? filename
        : configurable_settings::instance().custom_input_1_filename()
        ;
    if(0 != access(actual_filename.c_str(), F_OK))
        {
        fatal_error()
            << "File '"
            << actual_filename
            << "' is required but could not be found."
            << LMI_FLUSH
            ;
        }

    // Always use the current declared rate.
    z["UseCurrentDeclaredRate"] = "Yes";

    warning() << "Testing: simulation of reading custom input." << LMI_FLUSH;

    return true;
#if 0
    name_value_pairs n_v_pairs(actual_filename);

    // The list is not complete; other items may be required eventually.
    z["InforceYear"]                     = n_v_pairs.string_numeric_value("InforceYear");
    z["InforceMonth"]                    = n_v_pairs.string_numeric_value("InforceMonth");
    z["InforceGeneralAccountValue"]      = n_v_pairs.string_numeric_value("InforceAVGenAcct");
    z["InforceSeparateAccountValue"]     = n_v_pairs.string_numeric_value("InforceAVSepAcct");
    z["InforceRegularLoanValue"]         = n_v_pairs.string_numeric_value("InforceAVRegLn");
    z["InforcePreferredLoanValue"]       = n_v_pairs.string_numeric_value("InforceAVPrfLn");
    z["InforceCumulativeNoLapsePremium"] = n_v_pairs.string_numeric_value("InforceCumNoLapsePrem");
    z["InforceCumulativeNoLapsePayments"]= n_v_pairs.string_numeric_value("InforceCumPmts");

// TRICKY !! Other input methods distinguish the insured's first, middle,
// and last names. This method uses a single field to meet customer
// requirements. Combining that single field with the middle and last
// names works as long as we initialize the others to a nonempty string.
    z["InsuredName"]               = n_v_pairs.string_value("ApplicantName");
// Not yet used, but might be wanted someday:
//  n_v_pairs.string_value("ApplicantDOB"); // ApplicantDOB=01/01/1968
    z["IssueAge"]                  = n_v_pairs.string_numeric_value("ApplicantIssueAge");
    z["RetirementAge"]             = "100";

    std::string gender = n_v_pairs.string_value("ApplicantGender");
    if("F" == gender)
        {
        z["Gender"] = "Female";
        }
    else if("M" == gender)
        {
        z["Gender"] = "Male";
        }
    else if("U" == gender)
        {
        z["Gender"] = "Unisex";
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

    std::string tobacco = n_v_pairs.string_value("ApplicantTobacco");
    if("Y" == tobacco)
        {
        z["Smoking"] = "Smoker";
        }
    else if("N" == tobacco)
        {
        z["Smoking"] = "Nonsmoker";
        }
    else if("U" == tobacco)
        {
        z["Smoking"] = "Unismoke";
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

    z["StateOfJurisdiction"]       = n_v_pairs.string_value("ApplicantState");
    z["PremiumTaxState"]           = n_v_pairs.string_value("ApplicantState");

// Not yet used, but might be wanted someday:
// PaymentsPerYear=1

    z["ProductName"]               = n_v_pairs.string_value("ProductCode");

    if("Standard" != z["UnderwritingClass"].str())
        {
        fatal_error()
            << "Internal error: not initialized to standard rate class."
            << LMI_FLUSH
            ;
        }

    std::string undw = n_v_pairs.string_value("ProductOption");
    if("P" == undw)
        {
        z["UnderwritingClass"]     = "Preferred";
        z["GroupUnderwritingType"] = "Medical";
        }
    else if("F" == undw)
        {
        z["GroupUnderwritingType"] = "Medical";
        }
    else if("S" == undw)
        {
        z["GroupUnderwritingType"] = "Simplified issue";
        }
    else if("G" == undw)
        {
        z["GroupUnderwritingType"] = "Guaranteed issue";
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

    std::string dbopt = n_v_pairs.string_value("DeathBenefitOption");
    if("L" == dbopt)
        {
        z["DeathBenefitOption"] = "a";
        }
    else if("I" == dbopt)
        {
        z["DeathBenefitOption"] = "b";
        }
    else if("ROP" == dbopt)
        {
        z["DeathBenefitOption"] = "rop";
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
    // be calculated by using a "corridor" specified-amount strategy,
    // but the customer wants to enter the specified amount explicitly.
    z["SpecifiedAmount"]           = n_v_pairs.string_value("FaceAmt");

    // Zero out any default er premium.
    z["CorporationPayment"]        = "0";
    // Assume single premium. Although the corporation pays it,
    // treat it, contrary to fact, as paid by the insured; reason:
    // consistency with GUI--see ChangeLog for 20050825T0122Z .
    // SOMEDAY !! Revisit this later.
    z["Payment"]                   = n_v_pairs.string_value("PremiumAmt") + ";0";

// Not yet used, but might be wanted someday:
//ExchangeAmt=0
//PremiumYears=01 [single premium assumed for now]
//Revised=N
//Mortality=C

// Table ratings: not yet used, but might be wanted someday:
// ApplicantRating=
// ApplicantThruAge=
    z["SubstandardTable"]          = n_v_pairs.string_value("ApplicantRating");

    double permanent_flat         = n_v_pairs.numeric_value("PermFlatExtraAmt");
    double temporary_flat         = n_v_pairs.numeric_value("TempFlatExtraAmt");
    double temporary_flat_max_age = n_v_pairs.numeric_value("TempFlatExtraThruAge");
    if(z.issue_age() < temporary_flat_max_age)
        {
        z["FlatExtra"] =
              value_cast<std::string>(permanent_flat + temporary_flat)
            + "[0, @"
            + value_cast<std::string>(temporary_flat_max_age)
            + "); " // Apparently this ')' should be ']' for "ThruAge".
            + value_cast<std::string>(permanent_flat)
            ;
        }

    if("None" != z["SubstandardTable"].str())
        {
        z["UnderwritingClass"]     = "Rated";
        }

    // The current declared rate isn't necessarily used: see function
    // adjust_interest_rates() and its documentation.
    z["UseCurrentDeclaredRate"] = "No";

    yare_input const yip(z);
    product_database database(yip);

    double first_year_general_account_rate =
            0.01
        *   n_v_pairs.numeric_value("InterestRateFirstYr")
        ;
    double renewal_year_general_account_rate =
            0.01
        *   n_v_pairs.numeric_value("InterestRateOngoing")
        ;

    std::vector<double> declared_rate;
    database.Query(declared_rate, DB_MaxGenAcctRate);
    z["GeneralAccountRate"] = adjust_interest_rates
        (first_year_general_account_rate
        ,renewal_year_general_account_rate
        ,declared_rate
        );

// Reenable this line to test the interest calculation when changing it.
// This doesn't merit a formal, permanent unit test for now.
//    test_adjust_interest_rates();

// TRICKY !! Other input methods distinguish the agent's first, middle,
// and last names. This method uses a single field to meet customer
// requirements. Combining that single field with the middle and last
// names works only  as long as we initialize the latter to a nonempty
// string, which we do as a temporary workaround elsewhere; when that's
// resolved, revisit this.
    z["AgentName"]                 = n_v_pairs.string_value("AgentName");
    z["AgentAddress"]              = n_v_pairs.string_value("AgentAddress");
    z["AgentCity"]                 = n_v_pairs.string_value("AgentCity");
    z["AgentState"]                = n_v_pairs.string_value("AgentState");
    z["AgentZipCode"]              = n_v_pairs.string_value("AgentZip");
    z["AgentPhone"]                = n_v_pairs.string_value("AgentPhone");
    z["AgentId"]                   = n_v_pairs.string_value("AgentLicense");
// Not yet used, but might be wanted someday:
// AgentCompanyName
// AgentLicense

    double separate_account_rate =
            0.01
        *   n_v_pairs.numeric_value("InterestRateSepAcctFirstYr")
        ;

    z["SeparateAccountRate"]       = value_cast<std::string>(separate_account_rate);

    // TRICKY !! We need to consider the unconverted string: if it's empty,
    // it should be ignored, and must not be incorrectly converted to
    // zero. Yet one might actually wish to set the multiplier to zero;
    // that would be indicated by non-empty input evaluating to zero.
    std::string coi_mult            = n_v_pairs.string_value("COIMult");
    if(!coi_mult.empty())
        {
        z["OverrideCoiMultiplier"] = "Yes";
        z["CountryCoiMultiplier"]  = coi_mult;
        }

    // "AutoClose": "Y" or "N". Either way, read the custom input file
    // and write the custom output file. Then:
    //   if "Y", then exit;
    //   else, leave the GUI active.
    // Ignored for command-line regression testing.
    return "Y" == n_v_pairs.string_value("AutoClose");
#endif // 0
}

/// Write custom output for a particular customer.
///
/// Assumptions:
///   values are all as of EOY
///   "interest earned" is net interest credited, net of any spread
///   "mortality cost" is sum of actual COIs deducted throughout the year
///   "load" is premium load including any sales load and premium-based
///      loads for premium tax and dac tax, but excluding policy fee
///   "minimum premium" is a required premium as is typical of interest
///      sensitive whole life, and should be zero for flexible premium
///      universal life
///   "surrender cost" is account value minus cash surrender value; if
///      there is any refund in the early years, this value can be negative

void custom_io_1_write(Ledger const& ledger_values, std::string const& filename)
{
    std::string actual_filename =
        !filename.empty()
        ? filename
        : configurable_settings::instance().custom_output_1_filename()
        ;
    // Don't specify 'binary' here: the file is to be read by another
    // program that probably expects platform-specific behavior.
    std::ofstream os
        (actual_filename.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    if(!os.good())
        {
        fatal_error()
            << "File '"
            << actual_filename
            << "' could not be opened for writing."
            << LMI_FLUSH
            ;
        }

    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();

    os
        << "CashValu,SurrValu,DeathBen,IntEarned,"
        << "MortCost,Load,MinPrem,SurrCost,PremAmt,IntRate\n"
        ;

    std::vector<double> surr_chg(Invar.GetLength());
    assign(surr_chg, Curr_.AcctVal - Curr_.CSVNet);

    std::vector<double> prem_load(Invar.GetLength());
    assign(prem_load, Invar.GrossPmt - Curr_.NetPmt);

    os.setf(std::ios_base::fixed, std::ios_base::floatfield);

    int max_duration = static_cast<int>(Invar.EndtAge - Invar.Age);
    for(int j = 0; j < max_duration; j++)
        {
        os
            << std::setprecision(0)
            <<        Curr_.AcctVal        [j]
            << ',' << Curr_.CSVNet         [j]
            << ',' << Curr_.EOYDeathBft    [j]
            << ',' << Curr_.NetIntCredited [j]
            << ',' << Curr_.COICharge      [j]
// Column headers suggest that 'Load' should precede 'MinPrem',
// but this order was accepted; perhaps both were always zero
// in actual practice.
            << ',' << 0                                  // 'MinPrem' always zero.
            << ',' << prem_load            [j]
            << ',' << surr_chg             [j]
            << ',' << Invar.GrossPmt       [j]
            << ',' << Curr_.AnnGAIntRate   [j] * 10000.0 // 'IntRate' in bp.
            << '\n'
            ;
        }
    if(!os.good())
        {
        fatal_error() << "Error writing output file." << LMI_FLUSH;
        }
}

