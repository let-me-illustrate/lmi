// Ledger xml input and output.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger_xml_io.cpp,v 1.20 2005-06-21 05:27:48 chicares Exp $

#include "config.hpp"

#include "alert.hpp" // TODO ?? expunge after testing

#include "build.hpp"
#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"
#include "global_settings.hpp"
#include "ledger.hpp"
#include "ledger_base.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "security.hpp"
#include "value_cast.hpp"
#include "xmlwrapp_ex.hpp"

#ifdef USING_CURRENT_XMLWRAPP
#   include <xmlwrapp/attributes.h>
// TODO ?? Gross hack to be undone when USING_CURRENT_XMLWRAPP becomes
// the only supported version.
#   define set_attr get_attributes().insert
#endif

#include <xmlwrapp/init.h>
#include <xmlwrapp/node.h>
#include <xmlwrapp/tree_parser.h>

#include <fstream>
#include <iomanip>
#include <ios>
#include <locale>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

void Ledger::read(xml::node&)
{
    // TODO ?? Not yet implemented.
}

namespace
{
int const n = 7;

char const* char_p_suffixes[n] =
    {"_Current"        // e_run_curr_basis
    ,"_Guaranteed"     // e_run_guar_basis
    ,"_Midpoint"       // e_run_mdpt_basis
    ,"_CurrentZero"    // e_run_curr_basis_sa_zero
    ,"_GuaranteedZero" // e_run_guar_basis_sa_zero
    ,"_CurrentHalf"    // e_run_curr_basis_sa_half
    ,"_GuaranteedHalf" // e_run_guar_basis_sa_half
    };

std::vector<std::string> const suffixes
    (char_p_suffixes
    ,char_p_suffixes + n
    );

template<typename EnumType, int N>
std::vector<std::string> enum_vector_to_string_vector
    (std::vector<xenum<EnumType, N> > const& ve
    )
{
    std::vector<std::string> vs;
    typename std::vector<xenum<EnumType, N> >::const_iterator ve_i;
    for(ve_i = ve.begin(); ve_i != ve.end(); ++ve_i)
        {
        vs.push_back(ve_i->str());
        }
    return vs;
}

// The std::pair argument is notionally <int precision, bool percentage>.
std::string format(double d, std::pair<int,bool> f)
{
    std::stringstream interpreter;
    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    interpreter.imbue(new_loc);
    interpreter.setf(std::ios_base::fixed, std::ios_base::floatfield);
    interpreter.precision(f.first);
    std::string s;
    if(f.second) d *= 100;
    interpreter << d;
    interpreter >> s;
    if(f.second) s += '%';
    if(!interpreter.eof()) throw std::runtime_error("Format error");
    return s;
}

std::vector<std::string> format(std::vector<double> dv, std::pair<int,bool> f)
{
    std::vector<std::string> sv;
    for(unsigned int j = 0; j < dv.size(); ++j)
        {
        sv.push_back(format(dv[j], f));
        }
/* Broken:
    std::vector<std::string> sv;
    std::stringstream interpreter;
    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    interpreter.imbue(new_loc);
    interpreter.setf(std::ios_base::fixed, std::ios_base::floatfield);
    interpreter.precision(f.first);
    for(unsigned int j = 0; j < dv.size(); ++j)
        {
        std::string s;
        if(f.second) dv[j] *= 100;
        interpreter << dv[j];
        interpreter >> s;
        if(f.second) s += '%';
        if(!interpreter.eof()) throw std::runtime_error("Format error");
        interpreter.str("");
        sv.push_back(s);
        }
*/
    return sv;
}

typedef std::map<std::string, std::pair<int, bool> > format_map_t;
typedef std::map<std::string, std::string> title_map_t;

// Look at file 'missing_formats'. It's important. You want
// it to be empty; once it is, you can suppress the code that creates
// and writes to it.
//
// Here's what it means. For all numbers (so-called 'scalars' and
// 'vectors', but not 'strings') grabbed from all ledgers, we look
// for a format. If we find one, we use it to turn the number into
// a string. If not, we ignore it.
//
// Some things you probably want are so ignored.
//
// Why did I think this reasonble? Because no other reasonable
// behavior occurs to me, for one thing: silently falling back on
// some 'default' format can't be right, because it masks defects
// that we want to fix. For another thing, this gives you a handy
// way to do the 'subsetting' we'd spoken of. If you want a (numeric)
// field, then give it a format; if you don't, then don't.
//
// Speaking of masked defects--now I'm really glad I did this.
// Look at that 'missing_formats' file. Not only does it list
// everything you consciously decided you didn't want, like
//   EffDateJdn
// it also shows stuff that I think we never had, but need, like
//   AllowDbo3
// which I think is used for some purpose that was important to
// Compliance.
//
// I've designed this to be maintained, except for the ugly parts
// we aren't talking about here, so I think you'll be able to fix
// this stuff easily. Where your specs said, e.g.
//
// > Format as a number with thousand separators and no decimal places (#,###,##0)
// >
// > AcctVal_*
// > AcctValLoadAMD_*
// > AcctValLoadBOM_*
// > AccumulatedPremium
//
// I translated that into
//
//    format_map["AcctVal"                           ] = f1;
//    format_map["AcctValLoadAMD"                    ] = f1;
//    format_map["AcctValLoadBOM"                    ] = f1;
//    format_map["AccumulatedPremium"                ] = f1;
//
// where 'f1' is one of several formats I abstracted from your specs
// at the top level. For names formed as
//   basename + '_' + suffix
// the map needs only the basename, which makes things a lot
// terser, simpler, and likelier to be right.
//
// This translation is just text transformations on your specs.
// I imported your specs and then did regex search-and-replace
// to write this code.
//
// To make a missing (numeric) variable appear in the xml,
// just add a line like those.
//
// Searching for the first occurrence of, say, 'f1' will take you
// to the section where I analyze your formats. It's marked with
// your name in caps so that you can find it easily.
//
// BTW, part of your specs included suffixes like "_Current", but
// for the most part you omitted them. I didn't rectify that, so
// would you please do the honors? For instance:
//    format_map["AnnHoneymoonValueRate_Current"     ] = f4;
//    format_map["AnnPostHoneymoonRate"              ] = f4;
// Right now, the second gets formatted, but the first doesn't.
// Lookups in the format map are strict, and they have to be,
// else one key like "A" would match anything beginning with
// that letter.

bool format_exists(std::string const& s, std::string const& suffix, format_map_t const& m)
{
    if(m.end() != m.find(s))
        {
        return true;
        }
    else
        {
(void)suffix;
#ifdef SHOW_MISSING_FORMATS
        std::ofstream ofs("missing_formats", std::ios_base::out | std::ios_base::ate | std::ios_base::app);
        ofs << s << suffix << "\n";
#endif // defined SHOW_MISSING_FORMATS
        return false;
        }
}

} // Unnamed namespace.

