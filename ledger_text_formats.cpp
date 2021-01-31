// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "ledger_text_formats.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "authenticity.hpp"
#include "bourn_cast.hpp"
#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"    // effective_calculation_summary_columns()
#include "contains.hpp"
#include "global_settings.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "map_lookup.hpp"
#include "mc_enum_types_aux.hpp"        // is_subject_to_ill_reg()
#include "miscellany.hpp"
#include "ssize_lmi.hpp"
#include "value_cast.hpp"

#include <algorithm>                    // find()
#include <fstream>
#include <iomanip>                      // setprecision()
#include <ios>                          // ios_base
#include <locale>
#include <map>
#include <ostream>
#include <sstream>

namespace
{
/// Ledger metadata, for calculation-summary columns only.
///
/// The legends here are from 'ledger_xml_io.cpp', with underscores
/// removed. Alternative legends from 'mc_enum_types.xpp' are shown in
/// comments when they differ. Exception:
///  - 'AttainedAge' is not labelled "End of Year Age" as in
///    'ledger_xml_io.cpp', where it differs numerically. Insurance
///    age changes not at the end of the current year, but rather at
///    the beginning of the next year.

std::map<std::string,ledger_metadata> const& ledger_metadata_map()
{
    static std::map<std::string,ledger_metadata> m;

    if(m.empty())
        {
        m["[none]"                     ] = ledger_metadata(0, oe_format_normal    , "[none]"                                );
        m["AttainedAge"                ] = ledger_metadata(0, oe_format_normal    , "Attained Age"                          );
        m["PolicyYear"                 ] = ledger_metadata(0, oe_format_normal    , "Policy Year"                           );
        m["InforceLives"               ] = ledger_metadata(4, oe_format_normal    , "BOY Lives Inforce"                     ); // "Inforce Lives BOY"
        m["SpecAmt"                    ] = ledger_metadata(0, oe_format_normal    , "Specified Amount"                      ); // "Base Specified Amount"
        m["TermSpecAmt"                ] = ledger_metadata(0, oe_format_normal    , "Term Specified Amount"                 );
        m["SupplSpecAmt"               ] = ledger_metadata(0, oe_format_normal    , "Suppl Specified Amount"                ); // "Supplemental Specified Amount"
        m["CorridorFactor"             ] = ledger_metadata(0, oe_format_percentage, "Corridor Factor"                       );
        m["AnnGAIntRate_Current"       ] = ledger_metadata(2, oe_format_percentage, "Curr Ann Gen Acct Int Rate"            ); // "General Account Crediting Rate"
        m["AnnSAIntRate_Current"       ] = ledger_metadata(2, oe_format_percentage, "Curr Ann Sep Acct Int Rate"            ); // "Separate Account Net Rate"
        m["Outlay"                     ] = ledger_metadata(0, oe_format_normal    , "Net Outlay"                            );
        m["EeGrossPmt"                 ] = ledger_metadata(0, oe_format_normal    , "EE Gross Payment"                      ); // "Employee Gross Payment"
        m["ErGrossPmt"                 ] = ledger_metadata(0, oe_format_normal    , "ER Gross Payment"                      ); // "Employer Gross Payment"
        m["ModalMinimumPremium"        ] = ledger_metadata(0, oe_format_normal    , "Modal Minimum Premium"                 );
        m["EeModalMinimumPremium"      ] = ledger_metadata(0, oe_format_normal    , "EE Modal Minimum Premium"              );
        m["ErModalMinimumPremium"      ] = ledger_metadata(0, oe_format_normal    , "ER Modal Minimum Premium"              );
        m["NetWD"                      ] = ledger_metadata(0, oe_format_normal    , "Withdrawal"                            );
        m["NewCashLoan"                ] = ledger_metadata(0, oe_format_normal    , "Annual Loan"                           ); // "New Cash Loan"
        m["TotalLoanBalance_Current"   ] = ledger_metadata(0, oe_format_normal    , "Curr Total Loan Balance"               ); // "Current Total Loan Balance"
        m["TotalLoanBalance_Guaranteed"] = ledger_metadata(0, oe_format_normal    , "Guar Total Loan Balance"               ); // "Guaranteed Total Loan Balance"
        m["AcctVal_Current"            ] = ledger_metadata(0, oe_format_normal    , "Curr Account Value"                    ); // "Current Account Value"
        m["AcctVal_Guaranteed"         ] = ledger_metadata(0, oe_format_normal    , "Guar Account Value"                    ); // "Guaranteed Account Value"
        m["CSVNet_Current"             ] = ledger_metadata(0, oe_format_normal    , "Curr Net Cash Surr Value"              ); // "Current Cash Surrender Value"
        m["CSVNet_Guaranteed"          ] = ledger_metadata(0, oe_format_normal    , "Guar Net Cash Surr Value"              ); // "Guaranteed Cash Surrender Value"
        m["EOYDeathBft_Current"        ] = ledger_metadata(0, oe_format_normal    , "Curr EOY Death Benefit"                ); // "Current Death Benefit"
        m["EOYDeathBft_Guaranteed"     ] = ledger_metadata(0, oe_format_normal    , "Guar EOY Death Benefit"                ); // "Guaranteed Death Benefit"
        m["BaseDeathBft_Current"       ] = ledger_metadata(0, oe_format_normal    , "Curr Base Death Benefit"               ); // "Current Base Death Benefit"
        m["BaseDeathBft_Guaranteed"    ] = ledger_metadata(0, oe_format_normal    , "Guar Base Death Benefit"               ); // "Guaranteed Base Death Benefit"
        m["TermPurchased_Current"      ] = ledger_metadata(0, oe_format_normal    , "Curr Term Amt Purchased"               ); // "Current Term Purchased"
        m["TermPurchased_Guaranteed"   ] = ledger_metadata(0, oe_format_normal    , "Guar Term Amt Purchased"               ); // "Guaranteed Term Purchased"
        m["SupplDeathBft_Current"      ] = ledger_metadata(0, oe_format_normal    , "Curr Suppl Death Benefit"              ); // "Current Supplemental Death Benefit"
        m["SupplDeathBft_Guaranteed"   ] = ledger_metadata(0, oe_format_normal    , "Guar Suppl Death Benefit"              ); // "Guaranteed Supplemental Death Benefit"
        m["COICharge_Current"          ] = ledger_metadata(0, oe_format_normal    , "Curr COI Charge"                       ); // "Current Mortality Charge"
        m["COICharge_Guaranteed"       ] = ledger_metadata(0, oe_format_normal    , "Guar COI Charge"                       ); // "Guaranteed Mortality Charge"
        m["RiderCharges_Current"       ] = ledger_metadata(0, oe_format_normal    , "Curr Rider Charges"                    ); // "Current Rider Charges"
        m["IrrCsv_Current"             ] = ledger_metadata(2, oe_format_percentage, "Curr IRR on CSV"                       ); // "Current Cash Value IRR"
        m["IrrCsv_Guaranteed"          ] = ledger_metadata(2, oe_format_percentage, "Guar IRR on CSV"                       ); // "Guaranteed Cash Value IRR"
        m["IrrDb_Current"              ] = ledger_metadata(2, oe_format_percentage, "Curr IRR on DB"                        ); // "Current Death Benefit IRR"
        m["IrrDb_Guaranteed"           ] = ledger_metadata(2, oe_format_percentage, "Guar IRR on DB"                        ); // "Guaranteed Death Benefit IRR"
        m["NetClaims_Current"          ] = ledger_metadata(0, oe_format_normal    , "Curr Net Claims"                       ); // "Experience Rating Current Net Claims"
        m["GrossPmt"                   ] = ledger_metadata(0, oe_format_normal    , "Premium Outlay"                        ); // "Total Payment"
        m["LoanIntAccrued_Current"     ] = ledger_metadata(0, oe_format_normal    , "Curr Loan Int Accrued"                 ); // "Current Accrued Loan Interest"
        m["NetDeathBenefit"            ] = ledger_metadata(0, oe_format_normal    , "Net Death Benefit"                     ); // "Current Net Death Benefit"
        m["DeathProceedsPaid_Current"  ] = ledger_metadata(0, oe_format_normal    , "Curr Death Proceeds Paid"              ); // "Current Death Proceeds Paid"
        m["ClaimsPaid_Current"         ] = ledger_metadata(0, oe_format_normal    , "Curr Claims Paid"                      ); // "Current Claims Paid"
        m["AVRelOnDeath_Current"       ] = ledger_metadata(0, oe_format_normal    , "Account Value Released on Death"       ); // "Current Account Value Released on Death"
        m["SpecAmtLoad_Current"        ] = ledger_metadata(0, oe_format_normal    , "Curr Spec Amt Load"                    ); // "Current Load on Specified Amount"
        m["GrossIntCredited_Current"   ] = ledger_metadata(0, oe_format_normal    , "Curr Gross Int Credited"               ); // "Current Interest Credited before Separate Account Charges"
        m["NetIntCredited_Current"     ] = ledger_metadata(0, oe_format_normal    , "Curr Net Int Credited"                 ); // "Current Interest Credited Net of Separate Account Charges"
        m["SepAcctCharges_Current"     ] = ledger_metadata(0, oe_format_normal    , "Curr Sep Acct Charges"                 ); // "Current Separate Account Asset Charges"
        m["PolicyFee_Current"          ] = ledger_metadata(0, oe_format_normal    , "Curr Policy Fee"                       ); // "Current Policy Fee"
        }

    return m;
}

class calculation_summary_formatter final
{
  public:
    calculation_summary_formatter(Ledger const&);
    ~calculation_summary_formatter() = default;

