// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: ledger_text_formats.cpp,v 1.37 2007-05-22 00:59:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_text_formats.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "financial.hpp"
#include "global_settings.hpp"
#include "input_sequence.hpp"
#include "ledger.hpp"
#if defined LMI_USE_NEW_REPORTS
#   include "ledger_formatter.hpp"
#endif // defined LMI_USE_NEW_REPORTS
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "obstruct_slicing.hpp"
#include "value_cast.hpp"

#include <boost/utility.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iterator>
#include <locale>
#include <ostream>
#include <sstream>
#include <vector>

// TODO ?? Work around this problem
//   http://sv.nongnu.org/bugs/index.php?func=detailitem&item_id=13856
// here, as was done in 'ledger_xml_io.cpp' revision 1.45 .

std::string FormatSelectedValuesAsHtml(Ledger const& ledger_values)
{
    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();
    LedgerVariant   const& Guar_ = ledger_values.GetGuarFull();
    int max_length = ledger_values.GetMaxLength();

    std::ostringstream oss;

    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    oss.imbue(new_loc);
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

    oss
        << "<html>"
        << "<head><title>Let me illustrate...</title></head>"
        << "<body>"

        << "<p>"
        << "Calculation summary for "
        ;

    if(ledger_values.GetIsComposite())
        {
        oss
            << " composite<br>"
            ;
        }
    else
        {
        oss
            << Invar.Insured1
            << "<br>"
            << Invar.Gender << ", " << Invar.Smoker
            << std::setprecision(0)
            << ", age " << Invar.Age
            << std::setprecision(2)
            << "<br>"
            ;

        if(is_subject_to_ill_reg(ledger_values.GetLedgerType()))
            {
            oss
                << Invar.GuarPrem << "   guaranteed premium<br>"
                ;
            }

        oss
            << "<br>"
            << Invar.InitGLP          << "   initial guideline level premium<br>"
            << Invar.InitGSP          << "   initial guideline single premium<br>"
            << Invar.InitSevenPayPrem << "   initial seven-pay premium<br>"
            << ((Invar.IsMec) ? "MEC" : "Non-MEC") << "<br>"
            << "<br>"
            << Invar.InitTgtPrem      << "   initial target premium<br>"
            << Invar.InitBaseSpecAmt  << "   initial base specified amount<br>"
            << Invar.InitTermSpecAmt  << "   initial term specified amount<br>"
            << Invar.InitBaseSpecAmt + Invar.InitTermSpecAmt << "   initial total specified amount<br>"
            << Invar.GetStatePostalAbbrev() << "   state of jurisdiction<br>"
            ;
        }

    oss
        << "</p>"

        << "<hr>"
        << "<table align=right>"
        << "  <tr>"
        << "    <th></th>    <th></th>"
        << "    <th>Guaranteed</th> <th>Guaranteed</th> <th>Guaranteed</th>"
        << "    <th>Current</th>    <th>Current</th>    <th>Current</th>"
        << "  </tr>"
        << "  <tr>"
        << "    <th></th>    <th></th>"
        << "    <th>Account</th>    <th>Surrender</th>  <th>Death</th>"
        << "    <th>Account</th>    <th>Surrender</th>  <th>Death</th>"
        << "  </tr>"
        << "  <tr>"
        << "    <th>Age</th> <th>Outlay</th>"
        << "    <th>Value</th>      <th>Value</th>      <th>Benefit</th>"
        << "    <th>Value</th>      <th>Value</th>      <th>Benefit</th>"
        << "  </tr>"
        ;

    for(int j = 0; j < max_length; ++j)
        {
        oss
            << "<tr>"
            << std::setprecision(0)
            << "<td>" << j + Invar.Age        << "</td>"
            << std::setprecision(2)
            << "<td>" << Invar.Outlay[j]      << "</td>"
            << "<td>" << Guar_.AcctVal[j]     << "</td>"
            << "<td>" << Guar_.CSVNet[j]      << "</td>"
            << "<td>" << Guar_.EOYDeathBft[j] << "</td>"
            << "<td>" << Curr_.AcctVal[j]     << "</td>"
            << "<td>" << Curr_.CSVNet[j]      << "</td>"
            << "<td>" << Curr_.EOYDeathBft[j] << "</td>"
            << "</tr>"
            ;
        }

    oss
        << "</table>"
        << "</body>"
        ;
    return oss.str();
}

