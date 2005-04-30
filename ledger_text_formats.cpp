// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger_text_formats.cpp,v 1.1 2005-04-30 18:15:33 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_text_formats.hpp"

#include "calendar_date.hpp"
#include "configurable_settings.hpp"
#include "financial.hpp"
#include "global_settings.hpp"
#include "input_sequence.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "security.hpp"
#include "value_cast_ihs.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iterator>
#include <sstream>
#include <vector>

namespace
{
// TODO ?? Move this to a place whence it can be reused, and add unit
// tests: it is suspected of formatting -100 as "-,100".
//
class comma_punct
    :public std::numpunct<char>
{
  protected:
    char do_thousands_sep() const {return ',';}
    std::string do_grouping() const {return "\3";}
  public:
    comma_punct()
        :std::numpunct<char>()
        {}
};
} // Unnamed namespace.

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
        << "<body text=\"#000000\" bgcolor=\"#B0B0B0\">"

        << "<p>"
        << " <h5>"
        << "  <font color=\"#804040\">"
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
            << "<br>"
            ;

        if(is_subject_to_ill_reg(ledger_values.GetLedgerType()))
            {
            oss
                << std::setprecision(2)
                << Invar.GuarPrem << "   guaranteed premium<br>"
                ;
            }

        oss
            << std::setprecision(2)
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
        << "  </font>"
        << " </h5>"
        << "</p>"

        << "<hr>"
        << "<table align=right>"
        << "  <tr>"
        << "    <th>Age</th> <th>Outlay</th>"
        << "    <th>GuarAV</th> <th>GuarCSV</th> <th>GuarDB</th>"
        << "    <th>CurrAV</th> <th>CurrCSV</th> <th>CurrDB</th>"
        << "  </tr>"
        ;

    for(int j = 0; j < max_length; ++j)
        {
        oss
            << "<tr>"
            << "<td>" << std::setprecision(0) << j + Invar.Age        << "</td>"
            << "<td>" << std::setprecision(2) << Invar.Outlay[j]      << "</td>"
            << "<td>" << std::setprecision(2) << Guar_.AcctVal[j]     << "</td>"
            << "<td>" << std::setprecision(2) << Guar_.CSVNet[j]      << "</td>"
            << "<td>" << std::setprecision(2) << Guar_.EOYDeathBft[j] << "</td>"
            << "<td>" << std::setprecision(2) << Curr_.AcctVal[j]     << "</td>"
            << "<td>" << std::setprecision(2) << Curr_.CSVNet[j]      << "</td>"
            << "<td>" << std::setprecision(2) << Curr_.EOYDeathBft[j] << "</td>"
            << "</tr>"
            ;
        }

    oss
        << "</table>"
        << "</body>"
        ;
    return oss.str();
}

// Experimental--see development notes for 2003-03-04 for a discussion
// of other approaches.
namespace
{
// Subtract loans and WDs from gross payments, yielding net payments.
// TODO ?? Obsolete--use LedgerInvariant::Outlay instead.
void set_net_payment
    (LedgerInvariant const& Invar
    ,std::vector<double>&   net_payment
    )
{
    net_payment = Invar.GrossPmt;
    std::transform
        (net_payment.begin()
        ,net_payment.end()
        ,Invar.Loan.begin()
        ,net_payment.begin()
        ,std::minus<double>()
        );
    std::transform
        (net_payment.begin()
        ,net_payment.end()
        ,Invar.NetWD.begin()
        ,net_payment.begin()
        ,std::minus<double>()
        );
}
} // Unnamed namespace.