    std::string format_as_html() const;
    std::string format_as_tsv () const;

    std::string top_note(std::string const& line_break) const;

  private:
    calculation_summary_formatter(calculation_summary_formatter const&) = delete;
    calculation_summary_formatter& operator=(calculation_summary_formatter const&) = delete;

    Ledger          const&   ledger_;
    LedgerInvariant const&   invar_;
    int             const    max_length_;
    std::vector<std::string> columns_;
};

calculation_summary_formatter::calculation_summary_formatter
    (Ledger const& ledger_values
    )
    :ledger_     {ledger_values}
    ,invar_      {ledger_values.GetLedgerInvariant()}
    ,max_length_ {ledger_values.greatest_lapse_dur()}
{
    columns_ = effective_calculation_summary_columns();
    std::vector<std::string>::iterator p = std::find
        (columns_.begin()
        ,columns_.end()
        ,"PolicyYear"
        );
    // TODO ?? This should be done in effective_calculation_summary_columns(),
    // but that requires a difficult-to-test change in 'ledger_xml_io2.cpp'.
    // As long as "PolicyYear" is always the first column, it shouldn't be
    // offered for selection anyway.
    if(columns_.end() != p)
        {
        columns_.erase(p);
        }
    columns_.insert(columns_.begin(), "PolicyYear");

    // Calculate IRRs only when necessary, because of the palpable
    // effect on responsiveness--see:
    //   https://lists.nongnu.org/archive/html/lmi/2018-02/msg00098.html
    bool want_any_irr =
           contains(columns_, "IrrCsv_Current"   )
        || contains(columns_, "IrrCsv_Guaranteed")
        || contains(columns_, "IrrDb_Current"    )
        || contains(columns_, "IrrDb_Guaranteed" )
        ;
    if(want_any_irr && !invar_.is_irr_initialized())
        {
        // TODO ?? This const_cast is safe, but it's still unclean.
        LedgerInvariant& unclean = const_cast<LedgerInvariant&>(invar_);
        unclean.CalculateIrrs(ledger_);
        }
}

std::string calculation_summary_formatter::top_note
    (std::string const& line_break
    ) const
{
    std::ostringstream oss;
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

    if(ledger_.is_composite())
        {
        oss << "Composite calculation summary\n";
        }
    else
        {
        oss
            << "Calculation summary for: "
            << invar_.Insured1
            << line_break
            << invar_.Gender << ", " << invar_.Smoker
            << std::setprecision(0)
            << ", age " << invar_.Age
            << ", " << invar_.StateOfJurisdiction << " jurisdiction"
            << line_break
            ;
        if(invar_.IsMec)
            {
            oss << "MEC in policy year " << 1 + invar_.MecYear;
            }
        else
            {
            oss << "Not a MEC";
            }
        oss << '\n';
        }

    return oss.str();
}

std::string calculation_summary_formatter::format_as_html() const
{
    std::ostringstream oss;

    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    oss.imbue(new_loc);
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

    oss
        << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
        << "    \"http://www.w3.org/TR/html4/loose.dtd\">\n"
        << "<html>\n"
        << "<head>\n"
        << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n"
        << "<title>Let me illustrate...</title>\n"
        << "</head>\n"
        << "<body>\n"
        ;

    oss << "<p>\n" << top_note("<br>\n") << "</p>\n";

    if(!ledger_.is_composite())
        {
        oss << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n";
        if(is_subject_to_ill_reg(ledger_.ledger_type()))
            {
            oss
            << "<tr>\n"
            << "  <td align=\"right\" nowrap></td>\n"
            << "  <td align=\"left\"  nowrap></td>\n"
            << std::setprecision(2)
            << "  <td align=\"right\" nowrap>" << invar_.GuarPrem         << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;guaranteed premium</td>\n"
            << "</tr>\n"
            ;
            }
        oss
            << "<tr>\n"
            << "  <td align=\"right\" nowrap></td>\n"
            << "  <td align=\"left\"  nowrap></td>\n"
            << std::setprecision(2)
            << "  <td align=\"right\" nowrap>" << invar_.InitGLP          << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;initial guideline level premium</td>\n"
            << "</tr>\n"
            << "<tr>\n"
            << std::setprecision(0)
            << "  <td align=\"right\" nowrap>" << invar_.InitBaseSpecAmt  << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;initial base specified amount</td>\n"
            << std::setprecision(2)
            << "  <td align=\"right\" nowrap>" << invar_.InitGSP          << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;initial guideline single premium</td>\n"
            << "</tr>\n"
            << "<tr>\n"
            << std::setprecision(0)
            << "  <td align=\"right\" nowrap>" << invar_.InitTermSpecAmt << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;initial term specified amount</td>\n"
            << std::setprecision(2)
            << "  <td align=\"right\" nowrap>" << invar_.InitSevenPayPrem << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;initial seven-pay premium</td>\n"
            << "</tr>\n"
            << "<tr>\n"
            << std::setprecision(0)
            << "  <td align=\"right\" nowrap>" << invar_.InitBaseSpecAmt + invar_.InitTermSpecAmt << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;initial total specified amount</td>\n"
            << std::setprecision(2)
            << "  <td align=\"right\" nowrap>" << invar_.InitTgtPrem      << "</td>\n"
            << "  <td align=\"left\"  nowrap>&nbsp;initial target premium</td>\n"
            << "</tr>\n"
            << "</table>\n"
            ;
        }

    std::string const width = value_cast<std::string>(100 / columns_.size());
    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"
        << "<tr align=\"right\">\n"
        ;
    for(auto const& i : columns_)
        {
        ledger_metadata const& z = map_lookup(ledger_metadata_map(), i);
        oss << "<td valign=\"bottom\" width=\"" << width << "%\">" << z.legend_ << " </td>\n";
        }
    oss << "</tr>\n";

    for(int j = 0; j < max_length_; ++j)
        {
        if(0 == j % 5)
            {
            oss << "<tr><td><br></td></tr>\n";
            }
        oss << "<tr align=\"right\">\n";
        for(auto const& i : columns_)
            {
            ledger_metadata const& z = map_lookup(ledger_metadata_map(), i);
            std::string s = ledger_format
                (numeric_vector(ledger_, i)[j]
                ,std::make_pair(z.decimals_, z.style_)
                );
            oss << "<td nowrap>&nbsp;&nbsp;&nbsp;" << s << "</td>\n";
            }
        oss << "</tr>\n";
        }

    oss
        << "</table>\n"
        << "</body>\n"
        << "</html>\n"
        ;
    return oss.str();
}

std::string calculation_summary_formatter::format_as_tsv() const
{
    std::ostringstream oss;

    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    oss.imbue(new_loc);
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

    oss << "\n\nFOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.\n\n";

    oss << top_note("\n") << '\n';

    if(!ledger_.is_composite())
        {
        if(is_subject_to_ill_reg(ledger_.ledger_type()))
            {
            oss
            << std::setprecision(2)
            << invar_.GuarPrem
            << "\tguaranteed premium\n"
            ;
            }
        oss
            << std::setprecision(2)
            << invar_.InitGLP
            << "\tinitial guideline level premium\n"
            << std::setprecision(2)
            << invar_.InitGSP
            << "\tinitial guideline single premium\n"
            << std::setprecision(2)
            << invar_.InitSevenPayPrem
            << "\tinitial seven-pay premium\n"
            << std::setprecision(2)
            << invar_.InitTgtPrem
            << "\tinitial target premium\n"
            << std::setprecision(0)
            << invar_.InitBaseSpecAmt
            << "\tinitial base specified amount\n"
            << std::setprecision(0)
            << invar_.InitTermSpecAmt
            << "\tinitial term specified amount\n"
            << std::setprecision(0)
            << invar_.InitBaseSpecAmt + invar_.InitTermSpecAmt
            << "\tinitial total specified amount\n"
            ;
        }

    std::string const width = value_cast<std::string>(100 / columns_.size());
    oss << '\n';
    for(auto const& i : columns_)
        {
        ledger_metadata const& z = map_lookup(ledger_metadata_map(), i);
        oss << z.legend_ << '\t';
        }
    oss << '\n';

    for(int j = 0; j < max_length_; ++j)
        {
        for(auto const& i : columns_)
            {
            ledger_metadata const& z = map_lookup(ledger_metadata_map(), i);
            std::string s = ledger_format
                (numeric_vector(ledger_, i)[j]
                ,std::make_pair(z.decimals_, z.style_)
                );
            oss << s << '\t';
            }
        oss << '\n';
        }

    return oss.str();
}
} // Unnamed namespace.