void PrintFormTabDelimited
    (Ledger const& ledger_values
    ,std::string const& file_name
    )
{
#if defined LMI_USE_NEW_REPORTS
    PrintFormTabDelimitedXXX(ledger_values, file_name);
#else  // !defined LMI_USE_NEW_REPORTS
    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();
    LedgerVariant   const& Guar_ = ledger_values.GetGuarFull();

    int max_length = ledger_values.GetMaxLength();

    std::vector<double> net_payment(Invar.Outlay);

    std::vector<double> real_claims;
    if(ledger_values.GetIsComposite())
        {
        real_claims = Curr_.ClaimsPaid;
        }
    else
        {
        real_claims.assign(Curr_.ClaimsPaid.size(), 0.0);
        }

    std::vector<double> cash_flow;
    cash_flow.push_back(0.0); // No claims paid on issue date.
    std::copy
        (real_claims.begin()
        ,real_claims.end()
        ,std::inserter(cash_flow, cash_flow.end())
        );
    // ET !! This is tricky: incompatible lengths.
    // ET !! cash_flow = catenate(0.0, -real_claims);
    std::transform
        (cash_flow.begin()
        ,cash_flow.end()
        ,cash_flow.begin()
        ,std::negate<double>()
        );
    // ET !! This is tricky: incompatible lengths.
    // ET !! drop(-1, cash_flow) = net_payment;
    std::transform
        (net_payment.begin()
        ,net_payment.end()
        ,cash_flow.begin()
        ,cash_flow.begin()
        ,std::plus<double>()
        );

    cash_flow.pop_back(); // Here we no longer need cash_flow[omega].

    // ET !! std::vector<double> csv_plus_claims = Curr_.CSVNet + real_claims;
    std::vector<double> csv_plus_claims(Curr_.CSVNet);
    std::transform
        (csv_plus_claims.begin()
        ,csv_plus_claims.end()
        ,real_claims.begin()
        ,csv_plus_claims.begin()
        ,std::plus<double>()
        );
// TODO ?? Is this irr valid?
    std::vector<double> irr_on_surrender(Curr_.CSVNet.size());
    if(!Invar.IsInforce)
        {
        irr
            (cash_flow
            ,csv_plus_claims
            ,irr_on_surrender
            ,static_cast<std::vector<double>::size_type>(Curr_.LapseYear)
            ,max_length
            ,Invar.irr_precision
            );

#ifdef DEBUGGING_IRR
std::ofstream os("irr.txt", ios_out_app_binary());
os
<< "  PrintFormTabDelimited():\n"
        << "\n\tcash_flow.size() = " << cash_flow.size()
        << "\n\tcsv_plus_claims.size() = " << csv_plus_claims.size()
        << "\n\tirr_on_surrender.size() = " << irr_on_surrender.size()
        << "\n\tCurr_.LapseYear = " << Curr_.LapseYear
        << "\n\tmax_length = " << max_length
        ;
os << "\n\tcash_flow = ";
std::copy(cash_flow.begin(), cash_flow.end(), std::ostream_iterator<double>(os, " "));
os << "\n\tcsv_plus_claims = ";
std::copy(csv_plus_claims.begin(), csv_plus_claims.end(), std::ostream_iterator<double>(os, " "));
os << "\n\tirr_on_surrender = ";
std::copy(irr_on_surrender.begin(), irr_on_surrender.end(), std::ostream_iterator<double>(os, " "));
os << "\n\n" ;
#endif // DEBUGGING_IRR
        }

    // ET !! std::vector<double> db_plus_claims = Curr_.EOYDeathBft + real_claims;
    std::vector<double> db_plus_claims(Curr_.EOYDeathBft);
    std::transform
        (db_plus_claims.begin()
        ,db_plus_claims.end()
        ,real_claims.begin()
        ,db_plus_claims.begin()
        ,std::plus<double>()
        );
    std::vector<double> irr_on_death(Curr_.EOYDeathBft.size(), -1.0);
    if(!Invar.IsInforce)
        {
        irr
            (cash_flow
            ,db_plus_claims
            ,irr_on_death
            ,static_cast<std::vector<double>::size_type>(Curr_.LapseYear)
            ,max_length
            ,Invar.irr_precision
            );
        }

    std::ofstream os(file_name.c_str(), ios_out_app_binary());

    os << "\n\nFOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.\n\n";

    os << "ProducerName\t\t"      << Invar.value_str("ProducerName"   ) << '\n';
    os << "ProducerStreet\t\t"    << Invar.value_str("ProducerStreet" ) << '\n';
    os << "ProducerCity\t\t"      << Invar.value_str("ProducerCity"   ) << '\n';
    os << "CorpName\t\t"          << Invar.value_str("CorpName"       ) << '\n';
    os << "Insured1\t\t"          << Invar.value_str("Insured1"       ) << '\n';
    os << "Gender\t\t"            << Invar.value_str("Gender"         ) << '\n';
    os << "Smoker\t\t"            << Invar.value_str("Smoker"         ) << '\n';
    os << "IssueAge\t\t"          << Invar.value_str("Age"            ) << '\n';
    os << "InitBaseSpecAmt\t\t"   << Invar.value_str("InitBaseSpecAmt") << '\n';
    os << "InitTermSpecAmt\t\t"   << Invar.value_str("InitTermSpecAmt") << '\n';
    double total_spec_amt = Invar.InitBaseSpecAmt + Invar.InitTermSpecAmt;
    os << "  Total:\t\t"     << value_cast<std::string>(total_spec_amt) << '\n';
    os << "PolicyMktgName\t\t"    << Invar.value_str("PolicyMktgName" ) << '\n';
    os << "PolicyLegalName\t\t"   << Invar.value_str("PolicyLegalName") << '\n';
    os << "PolicyForm\t\t"        << Invar.value_str("PolicyForm"     ) << '\n';
    os << "UWClass\t\t"           << Invar.value_str("UWClass"        ) << '\n';
    os << "UWType\t\t"            << Invar.value_str("UWType"         ) << '\n';

    // Skip authentication for non-interactive regression testing.
    // Surround the date in single quotes because one popular
    // spreadsheet would otherwise interpret it as a date, which
    // is likely not to fit in a default-width cell.
    if(!global_settings::instance().regression_testing())
        {
        authenticate_system();
        os << "DatePrepared\t\t'" << calendar_date().str() << "'\n";
        }
    else
        {
        // For regression tests, use EffDate as date prepared,
        // in order to avoid gratuitous failures.
        os << "DatePrepared\t\t'" << Invar.EffDate << "'\n";
        }

        os << '\n';

    char const* cheaders[] =
        {"PolicyYear"
        ,"AttainedAge"
        ,"DeathBenefitOption"
        ,"EmployeeGrossPremium"
        ,"CorporationGrossPremium"
        ,"GrossWithdrawal"
        ,"NewCashLoan"
        ,"LoanBalance"
// TODO ?? Add loan interest?
        ,"Outlay"
        ,"NetPremium"
        ,"PremiumTaxLoad"
        ,"DacTaxLoad"
// TODO ?? Also:
//   M&E
//   stable value
//   DAC- and premium-tax charge
//   comp
//   IMF
//   custodial expense?
//   account value released?
        ,"PolicyFee"
        ,"SpecifiedAmountLoad"
        ,"MonthlyFlatExtra"
        ,"MortalityCharge"
        ,"NetMortalityCharge"
        ,"AccountValueLoadAfterMonthlyDeduction"
        ,"CurrentSeparateAccountInterestRate"
        ,"CurrentGeneralAccountInterestRate"
        ,"CurrentGrossInterestCredited"
        ,"CurrentNetInterestCredited"
        ,"GuaranteedAccountValue"
        ,"GuaranteedNetCashSurrenderValue"
        ,"GuaranteedYearEndDeathBenefit"
        ,"CurrentAccountValue"
        ,"CurrentNetCashSurrenderValue"
        ,"CurrentYearEndDeathBenefit"
        ,"IrrOnSurrender"
        ,"IrrOnDeath"
        ,"YearEndInforceLives"
        ,"ClaimsPaid"
        ,"NetClaims"
        ,"ExperienceReserve"
        ,"ProjectedMortalityCharge"
        ,"KFactor"
        ,"NetMortalityCharge0Int"
        ,"NetClaims0Int"
        ,"ExperienceReserve0Int"
        ,"ProjectedMortalityCharge0Int"
        ,"KFactor0Int"
        ,"ProducerCompensation"
        };

    std::vector<std::vector<std::string> > sheaders;

    std::vector<std::string>::size_type max_header_rows = 0U;
    for(unsigned int j = 0; j < lmi_array_size(cheaders); ++j)
        {
        std::istringstream iss(cheaders[j]);
        std::vector<std::string> v;
        std::copy
            (std::istream_iterator<std::string>(iss)
            ,std::istream_iterator<std::string>()
            ,std::back_inserter(v)
            );
        sheaders.push_back(v);
        max_header_rows = std::max(max_header_rows, v.size());
        }
    std::vector<std::vector<std::string> >::iterator shi;
    for(shi = sheaders.begin(); shi != sheaders.end(); ++shi)
        {
        std::reverse(shi->begin(), shi->end());
        shi->resize(max_header_rows);
        std::reverse(shi->begin(), shi->end());
        }
    for(unsigned int j = 0; j < max_header_rows; ++j)
        {
        for(shi = sheaders.begin(); shi != sheaders.end(); ++shi)
            {
            os << (*shi)[j] << '\t';
            }
        os << '\n';
        }
    os << '\n';

    for(int j = 0; j < max_length; ++j)
        {
        os << (j + 1                                    ) << '\t';
        os << (j + Invar.Age                            ) << '\t';

        os << Invar.DBOpt[j]                              << '\t';

        os << Invar.value_str("EeGrossPmt"            ,j) << '\t';
        os << Invar.value_str("ErGrossPmt"            ,j) << '\t';
        os << Invar.value_str("NetWD"                 ,j) << '\t'; // TODO ?? It's *gross* WD.
        os << Invar.value_str("NewCashLoan"           ,j) << '\t';
        os << Curr_.value_str("TotalLoanBalance"      ,j) << '\t';
        os << Invar.value_str("Outlay"                ,j) << '\t';

        os << Curr_.value_str("NetPmt"                ,j) << '\t';

        os << Curr_.value_str("PremTaxLoad"           ,j) << '\t';
        os << Curr_.value_str("DacTaxLoad"            ,j) << '\t';
        os << Curr_.value_str("PolicyFee"             ,j) << '\t';
        os << Curr_.value_str("SpecAmtLoad"           ,j) << '\t';
        os << Invar.value_str("MonthlyFlatExtra"      ,j) << '\t';
        os << Curr_.value_str("COICharge"             ,j) << '\t';
        os << Curr_.value_str("NetCOICharge"          ,j) << '\t';
        os << Curr_.value_str("SepAcctLoad"           ,j) << '\t';

        os << Curr_.value_str("AnnSAIntRate"          ,j) << '\t';
        os << Curr_.value_str("AnnGAIntRate"          ,j) << '\t';
        os << Curr_.value_str("GrossIntCredited"      ,j) << '\t';
        os << Curr_.value_str("NetIntCredited"        ,j) << '\t';

        os << Guar_.value_str("AcctVal"               ,j) << '\t';
        os << Guar_.value_str("CSVNet"                ,j) << '\t';
        os << Guar_.value_str("EOYDeathBft"           ,j) << '\t';
        os << Curr_.value_str("AcctVal"               ,j) << '\t';
        os << Curr_.value_str("CSVNet"                ,j) << '\t';
        os << Curr_.value_str("EOYDeathBft"           ,j) << '\t';

        if(Invar.IsInforce)
            {
            os << "(inforce)"                             << '\t';
            os << "(inforce)"                             << '\t';
            }
        else
            {
            os << irr_on_surrender[j]                     << '\t';
            os << irr_on_death[j]                         << '\t';
            }

        // First element of InforceLives is BOY--show only EOY.
        os << value_cast<std::string>(Invar.InforceLives[1 + j]) << '\t';

        os << Curr_.value_str("ClaimsPaid"            ,j) << '\t';
        os << Curr_.value_str("NetClaims"             ,j) << '\t';
        os << Curr_.value_str("ExperienceReserve"     ,j) << '\t';
        os << Curr_.value_str("ProjectedCoiCharge"    ,j) << '\t';
        os << Curr_.value_str("KFactor"               ,j) << '\t';

        // Show experience-rating columns for current-expense, zero-
        // interest basis if used, to support testing.
        std::vector<e_run_basis> const& bases(ledger_values.GetRunBases());
        if
            (   bases.end()
            !=  std::find(bases.begin(), bases.end(), e_run_curr_basis_sa_zero)
            )
            {
            LedgerVariant const& Curr0 = ledger_values.GetCurrZero();
            os << Curr0.value_str("NetCOICharge"          ,j) << '\t';
            os << Curr0.value_str("NetClaims"             ,j) << '\t';
            os << Curr0.value_str("ExperienceReserve"     ,j) << '\t';
            os << Curr0.value_str("ProjectedCoiCharge"    ,j) << '\t';
            os << Curr0.value_str("KFactor"               ,j) << '\t';
            }
        else
            {
            os << "0\t";
            os << "0\t";
            os << "0\t";
            os << "0\t";
            os << "0\t";
            }

        if
            (   bases.end()
            !=  std::find(bases.begin(), bases.end(), e_run_curr_basis_sa_half)
            )
            {
            fatal_error()
                << "Three-rate illustrations not supported."
                << LMI_FLUSH
                ;
            }

        os << Invar.value_str("ProducerCompensation"  ,j) << '\t';

        os << '\n';
        }
#endif // !defined LMI_USE_NEW_REPORTS
}