void Ledger::write(xml::node& x) const
{
    title_map_t title_map;

// Can't seem to get a literal &nbsp; into the output.

// Original:   title_map["AttainedAge"                     ] = " &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0; End of &#xA0;&#xA0;Year Age";
// No good:    title_map["AttainedAge"                     ] = " &&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0; End of &&#xA0;&&#xA0;Year Age";
// No good:    title_map["AttainedAge"                     ] = " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; End of &nbsp;&nbsp;Year Age";
// No good:    title_map["AttainedAge"                     ] = " &amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp; End of &amp;nbsp;&amp;nbsp;Year Age";
// No good:    title_map["AttainedAge"                     ] = "<![CDATA[ &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0; End of &#xA0;&#xA0;Year Age]]>";
// No good:    title_map["AttainedAge"                     ] = " בבבבבבבבבבבבב End of בבYear Age";
// No good:    title_map["AttainedAge"                     ] = " &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; End of &#160;&#160;Year Age";

//  Here are the columns to be listed in the user interface
//  as well as their corresponding titles.
    title_map["AVRelOnDeath_Current"            ] = "Acct Val Rel on Death";
    title_map["AcctValLoadAMD_Current"          ] = "Curr Acct Val Load Aft MDed";
    title_map["AcctValLoadAMD_Guaranteed"       ] = "Guar Acct Val Load Aft MDed";
    title_map["AcctValLoadBOM_Current"          ] = "Curr Acct Value Load BOM";
    title_map["AcctValLoadBOM_Guaranteed"       ] = "Guar Acct Value Load BOM";
    title_map["AcctVal_Current"                 ] = "Curr Account Value";
    title_map["AcctVal_Guaranteed"              ] = "Guar Account Value";
    title_map["AddonCompOnAssets"               ] = "Additional Comp on Assets";
    title_map["AddonCompOnPremium"              ] = "Additional Comp on Premium";
    title_map["AddonMonthlyFee"                 ] = "Additional Monthly Fee";
    title_map["AnnGAIntRate_Current"            ] = "Curr Ann Gen Acct Int Rate";
    title_map["AnnGAIntRate_Guaranteed"         ] = "Guar Ann Gen Acct Int Rate";
    title_map["AnnHoneymoonValueRate_Current"   ] = "Curr Ann Honeymoon Val Rate";
    title_map["AnnHoneymoonValueRate_Guaranteed"] = "Guar Ann Honeymoon Val Rate";
    title_map["AnnPostHoneymoonRate_Current"    ] = "Curr Post Honeymoon Rate";
    title_map["AnnPostHoneymoonRate_Guaranteed" ] = "Guar Post Honeymoon Rate";
    title_map["AnnSAIntRate_Current"            ] = "Curr Ann Sep Acct Int Rate";
    title_map["AnnSAIntRate_Guaranteed"         ] = "Guar Ann Sep Acct Int Rate";
    title_map["AttainedAge"                     ] = " _____________ End of __Year Age";
    title_map["AvgDeathBft_Current"             ] = "Curr Avg Death Benefit";
    title_map["AvgDeathBft_Guaranteed"          ] = "Guar Avg Death Benefit";
    title_map["BOYPrefLoan_Current"             ] = "Curr BOY Pref Loan Balance";
    title_map["BOYPrefLoan_Guaranteed"          ] = "Guar BOY Pref Loan Balance";
    title_map["BaseDeathBft_Current"            ] = "Curr Base Death Benefit";
    title_map["BaseDeathBft_Guaranteed"         ] = "Guar Base Death Benefit";
    title_map["COICharge_Current"               ] = " _____________ Curr COI Charge";
    title_map["COICharge_Guaranteed"            ] = " _____________ Guar COI Charge";
    title_map["CSVNet_Current"                  ] = "Curr Net Cash Surr Value";
    title_map["CSVNet_Guaranteed"               ] = "Guar Net Cash Surr Value";
    title_map["CV7702_Current"                  ] = "Curr 7702 Cash Value";
    title_map["CV7702_Guaranteed"               ] = "Guar 7702 Cash Value";
    title_map["ClaimsPaid_Current"              ] = " _____________ Curr Claims Paid";
    title_map["ClaimsPaid_Guaranteed"           ] = " _____________ Guar Claims Paid";
    title_map["CorpTaxBracket"                  ] = " _____________ Corp Tax Bracket";
    title_map["CorridorFactor"                  ] = " _____________ Corridor Factor";
    title_map["CurrMandE"                       ] = "Mortality and Expense";
    title_map["DBOpt"                           ] = "Death Benefit Option";
    title_map["DacTaxLoad_Current"              ] = " _____________ Curr DAC Tax Load";
    title_map["DacTaxLoad_Guaranteed"           ] = " _____________ Guar DAC Tax Load";
    title_map["DacTaxRsv_Current"               ] = "Curr DAC Tax Reserve";
    title_map["DacTaxRsv_Guaranteed"            ] = "Guar DAC Tax Reserve";
    title_map["EOYDeathBft_Current"             ] = "Curr EOY Death Benefit";
    title_map["EOYDeathBft_Guaranteed"          ] = "Guar EOY Death Benefit";
    title_map["EeGrossPmt"                      ] = "______ EE Gross Payment";
    title_map["EeMode"                          ] = "EE Payment Mode";
// TODO ?? This is incorrect.
// STEVEN This can't be a mode. I don't know how it differs from 'EeGrossPmt' above.
    title_map["EePmt"                           ] = "EE Payment Mode";
    title_map["ErGrossPmt"                      ] = "______ ER Gross Payment";
    title_map["ErMode"                          ] = "ER Payment Mode";
// TODO ?? This is incorrect.
// STEVEN This can't be a mode. I don't know how it differs from 'ErGrossPmt' above.
    title_map["ErPmt"                           ] = "ER Payment Mode";
    title_map["ExcessLoan_Current"              ] = " _ Curr Excess Loan";
    title_map["ExcessLoan_Guaranteed"           ] = "Guar Excess Loan";
    title_map["ExpRatRsvCash_Current"           ] = "Mortality Reserve Cash";
    title_map["ExpRatRsvForborne_Current"       ] = "Mortality Reserve Forborne";
    title_map["ExpenseCharges_Current"          ] = "Curr Expense Charge";
    title_map["ExpenseCharges_Guaranteed"       ] = "Guar Expense Charge";
// STEVEN Can you make this one word?
//   s/Force Out/Forceout/
// which is the conventional spelling of the noun.
// ('force out' is the conventional spelling of the verb phrase.)
    title_map["GptForceout"                     ] = " _____________ GPT __Force Out";
    title_map["GrossIntCredited_Current"        ] = "Curr Gross Int Credited";
    title_map["GrossIntCredited_Guaranteed"     ] = "Guar Gross Int Credited";
    title_map["GrossPmt"                        ] = " _____________ Gross Payment";
    title_map["HoneymoonValueSpread"            ] = "Honeymoon Value Spread";
    title_map["IndvTaxBracket"                  ] = " _____________ EE Tax Bracket";
    title_map["InforceLives"                    ] = " _____ BOY Lives Inforce";
    title_map["IrrCsv_Current"                  ] = " _____________ Curr IRR on CSV";
    title_map["IrrCsv_Guaranteed"               ] = " _____________ Guar IRR on CSV";
    title_map["IrrDb_Current"                   ] = " _____________ Curr IRR on DB";
    title_map["IrrDb_Guaranteed"                ] = " _____________ Guar IRR on DB";
    title_map["Loan"                            ] = " ________ Annual Loan";
    title_map["MlyGAIntRate_Current"            ] = "Curr Mon Gen Acct Int Rate";
    title_map["MlyGAIntRate_Guaranteed"         ] = "Guar Mon Gen Acct Int Rate";
    title_map["MlyHoneymoonValueRate_Current"   ] = "Curr Mon Honeymoon Val Rate";
    title_map["MlyHoneymoonValueRate_Guaranteed"] = "Guar Mon Honeymoon Val Rate";
    title_map["MlyPolFee_Current"               ] = "Curr Monthly Policy Fee";
    title_map["MlyPolFee_Guaranteed"            ] = "Guar Monthly Policy Fee";
    title_map["MlyPostHoneymoonRate_Current"    ] = "Curr Mon Post HM Rate";
    title_map["MlyPostHoneymoonRate_Guaranteed" ] = "Guar Mon Post HM Rate";
    title_map["MlySAIntRate_Current"            ] = "Curr Mon Sep Acct Int Rate";
    title_map["MlySAIntRate_Guaranteed"         ] = "Guar Mon Sep Acct Int Rate";
    title_map["MonthlyFlatExtra"                ] = " _____________ Monthly Flat Extra";
// STEVEN s/Force Out/Forceout/ as above, and can you distinguish the two?
// The one above is a GPT forceout, and I consider 'Forceout' clear enough
// for that.
//
// This one's different. In the high-net-worth market, often the availability
// of reinsurance constrains the NAAR we'll permit. But neither NAAR nor DB
// is a free variable: only specamt and payments are, so we constrain those.
// The specamt constraint is trivial. Payments are limited in two ways: we
// won't accept a payment that results immediately in a greater NAAR than our
// internal retention and reinsurance combined; and if NAAR ever increases
// past that limit as a downstream consequence of the corridor, say, then
// we'll...uh...push money out of the policy to avoid that.
//
// When you're desigining such a feature, you're tempted to call it a
// "forceout" because it's sort of like a real forceout--a GPT forceout.
// Give in to that temptation, forgetting what Confucius said about the
// rectification of names, and you cause confusion later on. I don't much
// like "NAAR forceout" because it's awfully cryptic; but "forced withdrawal"
// is arguably even worse, and I can't think of any better name right now.
// But somehow the present concept must be distinguished.
//
// BTW, this feature has never been implemented, and the column would
// presumably always be zero, though I'd be prepared for surprise.
// If sales ever take off in this market, we'll need to add this quickly.

// Greg--I conclude that we should comment this line out for now, since it is not
//  used, but be prepared to add it in the future
//    title_map["NaarForceout"                    ] = "Net Amt At Risk __Force Out";
    title_map["NetCOICharge_Current"            ] = " _____________ Curr Net COI Charge";
    title_map["NetCOICharge_Guaranteed"         ] = " _____________ Guar Net COI Charge";
    title_map["NetClaims_Current"               ] = " _____________ Curr Net Claims";
    title_map["NetClaims_Guaranteed"            ] = " _____________ Guar Net Claims";
    title_map["NetDeathBft_Current"             ] = "Curr Net Death Benefit";
    title_map["NetDeathBft_Guaranteed"          ] = "Guar Net Death Benefit";
    title_map["NetIntCredited_Current"          ] = " _____________ Curr Net Int Credited";
    title_map["NetIntCredited_Guaranteed"       ] = " _____________ Guar Net Int Credited";
    title_map["NetPmt_Current"                  ] = " _____________ Curr Net Payment";
    title_map["NetPmt_Guaranteed"               ] = " _____________ Guar Net Payment";
    title_map["NetWD"                           ] = " _____________  _____________ Withdrawal";
    title_map["Outlay"                          ] = " _____________ Premium Outlay";
    title_map["PartMortTableMult"               ] = "Partial Mortality Muliplier";
    title_map["PolFee_Current"                  ] = "Curr ____Policy Fee";
    title_map["PolFee_Guaranteed"               ] = "Guar ____Policy Fee";
    title_map["PolicyYear"                      ] = " _____________ Policy __Year";
    title_map["PrefLoanBalance_Current"         ] = "Curr Preferred Loan Bal";
    title_map["PrefLoanBalance_Guaranteed"      ] = "Guar Preferred Loan Bal";
    title_map["PremTaxLoad_Current"             ] = "Curr Premium Tax Load";
    title_map["PremTaxLoad_Guaranteed"          ] = "Guar Premium Tax Load";
// STEVEN Implemented only for two products, and defectively at that.
// Greg-- 1) Keep in and limit use in input interface? (ideal)
//        2) Keep in and add for other products?
//        3) Keep in with obvious warts? (preferred given time constraints)
//        4) Comment out for now until a more comprehensive implementation?
// TODO ?? It's defective, so it's excluded for now.
//    title_map["ProducerCompensation"            ] = " _____________ Producer Comp";
    title_map["RefundableSalesLoad"             ] = " _____________ Refundable Sales Load";
    title_map["Salary"                          ] = " _____________  _____________ Salary";
    title_map["SpecAmt"                         ] = " _____________ Specified Amount";
    title_map["SpecAmtLoad_Current"             ] = " _____________ Curr Spec Amt Load";
    title_map["SpecAmtLoad_Guaranteed"          ] = " _____________ Guar Spec Amt Load";
    title_map["SurrChg_Current"                 ] = " _____________ Curr Surr Charge";
    title_map["SurrChg_Guaranteed"              ] = " _____________ Guar Surr Charge";
    title_map["TermPurchased_Current"           ] = "Curr Term Amt Purchased";
    title_map["TermPurchased_Guaranteed"        ] = "Guar Term Amt Purchased";
    title_map["TermSpecAmt"                     ] = "Term Specified Amount";
    title_map["TgtPrem"                         ] = " _____________ Target Premium";
    title_map["TotalIMF"                        ] = "Total Investment Mgt Fee";
    title_map["TotalLoanBalance_Current"        ] = "Curr Tot Loan Balance";
    title_map["TotalLoanBalance_Guaranteed"     ] = "Guar Tot Loan Balance";

    {
#ifdef SHOW_MISSING_FORMATS
    std::ofstream ofs("missing_formats", std::ios_base::out | std::ios_base::trunc);
    ofs << "No format found for the following numeric data.\n";
    ofs << "These data were therefore not written to xml.\n";
#endif // defined SHOW_MISSING_FORMATS
    }

// Here's my top-level analysis of the formatting specification.
//
// Formats
//
// F0: zero decimals
// F1: zero decimals, commas
// F2: two decimals, commas
// F3: scaled by 100, zero decimals, with '%' at end:
// F4: scaled by 100, two decimals, with '%' at end:
//
// Presumably all use commas as thousands-separators, so that
// an IRR of 12345.67% would be formatted as "12,345.67%".
//
// So the differences are:
//   'precision' (number of decimal places)
//   percentage (scaled by 100, '%' at end) or not
// and therefore F0 is equivalent to F1

    std::pair<int, bool> f1(0, false);
    std::pair<int, bool> f2(2, false);
    std::pair<int, bool> f3(0, true);
    std::pair<int, bool> f4(2, true);

    format_map_t format_map;

// > Special Formatting for Scalar Items
// >
// F4: scaled by 100, two decimals, with '%' at end:
// > Format as percentage "0.00%"
// >
    format_map["GuarMaxMandE"                      ] = f4;
    format_map["InitAnnGenAcctInt"                 ] = f4;
    format_map["InitAnnLoanCredRate"               ] = f4;
    format_map["InitAnnLoanDueRate"                ] = f4;
    format_map["InitAnnSepAcctGrossInt"            ] = f4;
    format_map["InitAnnSepAcctCurrGross0Rate"      ] = f4;
    format_map["InitAnnSepAcctCurrGrossHalfRate"   ] = f4;
    format_map["InitAnnSepAcctNetInt"              ] = f4;
    format_map["InitAnnSepAcctCurrNet0Rate"        ] = f4;
    format_map["InitAnnSepAcctCurrNetHalfRate"     ] = f4;
    format_map["InitAnnSepAcctGrossInt"            ] = f4;
    format_map["InitAnnSepAcctGuarGross0Rate"      ] = f4;
    format_map["InitAnnSepAcctGuarGrossHalfRate"   ] = f4;
    format_map["InitAnnSepAcctNetInt"              ] = f4;
    format_map["InitAnnSepAcctGuarNet0Rate"        ] = f4;
    format_map["InitAnnSepAcctGuarNetHalfRate"     ] = f4;
    format_map["PostHoneymoonSpread"               ] = f4;
    format_map["Preferred"                         ] = f4;
    format_map["PremTaxLoad"                       ] = f4;
    format_map["PremTaxRate"                       ] = f4;
    format_map["StatePremTaxRate"                  ] = f4;



// F3: scaled by 100, zero decimals, with '%' at end:
// > Format as percentage with no decimal places (##0%)
    format_map["GenAcctAllocation"                 ] = f3;
    format_map["SalesLoadRefund"                   ] = f3;
    format_map["SalesLoadRefundRate0"              ] = f3;
    format_map["SalesLoadRefundRate1"              ] = f3;

// >
// F2: two decimals, commas
// > Format as a number with thousand separators and two decimal places (#,###,###.00)
// >
    format_map["CountryCOIMultiplier"              ] = f2;
    format_map["GuarPrem"                          ] = f2;
    format_map["InitGLP"                           ] = f2;
    format_map["InitGSP"                           ] = f2;
    format_map["InitPrem"                          ] = f2;
    format_map["InitSevenPayPrem"                  ] = f2;
    format_map["InitTgtPrem"                       ] = f2;
// >
// F1: zero decimals, commas
// > Format as a number with thousand separators and no decimal places (#,###,###)
// >
    format_map["Age"                               ] = f1;
    format_map["AvgFund"                           ] = f1;
    format_map["AllowDbo3"                         ] = f1;
    format_map["ChildRiderAmount"                  ] = f1;
    format_map["CustomFund"                        ] = f1;
    format_map["SupplementalReport"                ] = f1;
    format_map["DBOptInitInteger"                  ] = f1;
    format_map["Dumpin"                            ] = f1;
    format_map["EndtAge"                           ] = f1;
    format_map["External1035Amount"                ] = f1;
    format_map["GenderBlended"                     ] = f1;
    format_map["GenderDistinct"                    ] = f1;
    format_map["Has1035ExchCharge"                 ] = f1;
    format_map["HasADD"                            ] = f1;
    format_map["HasChildRider"                     ] = f1;
    format_map["HasHoneymoon"                      ] = f1;
    format_map["HasSpouseRider"                    ] = f1;
    format_map["HasTerm"                           ] = f1;
    format_map["HasWP"                             ] = f1;
    format_map["InitBaseSpecAmt"                   ] = f1;
    format_map["InitTermSpecAmt"                   ] = f1;
    format_map["InitTotalSA"                       ] = f1;
    format_map["InforceIsMec"                      ] = f1;
    format_map["Internal1035Amount"                ] = f1;
    format_map["IsInforce"                         ] = f1;
    format_map["IsMec"                             ] = f1;
    format_map["LapseMonth"                        ] = f1;
    format_map["LapseYear"                         ] = f1;
    format_map["MaxDuration"                       ] = f1;
    format_map["MecMonth"                          ] = f1;
    format_map["MecYear"                           ] = f1;
    format_map["NoLapse"                           ] = f1;
    format_map["NoLapseAlwaysActive"               ] = f1;
    format_map["NoLapseMinAge"                     ] = f1;
    format_map["NoLapseMinDur"                     ] = f1;
    format_map["NominallyPar"                      ] = f1;
    format_map["OffersRiders"                      ] = f1;
    format_map["PremiumTaxIsTiered"                ] = f1;
    format_map["RetAge"                            ] = f1;
    format_map["SmokerBlended"                     ] = f1;
    format_map["SmokerDistinct"                    ] = f1;
    format_map["SpouseIssueAge"                    ] = f1;
    format_map["StatePremTaxLoad"                  ] = f1;
    format_map["UseExperienceRating"               ] = f1;
    format_map["UsePartialMort"                    ] = f1;


// > Vector Formatting
// >
// > Here are the vectors enumerated
// >
// F3: scaled by 100, zero decimals, with '%' at end:
// > Format as percentage with no decimal places (##0%)
// >
    format_map["CorridorFactor"                    ] = f3;
    format_map["FundAllocations"                   ] = f3;
    format_map["MaleProportion"                    ] = f3;
    format_map["NonsmokerProportion"               ] = f3;

// >
// F4: scaled by 100, two decimals, with '%' at end:
// > Format as percentage with two decimal places (##0.00%)
// >
    format_map["AnnGAIntRate"                      ] = f4;
    format_map["AnnHoneymoonValueRate"             ] = f4;
    format_map["AnnPostHoneymoonRate"              ] = f4;
    format_map["AnnSAIntRate"                      ] = f4;
    format_map["CashFlowIRR"                       ] = f4;
    format_map["CorpTaxBracket"                    ] = f4;
    format_map["CurrMandE"                         ] = f4;
    format_map["DacTaxPremLoadRate"                ] = f4;
    format_map["HoneymoonValueSpread"              ] = f4;
    format_map["IndvTaxBracket"                    ] = f4;
    format_map["InforceHMVector"                   ] = f4;

    format_map["IrrCsv_GuaranteedZero"             ] = f4;
    format_map["IrrDb_GuaranteedZero"              ] = f4;
    format_map["IrrCsv_CurrentZero"                ] = f4;
    format_map["IrrDb_CurrentZero"                 ] = f4;
    format_map["IrrCsv_Guaranteed"                 ] = f4;
    format_map["IrrDb_Guaranteed"                  ] = f4;
    format_map["IrrCsv_Current"                    ] = f4;
    format_map["IrrDb_Current"                     ] = f4;

    format_map["MlyGAIntRate"                      ] = f4;
    format_map["MlyHoneymoonValueRate"             ] = f4;
    format_map["MlyPostHoneymoonRate"              ] = f4;
    format_map["MlySAIntRate"                      ] = f4;
    format_map["PartMortTableMult"                 ] = f4;
    format_map["TotalIMF"                          ] = f4;
// >
// F0: zero decimals
// > Format as a number no thousand separator or decimal point (##0%)
// >
    format_map["AttainedAge"                       ] = f1;
//    format_map["DBOpt"                             ] = f1; // Not numeric.
    format_map["Duration"                          ] = f1;
    format_map["LapseYears"                        ] = f1;
    format_map["PolicyYear"                        ] = f1;
// >
// F2: two decimals, commas
// > Format as a number with thousand separators and two decimal places (#,###,###.00)
// >
    format_map["AddonMonthlyFee"                   ] = f2;
    format_map["MonthlyFlatExtra"                  ] = f2;
// >
// F1: zero decimals, commas
// > Format as a number with thousand separators and no decimal places (#,###,##0)
// >
    format_map["AcctVal"                           ] = f1;
    format_map["AcctValLoadAMD"                    ] = f1;
    format_map["AcctValLoadBOM"                    ] = f1;
    format_map["AccumulatedPremium"                ] = f1;
    format_map["AddonCompOnAssets"                 ] = f1;
    format_map["AddonCompOnPremium"                ] = f1;
    format_map["AnnPolFee"                         ] = f1;
    format_map["AvgDeathBft"                       ] = f1;
    format_map["AVRelOnDeath"                      ] = f1;
    format_map["BaseDeathBft"                      ] = f1;
    format_map["BOYAssets"                         ] = f1;
    format_map["BOYPrefLoan"                       ] = f1;
    format_map["ClaimsPaid"                        ] = f1;
    format_map["COICharge"                         ] = f1;
    format_map["Composite"                         ] = f1;
    format_map["CSVNet"                            ] = f1;
    format_map["CV7702"                            ] = f1;
    format_map["DacTaxLoad"                        ] = f1;
    format_map["DacTaxRsv"                         ] = f1;
    format_map["EeGrossPmt"                        ] = f1;
//    format_map["EeMode"                            ] = f1; // Not numeric.
    format_map["EePmt"                             ] = f1;
    format_map["EOYDeathBft"                       ] = f1;
    format_map["ErGrossPmt"                        ] = f1;
//    format_map["ErMode"                            ] = f1; // Not numeric.
    format_map["ErPmt"                             ] = f1;
    format_map["ExcessLoan"                        ] = f1;
    format_map["ExpenseCharges"                    ] = f1;
    format_map["ExpRatRsvCash"                     ] = f1;
    format_map["ExpRatRsvForborne"                 ] = f1;
    format_map["ExpRfd"                            ] = f1;
    format_map["ExpRsv"                            ] = f1;
    format_map["ExpRsvInt"                         ] = f1;
    format_map["FundNumbers"                       ] = f1;
    format_map["GptForceout"                       ] = f1;
    format_map["GrossIntCredited"                  ] = f1;
    format_map["GrossPmt"                          ] = f1;
// TODO ?? This precision is inadequate; are all the others OK?
    format_map["InforceLives"                      ] = f1;
    format_map["Loads"                             ] = f1;
    format_map["Loan"                              ] = f1;
    format_map["LoanInt"                           ] = f1;
    format_map["MlyPolFee"                         ] = f1;
    format_map["NaarForceout"                      ] = f1;
    format_map["NetClaims"                         ] = f1;
    format_map["NetCOICharge"                      ] = f1;
    format_map["NetDeathBft"                       ] = f1;
    format_map["NetIntCredited"                    ] = f1;
    format_map["NetPmt"                            ] = f1;
    format_map["NetWD"                             ] = f1;
    format_map["Outlay"                            ] = f1;
    format_map["PolFee"                            ] = f1;
    format_map["PrefLoanBalance"                   ] = f1;
    format_map["PremTaxLoad"                       ] = f1;
    format_map["ProducerCompensation"              ] = f1;
    format_map["RefundableSalesLoad"               ] = f1;
    format_map["Salary"                            ] = f1;
    format_map["SpecAmt"                           ] = f1;
    format_map["SpecAmtLoad"                       ] = f1;
    format_map["SpouseRiderAmount"                 ] = f1;
    format_map["SurrChg"                           ] = f1;
    format_map["TermPurchased"                     ] = f1;
    format_map["TermSpecAmt"                       ] = f1;
    format_map["TgtPrem"                           ] = f1;
    format_map["TotalLoanBalance"                  ] = f1;

    format_map["IrrCsv_GuaranteedZero" ] = f4;
    format_map["IrrDb_GuaranteedZero"  ] = f4;
    format_map["IrrCsv_CurrentZero"    ] = f4;
    format_map["IrrDb_CurrentZero"     ] = f4;
    format_map["IrrCsv_Guaranteed"     ] = f4;
    format_map["IrrDb_Guaranteed"      ] = f4;
    format_map["IrrCsv_Current"        ] = f4;
    format_map["IrrDb_Current"         ] = f4;

    // This is a little tricky. We have some stuff that
    // isn't in the maps inside the ledger classes. We're going to
    // stuff it into a copy of the invariant-ledger class's data.
    // To avoid copying, we'll use pointers to the data. Most of
    // this stuff is invariant anyway, so that's a reasonable
    // place to put it.
    //
    // First we make a copy of the invariant ledger:

    double_vector_map   vectors = ledger_invariant_->AllVectors;
    scalar_map          scalars = ledger_invariant_->AllScalars;
    string_map          strings = ledger_invariant_->Strings;

    // Now we add the stuff that wasn't in the invariant
    // ledger's class's maps (indexable by name). Because we're
    // working with maps of pointers, we need pointers here.
    //
    // The IRRs are the worst of all.

    if(!ledger_invariant_->IsInforce)
        {
        ledger_invariant_->CalculateIrrs(*this);
        }
    vectors["IrrCsv_GuaranteedZero" ] = &ledger_invariant_->IrrCsvGuar0    ;
    vectors["IrrDb_GuaranteedZero"  ] = &ledger_invariant_->IrrDbGuar0     ;
    vectors["IrrCsv_CurrentZero"    ] = &ledger_invariant_->IrrCsvCurr0    ;
    vectors["IrrDb_CurrentZero"     ] = &ledger_invariant_->IrrDbCurr0     ;
    vectors["IrrCsv_Guaranteed"     ] = &ledger_invariant_->IrrCsvGuarInput;
    vectors["IrrDb_Guaranteed"      ] = &ledger_invariant_->IrrDbGuarInput ;
    vectors["IrrCsv_Current"        ] = &ledger_invariant_->IrrCsvCurrInput;
    vectors["IrrDb_Current"         ] = &ledger_invariant_->IrrDbCurrInput ;

// GetMaxLength() is max *composite* length.
//    int max_length = GetMaxLength();
    double MaxDuration = ledger_invariant_->EndtAge - ledger_invariant_->Age;
    scalars["MaxDuration"] = &MaxDuration;
    int max_duration = static_cast<int>(MaxDuration);

    std::vector<double> PolicyYear;
    std::vector<double> AttainedAge;

    PolicyYear .resize(max_duration);
    AttainedAge.resize(max_duration);

    int issue_age = static_cast<int>(ledger_invariant_->Age);
    for(int j = 0; j < max_duration; ++j)
        {
        PolicyYear[j]  = 1 + j;
        AttainedAge[j] = 1 + j + issue_age;
        }

// STEVEN What about the composite? I think you want to avoid using
// an attained-age column there, because it'd be meaningless.
    vectors["AttainedAge"] = &AttainedAge;
    vectors["PolicyYear" ] = &PolicyYear ;

    vectors["InforceLives"] = &ledger_invariant_->InforceLives;

    vectors["FundNumbers"    ] = &ledger_invariant_->FundNumbers    ;
    vectors["FundAllocations"] = &ledger_invariant_->FundAllocations;

    double Composite = GetIsComposite();
    scalars["Composite"] = &Composite;

    double NoLapse =
            0 != ledger_invariant_->NoLapseMinDur
        ||  0 != ledger_invariant_->NoLapseMinAge
        ;
    scalars["NoLapse"] = &NoLapse;

    double OffersRiders = ledger_invariant_->OffersRiders();
    scalars["OffersRiders"] = &OffersRiders;

    calendar_date prep_date;

    // Skip security validation for non-interactive regression testing.
    if(!global_settings::instance().regression_testing())
        {
        // Skip security validation for the most privileged password.
        validate_security(!global_settings::instance().ash_nazg());
        prep_date = calendar_date();
        }
    else
        {
        // For regression tests, use EffDate as date prepared,
        // in order to avoid gratuitous failures.
        prep_date.julian_day_number(static_cast<int>(ledger_invariant_->EffDateJdn));
        }

    std::string PrepYear  = value_cast<std::string>(prep_date.year());
    std::string PrepMonth = calendar_date::month_name(prep_date.month());
    std::string PrepDay   = value_cast<std::string>(prep_date.day());

    strings["PrepYear" ] = &PrepYear;
    strings["PrepMonth"] = &PrepMonth;
    strings["PrepDay"  ] = &PrepDay;

    double SalesLoadRefund =
        !each_equal
            (ledger_invariant_->RefundableSalesLoad.begin()
            ,ledger_invariant_->RefundableSalesLoad.end()
            ,0.0
            );
    double SalesLoadRefundRate0 = ledger_invariant_->RefundableSalesLoad[0];
    double SalesLoadRefundRate1 = ledger_invariant_->RefundableSalesLoad[1];

    scalars["SalesLoadRefund"     ] = &SalesLoadRefund     ;
    scalars["SalesLoadRefundRate0"] = &SalesLoadRefundRate0;
    scalars["SalesLoadRefundRate1"] = &SalesLoadRefundRate1;

    std::string ScaleUnit = ledger_invariant_->ScaleUnit();
    strings["ScaleUnit"] = &ScaleUnit;

    // STEVEN Presumably you're translating this to a string in xsl;
    // why not use the first element of <DbOpt>, which is already
    // so formatted? Wouldn't that rule out any possibility of
    // inconsistency between xsl's and the program's translations?
    double DBOptInitInteger = ledger_invariant_->DBOpt[0].value();
    scalars["DBOptInitInteger"] = &DBOptInitInteger;

    double InitTotalSA =
            ledger_invariant_->InitBaseSpecAmt
        +   ledger_invariant_->InitTermSpecAmt
        ;
    scalars["InitTotalSA"] = &InitTotalSA;

    // TODO ?? This isn't the *source version*--it's the *build*.
    std::string LmiVersion(LMI_BUILD);
    strings["LmiVersion"] = &LmiVersion;

    // Maps to hold the results of formatting numeric data.

    std::map<std::string, std::string> stringscalars;
    std::map<std::string, std::vector<std::string> > stringvectors;

    stringvectors["FundNames"] = ledger_invariant_->FundNames;

    // Map the data, formatting it as necessary.

    // First we'll get the invariant stuff--the copy we made,
    // along with all the stuff we plugged into it above.

    std::string suffix = "";
    for
        (scalar_map::const_iterator j = scalars.begin()
        ;j != scalars.end()
        ;++j
        )
        {
        if(format_exists(j->first, suffix, format_map))
            stringscalars[j->first + suffix] = format(*j->second, format_map[j->first]);
        }
    for
        (string_map::const_iterator j = strings.begin()
        ;j != strings.end()
        ;++j
        )
        {
        stringscalars[j->first + suffix] = *j->second;
        }
    for
        (double_vector_map::const_iterator j = vectors.begin()
        ;j != vectors.end()
        ;++j
        )
        {
        if(format_exists(j->first, suffix, format_map))
            stringvectors[j->first + suffix] = format(*j->second, format_map[j->first]);
        }

//    stringscalars["GuarMaxMandE"] = format(*scalars["GuarMaxMandE"], 2, true);
//    stringvectors["CorridorFactor"] = format(*vectors["CorridorFactor"], 0, true);
//    stringscalars["InitAnnGenAcctInt_Current"] = format(*scalars["InitAnnGenAcctInt_Current"], 0, true);

    // That was the tricky part. Now it's all downhill.

    ledger_map const& l_map_rep = ledger_map_->held();
    ledger_map::const_iterator i = l_map_rep.begin();
    for(;i != l_map_rep.end(); i++)
        {
        std::string suffix = suffixes[i->first];
        for
            (scalar_map::const_iterator j = i->second.AllScalars.begin()
            ;j != i->second.AllScalars.end()
            ;++j
            )
            {
//            scalars[j->first + suffix] = j->second;
            if(format_exists(j->first, suffix, format_map))
                stringscalars[j->first + suffix] = format(*j->second, format_map[j->first]);
            }
        for
            (string_map::const_iterator j = i->second.Strings.begin()
            ;j != i->second.Strings.end()
            ;++j
            )
            {
            strings[j->first + suffix] = j->second;
            }
        for
            (double_vector_map::const_iterator j = i->second.AllVectors.begin()
            ;j != i->second.AllVectors.end()
            ;++j
            )
            {
//            vectors[j->first + suffix] = j->second;
            if(format_exists(j->first, suffix, format_map))
                stringvectors[j->first + suffix] = format(*j->second, format_map[j->first]);
            }
        }

    stringvectors["EeMode"] = enum_vector_to_string_vector(ledger_invariant_->EeMode);
    stringvectors["ErMode"] = enum_vector_to_string_vector(ledger_invariant_->ErMode);
    stringvectors["DBOpt"]  = enum_vector_to_string_vector(ledger_invariant_->DBOpt );

// STEVEN Here I copied some stuff from the ledger class files: the
// parts that speak of odd members that aren't in those class's
// maps. Take a look--it's a crosscheck against the systems analysis
// you've done.

// Invariant
//
//    // Special-case vectors (not <double>, or different length than others).
//    EeMode              .reserve(Length);
//    ErMode              .reserve(Length);
//    DBOpt               .reserve(Length);
//
//    std::vector<int>            FundNumbers; [not handled yet]
//    std::vector<std::string>    FundNames;   [not handled yet]
//    std::vector<int>            FundAllocs;  [not handled yet]
//
//    std::vector<double> InforceLives;
//    std::vector<double> TieredSepAcctLoadBands; [used only for cover page?]
//    std::vector<double> TieredSepAcctLoadRates; [used only for cover page?]
//
//    // Special-case strings.
//    std::string     EffDate; [furnished as PrepYear, PrepMonth, PrepDay]
//
// Variant
//
// [None of these are written to xml, and I think none is wanted.]
//
//    // special cases
//    int              Length;
//    e_basis          ExpAndGABasis;
//    e_sep_acct_basis SABasis;
//    bool             FullyInitialized;   // i.e. by Init(BasicValues* b)

// Now we're ready to write the xml.

    xml::init init;

//  want: <?xml-stylesheet type="text/xsl" href="NewTransform.xsl"?>
//  want: <!DOCTYPE sales []>
// kludged in write(std::ostream& os) below

    xml::node scalar("scalar");
    xml::node data("data");
/*
    for
        (scalar_map::const_iterator j = scalars.begin()
        ;j != scalars.end()
        ;++j
        )
        {
        std::string node_tag = j->first;
        std::string value = value_cast<std::string>(*j->second);
        scalar.push_back(xml::node(node_tag.c_str(), value.c_str()));
        }
    for
        (string_map::const_iterator j = strings.begin()
        ;j != strings.end()
        ;++j
        )
        {
        std::string node_tag = j->first;
        std::string value = value_cast<std::string>(*j->second);
        scalar.push_back(xml::node(node_tag.c_str(), value.c_str()));
        }
    for
        (double_vector_map::const_iterator j = vectors.begin()
        ;j != vectors.end()
        ;++j
        )
        {
        xml::node newcolumn("newcolumn");
        xml::node column("column");
        column.set_attr("name", j->first.c_str());
        std::vector<double> const& v = *j->second;
        for(unsigned int k = 0; k < v.size(); ++k)
            {
            xml::node duration("duration");
            duration.set_attr("number", value_cast<std::string>(k).c_str());
            duration.set_attr("column_value", value_cast<std::string>(v[k]).c_str());
            column.push_back(duration);
            }
// TODO ?? Is <newcolumn> really useful?
        newcolumn.push_back(column);
        data.push_back(newcolumn);
        }
*/
    for
        (std::map<std::string,std::string>::const_iterator j = stringscalars.begin()
        ;j != stringscalars.end()
        ;++j
        )
        {
        std::string node_tag = j->first;
        std::string value = j->second;
        scalar.push_back(xml::node(node_tag.c_str(), value.c_str()));
        }
    for
        (std::map<std::string,std::vector<std::string> >::const_iterator j = stringvectors.begin()
        ;j != stringvectors.end()
        ;++j
        )
        {
        xml::node newcolumn("newcolumn");
        xml::node column("column");
        column.set_attr("name", j->first.c_str());
        std::vector<std::string> const& v = j->second;
// TODO ?? InforceLives shows an extra value past the end; should it
// be truncated here?
        for(unsigned int k = 0; k < v.size(); ++k)
            {
            xml::node duration("duration");
            duration.set_attr("number", value_cast<std::string>(k).c_str());
            duration.set_attr("column_value", v[k].c_str());
            column.push_back(duration);
            }
// TODO ?? Is <newcolumn> really useful?
        newcolumn.push_back(column);
        data.push_back(newcolumn);
        }

    std::vector<std::string> SupplementalReportColumns;
    if(ledger_invariant_->SupplementalReport)
        {
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn00);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn01);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn02);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn03);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn04);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn05);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn06);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn07);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn08);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn09);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn10);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn11);
        }

    xml::node supplementalreport("supplementalreport");
    if(ledger_invariant_->SupplementalReport)
        {
        // Eventually customize the report name.
        supplementalreport.push_back(xml::node("title", "Supplemental Report"));
//warning() << "size " << ledger_invariant_->SupplementalReportColumns.size() << LMI_FLUSH;

        std::vector<std::string>::const_iterator j;
        for
            (j = SupplementalReportColumns.begin()
            ;j != SupplementalReportColumns.end()
            ;++j
            )
            {
//warning() << "column " << *j << " title " << title_map[*j] << LMI_FLUSH;
            xml::node columns("columns");
            columns.push_back(xml::node("name", (*j).c_str()));
            columns.push_back(xml::node("title", title_map[*j].c_str()));
            supplementalreport.push_back(columns);
            }
        }