/// Write calculation summary to an html string.

std::string FormatSelectedValuesAsHtml(Ledger const& ledger_values)
{
    return calculation_summary_formatter(ledger_values).format_as_html();
}

/// Write calculation summary to a tab-delimited string.

std::string FormatSelectedValuesAsTsv(Ledger const& ledger_values)
{
    throw_if_interdicted(ledger_values);

    return calculation_summary_formatter(ledger_values).format_as_tsv();
}

/// Write ledger to a tab-delimited file suitable for spreadsheets.
///
/// The file is appended to, rather than replaced, so that all cells
/// in a census can be written to the same file.

void PrintCellTabDelimited
    (Ledger const& ledger_values
    ,std::string const& file_name
    )
{
    throw_if_interdicted(ledger_values);

    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();
    LedgerVariant   const& Guar_ = ledger_values.GetGuarFull();

    int const max_length = ledger_values.greatest_lapse_dur();

    // TODO ?? This const_cast is safe, but it's still unclean.
    LedgerInvariant& unclean = const_cast<LedgerInvariant&>(Invar);
    unclean.CalculateIrrs(ledger_values);

    std::ofstream os(file_name.c_str(), ios_out_app_binary());

    os << "\n\nFOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.\n\n";

    os << "ContractNumber\t\t"    << Invar.value_str("ContractNumber" ) << '\n';
    os << "ProducerName\t\t"      << Invar.value_str("ProducerName"   ) << '\n';
    os << "ProducerStreet\t\t"    << Invar.value_str("ProducerStreet" ) << '\n';
    os << "ProducerCityEtc\t\t"   << Invar.value_str("ProducerCityEtc") << '\n';
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

    std::vector<std::string> const sheaders
        {"PolicyYear"
        ,"AttainedAge"
        ,"DeathBenefitOption"
        ,"EmployeeGrossPremium"
        ,"CorporationGrossPremium"
        ,"GrossWithdrawal"
        ,"NewCashLoan"
        ,"LoanBalance"
// SOMEDAY !! Add loan interest?
        ,"Outlay"
        ,"NetPremium"
        ,"PremiumTaxLoad"
        ,"DacTaxLoad"
// SOMEDAY !! Also:
//   M&E
//   stable value
//   DAC- and premium-tax charge
//   comp
//   IMF
//   custodial expense?
//   account value released?
        ,"PolicyFee"
        ,"SpecifiedAmountLoad"
        ,"AnnualFlatExtra"
        ,"MortalityCharge"
        ,"RiderCharges"
        ,"NetMortalityCharge"
        ,"SeparateAccountCharges"
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
        ,"CumulativeIrrOnSurrender"
        ,"CumulativeIrrOnDeath"
        ,"YearEndInforceLives"
        ,"ClaimsPaid"
        ,"NetClaims"
        ,"[obsolete]"
        ,"ProjectedMortalityCharge"
        ,"[obsolete]"
        ,"NetMortalityCharge0Int"
        ,"NetClaims0Int"
        ,"[obsolete]"
        ,"ProjectedMortalityCharge0Int"
        ,"[obsolete]"
        };

    for(auto const& i : sheaders)
        {
        os << i << '\t';
        }
    os << "\n\n";

    for(int j = 0; j < max_length; ++j)
        {
        os << (j + 1                                    ) << '\t';

        if(ledger_values.is_composite())
            {
            os                                            << '\t';
            os                                            << '\t';
            }
        else
            {
            os << (j + Invar.Age                        ) << '\t';
            os << Invar.DBOpt[j]                          << '\t';
            }

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
        os << Invar.value_str("AnnualFlatExtra"       ,j) << '\t';
        os << Curr_.value_str("COICharge"             ,j) << '\t';
        os << Curr_.value_str("RiderCharges"          ,j) << '\t';
        os << "0\t"; // obsolete
        os << Curr_.value_str("SepAcctCharges"        ,j) << '\t';

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
            os << Invar.IrrCsvCurrInput               [j] << '\t';
            os << Invar.IrrDbCurrInput                [j] << '\t';
            }

        // First element of InforceLives is BOY--show only EOY.
        os << value_cast<std::string>(Invar.InforceLives[1 + j]) << '\t';

        os << Curr_.value_str("ClaimsPaid"            ,j) << '\t';
        os << Curr_.value_str("NetClaims"             ,j) << '\t';
        os << "0\t"; // obsolete
        os << "0\t"; // obsolete
        os << "0\t"; // obsolete
        os << "0\t"; // obsolete
        os << "0\t"; // obsolete
        os << "0\t"; // obsolete
        os << "0\t"; // obsolete
        os << "0\t"; // obsolete

        os << '\n';
        }
    if(!os)
        {
        alarum() << "Unable to write '" << file_name << "'." << LMI_FLUSH;
        }
}