class FlatTextLedgerPrinter
    :private boost::noncopyable
    ,virtual private obstruct_slicing<FlatTextLedgerPrinter>
{
  public:
    FlatTextLedgerPrinter(Ledger const&, std::ostream&);
    ~FlatTextLedgerPrinter();

    void Print() const;

  private:
    void PrintHeader             () const;
    void PrintFooter             () const;
    void PrintNarrativeSummary   () const;
    void PrintKeyTerms           () const;
    void PrintNumericalSummary   () const;
    void PrintRequiredSignatures () const;
    void PrintTabularDetailHeader() const;
    void PrintTabularDetail      () const;

    LedgerInvariant const& invar() const;
    LedgerVariant   const& curr_() const;
    LedgerVariant   const& guar_() const;
    LedgerVariant   const& mdpt_() const;

    Ledger const& ledger_;
    std::ostream& os_;
};

void PrintLedgerFlatText
    (Ledger const& ledger
    ,std::ostream& os
    )
{
    FlatTextLedgerPrinter(ledger, os).Print();
}

// One column of seven characters ('Age 100')
// plus ten columns of twelve characters each (' 999,999,999')
// equals 127 columns; a nine-point font can do that on a page
// eight inches wide and still leave about a half-inch margin
// on both sides.
//
// TODO ?? Avoid overflow by scaling everything if anything's $1B or over.
// TODO ?? Add thousands separators.
namespace
{
    int g_width = 128;
    std::string center(std::string const& s)
        {
        int z = s.length();
        // TODO ?? Strings in the input class might be too wide;
        // absent more graceful handling, at least no attempt is made
        // to cure that problem with a negative number of spaces.
        std::string spaces(std::max(0, (g_width - z) / 2), char(' '));
        return spaces + s;
        }

