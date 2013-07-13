// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#include "ledger_text_formats.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"    // parsed_calculation_summary_columns()
#include "contains.hpp"
#include "global_settings.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "map_lookup.hpp"
#include "mc_enum_types_aux.hpp"        // is_subject_to_ill_reg()
#include "miscellany.hpp"
#include "obstruct_slicing.hpp"
#include "uncopyable_lmi.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iterator>
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
        m["CorridorFactor"             ] = ledger_metadata(0, oe_format_percentage, "Corridor Factor"                       );
        m["AnnGAIntRate_Current"       ] = ledger_metadata(2, oe_format_percentage, "Curr Ann Gen Acct Int Rate"            ); // "General Account Crediting Rate"
        m["AnnSAIntRate_Current"       ] = ledger_metadata(2, oe_format_percentage, "Curr Ann Sep Acct Int Rate"            ); // "Separate Account Net Rate"
        m["Outlay"                     ] = ledger_metadata(0, oe_format_normal    , "Net Outlay"                            );
        m["EeGrossPmt"                 ] = ledger_metadata(0, oe_format_normal    , "EE Gross Payment"                      ); // "Employee Gross Payment"
        m["ErGrossPmt"                 ] = ledger_metadata(0, oe_format_normal    , "ER Gross Payment"                      ); // "Employer Gross Payment"
        m["ModalMinimumPremium"        ] = ledger_metadata(0, oe_format_normal    , "Modal Minimum Premium"                 );
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
        m["COICharge_Current"          ] = ledger_metadata(0, oe_format_normal    , "Curr COI Charge"                       ); // "Current Mortality Charge"
        m["COICharge_Guaranteed"       ] = ledger_metadata(0, oe_format_normal    , "Guar COI Charge"                       ); // "Guaranteed Mortality Charge"
        m["RiderCharges_Current"       ] = ledger_metadata(0, oe_format_normal    , "Curr Rider Charges"                    ); // "Current Rider Charges"
        m["IrrCsv_Current"             ] = ledger_metadata(2, oe_format_percentage, "Curr IRR on CSV"                       ); // "Current Cash Value IRR"
        m["IrrCsv_Guaranteed"          ] = ledger_metadata(2, oe_format_percentage, "Guar IRR on CSV"                       ); // "Guaranteed Cash Value IRR"
        m["IrrDb_Current"              ] = ledger_metadata(2, oe_format_percentage, "Curr IRR on DB"                        ); // "Current Death Benefit IRR"
        m["IrrDb_Guaranteed"           ] = ledger_metadata(2, oe_format_percentage, "Guar IRR on DB"                        ); // "Guaranteed Death Benefit IRR"
        m["ExperienceReserve_Current"  ] = ledger_metadata(0, oe_format_normal    , "Experience Rating Reserve"             ); // "Net Mortality Reserve"
        m["NetClaims_Current"          ] = ledger_metadata(0, oe_format_normal    , "Curr Net Claims"                       ); // "Experience Rating Current Net Claims"
        m["NetCOICharge_Current"       ] = ledger_metadata(0, oe_format_normal    , "Experience Rating Net COI Charge"      ); // "Net Mortality Charge"
        m["ProjectedCoiCharge_Current" ] = ledger_metadata(0, oe_format_normal    , "Experience Rating Projected COI Charge"); // "Projected Mortality Charge"
        m["KFactor_Current"            ] = ledger_metadata(4, oe_format_normal    , "Experience Rating K Factor"            );
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
// '*_CurrentZero' and '*_GuaranteedZero' columns deliberately suppressed--see:
//   http://lists.nongnu.org/archive/html/lmi/2009-09/msg00012.html
// TODO ?? EGREGIOUS_DEFECT: instead, don't offer these columns at all.
//      m["AVGenAcct_CurrentZero"      ] = ledger_metadata(0, oe_format_normal    , "Curr Charges Account Value Gen Acct"   ); // "Curr Charges Account Value General Account"
//      m["AVGenAcct_GuaranteedZero"   ] = ledger_metadata(0, oe_format_normal    , "Guar Charges Account Value Gen Acct"   ); // "Guar Charges Account Value General Account"
//      m["AVSepAcct_CurrentZero"      ] = ledger_metadata(0, oe_format_normal    , "Curr Charges 0% Account Value Sep Acct"); // "Curr Charges 0% Account Value Separate Account"
//      m["AVSepAcct_GuaranteedZero"   ] = ledger_metadata(0, oe_format_normal    , "Guar Charges 0% Account Value Sep Acct"); // "Guar Charges 0% Account Value Separate Account"
//      m["AcctVal_CurrentZero"        ] = ledger_metadata(0, oe_format_normal    , "Curr Charges 0% Account Value"         ); // "Curr Charges 0% Account Value"
//      m["AcctVal_GuaranteedZero"     ] = ledger_metadata(0, oe_format_normal    , "Guar Charges 0% Account Value"         ); // "Guar Charges 0% Account Value"
//      m["CSVNet_CurrentZero"         ] = ledger_metadata(0, oe_format_normal    , "Curr Charges 0% Net Cash Surr Value"   ); // "Curr Charges 0% Net Cash Surrender Value"
//      m["CSVNet_GuaranteedZero"      ] = ledger_metadata(0, oe_format_normal    , "Guar Charges 0% Net Cash Surr Value"   ); // "Guar Charges 0% Net Cash Surrender Value"
        }

    return m;
}