/// Write group-roster headers to a tab-delimited file suitable for spreadsheets.

void PrintRosterHeaders(std::string const& file_name)
{
    std::ofstream os(file_name.c_str(), ios_out_app_binary());

    os << "FOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.\n\n";

    // Skip authentication for non-interactive regression testing.
    // Surround the date in single quotes because one popular
    // spreadsheet would otherwise interpret it as a date, which
    // is likely not to fit in a default-width cell.
    if(!global_settings::instance().regression_testing())
        {
        authenticate_system();
        os << "DatePrepared\t\t'" << calendar_date().str() << "'\n\n";
        }
    else
        {
        // For regression tests, write an arbitrary constant as
        // date prepared, in order to avoid gratuitous failures.
        os << "DatePrepared\t\t'" << calendar_date(2000, 1, 1).str() << "'\n\n";
        }

    std::vector<std::string> const sheaders
        {"Insured1"
        ,"ContractNumber"
        ,"DateOfBirth"
        ,"IssueAge"
        ,"CurrentAge"
        ,"UWClass"
        ,"Smoker"
        ,"Salary"
        ,"SpecifiedAmount"
        ,"SupplSpecAmt"
        ,"InitialTargetPremium"
        ,"ModalMinimumPremium"
        ,"EeModalMinimumPremium"
        ,"ErModalMinimumPremium"
        ,"ListBillPremium"
        ,"EeListBillPremium"
        ,"ErListBillPremium"
        ,"ListBillDate"
        ,"EeMode"
        ,"ErMode"
        ,"CorpName"
        ,"EffDate"
        ,"LastCoiReentryDate"
        ,"InforceAsOfDate"
        ,"PremiumTaxState"
        ,"StateOfJurisdiction"
        ,"CurrentGeneralAccountInterestRate"
        ,"PolicyFee"
        ,"DacTaxLoad"
        ,"MaxPremiumTaxLoad"
        ,"MaxTargetPremiumLoad"
        ,"ProductName"
        ,"PolicyForm"
        ,"CurrentCoiMultiplier"
        ,"Waiver"
        ,"AccidentalDeath"
        ,"TermRider"
        ,"ChildRider"
        ,"SpouseRider"
        ,"SpouseRiderAmount"
        };

    for(auto const& i : sheaders)
        {
        os << i << '\t';
        }
    os << "\n\n";

    if(!os)
        {
        alarum() << "Unable to write '" << file_name << "'." << LMI_FLUSH;
        }
}

