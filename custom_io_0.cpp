// Custom interface number zero.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "custom_io_0.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "input.hpp"
#include "input_sequence.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "name_value_pairs.hpp"
#include "platform_dependent.hpp" // access()
#include "value_cast.hpp"
#include "yare_input.hpp"

#include <map>
#include <fstream>
#include <stdexcept>
#include <vector>

bool custom_io_0_file_exists()
{
    return 0 == access
        (configurable_settings::instance().custom_input_filename().c_str()
        ,F_OK
        );
}

namespace
{
/// Set interest rates from "special" input.

std::string adjust_interest_rates
    (double                     first_year_general_account_rate
    ,double                     renewal_year_general_account_rate
    ,std::vector<double> const& declared_rate
    )
{
    // The customer's front end provides two interest rates: one for
    // the first year only, and another for all renewal years. It's
    // our understanding that the "InterestRateOngoing" is enabled iff
    // the wire date precedes the rate effective date.
    //
    // Some general-account products have a non-level declared-rate
    // structure that doesn't fit that paradigm.

    bool credited_rates_fit_customer_paradigm = each_equal
        (1 + declared_rate.begin()
        ,    declared_rate.end()
        ,    declared_rate[1]
        );

    // For products that fit the customer paradigm, respect the
    // 'ongoing' interest field if anything is entered there;
    // otherwise, treat it as though
    //   first-year field + delta
    // had been entered, where
    //   delta = (renewal credited rate - initial credited rate).
    // Thus, entering the declared rate as "InterestRateFirstYr" while
    // leaving "InterestRateOngoing" empty suffices for illustrating
    // the declared scale; and entering a lower "InterestRateFirstYr"
    // preserves the shape of the declared scale, offsetting it by a
    // constant difference.
    //
    // This is always correct for products with a level declared rate.
    // For products that follow the first-and-renewal paradigm, it's
    // correct as long as delta doesn't change, and conservative if
    // delta is understated, which it is if a current release of the
    // product files (which embody the declared rate) is used to
    // illustrate a contract issued before the current release's
    // effective date--as long as delta has not become more positive.
    // For example:
    //
    //   0.07 0.09 0.09 prior   declared rate: delta = 0.02
    //   0.06 0.07 0.07 current declared rate: delta = 0.01
    //   0.07 specified, leaving renewal field empty
    //   0.07 0.08 0.08 illustrated: 0.09, (0.07 + 0.01)...
    //
    // This is not conservative, OTOH, if delta has "become more
    // positive". That expression is used here instead of "increased"
    // because delta may be either positive or negative, and some may
    // say that a delta that changed from -0.04 to -0.03 has in their
    // opinion "decreased" (it became closer to zero, hence smaller in
    // absolute magnitude), while all should agree that it has become
    // more positive (closer to +infinity).

    // For products that don't fit the customer pardigm, input in
    // "InterestRateFirstYr" or "InterestRateOngoing" is ignored,
    // and the current declared rate is used. It is expected that
    // this will not be acceptable for long.

    HOPEFULLY(!declared_rate.empty());
    std::vector<double> general_account_rate(declared_rate);

    if(credited_rates_fit_customer_paradigm)
        {
        if(0.0 == renewal_year_general_account_rate)
            {
            renewal_year_general_account_rate =
                    first_year_general_account_rate
                +   declared_rate.back()
                -   declared_rate.front()
                ;
            }
        general_account_rate.resize(2);
        general_account_rate[0] = first_year_general_account_rate;
        general_account_rate[1] = renewal_year_general_account_rate;
        }
    else
        {
        ; // Do nothing.
        }
    return InputSequence(general_account_rate).mathematical_representation();
}

void test_adjust_interest_rates()
{
    std::vector<double> declared_rate;
    declared_rate.push_back(0.06);
    warning()
        << "  Expect level 0.06: "
        << adjust_interest_rates(0.06, 0.00, declared_rate) << '\n'
        ;
    warning()
        << "  Expect 0.06, 0.07...: "
        << adjust_interest_rates(0.06, 0.07, declared_rate) << '\n'
        ;
    declared_rate.push_back(0.07);
    warning()
        << "  Expect 0.06, 0.07...: "
        << adjust_interest_rates(0.06, 0.00, declared_rate) << '\n'
        ;
    warning()
        << "  Expect 0.07, 0.08...: "
        << adjust_interest_rates(0.07, 0.00, declared_rate) << '\n'
        ;
    warning()
        << "  Expect 0.06, 0.07...: "
        << adjust_interest_rates(0.06, 0.07, declared_rate) << '\n'
        ;
    warning()
        << "  Expect 0.05, 0.08...: "
        << adjust_interest_rates(0.05, 0.08, declared_rate) << '\n'
        ;
    declared_rate.push_back(0.08);
    warning()
        << "  Expect 0.06, 0.07, 0.08...: "
        << adjust_interest_rates(0.00, 0.00, declared_rate) << '\n'
        ;
    warning()
        << "  Expect 0.06, 0.07, 0.08...: "
        << adjust_interest_rates(0.00, 0.09, declared_rate) << '\n'
        ;
    warning()
        << "  Expect 0.06, 0.07, 0.08...: "
        << adjust_interest_rates(0.05, 0.00, declared_rate) << '\n'
        ;
    warning()
        << "  Expect 0.06, 0.07, 0.08...: "
        << adjust_interest_rates(0.05, 0.09, declared_rate) << '\n'
        ;
    warning() << std::flush;
}

} // Unnamed namespace.