/*
<supplementalreport>
    <title>Steve's Report</title>
    <columns>
      <name>NetDeathBft_Current</name>
      <title>Curr Net Death Benefit</title>
    </columns>
    <columns>
      <name>etc...</name>
      <title>etc...</title>
    </columns>
</supplementalreport>
*/

    x.push_back(scalar);
    x.push_back(data);
    x.push_back(supplementalreport);

    if
        (   GetIsComposite()
        &&  std::string::npos != ledger_invariant_->Comments.find("idiosyncrasy_spreadsheet")
        )
        {
        std::ofstream ofs
            (("values" + configurable_settings::instance().spreadsheet_file_extension()).c_str()
            ,std::ios_base::out | std::ios_base::trunc
            );
        for
            (std::map<std::string,std::vector<std::string> >::const_iterator j = stringvectors.begin()
            ;j != stringvectors.end()
            ;++j
            )
            {
            ofs << j->first.c_str() << '\t';
            }
        ofs << '\n';

        for(unsigned int i = 0; i < static_cast<unsigned int>(GetMaxLength()); ++i)
            {
            for
                (std::map<std::string,std::vector<std::string> >::const_iterator j = stringvectors.begin()
                ;j != stringvectors.end()
                ;++j
                )
                {
                std::vector<std::string> const& v = j->second;
// TODO ?? InforceLives shows an extra value past the end; should it
// be truncated here?
                if(i < v.size())
                    {
                    ofs << v[i].c_str() << '\t';
                    }
                else
                    {
                    ofs << '\t';
                    }
                }
            ofs << '\n';
            }
        }
}

int Ledger::class_version() const
{
    return 0;
}

std::string Ledger::xml_root_name() const
{
    return "illustration";
}

void Ledger::write(std::ostream& os) const
{
    xml::init init;
    xml::node root(xml_root_name().c_str());
    root << *this;
// Need DOCTYPE support, which xmlwrapp lacks--so can't do this:
//    os << root;

    std::string s;
    root.node_to_string(s);
    std::string token("<?xml version=\"1.0\"?>");
    std::string string_to_insert
        ("\n<!DOCTYPE sales [\n]>\n"
        "<?xml-stylesheet type=\"text/xsl\" href=\"NewTransform.xsl\"?>"
        );
    s.insert
        (s.find(token) + token.length()
        ,string_to_insert
        );
    os << s;
}

#ifdef USING_CURRENT_XMLWRAPP
#   undef set_attr
#endif

