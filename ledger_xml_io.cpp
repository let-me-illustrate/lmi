// Ledger xml input and output.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "ledger.hpp"

#include "alert.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"
#include "ledger_invariant.hpp"
#include "ledger_text_formats.hpp"      // ledger_format()
#include "ledger_variant.hpp"
#include "ledger_xsl.hpp"               // xsl_filepath()
#include "mc_enum_aux.hpp"              // mc_e_vector_to_string_vector()
#include "miscellany.hpp"               // each_equal(), ios_out_trunc_binary(), lmi_array_size()
#include "oecumenic_enumerations.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "value_cast.hpp"
#include "version.hpp"
#include "xml_lmi.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <xsltwrapp/xsltwrapp.h>

#include <algorithm>                    // std::transform()
#include <functional>                   // std::minus
#include <map>
#include <ostream>
#include <utility>                      // std::pair

namespace
{
int const n = 7;

char const* char_p_suffixes[n] =
    {"_Current"        // mce_run_gen_curr_sep_full
    ,"_Guaranteed"     // mce_run_gen_guar_sep_full
    ,"_Midpoint"       // mce_run_gen_mdpt_sep_full
    ,"_CurrentZero"    // mce_run_gen_curr_sep_zero
    ,"_GuaranteedZero" // mce_run_gen_guar_sep_zero
    ,"_CurrentHalf"    // mce_run_gen_curr_sep_half
    ,"_GuaranteedHalf" // mce_run_gen_guar_sep_half
    };

std::vector<std::string> const suffixes
    (char_p_suffixes
    ,char_p_suffixes + n
    );

typedef std::map<std::string, std::pair<int, oenum_format_style> > format_map_t;
typedef std::map<std::string, std::string> title_map_t;

// For all numbers (so-called 'scalars' and 'vectors', but not
// 'strings') grabbed from all ledgers, look for a format. If one
// is found, use it to turn the number into a string. If not, and
// the field is named in unavailable(), then it's ignored. Otherwise,
// format_exists() displays a warning and ignores the field (because
// throwing an exception would cause only the first warning to be
// displayed).
//
// Rationale: Silently falling back on some default format can't be
// right, because it masks defects that should be fixed: no default
// can be universally appropriate.
//
// For names formed as
//   basename + '_' + suffix
// only the basename is used as a map key. Lookups in the format map
// are strict, as they must be, else one key like "A" would match
// anything beginning with that letter.
//
// Some of the unavailable fields could easily be made available
// someday; perhaps others should be eliminated from class Ledger.

bool unavailable(std::string const& s)
{
    static std::string const a[] =
        {"DateOfBirthJdn"        // used by group quotes
        ,"EffDateJdn"            // used by group quotes
        ,"InitDacTaxRate"        // used by PrintRosterTabDelimited(); not cents
        ,"InitPremTaxRate"       // used by PrintRosterTabDelimited(); not cents
        ,"SubstdTable"           // probably not needed
        ,"InitMlyPolFee"         // used by PrintRosterTabDelimited()
        ,"InitTgtPremHiLoadRate" // used by PrintRosterTabDelimited(); not cents
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return contains(v, s);
}

bool format_exists
    (std::string  const& s
    ,std::string  const& suffix
    ,format_map_t const& m
    )
{
    if(contains(m, s))
        {
        return true;
        }
    else if(unavailable(s))
        {
        return false;
        }
    else
        {
        warning() << "No format found for " << s << suffix << LMI_FLUSH;
        return false;
        }
}

} // Unnamed namespace.

void Ledger::write(xml::element& x) const
{
    title_map_t title_map;

// Can't seem to get a literal &nbsp; into the output.

// Original:   title_map["AttainedAge"                     ] = " &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0; End of &#xA0;&#xA0;Year Age";
// No good:    title_map["AttainedAge"                     ] = " &&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0;&&#xA0; End of &&#xA0;&&#xA0;Year Age";
// No good:    title_map["AttainedAge"                     ] = " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; End of &nbsp;&nbsp;Year Age";
// No good:    title_map["AttainedAge"                     ] = " &amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp;&amp;nbsp; End of &amp;nbsp;&amp;nbsp;Year Age";
// No good:    title_map["AttainedAge"                     ] = "<![CDATA[ &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0; End of &#xA0;&#xA0;Year Age]]>";
// No good:    title_map["AttainedAge"                     ] = " ááááááááááááá End of ááYear Age";
// No good:    title_map["AttainedAge"                     ] = " &#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160; End of &#160;&#160;Year Age";

//  Here are the columns to be listed in the user interface
//  as well as their corresponding titles.

    // Current and guaranteed variants are generally given for columns
    // that vary by basis. Some offer only a current variant because
    // they are defined only on a current basis--experience-rating
    // columns, e.g.

    title_map["AVGenAcct_CurrentZero"           ] = "Curr Charges Account Value Gen Acct";
    title_map["AVGenAcct_GuaranteedZero"        ] = "Guar Charges Account Value Gen Acct";
    title_map["AVRelOnDeath_Current"            ] = "Account Value ____Released on Death";
    title_map["AVSepAcct_CurrentZero"           ] = "Curr Charges 0% Account Value Sep Acct";
    title_map["AVSepAcct_GuaranteedZero"        ] = "Guar Charges 0% Account Value Sep Acct";
    title_map["AcctVal_Current"                 ] = " _____________ Curr Account Value";
    title_map["AcctVal_CurrentZero"             ] = "Curr Charges 0% Account Value";
    title_map["AcctVal_Guaranteed"              ] = " _____________ Guar Account Value";
    title_map["AcctVal_GuaranteedZero"          ] = "Guar Charges 0% Account Value";
    title_map["AddonCompOnAssets"               ] = "Additional Comp on Assets";
    title_map["AddonCompOnPremium"              ] = "Additional Comp on Premium";
    title_map["AddonMonthlyFee"                 ] = "Additional Monthly Fee";
    title_map["AnnGAIntRate_Current"            ] = " _____________ Curr Ann Gen Acct Int Rate";
    title_map["AnnGAIntRate_Guaranteed"         ] = " _____________ Guar Ann Gen Acct Int Rate";
    title_map["AnnHoneymoonValueRate_Current"   ] = "Curr Ann Honeymoon Value Rate";
    title_map["AnnHoneymoonValueRate_Guaranteed"] = "Guar Ann Honeymoon Value Rate";
    title_map["AnnPostHoneymoonRate_Current"    ] = "Curr Post Honeymoon Rate";
    title_map["AnnPostHoneymoonRate_Guaranteed" ] = "Guar Post Honeymoon Rate";
    title_map["AnnSAIntRate_Current"            ] = " _____________ Curr Ann Sep Acct Int Rate";
    title_map["AnnSAIntRate_Guaranteed"         ] = " _____________ Guar Ann Sep Acct Int Rate";
    title_map["AttainedAge"                     ] = " _____________ _____________ End of __Year Age";
    title_map["AvgDeathBft_Current"             ] = "Curr Avg Death Benefit";
    title_map["AvgDeathBft_Guaranteed"          ] = "Guar Avg Death Benefit";
    title_map["BaseDeathBft_Current"            ] = " _____________ Curr Base Death Benefit";
    title_map["BaseDeathBft_Guaranteed"         ] = " _____________ Guar Base Death Benefit";
    title_map["COICharge_Current"               ] = " _____________ _____________ Curr COI Charge";
    title_map["COICharge_Guaranteed"            ] = " _____________ _____________ Guar COI Charge";
    title_map["CSVNet_Current"                  ] = " _____________ Curr Net Cash Surr Value";
    title_map["CSVNet_CurrentZero"              ] = "Curr Charges 0% Net Cash Surr Value";
    title_map["CSVNet_Guaranteed"               ] = " _____________ Guar Net Cash Surr Value";
    title_map["CSVNet_GuaranteedZero"           ] = "Guar Charges 0% Net Cash Surr Value";
    title_map["CV7702_Current"                  ] = "Curr 7702 Cash Value";
    title_map["CV7702_Guaranteed"               ] = "Guar 7702 Cash Value";
    title_map["ClaimsPaid_Current"              ] = " _____________ _______ Curr ___Claims ___Paid";
    title_map["ClaimsPaid_Guaranteed"           ] = " _____________ _______ Guar ___Claims ___Paid";
    title_map["CorpTaxBracket"                  ] = " _____________ Corp Tax Bracket";
    title_map["CorridorFactor"                  ] = " _____________ _____________ Corridor Factor";
    title_map["CurrMandE"                       ] = "Mortality and Expense Charge";
    title_map["DBOpt"                           ] = "Death Benefit Option";
    title_map["DacTaxLoad_Current"              ] = " _____________ Curr DAC Tax Load";
    title_map["DacTaxLoad_Guaranteed"           ] = " _____________ Guar DAC Tax Load";
    title_map["DacTaxRsv_Current"               ] = "Curr DAC Tax Reserve";
    title_map["DacTaxRsv_Guaranteed"            ] = "Guar DAC Tax Reserve";
    title_map["DeathProceedsPaid_Current"       ] = " _____________ Curr Death Proceeds Paid";
    title_map["DeathProceedsPaid_Guaranteed"    ] = " _____________ Guar Death Proceeds Paid";
    title_map["EOYDeathBft_Current"             ] = " _____________ Curr EOY Death Benefit";
    title_map["EOYDeathBft_Guaranteed"          ] = " _____________ Guar EOY Death Benefit";
    title_map["EeGrossPmt"                      ] = " _____________ ______ EE Gross Payment";
    title_map["EeModalMinimumPremium"           ] = "EE Modal Minimum Premium";
    title_map["EeMode"                          ] = "EE Payment Mode";
// TODO ?? This can't be a mode. I don't know how it differs from 'EeGrossPmt' above.
    title_map["EePmt"                           ] = "EE Payment Mode";
    title_map["ErGrossPmt"                      ] = " _____________ ______ ER Gross Payment";
    title_map["ErModalMinimumPremium"           ] = "ER Modal Minimum Premium";
    title_map["ErMode"                          ] = "ER Payment Mode";
// TODO ?? This can't be a mode. I don't know how it differs from 'ErGrossPmt' above.
    title_map["ErPmt"                           ] = "ER Payment Mode";
    title_map["ExpenseCharges_Current"          ] = "Curr Expense Charge";
    title_map["ExpenseCharges_Guaranteed"       ] = "Guar Expense Charge";
    title_map["ExperienceReserve_Current"       ] = " _____________ Experience Rating Reserve";
    title_map["GptForceout"                     ] = "Forceout";
    title_map["GrossIntCredited_Current"        ] = "Curr Gross Int Credited";
    title_map["GrossIntCredited_Guaranteed"     ] = "Guar Gross Int Credited";
    title_map["GrossPmt"                        ] = " _____________ _____________ Premium Outlay";
    title_map["HoneymoonValueSpread"            ] = "Honeymoon Value Spread";
    title_map["IndvTaxBracket"                  ] = " _____________ EE Tax Bracket";
    title_map["InforceLives"                    ] = " _____________ ______BOY _______Lives _______Inforce";
    title_map["IrrCsv_Current"                  ] = " _____________ _____________ Curr IRR on CSV";
    title_map["IrrCsv_Guaranteed"               ] = " _____________ _____________ Guar IRR on CSV";
    title_map["IrrDb_Current"                   ] = " _____________ _____________ Curr IRR on DB";
    title_map["IrrDb_Guaranteed"                ] = " _____________ _____________ Guar IRR on DB";
    title_map["KFactor_Current"                 ] = " _____________ Experience _______Rating K Factor";
    title_map["LoanIntAccrued_Current"          ] = " _____________ ____Curr Loan Int __Accrued";
    title_map["LoanIntAccrued_Guaranteed"       ] = " _____________ ____Guar Loan Int __Accrued";
    title_map["MlyGAIntRate_Current"            ] = "Curr Monthly Gen Acct Int Rate";
    title_map["MlyGAIntRate_Guaranteed"         ] = "Guar Monthly Gen Acct Int Rate";
    title_map["MlyHoneymoonValueRate_Current"   ] = "Curr Monthly Honeymoon Value Rate";
    title_map["MlyHoneymoonValueRate_Guaranteed"] = "Guar Monthly Honeymoon Value Rate";
    title_map["MlyPostHoneymoonRate_Current"    ] = "Curr Monthly Post Honeymoon Rate";
    title_map["MlyPostHoneymoonRate_Guaranteed" ] = "Guar Monthly Post Honeymoon Rate";
    title_map["MlySAIntRate_Current"            ] = "Curr Monthly Sep Acct Int Rate";
    title_map["MlySAIntRate_Guaranteed"         ] = "Guar Monthly Sep Acct Int Rate";
    title_map["ModalMinimumPremium"             ] = "Modal Minimum Premium";
    title_map["AnnualFlatExtra"                 ] = " _____________ Annual Flat Extra";
//    title_map["NaarForceout"                    ] = "Forced Withdrawal due to NAAR Limit";
    title_map["NetCOICharge_Current"            ] = "Experience _______Rating _______Net COI Charge";
    title_map["NetClaims_Current"               ] = " _____________ _____________ Curr Net Claims";
    title_map["NetClaims_Guaranteed"            ] = " _____________ _____________ Guar Net Claims";
    title_map["NetIntCredited_Current"          ] = " _____________ Curr Net Int Credited";
    title_map["NetIntCredited_Guaranteed"       ] = " _____________ Guar Net Int Credited";
    title_map["NetPmt_Current"                  ] = " _____________ Curr Net Payment";
    title_map["NetPmt_Guaranteed"               ] = " _____________ Guar Net Payment";
    title_map["NetWD"                           ] = " _____________ _____________ _____________ Withdrawal";
    title_map["NewCashLoan"                     ] = " _____________ _____________ Annual Loan";
    title_map["Outlay"                          ] = " _____________ _____________ ____Net Outlay";
    title_map["PartMortTableMult"               ] = "Partial Mortality Muliplier";
    title_map["PolicyFee_Current"               ] = "Curr ____Policy Fee";
    title_map["PolicyFee_Guaranteed"            ] = "Guar ____Policy Fee";
    title_map["PolicyYear"                      ] = " _____________ _____________ Policy __Year";
    title_map["PrefLoanBalance_Current"         ] = "Curr Preferred Loan Bal";
    title_map["PrefLoanBalance_Guaranteed"      ] = "Guar Preferred Loan Bal";
    title_map["PremTaxLoad_Current"             ] = "Curr Premium Tax Load";
    title_map["PremTaxLoad_Guaranteed"          ] = "Guar Premium Tax Load";
// TODO ?? Excluded because it's defectively implemented:
//    title_map["ProducerCompensation"            ] = " _____________ Producer Compensation";
    title_map["ProjectedCoiCharge_Current"      ] = "Experience Rating Projected COI Charge";
    title_map["RefundableSalesLoad"             ] = " _____________ Refundable Sales Load";
    title_map["RiderCharges_Current"            ] = " _____________ _____________ Curr Rider Charges";
    title_map["Salary"                          ] = " _____________ _____________ Salary";
    title_map["SepAcctCharges_Current"          ] = "Curr Sep Acct Charges";
    title_map["SepAcctCharges_Guaranteed"       ] = "Guar Sep Acct Charges";
    title_map["SpecAmt"                         ] = " _____________ _____________ Specified Amount";
    title_map["SpecAmtLoad_Current"             ] = " _____________ Curr Spec Amt Load";
    title_map["SpecAmtLoad_Guaranteed"          ] = " _____________ Guar Spec Amt Load";
    title_map["SurrChg_Current"                 ] = " _____________ Curr Surr Charge";
    title_map["SurrChg_Guaranteed"              ] = " _____________ Guar Surr Charge";
    title_map["TermPurchased_Current"           ] = " _____________ Curr Term Amt Purchased";
    title_map["TermPurchased_Guaranteed"        ] = " _____________ Guar Term Amt Purchased";
    title_map["TermSpecAmt"                     ] = " _____________ Term Specified Amount";
    title_map["TgtPrem"                         ] = " _____________ Target Premium";
    title_map["TotalIMF"                        ] = "Total Investment Mgt Fee";
    title_map["TotalLoanBalance_Current"        ] = " _____________ Curr Total Loan Balance";
    title_map["TotalLoanBalance_Guaranteed"     ] = " _____________ Guar Total Loan Balance";

    // TODO ?? Titles ought to be read from an external file that
    // permits flexible customization. Compliance might require that
    // 'AcctVal_Current' be called "Cash Value" for one policy form,
    // and "Account Value" for another, in order to match the terms
    // used in the contract exactly. Therefore, these titles probably
    // belong in the product database, which permits variation by
    // product--though it does not accommodate strings as this is
    // written in 2006-07. DATABASE !! So consider adding them there
    // when the database is revamped.

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

    std::pair<int, oenum_format_style> f1(0, oe_format_normal);
    std::pair<int, oenum_format_style> f2(2, oe_format_normal);
    std::pair<int, oenum_format_style> f3(0, oe_format_percentage);
    std::pair<int, oenum_format_style> f4(2, oe_format_percentage);

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
    format_map["InitAnnSepAcctCurrGross0Rate"      ] = f4;
    format_map["InitAnnSepAcctCurrGrossHalfRate"   ] = f4;
    format_map["InitAnnSepAcctCurrNet0Rate"        ] = f4;
    format_map["InitAnnSepAcctCurrNetHalfRate"     ] = f4;
    format_map["InitAnnSepAcctGrossInt"            ] = f4;
    format_map["InitAnnSepAcctGuarGross0Rate"      ] = f4;
    format_map["InitAnnSepAcctGuarGrossHalfRate"   ] = f4;
    format_map["InitAnnSepAcctGuarNet0Rate"        ] = f4;
    format_map["InitAnnSepAcctGuarNetHalfRate"     ] = f4;
    format_map["InitAnnSepAcctNetInt"              ] = f4;
    format_map["PostHoneymoonSpread"               ] = f4;
    format_map["Preferred"                         ] = f4;
    format_map["PremTaxRate"                       ] = f4;

// F3: scaled by 100, zero decimals, with '%' at end:
// > Format as percentage with no decimal places (##0%)
    format_map["SalesLoadRefund"                   ] = f3;
    format_map["SalesLoadRefundRate0"              ] = f3;
    format_map["SalesLoadRefundRate1"              ] = f3;

// >
// F2: two decimals, commas
// > Format as a number with thousand separators and two decimal places (#,###,###.00)
// >
    format_map["CurrentCoiMultiplier"              ] = f2;
    format_map["GuarPrem"                          ] = f2;
    format_map["InforceTaxBasis"                   ] = f2;
    format_map["InforceUnloanedAV"                 ] = f2;
    format_map["InitGLP"                           ] = f2;
    format_map["InitGSP"                           ] = f2;
    format_map["InitPrem"                          ] = f2;
    format_map["InitSevenPayPrem"                  ] = f2;
    format_map["InitTgtPrem"                       ] = f2;
    format_map["InitMinPrem"                       ] = f2;
    format_map["InitMinDumpin"                     ] = f2;
// >
// F1: zero decimals, commas
// > Format as a number with thousand separators and no decimal places (#,###,###)
// >
    format_map["Age"                               ] = f1;
    format_map["AllowDbo3"                         ] = f1;
    format_map["AvgFund"                           ] = f1;
    format_map["ChildRiderAmount"                  ] = f1;
    format_map["CustomFund"                        ] = f1;
    format_map["Dumpin"                            ] = f1;
    format_map["EndtAge"                           ] = f1;
    format_map["External1035Amount"                ] = f1;
    format_map["GenAcctAllocation"                 ] = f1;
    format_map["GenderBlended"                     ] = f1;
    format_map["GenderDistinct"                    ] = f1;
    format_map["Has1035ExchCharge"                 ] = f1;
    format_map["HasADD"                            ] = f1;
    format_map["HasChildRider"                     ] = f1;
    format_map["HasHoneymoon"                      ] = f1;
    format_map["HasSpouseRider"                    ] = f1;
    format_map["HasSupplSpecAmt"                   ] = f1;
    format_map["HasTerm"                           ] = f1;
    format_map["HasWP"                             ] = f1;
    format_map["InforceIsMec"                      ] = f1;
    format_map["InforceMonth"                      ] = f1;
    format_map["InforceYear"                       ] = f1;
    format_map["InitBaseSpecAmt"                   ] = f1;
    format_map["InitTermSpecAmt"                   ] = f1;
    format_map["InitTotalSA"                       ] = f1;
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
    format_map["RetAge"                            ] = f1;
    format_map["SmokerBlended"                     ] = f1;
    format_map["SmokerDistinct"                    ] = f1;
    format_map["SplitFundAllocation"               ] = f1;
    format_map["SplitMinPrem"                      ] = f1;
    format_map["SpouseIssueAge"                    ] = f1;
    format_map["SupplementalReport"                ] = f1;
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
    format_map["PartMortTableMult"                 ] = f3;

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
    format_map["HoneymoonValueSpread"              ] = f4;
    format_map["IndvTaxBracket"                    ] = f4;
    format_map["InforceHMVector"                   ] = f4;

    format_map["IrrCsv_Current"                    ] = f4;
    format_map["IrrCsv_CurrentZero"                ] = f4;
    format_map["IrrCsv_Guaranteed"                 ] = f4;
    format_map["IrrCsv_GuaranteedZero"             ] = f4;
    format_map["IrrDb_Current"                     ] = f4;
    format_map["IrrDb_CurrentZero"                 ] = f4;
    format_map["IrrDb_Guaranteed"                  ] = f4;
    format_map["IrrDb_GuaranteedZero"              ] = f4;

    format_map["MlyGAIntRate"                      ] = f4;
    format_map["MlyHoneymoonValueRate"             ] = f4;
    format_map["MlyPostHoneymoonRate"              ] = f4;
    format_map["MlySAIntRate"                      ] = f4;
    format_map["TotalIMF"                          ] = f4;
// >
// F0: zero decimals
// > Format as a number no thousand separator or decimal point (##0%)
// >
    format_map["AttainedAge"                       ] = f1;
    format_map["Duration"                          ] = f1;
    format_map["LapseYears"                        ] = f1;
    format_map["PolicyYear"                        ] = f1;
// >
// F2: two decimals, commas
// > Format as a number with thousand separators and two decimal places (#,###,###.00)
// >
    format_map["AddonMonthlyFee"                   ] = f2;
// TODO ?? The precision of 'InforceLives' and 'KFactor' is inadequate.
// Is every other format OK?
    format_map["InforceLives"                      ] = f2;
    format_map["KFactor"                           ] = f2;
    format_map["AnnualFlatExtra"                   ] = f2;
// >
// F1: zero decimals, commas
// > Format as a number with thousand separators and no decimal places (#,###,##0)
// >
    format_map["AcctVal"                           ] = f1;
    format_map["AccumulatedPremium"                ] = f1;
    format_map["AddonCompOnAssets"                 ] = f1;
    format_map["AddonCompOnPremium"                ] = f1;
    format_map["AvgDeathBft"                       ] = f1;
    format_map["AVGenAcct"                         ] = f1;
    format_map["AVRelOnDeath"                      ] = f1;
    format_map["AVSepAcct"                         ] = f1;
    format_map["BaseDeathBft"                      ] = f1;
    format_map["BOYAssets"                         ] = f1;
    format_map["ClaimsPaid"                        ] = f1;
    format_map["COICharge"                         ] = f1;
    format_map["Composite"                         ] = f1;
    format_map["CSVNet"                            ] = f1;
    format_map["CV7702"                            ] = f1;
    format_map["DacTaxLoad"                        ] = f1;
    format_map["DacTaxRsv"                         ] = f1;
    format_map["DeathProceedsPaid"                 ] = f1;
    format_map["EeGrossPmt"                        ] = f1;
    format_map["EeModalMinimumPremium"             ] = f1;
//    format_map["EeMode"                            ] = f1; // Not numeric.
    format_map["EePmt"                             ] = f1;
    format_map["EOYDeathBft"                       ] = f1;
    format_map["ErGrossPmt"                        ] = f1;
    format_map["ErModalMinimumPremium"             ] = f1;
//    format_map["ErMode"                            ] = f1; // Not numeric.
    format_map["ErPmt"                             ] = f1;
    format_map["ExpenseCharges"                    ] = f1;
    format_map["ExperienceReserve"                 ] = f1;
    format_map["FundNumbers"                       ] = f1;
    format_map["GptForceout"                       ] = f1;
    format_map["GrossIntCredited"                  ] = f1;
    format_map["GrossPmt"                          ] = f1;
    format_map["Loads"                             ] = f1;
    format_map["LoanInt"                           ] = f1;
    format_map["LoanIntAccrued"                    ] = f1;
    format_map["ModalMinimumPremium"               ] = f1;
    format_map["NaarForceout"                      ] = f1;
    format_map["NetClaims"                         ] = f1;
    format_map["NetCOICharge"                      ] = f1;
    format_map["NetIntCredited"                    ] = f1;
    format_map["NetPmt"                            ] = f1;
    format_map["NetWD"                             ] = f1;
    format_map["NewCashLoan"                       ] = f1;
    format_map["Outlay"                            ] = f1;
    format_map["PolicyFee"                         ] = f1;
    format_map["PrefLoanBalance"                   ] = f1;
    format_map["PremTaxLoad"                       ] = f1;
    format_map["ProducerCompensation"              ] = f1;
    format_map["ProjectedCoiCharge"                ] = f1;
    format_map["RefundableSalesLoad"               ] = f1;
    format_map["RiderCharges"                      ] = f1;
    format_map["Salary"                            ] = f1;
    format_map["SepAcctCharges"                    ] = f1;
    format_map["SpecAmt"                           ] = f1;
    format_map["SpecAmtLoad"                       ] = f1;
    format_map["SpouseRiderAmount"                 ] = f1;
    format_map["SurrChg"                           ] = f1;
    format_map["TermPurchased"                     ] = f1;
    format_map["TermSpecAmt"                       ] = f1;
    format_map["TgtPrem"                           ] = f1;
    format_map["TotalLoanBalance"                  ] = f1;

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

// TODO ?? An attained-age column is meaningless in a composite. So
// are several others--notably those affected by partial mortaility.
    vectors["AttainedAge"] = &AttainedAge;
    vectors["PolicyYear" ] = &PolicyYear ;

    vectors["InforceLives"] = &ledger_invariant_->InforceLives;

    vectors["FundNumbers"    ] = &ledger_invariant_->FundNumbers    ;
    vectors["FundAllocations"] = &ledger_invariant_->FundAllocations;

    // The Ledger object should contain a basic minimal set of columns
    // from which others may be derived. It must be kept small because
    // its size imposes a practical limit on the number of lives that
    // can be run as part of a single census.
    //
    // TODO ?? A really good design would give users the power to
    // define and store their own derived-column definitions. For now,
    // however, code changes are required, and this is as appropriate
    // a place as any to make them.

    LedgerInvariant const& Invar = GetLedgerInvariant();
    LedgerVariant   const& Curr_ = GetCurrFull();
    LedgerVariant   const& Guar_ = GetGuarFull();

    // ET !! Easier to write as
    //   std::vector<double> NetDeathBenefit =
    //     Curr_.EOYDeathBft - Curr_.TotalLoanBalance;
    std::vector<double> NetDeathBenefit(Curr_.EOYDeathBft);
    std::transform
        (NetDeathBenefit.begin()
        ,NetDeathBenefit.end()
        ,Curr_.TotalLoanBalance.begin()
        ,NetDeathBenefit.begin()
        ,std::minus<double>()
        );
    vectors   ["NetDeathBenefit"] = &NetDeathBenefit;
    title_map ["NetDeathBenefit"] = " _____________ __Net __Death Benefit";
    format_map["NetDeathBenefit"] = f1;

    std::vector<double> SupplDeathBft_Current   (Curr_.TermPurchased);
    std::vector<double> SupplDeathBft_Guaranteed(Guar_.TermPurchased);
    vectors   ["SupplDeathBft_Current"   ] = &SupplDeathBft_Current;
    vectors   ["SupplDeathBft_Guaranteed"] = &SupplDeathBft_Guaranteed;
    title_map ["SupplDeathBft_Current"   ] = " _____________ Curr Suppl Death Benefit";
    title_map ["SupplDeathBft_Guaranteed"] = " _____________ Guar Suppl Death Benefit";
    format_map["SupplDeathBft_Current"   ] = f1;
    format_map["SupplDeathBft_Guaranteed"] = f1;

    std::vector<double> SupplSpecAmt(Invar.TermSpecAmt);
    vectors   ["SupplSpecAmt"            ] = &SupplSpecAmt;
    title_map ["SupplSpecAmt"            ] = " _____________ Suppl Specified Amount";
    format_map["SupplSpecAmt"            ] = f1;

    // [End of derived columns.]

    double Composite = is_composite();
    scalars["Composite"] = &Composite;

    double NoLapse =
            0 != ledger_invariant_->NoLapseMinDur
        ||  0 != ledger_invariant_->NoLapseMinAge
        ;
    scalars["NoLapse"] = &NoLapse;

    std::string LmiVersion(LMI_VERSION);
    calendar_date prep_date;

    // Skip authentication for non-interactive regression testing.
    if(!global_settings::instance().regression_testing())
        {
        authenticate_system();
        }
    else
        {
        // For regression tests,
        //   - use an invariant string as version
        //   - use EffDate as date prepared
        // in order to avoid gratuitous failures.
        LmiVersion = "Regression testing";
        prep_date.julian_day_number(static_cast<int>(ledger_invariant_->EffDateJdn));
        }

    strings["LmiVersion"] = &LmiVersion;

    std::string PrepYear  = value_cast<std::string>(prep_date.year());
    std::string PrepMonth = month_name(prep_date.month());
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

    double InitTotalSA =
            ledger_invariant_->InitBaseSpecAmt
        +   ledger_invariant_->InitTermSpecAmt
        ;
    scalars["InitTotalSA"] = &InitTotalSA;

    // Maps to hold the results of formatting numeric data.

    std::map<std::string, std::string> stringscalars;
    std::map<std::string, std::vector<std::string> > stringvectors;

    stringvectors["FundNames"] = ledger_invariant_->FundNames;

    // Map the data, formatting it as necessary.

    // First we'll get the invariant stuff--the copy we made,
    // along with all the stuff we plugged into it above.
    {
    std::string suffix = "";
    for
        (scalar_map::const_iterator j = scalars.begin()
        ;j != scalars.end()
        ;++j
        )
        {
        if(format_exists(j->first, suffix, format_map))
            stringscalars[j->first + suffix] = ledger_format(*j->second, format_map[j->first]);
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
            stringvectors[j->first + suffix] = ledger_format(*j->second, format_map[j->first]);
        }
    }

//    stringscalars["GuarMaxMandE"] = ledger_format(*scalars["GuarMaxMandE"], 2, true);
//    stringvectors["CorridorFactor"] = ledger_format(*vectors["CorridorFactor"], 0, true);
//    stringscalars["InitAnnGenAcctInt_Current"] = ledger_format(*scalars["InitAnnGenAcctInt_Current"], 0, true);

    // That was the tricky part. Now it's all downhill.

    ledger_map_t const& l_map_rep = ledger_map_->held();
    typedef ledger_map_t::const_iterator lmci;
    for(lmci i = l_map_rep.begin(); i != l_map_rep.end(); i++)
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
                stringscalars[j->first + suffix] = ledger_format(*j->second, format_map[j->first]);
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
                stringvectors[j->first + suffix] = ledger_format(*j->second, format_map[j->first]);
            }
        }

    stringvectors["EeMode"] = mc_e_vector_to_string_vector(ledger_invariant_->EeMode);
    stringvectors["ErMode"] = mc_e_vector_to_string_vector(ledger_invariant_->ErMode);
    stringvectors["DBOpt"]  = mc_e_vector_to_string_vector(ledger_invariant_->DBOpt );