//==============================================================================
void PrintFormTabDelimited
    (Ledger const& a_LedgerValues
    ,std::string const& file_name
//    ) const
    )
{
    LedgerInvariant const& Invar = a_LedgerValues.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = a_LedgerValues.GetCurrFull();
    LedgerVariant   const& Guar_ = a_LedgerValues.GetGuarFull();

    int max_length = a_LedgerValues.GetMaxLength();

    std::vector<double> net_payment;
    set_net_payment(Invar, net_payment);

    std::vector<double> real_claims;
    if(a_LedgerValues.GetIsComposite())
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
    std::transform
        (cash_flow.begin()
        ,cash_flow.end()
        ,cash_flow.begin()
        ,std::negate<double>()
        );
    std::transform
        (net_payment.begin()
        ,net_payment.end()
        ,cash_flow.begin()
        ,cash_flow.begin()
        ,std::plus<double>()
        );

    cash_flow.pop_back(); // Here we no longer need cash_flow[omega].

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
std::ofstream os
        ("irr.txt"
        ,   std::ios_base::out
          | std::ios_base::ate
          | std::ios_base::app
        );
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

    std::ofstream os
        (file_name.c_str()
        ,  std::ios_base::out
         | std::ios_base::ate
         | std::ios_base::app
        );

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
    os << "  Total:\t\t"     << value_cast_ihs<std::string>(total_spec_amt) << '\n';
    os << "PolicyMktgName\t\t"    << Invar.value_str("PolicyMktgName" ) << '\n';
    os << "PolicyLegalName\t\t"   << Invar.value_str("PolicyLegalName") << '\n';
    os << "PolicyForm\t\t"        << Invar.value_str("PolicyForm"     ) << '\n';
    os << "UWClass\t\t"           << Invar.value_str("UWClass"        ) << '\n';
    os << "UWType\t\t"            << Invar.value_str("UWType"         ) << '\n';

    // We surround the date in single quotes because one popular
    // spreadsheet would otherwise interpret it as a date, which
    // is likely not to fit in a default-width cell.
    if(!global_settings::instance().regression_testing())
        {
        // Skip security validation for the most privileged password.
        validate_security(!global_settings::instance().ash_nazg);
        os << "DatePrepared\t\t'" << calendar_date().str() << "'\n";
        }
    else
        {
        // For regression tests, use EffDate as date prepared,
        // in order to avoid gratuitous failures.
        os << "DatePrepared\t\t'" << Invar.EffDate << "'\n";
        }

        os << '\n';

    // This column ordering reflects the natural processing order, but
    // with the most commonly wanted columns placed first.
    char const* cheaders[] =
        {"PolicyYear"
        ,"AttainedAge"
        ,"DeathBenefitOption"
        ,"TotalNetOutlay"
        ,"GuaranteedAccountValue"
        ,"GuaranteedNetCashSurrenderValue"
        ,"GuaranteedYearEndDeathBenefit"
        ,"CurrentAccountValue"
        ,"CurrentNetCashSurrenderValue"
        ,"CurrentYearEndDeathBenefit"
        ,"IrrOnSurrender"
        ,"IrrOnDeath"
        ,"EmployeeGrossPremium"
        ,"CorporationGrossPremium"
        ,"GrossWithdrawal"
        ,"NewCashLoan"
        ,"NetPayment"
        ,"PremiumTaxLoad"
        ,"DacTaxLoad"
        ,"MonthlyPolicyFee"
        ,"AnnualPolicyFee"
        ,"SpecifiedAmountLoad"
        ,"AccountValueLoadBeforeMonthlyDeduction"
        ,"AccountValueLoadAfterMonthlyDeduction"
        ,"MonthlyFlatExtra"
        ,"MortalityCharge"
        ,"SeparateAccountInterestRate"
        ,"GeneralAccountInterestRate"
        ,"GrossInterestCredited"
        ,"NetInterestCredited"
        ,"YearEndInforceLives"
        ,"ClaimsPaid"
        ,"ProducerCompensation"
        };

    std::vector<std::vector<std::string> > sheaders;

    unsigned int max_header_rows = 0;
    for(unsigned int j = 0; j < lmi_array_size(cheaders); ++j)
        {
//        std::istringstream iss(insert_spaces_between_words(cheaders[j]));
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

        double net_outlay =
              Invar.GrossPmt[j]
            - Invar.NetWD     [j]
            - Invar.Loan      [j]
            ;
        os << value_cast_ihs<std::string>(net_outlay) << '\t';

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

        // The following columns are in a logical order. The last few
        // columns immediately preceding aren't, because an important
        // group of users wanted the columns they use on the left,
        // before the columns they ignore.

        os << Invar.value_str("EeGrossPmt"            ,j) << '\t';
        os << Invar.value_str("ErGrossPmt"            ,j) << '\t';
        os << Invar.value_str("NetWD"                 ,j) << '\t'; // TODO ?? It's *gross* WD.
        os << Invar.value_str("Loan"                  ,j) << '\t';
        os << Curr_.value_str("NetPmt"                ,j) << '\t';

        os << Curr_.value_str("PremTaxLoad"           ,j) << '\t';
        os << Curr_.value_str("DacTaxLoad"            ,j) << '\t';
        os << Curr_.value_str("MlyPolFee"             ,j) << '\t';
        os << Curr_.value_str("AnnPolFee"             ,j) << '\t';
        os << Curr_.value_str("SpecAmtLoad"           ,j) << '\t';
        os << Curr_.value_str("AcctValLoadBOM"        ,j) << '\t';
        os << Curr_.value_str("AcctValLoadAMD"        ,j) << '\t';
        os << Invar.value_str("MonthlyFlatExtra"      ,j) << '\t';
        os << Curr_.value_str("COICharge"             ,j) << '\t';

        os << Curr_.value_str("AnnSAIntRate"          ,j) << '\t';
        os << Curr_.value_str("AnnGAIntRate"          ,j) << '\t';
        os << Curr_.value_str("GrossIntCredited"      ,j) << '\t';
        os << Curr_.value_str("NetIntCredited"        ,j) << '\t';

        // First element of InforceLives is BOY--show only EOY.
        os << value_cast_ihs<std::string>(Invar.InforceLives[1 + j]) << '\t';
        os << Curr_.value_str("ClaimsPaid"            ,j) << '\t';
        os << Invar.value_str("ProducerCompensation"  ,j) << '\t';

        os << '\n';
        }
// TODO ?? Should we add death benefit option?
}

