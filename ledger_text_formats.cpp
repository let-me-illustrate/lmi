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

// $Id: ledger_text_formats.cpp,v 1.12 2005-09-27 16:49:11 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_text_formats.hpp"

#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"
#include "financial.hpp"
#include "global_settings.hpp"
#include "input_sequence.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "security.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iterator>
#include <locale>
#include <sstream>
#include <vector>

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
    // ET !! net_payment = Invar.GrossPmt - Invar.Loan - Invar.NetWD;
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
    (Ledger const& ledger_values
    ,std::string const& file_name
    )
{
    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();
    LedgerVariant   const& Guar_ = ledger_values.GetGuarFull();

    int max_length = ledger_values.GetMaxLength();

    std::vector<double> net_payment;
    set_net_payment(Invar, net_payment);

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
    // ET !! net_payment += drop(-1, cash_flow);
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
    os << "  Total:\t\t"     << value_cast<std::string>(total_spec_amt) << '\n';
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
        validate_security(!global_settings::instance().ash_nazg());
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
        ,"NetClaims"
        ,"ExpRatRsvCash"
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
        os << value_cast<std::string>(net_outlay) << '\t';

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
        os << Curr_.value_str("AcctValLoadAMD"        ,j) << '\t';
        os << Invar.value_str("MonthlyFlatExtra"      ,j) << '\t';
        os << Curr_.value_str("COICharge"             ,j) << '\t';

        os << Curr_.value_str("AnnSAIntRate"          ,j) << '\t';
        os << Curr_.value_str("AnnGAIntRate"          ,j) << '\t';
        os << Curr_.value_str("GrossIntCredited"      ,j) << '\t';
        os << Curr_.value_str("NetIntCredited"        ,j) << '\t';

        // First element of InforceLives is BOY--show only EOY.
        os << value_cast<std::string>(Invar.InforceLives[1 + j]) << '\t';

        // TODO ?? For experience rating, it would be nice to add
        // projected mortality charge and k factor here. That requires
        // adding them to the ledger class first.
        os << Curr_.value_str("ClaimsPaid"            ,j) << '\t';
        os << Curr_.value_str("NetClaims"             ,j) << '\t';
        os << Curr_.value_str("ExpRatRsvCash"         ,j) << '\t';
        os << Invar.value_str("ProducerCompensation"  ,j) << '\t';

        os << '\n';
        }
// TODO ?? Should we add death benefit option?
}