// TODO ?? Here I copied some stuff from the ledger class files: the
// parts that speak of odd members that aren't in those class's
// maps. This may reveal incomplete or incorrect systems analysis.

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
//    mcenum_gen_basis GenBasis_;
//    mcenum_sep_basis SepBasis_;
//    bool             FullyInitialized;   // i.e. by Init(BasicValues* b)

// Now we're ready to write the xml.

    xml::element scalar("scalar");
    xml::element data("data");
    for
        (std::map<std::string,std::string>::const_iterator j = stringscalars.begin()
        ;j != stringscalars.end()
        ;++j
        )
        {
        std::string node_tag = j->first;
        std::string value = j->second;
        scalar.push_back(xml::element(node_tag.c_str(), value.c_str()));
        }
    for
        (std::map<std::string,std::vector<std::string> >::const_iterator j = stringvectors.begin()
        ;j != stringvectors.end()
        ;++j
        )
        {
        xml::element newcolumn("newcolumn");
        xml::element column("column");
        xml_lmi::set_attr(column, "name", j->first.c_str());
        std::vector<std::string> const& v = j->second;
// TODO ?? InforceLives shows an extra value past the end; should it
// be truncated here?
        for(unsigned int k = 0; k < v.size(); ++k)
            {
            xml::element duration("duration");
            xml_lmi::set_attr(duration, "number", value_cast<std::string>(k).c_str());
            xml_lmi::set_attr(duration, "column_value", v[k].c_str());
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

    xml::element supplementalreport("supplementalreport");
    if(ledger_invariant_->SupplementalReport)
        {
        // Eventually customize the report name.
        supplementalreport.push_back(xml::element("title", "Supplemental Report"));

        std::vector<std::string>::const_iterator j;
        for
            (j = SupplementalReportColumns.begin()
            ;j != SupplementalReportColumns.end()
            ;++j
            )
            {
            xml::element columns("columns");
            columns.push_back(xml::element("name", (*j).c_str()));
            columns.push_back(xml::element("title", title_map[*j].c_str()));
            supplementalreport.push_back(columns);
            }
        }

/*
<supplementalreport>
    <title>Some Report</title>
    <columns>
      <name>TotalLoanBalance_Current</name>
      <title>Curr Total Loan Balance</title>
    </columns>
    <columns>
      <name>etc...</name>
      <title>etc...</title>
    </columns>
</supplementalreport>
*/

// SOMEDAY !! It is inefficient to push_back() data into xml elements
// and then push_back() those populated elements into the root. Avoid
// needless copying by using insert() to create the intermediate
// elements and using the iterator it returns to write leaf elements.
    x.push_back(scalar);
    x.push_back(data);
    x.push_back(supplementalreport);

    if(is_composite() && contains(global_settings::instance().pyx(), "values_tsv"))
        {
        throw_if_interdicted(*this);

        configurable_settings const& z = configurable_settings::instance();
        fs::path filepath
            (   z.print_directory()
            +   "/values"
            +   z.spreadsheet_file_extension()
            );
        fs::ofstream ofs(filepath, ios_out_trunc_binary());

        for
            (std::map<std::string,std::vector<std::string> >::const_iterator j = stringvectors.begin()
            ;j != stringvectors.end()
            ;++j
            )
            {
            ofs << j->first << '\t';
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
                if(i < v.size())
                    {
                    ofs << v[i] << '\t';
                    }
                else
                    {
                    ofs << '\t';
                    }
                }
            ofs << '\n';
            }
        if(!ofs)
            {
            fatal_error() << "Unable to write '" << filepath << "'." << LMI_FLUSH;
            }
        }
}

int Ledger::class_version() const
{
    return 0;
}

std::string const& Ledger::xml_root_name() const
{
    static std::string const s("illustration");
    return s;
}

void Ledger::write(std::ostream& os) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    write(root);
    os << root;
}

/// Write a scaled copy of the ledger as xsl-fo.
///
/// The original ledger must not be modified because scaling is not
/// reentrant. However, copying does not prevent that problem here,
/// because what is scaled is actually not copied due to use of
/// shared_ptr; see comment on
///   https://savannah.nongnu.org/bugs/?13599
/// in the ledger-class implementation.

void Ledger::write_xsl_fo(std::ostream& os) const
{
    try
        {
        Ledger scaled_ledger(*this);
        scaled_ledger.AutoScale();

        xml_lmi::xml_document d(xml_root_name());
        scaled_ledger.write(d.root_node());

        xslt::stylesheet z(xsl_filepath(scaled_ledger).string().c_str());
        os << z.apply(d.document());
        }
    catch(...)
        {
        report_exception();
        }
}

