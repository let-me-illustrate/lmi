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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "custom_io_1.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "configurable_settings.hpp"
#include "et_vector.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"               // ios_out_trunc_binary()
#include "platform_dependent.hpp"       // access()
#include "single_cell_document.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <fstream>
#include <stdexcept>
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
    std::string fetch(xml::element const& parent, std::string const& name)
    {
        return xml_lmi::get_content(*xml_lmi::retrieve_element(parent, name));
    }

    // Convert a date, given in a particular format, to a JDN string.
    // position: 0123456789_ignored_
    // format:   12/31/1987-00:00:00 (ignore time of day)
    std::string convert_date(std::string const& s)
    {
        calendar_date const z
            (value_cast<int>(s.substr(6, 4)) // year
            ,value_cast<int>(s.substr(0, 2)) // month
            ,value_cast<int>(s.substr(3, 2)) // day
            );
        return value_cast<std::string>(z.julian_day_number());
    }
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

    xml_lmi::dom_parser parser(actual_filename);
    xml::element const& root = parser.root_node("lmi");

    xml::element Benefit     = *xml_lmi::retrieve_element(root, "Benefit");

    xml::element Bank        = *xml_lmi::retrieve_element(Benefit, "Bank"       );
    xml::element Applicant   = *xml_lmi::retrieve_element(Benefit, "Applicant"  );
    xml::element BenefitData = *xml_lmi::retrieve_element(Benefit, "BenefitData");
    // Unused: <InterestRateData>
    xml::element Agent       = *xml_lmi::retrieve_element(Benefit, "Agent"      );

    // Unused: <ProcessType>
    std::string AutoClose                     = fetch(root        , "AutoClose");

    // Unused: <BenefitId>
    // Unused: <BranchID>

    // Unused: <BankFDIC>
    std::string BankName                      = fetch(Bank       , "BankName"   );
    std::string BankAddress                   = fetch(Bank       , "BankAddress");
    std::string BankCity                      = fetch(Bank       , "BankCity"   );
    std::string BankState                     = fetch(Bank       , "BankState"  );
    std::string BankZip                       = fetch(Bank       , "BankZip"    );

    std::string ApplicantFirstName            = fetch(Applicant  , "ApplicantFirstName"           );
    std::string ApplicantLastName             = fetch(Applicant  , "ApplicantLastName"            );
    // Unused: <ApplicantSSN>
    std::string ApplicantDOB                  = fetch(Applicant  , "ApplicantDOB"                 );
    // Unused: <ApplicantIssueAge>
    std::string ApplicantGender               = fetch(Applicant  , "ApplicantGender"              );
    std::string ApplicantTobacco              = fetch(Applicant  , "ApplicantTobacco"             );
    // Unused: <ApplicantHomeState>
    std::string ApplicantState                = fetch(Applicant  , "ApplicantState"               );
    std::string ApplicantRating               = fetch(Applicant  , "ApplicantRating"              );
    // Unused: <ApplicantRatingThruAge>
    std::string ApplicantPermFlatExtraAmt     = fetch(Applicant  , "ApplicantPermFlatExtraAmt"    );
    std::string ApplicantTempFlatExtraAmt     = fetch(Applicant  , "ApplicantTempFlatExtraAmt"    );
    std::string ApplicantTempFlatExtraThruAge = fetch(Applicant  , "ApplicantTempFlatExtraThruAge");

    // Unused: <PaymentsPerYear>
    std::string ProductCode                   = fetch(BenefitData, "ProductCode"       );
    // Unused: <ProductOption>
    std::string DeathBenefitOption            = fetch(BenefitData, "DeathBenefitOption");
    // Unused: <LoadOption>
    std::string FaceAmt                       = fetch(BenefitData, "FaceAmt"           );
    std::string PremiumAmt                    = fetch(BenefitData, "PremiumAmt"        );
    std::string ExchangeAmt                   = fetch(BenefitData, "ExchangeAmt"       );
    // Unused: <PremiumYears>
    // Unused: <Revised>
    // Unused: <DefinitionOfLifeInsurance>
    std::string WireDate                      = fetch(BenefitData, "WireDate"          );
    // Unused: <Mortality>
    // Unused: <CompReduction>
    std::string Underwriting                  = fetch(BenefitData, "Underwriting"      );
    // Unused: <HedgedRate>

    // Unused: <InterestRateData> (all subelements)

    std::string AgentName                     = fetch(Agent      , "AgentName"   );
    std::string AgentAddress                  = fetch(Agent      , "AgentAddress");
    std::string AgentCity                     = fetch(Agent      , "AgentCity"   );
    std::string AgentState                    = fetch(Agent      , "AgentState"  );
    std::string AgentZip                      = fetch(Agent      , "AgentZip"    );
    std::string AgentPhone                    = fetch(Agent      , "AgentPhone"  );
    std::string AgentLicense                  = fetch(Agent      , "AgentLicense");
    // Unused: <AgentCompanyName>

    // For certain fields, empty strings are taken to imply default values.
    if(ApplicantRating          .empty()) ApplicantRating           = "[EMPTY]";
    if(ApplicantPermFlatExtraAmt.empty()) ApplicantPermFlatExtraAmt = "0.0";
    if(ApplicantTempFlatExtraAmt.empty()) ApplicantTempFlatExtraAmt = "0.0";

    // Always use the current declared rate; disregard <InterestRateData>.
    z["UseCurrentDeclaredRate"] = "Yes";

    // Always calculate issue age from DOB. <ApplicantDOB> is always
    // specified. This is robust: lmi knows exactly how to calculate
    // insurance age for every product it supports, using ALB or ANB
    // as the case may be; and age changes are handled automatically.
    z["UseDOB"                ] = "Yes";

    // Allow issue age to exceed default retirement age.
    z["RetireesCanEnroll"     ] = "Yes";

    // <ApplicantState> specifies the state of jurisdiction. It might
    // seem that <BankState> would be better for BOLI, but that would
    // not be right for an out-of-state trust. The client system
    // actually has an "Insurance State" GUI field, which it maps to
    // <ApplicantState>; it doesn't identify a distinct premium-tax
    // state, so <ApplicantState> is used for that purpose as well.
    z["StateOfJurisdiction"        ] = ApplicantState;
    z["PremiumTaxState"            ] = ApplicantState;

    z["CorporationName"            ] = BankName;
    z["CorporationAddress"         ] = BankAddress;
    z["CorporationCity"            ] = BankCity;
    z["CorporationState"           ] = BankState;
    z["CorporationZipCode"         ] = BankZip;

    bool need_space = !ApplicantFirstName.empty() && !ApplicantLastName.empty();
    std::string space(need_space, ' ');
    z["InsuredName"] = ApplicantFirstName + space + ApplicantLastName;
    z["DateOfBirth"                ] = convert_date(ApplicantDOB);
    z["Gender"] =
          ("F" == ApplicantGender) ? "Female"
        : ("M" == ApplicantGender) ? "Male"
        : ("U" == ApplicantGender) ? "Unisex"
        : throw std::runtime_error(ApplicantGender + ": ApplicantGender not in {F,M,U}.")
        ;
    z["Smoking"] =
          ("Y" == ApplicantTobacco) ? "Smoker"
        : ("N" == ApplicantTobacco) ? "Nonsmoker"
        : ("U" == ApplicantTobacco) ? "Unismoke"
        : throw std::runtime_error(ApplicantTobacco + ": ApplicantTobacco not in {Y,N,U}.")
        ;
    z["State"                      ] = ApplicantState;
    // lmi's 'UnderwritingClass' and 'SubstandardTable' are conflated
    // into the client system's <ApplicantRating>.
    z["UnderwritingClass"] =
          ("[EMPTY]" == ApplicantRating) ? "Standard"
        : ("PF"      == ApplicantRating) ? "Preferred"
        :                                  "Rated"
        ;
    z["SubstandardTable"] =
          ("[EMPTY]" == ApplicantRating) ? "None"
        : ("PF"      == ApplicantRating) ? "None"
        : ("A"       == ApplicantRating) ? "A=+25%"
        : ("B"       == ApplicantRating) ? "B=+50%"
        : ("C"       == ApplicantRating) ? "C=+75%"
        : ("D"       == ApplicantRating) ? "D=+100%"
        : ("E"       == ApplicantRating) ? "E=+125%"
        : ("F"       == ApplicantRating) ? "F=+150%"
        : ("H"       == ApplicantRating) ? "H=+200%"
        : ("J"       == ApplicantRating) ? "J=+250%"
        : ("L"       == ApplicantRating) ? "L=+300%"
        : ("P"       == ApplicantRating) ? "P=+400%"
        : throw std::runtime_error(ApplicantRating + ": ApplicantRating unrecognized.")
        ;
    double permanent_flat         = value_cast<double>(ApplicantPermFlatExtraAmt);
    double temporary_flat         = value_cast<double>(ApplicantTempFlatExtraAmt);
    int    temporary_flat_max_age = value_cast<int   >(ApplicantTempFlatExtraThruAge);
    z["FlatExtra"] =
          value_cast<std::string>(permanent_flat + temporary_flat)
        + " [0, @"
        + value_cast<std::string>(temporary_flat_max_age)
        + "]; "
        + value_cast<std::string>(permanent_flat)
        ;
    z["ProductName"                ] = ProductCode;
    z["DeathBenefitOption"] =
          ("L"   == DeathBenefitOption) ? "a"
        : ("I"   == DeathBenefitOption) ? "b"