    std::ostream& endrow(std::ostream& os)
        {
        os << '\n';
        return os;
        }
}

FlatTextLedgerPrinter::FlatTextLedgerPrinter
    (Ledger const& ledger
    ,std::ostream& os
    )
    :ledger_(ledger)
    ,os_    (os)
{
}

FlatTextLedgerPrinter::~FlatTextLedgerPrinter()
{
}

void FlatTextLedgerPrinter::Print() const
{
    set_default_format_flags(os_);

    // TODO ?? Split into numbered pages; add page number to footer.
    PrintHeader             ();
    PrintNarrativeSummary   ();
    PrintKeyTerms           ();
    PrintFooter             ();
    PrintNumericalSummary   ();
    PrintRequiredSignatures ();
    PrintFooter             ();
    PrintTabularDetailHeader();
    PrintTabularDetail      ();
    PrintFooter             ();

    LMI_ASSERT(os_.good());
}

void FlatTextLedgerPrinter::PrintHeader() const
{
    os_ << center("Life Insurance Basic Illustration") << endrow;
    os_ << endrow;
    os_ << center("Prepared on " + iso_8601_datestamp_terse() + " by") << endrow;
    os_ << center(invar().ProducerName) << endrow;
    os_ << center(invar().ProducerStreet) << endrow;
    os_ << center(invar().ProducerCity) << endrow;
    if(ledger_.GetIsComposite())
        {
        os_ << "Composite" << endrow;
        }
    else
        {
        os_ << "Insured: " << invar().Insured1 << endrow;
        os_
            << invar().Gender
            << ' ' << invar().UWClass
            << ' ' << invar().Smoker
            << ", issue age " << value_cast<int>(invar().Age)
            << endrow
            ;
        }
}