/// Read custom input for a particular customer.

bool custom_io_0_read(Input& z, std::string const& filename)
{
    // Set global flag to liberalize input restrictions slightly.
    global_settings::instance().set_custom_io_0(true);
    std::string actual_filename =
        !filename.empty()
        ? filename
        : configurable_settings::instance().custom_input_filename()
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

    name_value_pairs n_v_pairs(actual_filename);

    // The list is not complete; other items may be required eventually.
    z["InforceYear"]                     = n_v_pairs.string_numeric_value("InforceYear");
    z["InforceMonth"]                    = n_v_pairs.string_numeric_value("InforceMonth");
    z["InforceGeneralAccountValue"]      = n_v_pairs.string_numeric_value("InforceAVGenAcct");
    z["InforceSeparateAccountValue"]     = n_v_pairs.string_numeric_value("InforceAVSepAcct");
    z["InforceRegularLoanValue"]         = n_v_pairs.string_numeric_value("InforceAVRegLn");
    z["InforcePreferredLoanValue"]       = n_v_pairs.string_numeric_value("InforceAVPrfLn");
    z["InforceCumulativeNoLapsePremium"] = n_v_pairs.string_numeric_value("InforceCumNoLapsePrem");
    z["InforceCumulativePayments"]       = n_v_pairs.string_numeric_value("InforceCumPmts");

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

    z["State"]                     = n_v_pairs.string_value("ApplicantState");
    z["CorporationState"]          = z["State"];
    // TRICKY !! This is probably unnecessary, but if so, it's because
    // of some kludge that works around the problem. 'State' and
    // 'CorporationState' should not exist; only 'StateOfJurisdiction'
    // should. Therefore, when that felicitous state of affairs is
    // brought about, the following line (which is perhaps redundant
    // today) will do the right thing, and the preceding two lines
    // can be deleted. The customer's name 'ApplicantState' would seem
    // to correspond to correspond better to lmi's 'State', but
    // that's a red herring: the customer gives only two states, one
    // for "Applicant" and one for "Agent", but none for "Corporation"
    // that would correspond to 'CorporationState', and our intention
    // is to make lmi correspond to that paradigm anyway, someday.
    z["StateOfJurisdiction"]       = z["State"];

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
    // TODO ?? Revisit this later.
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
            + "); "
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
    TDatabase database(yip);

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

void custom_io_0_write(Ledger const& ledger_values, std::string const& filename)
{
    std::string actual_filename =
        !filename.empty()
        ? filename
        : configurable_settings::instance().custom_output_filename()
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

    // ET !! std::vector<double> surr_chg = Curr_.AcctVal - Curr_.CSVNet;
    std::vector<double> surr_chg(Curr_.AcctVal);
    std::transform
        (surr_chg.begin()
        ,surr_chg.end()
        ,Curr_.CSVNet.begin()
        ,surr_chg.begin()
        ,std::minus<double>()
        );

    // ET !! std::vector<double> prem_load = Invar.GrossPmt - Curr_.NetPmt;
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
        fatal_error() << "Error writing output file." << LMI_FLUSH;
        }
}