//      : ("ROP" == DeathBenefitOption) ? "rop" // Generally not offered for BOLI.
        : throw std::runtime_error(DeathBenefitOption + ": DeathBenefitOption not in {L,I}.")
        ;
    // <FaceAmt> and <PremiumAmt> are both specified, so that both can
    // be rounded in reasonable ways--even for single-premium products
    // that normally use a "corridor" specified-amount strategy.
    z["SpecifiedAmount"            ] = FaceAmt;
    // Assume single premium, paid by bank.
    z["CorporationPayment"         ] = PremiumAmt + "; 0.0";
    // Zero out default ee payment.
    z["Payment"                    ] = "0.0";
    // Assume that any 1035 exchange is external.
    z["External1035ExchangeAmount" ] = ExchangeAmt;
    // Assume that any 1035 exchange is from a MEC (for single-premium BOLI).
    z["External1035ExchangeFromMec"] = "Yes";
    std::string wire_date = convert_date(WireDate);
    z["EffectiveDate"              ] = wire_date;
    z["InforceAsOfDate"            ] = wire_date;
    z["LastMaterialChangeDate"     ] = wire_date;
    z["GroupUnderwritingType"] =
          "SI"  == Underwriting ? "Simplified issue"
        : "GI"  == Underwriting ? "Guaranteed issue"
        : "FUW" == Underwriting ? "Medical"
        : throw std::runtime_error(Underwriting + ": Underwriting not in {SI,GI,FUW}.")
        ;
    z["AgentName"                  ] = AgentName;
    z["AgentAddress"               ] = AgentAddress;
    z["AgentCity"                  ] = AgentCity;
    z["AgentState"                 ] = AgentState;
    z["AgentZipCode"               ] = AgentZip;
    z["AgentPhone"                 ] = AgentPhone;
    z["AgentId"                    ] = AgentLicense;

    // For internal testing only, if "AutoClose" has this special
    // value, then write input in lmi's usual format.
    if("X" == AutoClose)
        {
        std::ofstream ofs("custom_io_1.ill", ios_out_trunc_binary());
        single_cell_document(z).write(ofs);
        }

    // Meaning of return code based on "AutoClose":
    //   if "N", then write both PDF and custom output file;
    //   else, write custom output file only.
    // (Originally, "Y" meant that lmi should close automatically,
    // without displaying its GUI, and "N" meant that it should show
    // its GUI without closing automatically. In practice, "N" was
    // used only when it was desired to print a PDF file--which is
    // the opposite of the current sense. Now, the GUI is never
    // displayed, and lmi always closes automatically.)
    return "N" == AutoClose;
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
        << "CashValu,SurrValu,DeathBen,IntRate,IntEarned,"
        << "MortCost,MiscFees,Load,MinPrem,SurrCost,PremAmt\n"
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
            << ',' << Curr_.AnnGAIntRate   [j] * 10000.0 // 'IntRate' in bp.
            << ',' << Curr_.NetIntCredited [j]
            << ',' << Curr_.COICharge      [j]
            << ',' << 0                                  // 'MiscFees' always zero.
            << ',' << prem_load            [j]
            << ',' << 0                                  // 'MinPrem' always zero.
            << ',' << surr_chg             [j]
            << ',' << Invar.GrossPmt       [j]
            << '\n'
            ;
        }
    if(!os.good())
        {
        fatal_error() << "Error writing output file." << LMI_FLUSH;
        }
}