void FlatTextLedgerPrinter::PrintFooter() const
{
    os_ << "\f";
}

void FlatTextLedgerPrinter::PrintNarrativeSummary() const
{
//         "123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 "
    os_ << center("Narrative summary") << endrow;
    os_ << endrow;
    os_ << "This is an illustration of a life insurance policy. It is not an offer of insurance. Availability is subject to underwriting." << endrow;
    os_ << endrow;
    os_ << "The premium outlay that must be paid to guarantee coverage for the term of the contract, subject to maximum premiums allowable" << endrow;
    os_ << "to qualify as a life insurance policy under the applicable provisions of the Internal Revenue Code, is" << endrow;
    os_ << endrow;
    os_ << "Policy features, riders or options, guaranteed or non-guaranteed, shown in the basic illustration include:" << endrow;
    os_ << endrow;
    os_ << "Their charges reduce the benefits and values of the policy." << endrow;
    os_ << endrow;
    os_ << "This illustration assumes that the currently illustrated nonguaranteed elements will continue unchanged for all years shown." << endrow;
    os_ << "This is not likely to occur, and actual results may be more or less favorable than those shown." << endrow;
    os_ << endrow;
    os_ << "Overhead is fully allocated to expense charges." << endrow;
    os_ << endrow;
    os_ << "Except as otherwise specified, values and benefits are not guaranteed. The assumptions on which they are based are subject to" << endrow;
    os_ << "change by the insurer. Actual results may be more or less favorable. If policy charges are paid using non-guaranteed values, the" << endrow;
    os_ << "charges continue to be required. Depending on actual results, the premium payer may need to continue or resume premium outlays." << endrow;
    os_ << endrow;
    os_ << "This illustration assumes that premiums are paid at the beginning of each period. Values and benefits are shown as of the end of" << endrow;
    os_ << "the year." << endrow;
    os_ << endrow;
}