/// Write group roster to a tab-delimited file suitable for spreadsheets.
///
/// The file is appended to, rather than replaced, so that all cells
/// in a census can be written to the same file.
///
/// The composite is deliberately skipped. For an inforce census with
/// varying issue years, no year in the composite would match the sum
/// of inforce-year cell values, because the composite is summed by
/// policy year.

void PrintRosterTabDelimited
    (Ledger const& ledger_values
    ,std::string const& file_name
    )
{
    if(ledger_values.is_composite())
        {
        return;
        }

    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();

    std::ofstream os(file_name.c_str(), ios_out_app_binary());

    int d = static_cast<int>(Invar.InforceYear);
    LMI_ASSERT(d < Invar.GetLength());
    LMI_ASSERT(d < Curr_.GetLength());

    os
        << Invar.value_str("Insured1"               ) << '\t'
        << Invar.value_str("ContractNumber"         ) << '\t'
        << "'" << Invar.DateOfBirth                   << "'\t"
        << Invar.value_str("Age"                    ) << '\t'
        << Invar.Age + Invar.InforceYear              << '\t'
        << Invar.value_str("UWClass"                ) << '\t'
        << Invar.value_str("Smoker"                 ) << '\t'
        << Invar.value_str("Salary"               ,d) << '\t'
        << Invar.value_str("SpecAmt"              ,d) << '\t'
        << Invar.value_str("TermSpecAmt"          ,d) << '\t'
        << Invar.value_str("InitTgtPrem"            ) << '\t'
        << Invar.value_str("ModalMinimumPremium"  ,d) << '\t'
        << Invar.value_str("EeModalMinimumPremium",d) << '\t'
        << Invar.value_str("ErModalMinimumPremium",d) << '\t'
        << Invar.value_str("ListBillPremium"        ) << '\t'
        << Invar.value_str("EeListBillPremium"      ) << '\t'
        << Invar.value_str("ErListBillPremium"      ) << '\t'
        << "'" << Invar.ListBillDate                  << "'\t"
        << Invar.EeMode                           [d] << '\t'
        << Invar.ErMode                           [d] << '\t'
        << Invar.value_str("CorpName"               ) << '\t'
        << "'" << Invar.EffDate                       << "'\t"
        << "'" << Invar.LastCoiReentryDate            << "'\t"
        << "'" << Invar.InforceAsOfDate               << "'\t"
        << Invar.value_str("PremiumTaxState"        ) << '\t'
        << Invar.value_str("StateOfJurisdiction"    ) << '\t'
        << Curr_.value_str("AnnGAIntRate"         ,d) << '\t'
        << Curr_.value_str("InitMlyPolFee"          ) << '\t'
        << Invar.value_str("InitDacTaxRate"         ) << '\t'
        << Invar.value_str("InitPremTaxRate"        ) << '\t'
        << Curr_.value_str("InitTgtPremHiLoadRate"  ) << '\t'
        << Invar.value_str("ProductName"            ) << '\t'
        << Invar.value_str("PolicyForm"             ) << '\t'
        << Invar.value_str("CurrentCoiMultiplier"   ) << '\t'
        << Invar.value_str("HasWP"                  ) << '\t'
        << Invar.value_str("HasADD"                 ) << '\t'
        << Invar.value_str("HasTerm"                ) << '\t'
        << Invar.value_str("HasChildRider"          ) << '\t'
        << Invar.value_str("HasSpouseRider"         ) << '\t'
        << Invar.value_str("SpouseRiderAmount"      ) << '\t'
        << '\n'
        ;

    if(!os)
        {
        alarum() << "Unable to write '" << file_name << "'." << LMI_FLUSH;
        }
}