//==============================================================================
void PrintFormSpecial(Ledger const& a_LedgerValues)
{
    LedgerInvariant const& Invar = a_LedgerValues.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = a_LedgerValues.GetCurrFull();

    std::ofstream os
        (configurable_settings::instance().custom_output_filename().c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );

    if(!os.good())
        {
        hobsons_choice() << "Error initializing output file." << LMI_FLUSH;
        }

    os
        << "CashValu,SurrValu,DeathBen,IntEarned,"
        << "MortCost,Load,MinPrem,SurrCost,PremAmt,IntRate\n"
        ;

// TODO ?? "??? Is this exactly what the customer wanted?"
    // Surr charge = acct val - cash surr val
    std::vector<double> surr_chg(Curr_.AcctVal);
    std::transform
        (surr_chg.begin()
        ,surr_chg.end()
        ,Curr_.CSVNet.begin()
        ,surr_chg.begin()
        ,std::minus<double>()
        );

    // Load = gross pmt - net pmt.
    std::vector<double> prem_load(Invar.GrossPmt);
    std::transform
        (prem_load.begin()
        ,prem_load.end()
        ,Curr_.NetPmt.begin()
        ,prem_load.begin()
        ,std::minus<double>()
        );

    os.setf(std::ios_base::fixed, std::ios_base::floatfield);

    int max_duration = static_cast<int>(std::min(95.0, Invar.EndtAge) - Invar.Age);
    for(int j = 0; j < max_duration; j++)
        {
        // Customer requirement: show interest rate in bp.
        double gen_acct_int_rate_bp = 10000.0 * Curr_.AnnGAIntRate[j];
        os
            << std::setprecision(0) << Curr_.AcctVal         [j] << ','
            << std::setprecision(0) << Curr_.CSVNet          [j] << ','
            << std::setprecision(0) << Curr_.EOYDeathBft     [j] << ','
            << std::setprecision(0) << Curr_.NetIntCredited  [j] << ','
            << std::setprecision(0) << Curr_.COICharge       [j] << ','
            // Assume 'min prem' is zero--see comments below.
            << std::setprecision(0) << 0 << ','
            << std::setprecision(0) << prem_load             [j] << ','
            << std::setprecision(0) << surr_chg              [j] << ','
            << std::setprecision(0) << Invar.GrossPmt        [j] << ','
            << std::setprecision(0) << gen_acct_int_rate_bp
            << std::setprecision(0) << '\n'
            ;
        }
/*
TODO ?? Resolve these comments.

dir where installed: instead, dir from which run
age 95 even though pol goes to 100
behavior if file locked?

values: all as of EOY
assume "interest earned" is net interest credited, net of any spread
assume "mortality cost" is sum of actual COIs deducted throughout the year
assume "load" is premium load including any sales load and premium-based
  loads for premium tax and dac tax, but excluding policy fee
assume "minimum premium" is a required premium as is typical of interest
  sensitive whole life, and should be zero for flexible premium universal life
assume "surrender cost" is account value minus cash surrender value; if
  there is any refund in the early years, this value can be negative
assume file is terminated with a CRLF at the end of the last line,
  with no EOF character following

*/
    if(!os.good())
        {
        hobsons_choice() << "Error writing output file." << LMI_FLUSH;
        }
}