class calculation_summary_formatter
    :        private lmi::uncopyable <calculation_summary_formatter>
    ,virtual private obstruct_slicing<calculation_summary_formatter>
{
  public:
    calculation_summary_formatter(Ledger const&);
    ~calculation_summary_formatter();

    std::string format_as_html() const;
    std::string format_as_tsv () const;

    std::string top_note(std::string const& line_break) const;

  private:
    Ledger          const&   ledger_;
    LedgerInvariant const&   invar_;
    int             const    max_length_;
    std::vector<std::string> columns_;
};

calculation_summary_formatter::calculation_summary_formatter
    (Ledger const& ledger_values
    )
    :ledger_    (ledger_values)
    ,invar_     (ledger_values.GetLedgerInvariant())
    ,max_length_(ledger_values.GetMaxLength())
{
    columns_ = parsed_calculation_summary_columns();
    std::vector<std::string>::iterator p = std::find
        (columns_.begin()
        ,columns_.end()
        ,"PolicyYear"
        );
    // TODO ?? This should be done in parsed_calculation_summary_columns(),
    // but that requires a difficult-to-test change in 'ledger_xml_io2.cpp'.
    // As long as "PolicyYear" is always the first column, it shouldn't be
    // offered for selection anyway.
    if(columns_.end() != p)
        {
        columns_.erase(p);
        }
    columns_.insert(columns_.begin(), "PolicyYear");

    unsigned int const length = invar_.GetLength();
    if(length != invar_.IrrCsvCurrInput.size())
        {
        // TODO ?? This const_cast is safe, but it's still unclean.
        LedgerInvariant& unclean = const_cast<LedgerInvariant&>(invar_);
        bool want_any_irr =
               contains(columns_, "IrrCsv_Current"   )
            || contains(columns_, "IrrCsv_Guaranteed")
            || contains(columns_, "IrrDb_Current"    )
            || contains(columns_, "IrrDb_Guaranteed" )
            ;
        if(want_any_irr && !invar_.IsInforce)
            {
            unclean.CalculateIrrs(ledger_);
            }
        else
            {
            unclean.IrrCsvCurrInput.resize(length);
            unclean.IrrCsvGuarInput.resize(length);
            unclean.IrrDbCurrInput .resize(length);
            unclean.IrrDbGuarInput .resize(length);
            }
        }
}

calculation_summary_formatter::~calculation_summary_formatter()
{
}