class FlatTextLedgerPrinter final
{
  public:
    FlatTextLedgerPrinter(Ledger const&, std::ostream&);
    ~FlatTextLedgerPrinter() = default;

    void Print() const;

  private:
    FlatTextLedgerPrinter(FlatTextLedgerPrinter const&) = delete;
    FlatTextLedgerPrinter& operator=(FlatTextLedgerPrinter const&) = delete;

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

    // Required ctor arguments.
    Ledger const& ledger_;
    std::ostream& os_;

    mutable int page_number_ {0};
};

void PrintLedgerFlatText
    (Ledger const& ledger
    ,std::ostream& os
    )
{
    throw_if_interdicted(ledger);

    FlatTextLedgerPrinter(ledger, os).Print();
}

// One column of seven characters ('Age 100')
// plus ten columns of twelve characters each (' 999,999,999')
// equals 127 columns; a nine-point font can print that on a page
// eight inches wide and still leave about a half-inch margin
// on both sides.
//
// SOMEDAY !! The formatting of this simple reference implementation
// could be improved in several ways:
//   - Avoid overflow by scaling everything if anything's $1B or over.
//   - Add thousands separators.
//   - Show at least premiums as dollars and cents.
//   - Split into numbered pages, adding serial page numbers to footer.
//   - Wrap long strings.
namespace
{
    int const g_width = 128;
    std::string center(std::string const& s)
        {
        int const z = lmi::ssize(s);
        // Strings in the input class might be too wide; absent more
        // graceful handling, at least no attempt is made to cure that
        // problem with a negative number of spaces.
        std::string spaces(std::max(0, (g_width - z) / 2), ' ');
        return spaces + s;
        }

