// Ledger evaluator returning values of all ledger fields.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ledger_evaluator.hpp"

#include "alert.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_text_formats.hpp"      // ledger_format()
#include "ledger_variant.hpp"
#include "map_lookup.hpp"
#include "mc_enum_aux.hpp"              // mc_e_vector_to_string_vector()
#include "miscellany.hpp"               // each_equal(), lmi_array_size()
#include "oecumenic_enumerations.hpp"
#include "value_cast.hpp"
#include "version.hpp"

#include <algorithm>                    // transform()
#include <functional>                   // minus
#include <unordered_map>
#include <utility>                      // pair

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

typedef std::unordered_map<std::string, std::pair<int,oenum_format_style>> format_map_t;
typedef std::unordered_map<std::string, std::string> title_map_t;

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
        ,"ListBillDateJdn"       // probably not needed
        ,"InforceAsOfDateJdn"    // probably not needed
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

std::string ledger_evaluator::operator()(std::string const& scalar) const
{
    return map_lookup(scalars_, scalar);
}

std::string ledger_evaluator::operator()
    (std::string const& vector
    ,std::size_t index
    ) const
{
    return map_lookup(vectors_, vector).at(index);
}

ledger_evaluator Ledger::make_evaluator() const
{
    // This map defines titles for all columns that can appear in a
    // supplemental report.
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

    title_map["AVGenAcct_CurrentZero"           ] = "Curr Charges\nAccount Value\nGen Acct";
    title_map["AVGenAcct_GuaranteedZero"        ] = "Guar Charges\nAccount Value\nGen Acct";
    title_map["AVRelOnDeath_Current"            ] = "Account Value\nReleased on Death";
    title_map["AVSepAcct_CurrentZero"           ] = "Curr Charges\n0% Account Value\nSep Acct";
    title_map["AVSepAcct_GuaranteedZero"        ] = "Guar Charges\n0% Account Value\nSep Acct";
    title_map["AcctVal_Current"                 ] = "Curr Account\nValue";
    title_map["AcctVal_CurrentZero"             ] = "Curr Charges\n0% Account Value";
    title_map["AcctVal_Guaranteed"              ] = "Guar Account\nValue";
    title_map["AcctVal_GuaranteedZero"          ] = "Guar Charges\n0% Account Value";
    title_map["AnnGAIntRate_Current"            ] = "Curr Ann\nGen Acct\nInt Rate";
    title_map["AnnSAIntRate_Current"            ] = "Curr Ann\nSep Acct\nInt Rate";
    title_map["AttainedAge"                     ] = "End of\nYear Age";
    title_map["BaseDeathBft_Current"            ] = "Curr Base\nDeath Benefit";
    title_map["BaseDeathBft_Guaranteed"         ] = "Guar Base\nDeath Benefit";
    title_map["COICharge_Current"               ] = "Curr COI\nCharge";
    title_map["COICharge_Guaranteed"            ] = "Guar COI\nCharge";
    title_map["CSVNet_Current"                  ] = "Curr Net\nCash\nSurr Value";
    title_map["CSVNet_CurrentZero"              ] = "Curr Charges\n0% Net Cash\nSurr Value";
    title_map["CSVNet_Guaranteed"               ] = "Guar Net\nCash\nSurr Value";
    title_map["CSVNet_GuaranteedZero"           ] = "Guar Charges\n0% Net Cash\nSurr Value";
    title_map["ClaimsPaid_Current"              ] = "Curr\nClaims\nPaid";
    title_map["CorridorFactor"                  ] = "Corridor Factor";
    title_map["DeathProceedsPaid_Current"       ] = "Curr Death\nProceeds Paid";
    title_map["EOYDeathBft_Current"             ] = "Curr EOY\nDeath Benefit";
    title_map["EOYDeathBft_Guaranteed"          ] = "Guar EOY\nDeath Benefit";
    title_map["EeGrossPmt"                      ] = "EE Gross\nPayment";
    title_map["EeModalMinimumPremium"           ] = "EE Modal\nMinimum\nPremium";
    title_map["ErGrossPmt"                      ] = "ER Gross\nPayment";
    title_map["ErModalMinimumPremium"           ] = "ER Modal\nMinimum\nPremium";
    title_map["ExperienceReserve_Current"       ] = "Experience\nRating\nReserve";
    title_map["GrossIntCredited_Current"        ] = "Curr Gross\nInt Credited";
    title_map["GrossPmt"                        ] = "Premium Outlay";
    title_map["InforceLives"                    ] = "BOY\nLives\nInforce";
    title_map["IrrCsv_Current"                  ] = "Curr IRR\non CSV";
    title_map["IrrCsv_Guaranteed"               ] = "Guar IRR\non CSV";
    title_map["IrrDb_Current"                   ] = "Curr IRR\non DB";
    title_map["IrrDb_Guaranteed"                ] = "Guar IRR\non DB";
    title_map["KFactor_Current"                 ] = "Experience\nRating K Factor";
    title_map["LoanIntAccrued_Current"          ] = "Curr Loan\nInt\nAccrued";
    title_map["ModalMinimumPremium"             ] = "Modal Minimum Premium";
    title_map["NetCOICharge_Current"            ] = "Experience\nRating\nNet COI Charge";
    title_map["NetClaims_Current"               ] = "Curr Net\nClaims";
    title_map["NetDeathBenefit"                 ] = "Net\nDeath Benefit";
    title_map["NetIntCredited_Current"          ] = "Curr Net\nInt Credited";
    title_map["NetWD"                           ] = "Withdrawal";
    title_map["NewCashLoan"                     ] = "Annual Loan";
    title_map["Outlay"                          ] = "Net Outlay";
    title_map["PolicyFee_Current"               ] = "Curr\nPolicy Fee";
    title_map["PolicyYear"                      ] = "Policy\nYear";
    title_map["ProjectedCoiCharge_Current"      ] = "Experience\nRating\nProjected\nCOI Charge";
    title_map["RiderCharges_Current"            ] = "Curr Rider\nCharges";
    title_map["SepAcctCharges_Current"          ] = "Curr Sep\nAcct Charges";
    title_map["SpecAmt"                         ] = "Specified Amount";
    title_map["SpecAmtLoad_Current"             ] = "Curr Spec\nAmt Load";
    title_map["SupplDeathBft_Current"           ] = "Curr Suppl\nDeath Benefit";
    title_map["SupplDeathBft_Guaranteed"        ] = "Guar Suppl\nDeath Benefit";
    title_map["SupplSpecAmt"                    ] = "Suppl Specified Amount";
    title_map["TermPurchased_Current"           ] = "Curr Term\nAmt Purchased";
    title_map["TermPurchased_Guaranteed"        ] = "Guar Term\nAmt Purchased";
    title_map["TermSpecAmt"                     ] = "Term Specified Amount";
    title_map["TotalLoanBalance_Current"        ] = "Curr Total\nLoan Balance";
    title_map["TotalLoanBalance_Guaranteed"     ] = "Guar Total\nLoan Balance";

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

    std::pair<int,oenum_format_style> f1(0, oe_format_normal);
    std::pair<int,oenum_format_style> f2(2, oe_format_normal);
    std::pair<int,oenum_format_style> f3(0, oe_format_percentage);
    std::pair<int,oenum_format_style> f4(2, oe_format_percentage);

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
    format_map["SalesLoadRefundRate0"              ] = f3;
    format_map["SalesLoadRefundRate1"              ] = f3;
    format_map["GenAcctAllocationPercent"          ] = f3;
    format_map["GenAcctAllocationComplementPercent"] = f3;

// >
// F2: two decimals, commas
// > Format as a number with thousand separators and two decimal places (#,###,###.00)
// >
    format_map["CurrentCoiMultiplier"              ] = f2;
    format_map["EeListBillPremium"                 ] = f2;
    format_map["ErListBillPremium"                 ] = f2;
    format_map["GuarPrem"                          ] = f2;
    format_map["InforceTaxBasis"                   ] = f2;
    format_map["InforceUnloanedAV"                 ] = f2;
    format_map["InitGLP"                           ] = f2;
    format_map["InitGSP"                           ] = f2;
    format_map["InitPrem"                          ] = f2;
    format_map["InitSevenPayPrem"                  ] = f2;
    format_map["InitTgtPrem"                       ] = f2;
    format_map["InitMinPrem"                       ] = f2;
    format_map["ListBillPremium"                   ] = f2;
    format_map["ModalMinimumDumpin"                ] = f2;
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
    format_map["HasSalesLoadRefund"                ] = f1;
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
    format_map["GroupIndivSelection"               ] = f1;
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

    std::vector<double> InitAnnLoanDueRate(max_duration);
    std::fill
        (InitAnnLoanDueRate.begin()
        ,InitAnnLoanDueRate.end()
        ,ledger_invariant_->GetInitAnnLoanDueRate()
        );
    vectors["InitAnnLoanDueRate"] = &InitAnnLoanDueRate;

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

    std::vector<double> PremiumLoads(max_duration);
    std::vector<double> AdminCharges(max_duration);
    for(int j = 0; j < max_duration; ++j)
        {
        PremiumLoads[j] = Invar.GrossPmt[j] - Curr_.NetPmt[j];
        AdminCharges[j] = Curr_.SpecAmtLoad[j] + Curr_.PolicyFee[j];
        }

    vectors   ["PremiumLoads"] = &PremiumLoads;
    format_map["PremiumLoads"] = f1;
    vectors   ["AdminCharges"] = &AdminCharges;
    format_map["AdminCharges"] = f1;

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
    format_map["NetDeathBenefit"] = f1;

    std::vector<double> SupplDeathBft_Current   (Curr_.TermPurchased);
    std::vector<double> SupplDeathBft_Guaranteed(Guar_.TermPurchased);
    vectors   ["SupplDeathBft_Current"   ] = &SupplDeathBft_Current;
    vectors   ["SupplDeathBft_Guaranteed"] = &SupplDeathBft_Guaranteed;
    format_map["SupplDeathBft_Current"   ] = f1;
    format_map["SupplDeathBft_Guaranteed"] = f1;

    std::vector<double> SupplSpecAmt(Invar.TermSpecAmt);
    vectors   ["SupplSpecAmt"            ] = &SupplSpecAmt;
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

    double HasSalesLoadRefund =
        !each_equal(ledger_invariant_->RefundableSalesLoad, 0.0);
    double SalesLoadRefundRate0 = ledger_invariant_->RefundableSalesLoad[0];
    double SalesLoadRefundRate1 = ledger_invariant_->RefundableSalesLoad[1];

    scalars["HasSalesLoadRefund"  ] = &HasSalesLoadRefund  ;
    scalars["SalesLoadRefundRate0"] = &SalesLoadRefundRate0;
    scalars["SalesLoadRefundRate1"] = &SalesLoadRefundRate1;

    double GenAcctAllocation           = ledger_invariant_->GenAcctAllocation;
    double GenAcctAllocationComplement = 1. - GenAcctAllocation;

    scalars["GenAcctAllocationPercent"          ] = &GenAcctAllocation;
    scalars["GenAcctAllocationComplementPercent"] = &GenAcctAllocationComplement;

    std::string ScaleUnit = ledger_invariant_->ScaleUnit();
    strings["ScaleUnit"] = &ScaleUnit;

    double InitTotalSA =
            ledger_invariant_->InitBaseSpecAmt
        +   ledger_invariant_->InitTermSpecAmt
        ;
    scalars["InitTotalSA"] = &InitTotalSA;

    // Maps to hold the results of formatting numeric data.

    std::unordered_map<std::string, std::string> stringscalars;
    std::unordered_map<std::string, std::vector<std::string>> stringvectors;

    stringvectors["FundNames"] = ledger_invariant_->FundNames;

    // Map the data, formatting it as necessary.

    // First we'll get the invariant stuff--the copy we made,
    // along with all the stuff we plugged into it above.
    {
    std::string suffix = "";
    for(auto const& j : scalars)
        {
        if(format_exists(j.first, suffix, format_map))
            stringscalars[j.first + suffix] = ledger_format(*j.second, format_map[j.first]);
        }
    for(auto const& j : strings)
        {
        stringscalars[j.first + suffix] = *j.second;
        }
    for(auto const& j : vectors)
        {
        if(format_exists(j.first, suffix, format_map))
            stringvectors[j.first + suffix] = ledger_format(*j.second, format_map[j.first]);
        }
    }

//    stringscalars["GuarMaxMandE"] = ledger_format(*scalars["GuarMaxMandE"], 2, true);
//    stringvectors["CorridorFactor"] = ledger_format(*vectors["CorridorFactor"], 0, true);
//    stringscalars["InitAnnGenAcctInt_Current"] = ledger_format(*scalars["InitAnnGenAcctInt_Current"], 0, true);

    // That was the tricky part. Now it's all downhill.

    for(auto const& i : ledger_map_->held())
        {
        std::string suffix = suffixes[i.first];
        for(auto const& j : i.second.AllScalars)
            {
//            scalars[j.first + suffix] = j.second;
            if(format_exists(j.first, suffix, format_map))
                stringscalars[j.first + suffix] = ledger_format(*j.second, format_map[j.first]);
            }
        for(auto const& j : i.second.Strings)
            {
            strings[j.first + suffix] = j.second;
            }
        for(auto const& j : i.second.AllVectors)
            {
//            vectors[j.first + suffix] = j.second;
            if(format_exists(j.first, suffix, format_map))
                stringvectors[j.first + suffix] = ledger_format(*j.second, format_map[j.first]);
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
// [None of these are stored, and I think none is wanted.]
//
//    // special cases
//    int              Length;
//    mcenum_gen_basis GenBasis_;
//    mcenum_sep_basis SepBasis_;
//    bool             FullyInitialized;   // I.e. by Init(BasicValues const* b)

    if(ledger_invariant_->SupplementalReport)
        {
        std::vector<std::string> SupplementalReportColumns;
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

        // Eventually customize the report name.
        stringscalars["SupplementalReportTitle"] = "Supplemental Report";

        std::vector<std::string> SupplementalReportColumnsTitles;
        SupplementalReportColumnsTitles.reserve(SupplementalReportColumns.size());

        for(auto const& j : SupplementalReportColumns)
            {
            SupplementalReportColumnsTitles.push_back(title_map[j]);
            }

        stringvectors["SupplementalReportColumnsNames"] = std::move(SupplementalReportColumns);
        stringvectors["SupplementalReportColumnsTitles"] = std::move(SupplementalReportColumnsTitles);
        }

    return ledger_evaluator(std::move(stringscalars), std::move(stringvectors));
}