std::string calculation_summary_formatter::top_note
    (std::string const& line_break
    ) const
{
    std::ostringstream oss;
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

    if(ledger_.GetIsComposite())
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
            << ", " << invar_.GetStatePostalAbbrev() << " jurisdiction"
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

    if(!ledger_.GetIsComposite())
        {
        oss << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n";
        if(is_subject_to_ill_reg(ledger_.GetLedgerType()))
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
    typedef std::vector<std::string>::const_iterator vsci;
    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"
        << "<tr align=\"right\">\n"
        ;
    for(vsci i = columns_.begin(); i != columns_.end(); ++i)
        {
        ledger_metadata const& z = map_lookup(ledger_metadata_map(), *i);
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
        for(vsci i = columns_.begin(); i != columns_.end(); ++i)
            {
            ledger_metadata const& z = map_lookup(ledger_metadata_map(), *i);
            std::string s = ledger_format
                (numeric_vector(ledger_, *i)[j]
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

    if(!ledger_.GetIsComposite())
        {
        if(is_subject_to_ill_reg(ledger_.GetLedgerType()))
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
    typedef std::vector<std::string>::const_iterator vsci;
    oss << '\n';
    for(vsci i = columns_.begin(); i != columns_.end(); ++i)
        {
        ledger_metadata const& z = map_lookup(ledger_metadata_map(), *i);
        oss << z.legend_ << '\t';
        }
    oss << '\n';

    for(int j = 0; j < max_length_; ++j)
        {
        for(vsci i = columns_.begin(); i != columns_.end(); ++i)
            {
            ledger_metadata const& z = map_lookup(ledger_metadata_map(), *i);
            std::string s = ledger_format
                (numeric_vector(ledger_, *i)[j]
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
    LedgerInvariant const& Invar = ledger_values.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values.GetCurrFull();
    LedgerVariant   const& Guar_ = ledger_values.GetGuarFull();

    int max_length = ledger_values.GetMaxLength();

    // TODO ?? This const_cast is safe, but it's still unclean.
    LedgerInvariant& unclean = const_cast<LedgerInvariant&>(Invar);
    if(!Invar.IsInforce)
        {
        unclean.CalculateIrrs(ledger_values);
        }
    else
        {
        unclean.IrrCsvCurrInput.resize(max_length);
        unclean.IrrDbCurrInput .resize(max_length);
        }

    std::ofstream os(file_name.c_str(), ios_out_app_binary());

    os << "\n\nFOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.\n\n";

    os << "ContractNumber\t\t"    << Invar.value_str("ContractNumber" ) << '\n';
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

    std::vector<std::string> const sheaders
        (cheaders
        ,cheaders + lmi_array_size(cheaders)
        );
    typedef std::vector<std::string>::const_iterator vsi;
    for(vsi i = sheaders.begin(); i != sheaders.end(); ++i)
        {
        os << *i << '\t';
        }
    os << "\n\n";

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
        os << Curr_.value_str("RiderCharges"          ,j) << '\t';
        os << Curr_.value_str("NetCOICharge"          ,j) << '\t';
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
        os << Curr_.value_str("ExperienceReserve"     ,j) << '\t';
        os << Curr_.value_str("ProjectedCoiCharge"    ,j) << '\t';
        os << Curr_.value_str("KFactor"               ,j) << '\t';

        // Show experience-rating columns for current-expense, zero-
        // interest basis if used, to support testing.
        std::vector<mcenum_run_basis> const& bases(ledger_values.GetRunBases());
        if(contains(bases, mce_run_gen_curr_sep_zero))
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

        if(contains(bases, mce_run_gen_curr_sep_half))
            {
            fatal_error()
                << "Three-rate illustrations not supported."
                << LMI_FLUSH
                ;
            }

        os << Invar.value_str("ProducerCompensation"  ,j) << '\t';

        os << '\n';
        }
    LMI_ASSERT(os.good());
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

    char const* cheaders[] =
        {"Insured1"
        ,"ContractNumber"
        ,"DateOfBirth"
        ,"IssueAge"
        ,"CurrentAge"
        ,"UWClass"
        ,"Smoker"
        ,"SpecifiedAmount"
        ,"ModalMinimumPremium"
        ,"ErMode"
        ,"CorpName"
        ,"EffDate"
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
        };

    std::vector<std::string> const sheaders
        (cheaders
        ,cheaders + lmi_array_size(cheaders)
        );
    typedef std::vector<std::string>::const_iterator vsi;
    for(vsi i = sheaders.begin(); i != sheaders.end(); ++i)
        {
        os << *i << '\t';
        }
    os << "\n\n";

    LMI_ASSERT(os.good());
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
    if(ledger_values.GetIsComposite())
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
        << Invar.value_str("SpecAmt"              ,d) << '\t'
        << Invar.value_str("ModalMinimumPremium"  ,d) << '\t'
        << Invar.ErMode                           [d] << '\t'
        << Invar.value_str("CorpName"               ) << '\t'
        << "'" << Invar.EffDate                       << "'\t"
        << Invar.value_str("PremiumTaxState"        ) << '\t'
        << Invar.value_str("StatePostalAbbrev"      ) << '\t'
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
        << '\n'
        ;

    LMI_ASSERT(os.good());
}

class FlatTextLedgerPrinter
    :        private lmi::uncopyable <FlatTextLedgerPrinter>
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
} // Unnamed namespace.

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

// These are the formats used in older files:
//
// F1: zero decimals, commas
// F2: two decimals, commas
// F3: scaled by 100, zero decimals, with '%' at end:
// F4: scaled by 100, two decimals, with '%' at end:
//
// As discussed on the mailing list, e.g.:
//   http://lists.nongnu.org/archive/html/lmi/2006-10/msg00066.html
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
    std::stringstream interpreter;
    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    interpreter.imbue(new_loc);
    interpreter.setf(std::ios_base::fixed, std::ios_base::floatfield);
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
        fatal_error() << "Formatting error." << LMI_FLUSH;
        }

    if(f.second)
        {
        s += '%';
        }

#if defined __GNUC__ && LMI_GCC_VERSION <= 40001
    // COMPILER !! Work around a gcc defect fixed in gcc-4.0.1: see
    //   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=20914
    static std::string const old_string("-,");
    static std::string const new_string("-");
    std::string::size_type position = s.find(old_string);
    while(position != std::string::npos)
        {
        s.replace(position, old_string.length(), new_string);
        position = s.find(old_string, 1 + position);
        }
#endif // gcc version less than 4.0.1 .

    return s;
}

std::vector<std::string> ledger_format
    (std::vector<double>               dv
    ,std::pair<int,oenum_format_style> f
    )
{
    std::vector<std::string> sv;
    for(unsigned int j = 0; j < dv.size(); ++j)
        {
        sv.push_back(ledger_format(dv[j], f));
        }
    return sv;
}