    std::ostream& endrow(std::ostream& os)
        {
        os << '\n';
        return os;
        }
} // Unnamed namespace.

FlatTextLedgerPrinter::FlatTextLedgerPrinter
    (Ledger const& ledger
    ,std::ostream& os
    )
    :ledger_ {ledger}
    ,os_     {os}
{
}

void FlatTextLedgerPrinter::Print() const
{
    set_default_format_flags(os_);

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

    // 'os_' may be a std::ofstream, and files should end in a newline.
    os_ << '\n';

    LMI_ASSERT(os_.good());
}

void FlatTextLedgerPrinter::PrintHeader() const
{
    os_ << center("Life Insurance Basic Illustration") << endrow;
    os_ << endrow;
    os_ << center("Prepared on " + iso_8601_datestamp_terse() + " by") << endrow;
    os_ << center(invar().ProducerName) << endrow;
    os_ << center(invar().ProducerStreet) << endrow;
    os_ << center(invar().ProducerCityEtc) << endrow;
    if(ledger_.is_composite())
        {
        os_ << "Composite" << endrow;
        }
    else
        {
        os_
            << "Insured:"
            << (invar().Insured1.size() ? " " : "")
            << invar().Insured1
            << endrow
            ;
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
    os_ << "Page " << ++page_number_ << " of 3";
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
    os_ << "to qualify as a life insurance policy under the applicable provisions of the Internal Revenue Code, is " << value_cast<std::string>(invar().GuarPrem) << " annually." << endrow;
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
    int const age = value_cast<int>(invar().Age);
    int const max_length = ledger_.greatest_lapse_dur();

    os_ << center("Numerical summary") << endrow;
    os_ << endrow;
    os_ << "                    ------------Guaranteed------------- -------------Midpoint-------------- ----------Non-guaranteed-----------" << endrow;
    os_ << "            Premium     Account   Surrender       Death     Account   Surrender       Death     Account   Surrender       Death" << endrow;
    os_ << "   Year      Outlay       Value       Value     Benefit       Value       Value     Benefit       Value       Value     Benefit" << endrow;
    os_ << endrow;

    // For multi-life contracts (which lmi does not currently support),
    // substitute duration thirty for age seventy: see illustration reg
    // section (7)(C)(1).

    std::vector<int> summary_rows = {4, 9, 19, 69 - age};

    for(auto const& row : summary_rows)
        {
        // Skip row if it doesn't exist. For instance, if the issue
        // age is 85 and the contract remains in force until age 100,
        // then there is no twentieth duration and no age-70 row.
        if(!(0 <= row && row < max_length))
            {
            continue;
            }

        // For composites, don't print the age-70 row (because age
        // is undefined) or lapse durations (which generally vary).
        if(ledger_.is_composite() && &row == &summary_rows.back())
            {
            os_ << endrow;
            return;
            }

        os_.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os_.precision(0);
        os_.width(7);

        if(&row == &summary_rows.back())
            {
            os_ << " Age 70";
            }
        else
            {
            os_ << std::setw( 7) << (1 + row)               ;
            }

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

    os_ << endrow;

    // Illustration reg (7)(C)(2) "year in which coverage ceases".
    auto const brink = [age, max_length] (LedgerVariant const& basis)
        {
        int const z = bourn_cast<int>(basis.LapseYear);
        std::string s =
              (z < max_length)
            ? "Lapses in year " + value_cast<std::string>(1 + z)
            : "Matures at age " + value_cast<std::string>(    z + age)
            ;
        s = std::string(9, ' ') + s;
        s.resize(12 + 12 + 12, ' '); // Spans three columns.
        return s;
        };

    std::string s = brink(guar_()) + brink(mdpt_()) + brink(curr_());
    rtrim(s, " ");
    os_ << "                    " << s << endrow;

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
    int const age = value_cast<int>(invar().Age);
    int const max_length = ledger_.greatest_lapse_dur();
    for(int j = 0; j < max_length; ++j)
        {
        os_.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os_.precision(0);

        os_ << std::setw( 7) << (1 + j      )         ;

        if(ledger_.is_composite())
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

// These are the formats used in older files:
//
// F1: zero decimals, commas
// F2: two decimals, commas
// F3: scaled by 100, zero decimals, with '%' at end:
// F4: scaled by 100, two decimals, with '%' at end:
//
// As discussed on the mailing list, e.g.:
//   https://lists.nongnu.org/archive/html/lmi/2006-10/msg00066.html
// the appropriate abstraction is
//   {number of decimals [0, DECIMAL_DIG], style}
// where for now percentage is the only nondefault 'style', but others
// (e.g., basis points) can be envisioned.

/// Apply an appropriate format to a value.
///
/// The first element of the std::pair argument is the number of
/// digits to be shown to the right of the decimal point.

std::string ledger_format
    (double                            d
    ,std::pair<int,oenum_format_style> f
    )
{
    static std::stringstream interpreter = []
        {
        std::stringstream ss {};
        std::locale loc;
        std::locale new_loc(loc, new comma_punct);
        ss.imbue(new_loc);
        ss.setf(std::ios_base::fixed, std::ios_base::floatfield);
        return ss;
        } ();
    interpreter.str(std::string{});
    interpreter.clear();

    interpreter.precision(f.first);
    std::string s;
    if(f.second)
        {
        d *= 100;
        }
    interpreter << d;
    interpreter >> s;
    if(!interpreter.eof())
        {
        alarum() << "Formatting error." << LMI_FLUSH;
        }

    if(f.second)
        {
        s += '%';
        }

    return s;
}

std::vector<std::string> ledger_format
    (std::vector<double> const&        dv
    ,std::pair<int,oenum_format_style> f
    )
{
    std::vector<std::string> sv;
    sv.reserve(dv.size());
    for(auto const& i : dv)
        {
        sv.push_back(ledger_format(i, f));
        }
    return sv;
}