void FlatTextLedgerPrinter::PrintKeyTerms() const
{
    os_ << center("Column headings and key terms") << endrow;
    os_ << endrow;
    os_ << "Premium Outlay is the amount assumed to be paid out of pocket, net of any loan or withdrawal." << endrow;
    os_ << endrow;
    os_ << "Deductions are expense charges and mortality charges." << endrow;
    os_ << endrow;
    os_ << "Account Value is the accumulated value of Premium Outlay, minus Deductions, plus interest." << endrow;
    os_ << endrow;
    os_ << "Surrender Value is what you get if you surrender the policy: Account Value minus any surrender charges or outstanding loans." << endrow;
    os_ << endrow;
    os_ << "Death Benefit is the amount payable by reason of death." << endrow;
    os_ << endrow;
    os_ << "Guaranteed values reflect guaranteed interest and Deductions." << endrow;
    os_ << endrow;
    os_ << "Non-guaranteed values reflect non-guaranteed interest and Deductions, which are subject to change." << endrow;
    os_ << endrow;
    os_ << "Midpoint values reflect the midpoint of guaranteed and non-guaranteed interest and Deductions." << endrow;
    os_ << endrow;
}

void FlatTextLedgerPrinter::PrintNumericalSummary() const
{
    os_ << center("Numerical summary") << endrow;
    os_ << endrow;
    os_ << "                    ------------Guaranteed------------- -------------Midpoint-------------- ----------Non-guaranteed-----------" << endrow;
    os_ << "            Premium     Account   Surrender       Death     Account   Surrender       Death     Account   Surrender       Death" << endrow;
    os_ << "   Year      Outlay       Value       Value     Benefit       Value       Value     Benefit       Value       Value     Benefit" << endrow;
    os_ << endrow;

    int summary_rows[] = {4, 9, 19, std::min(99, 69 - value_cast<int>(invar().Age))};

    for(int j = 0; j < static_cast<int>(sizeof summary_rows / sizeof(int)); ++j)
        {
        int row = summary_rows[j];
        // Skip row if it doesn't exist. For instance, if issue age is
        // 85 and maturity age is 100, then there is no twentieth duration.
        if(ledger_.GetMaxLength() < 1 + row)
            {
            continue;
            }

        os_.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os_.precision(0);
        os_.width(7);

        os_ << std::setw( 7) << (1 + row)               ;

        os_.precision(2);

        os_ << std::setw(12) << invar().Outlay     [row];

        os_ << std::setw(12) << guar_().AcctVal    [row];
        os_ << std::setw(12) << guar_().CSVNet     [row];
        os_ << std::setw(12) << guar_().EOYDeathBft[row];

        os_ << std::setw(12) << mdpt_().AcctVal    [row];
        os_ << std::setw(12) << mdpt_().CSVNet     [row];
        os_ << std::setw(12) << mdpt_().EOYDeathBft[row];

        os_ << std::setw(12) << curr_().AcctVal    [row];
        os_ << std::setw(12) << curr_().CSVNet     [row];
        os_ << std::setw(12) << curr_().EOYDeathBft[row];

        os_ << endrow;
        }

// TODO ?? Print "Age  70" instead of duration for last row. Does the
// illustration reg require any other ages?
    os_ << endrow;
}

void FlatTextLedgerPrinter::PrintRequiredSignatures() const
{
    os_ << center("Required signatures") << endrow;
    os_ << endrow;
    os_ << "I have received a copy of this illustration and understand that" << endrow;
    os_ << "any non-guaranteed elements illustrated are subject to change" << endrow;
    os_ << "and could be either higher or lower." << endrow;
    os_ << "The agent has told me they are not guaranteed." << endrow;
    os_ << endrow;
    os_ << endrow;
    os_ << "_________________________  ______" << endrow;
    os_ << "Applicant or policy owner  Date" << endrow;
    os_ << endrow;
    os_ << "I certify that this illustration has been presented to the applicant" << endrow;
    os_ << "and that I have explained that any non-guaranteed elements" << endrow;
    os_ << "illustrated are subject to change. I have made no statements that" << endrow;
    os_ << "are inconsistent with the illustration." << endrow;
    os_ << endrow;
    os_ << endrow;
    os_ << "___________________________________________  ______" << endrow;
    os_ << "Producer or other authorized representative  Date" << endrow;
}

void FlatTextLedgerPrinter::PrintTabularDetailHeader() const
{
    os_ << "Tabular detail" << endrow;
    os_ << endrow;
    os_ << "                                ------------Guaranteed------------- ----------Non-guaranteed-----------" << endrow;
    os_ << "             Age at     Premium     Account   Surrender       Death     Account   Surrender       Death" << endrow;
    os_ << "   Year    Year End      Outlay       Value       Value     Benefit       Value       Value     Benefit" << endrow;
    os_ << endrow;
}

void FlatTextLedgerPrinter::PrintTabularDetail() const
{
    int age = value_cast<int>(invar().Age);
    for(int j = 0; j < ledger_.GetMaxLength(); ++j)
        {
        os_.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os_.precision(0);

        os_ << std::setw( 7) << (1 + j      )         ;

        if(ledger_.GetIsComposite())
            {
            os_ << std::setw(12) << ' '               ;
            }
        else
            {
            os_ << std::setw(12) << (1 + j + age)     ;
            }

        os_.precision(2);

        os_ << std::setw(12) << invar().Outlay     [j];

        os_ << std::setw(12) << guar_().AcctVal    [j];
        os_ << std::setw(12) << guar_().CSVNet     [j];
        os_ << std::setw(12) << guar_().EOYDeathBft[j];

        os_ << std::setw(12) << curr_().AcctVal    [j];
        os_ << std::setw(12) << curr_().CSVNet     [j];
        os_ << std::setw(12) << curr_().EOYDeathBft[j];

        os_ << endrow;
        }
}

inline LedgerInvariant const& FlatTextLedgerPrinter::invar() const
{return ledger_.GetLedgerInvariant();}

inline LedgerVariant   const& FlatTextLedgerPrinter::curr_() const
{return ledger_.GetCurrFull();}

inline LedgerVariant   const& FlatTextLedgerPrinter::guar_() const
{return ledger_.GetGuarFull();}

inline LedgerVariant   const& FlatTextLedgerPrinter::mdpt_() const
{return ledger_.GetMdptFull();}

