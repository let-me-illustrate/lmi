// Ledger evaluator returning values of all ledger fields.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "ledger.hpp"
#include "ledger_evaluator.hpp"

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
#include "map_lookup.hpp"
#include "mc_enum_aux.hpp"              // mc_e_vector_to_string_vector()
#include "miscellany.hpp"               // each_equal(), ios_out_trunc_binary()
#include "oecumenic_enumerations.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "ssize_lmi.hpp"
#include "value_cast.hpp"
#include "version.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>                    // transform()
#include <functional>                   // minus
#include <map>
#include <numeric>                      // iota()
#include <unordered_map>
#include <utility>                      // move(), pair

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
typedef std::unordered_map<std::string, std::string> mask_map_t;

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
    static std::vector<std::string> const v
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

ledger_evaluator Ledger::make_evaluator() const
{
    title_map_t title_map;

//  Here are the columns to be listed in the user interface
//  as well as their corresponding titles.

    // Current and guaranteed variants are generally given for columns
    // that vary by basis. Some offer only a current variant because
    // they are defined only on a current basis--experience-rating
    // columns, e.g.

    title_map["AVGenAcct_CurrentZero"           ] = "Curr\nCharges\nAccount\nValue\nGen Acct";
    title_map["AVGenAcct_GuaranteedZero"        ] = "Guar\nCharges\nAccount\nValue\nGen Acct";
    title_map["AVRelOnDeath_Current"            ] = "Account\nValue\nReleased\non Death";
    title_map["AVSepAcct_CurrentZero"           ] = "Curr\nCharges\n0% Account\nValue\nSep Acct";
    title_map["AVSepAcct_GuaranteedZero"        ] = "Guar\nCharges\n0% Account\nValue\nSep Acct";
    title_map["AcctVal_Current"                 ] = "Curr\nAccount\nValue";
    title_map["AcctVal_CurrentZero"             ] = "Curr\nCharges\n0% Account\nValue";
    title_map["AcctVal_Guaranteed"              ] = "Guar\nAccount\nValue";
    title_map["AcctVal_GuaranteedZero"          ] = "Guar\nCharges\n0% Account\nValue";
    title_map["AddonCompOnAssets"               ] = "Additional\nComp on\nAssets";
    title_map["AddonCompOnPremium"              ] = "Additional\nComp on\nPremium";
    title_map["AddonMonthlyFee"                 ] = "Additional\nMonthly\nFee";
    title_map["AnnGAIntRate_Current"            ] = "Curr Ann\nGen Acct\nInt Rate";
    title_map["AnnGAIntRate_Guaranteed"         ] = "Guar Ann\nGen Acct\nInt Rate";
    title_map["AnnHoneymoonValueRate_Current"   ] = "Curr Ann\nHoneymoon\nValue Rate";
    title_map["AnnHoneymoonValueRate_Guaranteed"] = "Guar Ann\nHoneymoon\nValue Rate";
    title_map["AnnLoanDueRate"                  ] = "Loan\nInt Rate";
    title_map["AnnPostHoneymoonRate_Current"    ] = "Curr Post\nHoneymoon\nRate";
    title_map["AnnPostHoneymoonRate_Guaranteed" ] = "Guar Post\nHoneymoon\nRate";
    title_map["AnnSAIntRate_Current"            ] = "Curr Ann\nSep Acct\nInt Rate";
    title_map["AnnSAIntRate_Guaranteed"         ] = "Guar Ann\nSep Acct\nInt Rate";
    title_map["AttainedAge"                     ] = "End of\nYear\nAge";
    title_map["AvgDeathBft_Current"             ] = "Curr Avg\nDeath\nBenefit";
    title_map["AvgDeathBft_Guaranteed"          ] = "Guar Avg\nDeath\nBenefit";
    title_map["BaseDeathBft_Current"            ] = "Curr Base\nDeath\nBenefit";
    title_map["BaseDeathBft_Guaranteed"         ] = "Guar Base\nDeath\nBenefit";
    title_map["COICharge_Current"               ] = "Curr COI\nCharge";
    title_map["COICharge_Guaranteed"            ] = "Guar COI\nCharge";
    title_map["CSVNet_Current"                  ] = "Curr Net\nCash\nSurr Value";
    title_map["CSVNet_CurrentZero"              ] = "Curr\nCharges\n0% Net Cash\nSurr Value";
    title_map["CSVNet_Guaranteed"               ] = "Guar Net\nCash\nSurr Value";
    title_map["CSVNet_GuaranteedZero"           ] = "Guar\nCharges\n0% Net Cash\nSurr Value";
    title_map["CV7702_Current"                  ] = "Curr 7702\nCash Value";
    title_map["CV7702_Guaranteed"               ] = "Guar 7702\nCash Value";
    title_map["ClaimsPaid_Current"              ] = "Curr\nClaims\nPaid";
    title_map["ClaimsPaid_Guaranteed"           ] = "Guar\nClaims\nPaid";
    title_map["CorpTaxBracket"                  ] = "Corp Tax\nBracket";
    title_map["CorridorFactor"                  ] = "Corridor\nFactor";
    title_map["CurrMandE"                       ] = "Mortality\nand\nExpense\nCharge";
    title_map["DBOpt"                           ] = "Death\nBenefit\nOption";
    title_map["DacTaxLoad_Current"              ] = "Curr DAC\nTax\nLoad";
    title_map["DacTaxLoad_Guaranteed"           ] = "Guar DAC\nTax\nLoad";
    title_map["DacTaxRsv_Current"               ] = "Curr DAC\nTax\nReserve";
    title_map["DacTaxRsv_Guaranteed"            ] = "Guar DAC\nTax\nReserve";
    title_map["DeathProceedsPaid_Current"       ] = "Curr Death\nProceeds\nPaid";
    title_map["DeathProceedsPaid_Guaranteed"    ] = "Guar Death\nProceeds\nPaid";
    title_map["EOYDeathBft_Current"             ] = "Curr EOY\nDeath\nBenefit";
    title_map["EOYDeathBft_Guaranteed"          ] = "Guar EOY\nDeath\nBenefit";
    title_map["EeGrossPmt"                      ] = "EE Gross\nPayment";
    title_map["EeModalMinimumPremium"           ] = "EE Modal\nMinimum\nPremium";
    title_map["EeMode"                          ] = "EE\nPayment\nMode";
// TODO ?? This can't be a mode. I don't know how it differs from 'EeGrossPmt' above.
    title_map["EePmt"                           ] = "EE\nPayment\nMode";
    title_map["ErGrossPmt"                      ] = "ER Gross\nPayment";
    title_map["ErModalMinimumPremium"           ] = "ER Modal\nMinimum\nPremium";
    title_map["ErMode"                          ] = "ER\nPayment\nMode";
// TODO ?? This can't be a mode. I don't know how it differs from 'ErGrossPmt' above.
    title_map["ErPmt"                           ] = "ER\nPayment\nMode";
    title_map["ExpenseCharges_Current"          ] = "Curr\nExpense\nCharge";
    title_map["ExpenseCharges_Guaranteed"       ] = "Guar\nExpense\nCharge";
    title_map["ExperienceReserve_Current"       ] = "Experience\nRating\nReserve";
    title_map["GptForceout"                     ] = "Forceout";
    title_map["GrossIntCredited_Current"        ] = "Curr Gross\nInt\nCredited";
    title_map["GrossIntCredited_Guaranteed"     ] = "Guar Gross\nInt\nCredited";
    title_map["GrossPmt"                        ] = "Premium\nOutlay";
    title_map["HoneymoonValueSpread"            ] = "Honeymoon\nValue\nSpread";
    title_map["IndvTaxBracket"                  ] = "EE Tax\nBracket";
    title_map["InforceLives"                    ] = "BOY\nLives\nInforce";
    title_map["IrrCsv_Current"                  ] = "Curr IRR\non CSV";
    title_map["IrrCsv_Guaranteed"               ] = "Guar IRR\non CSV";
    title_map["IrrDb_Current"                   ] = "Curr IRR\non DB";
    title_map["IrrDb_Guaranteed"                ] = "Guar IRR\non DB";
    title_map["KFactor_Current"                 ] = "Experience\nRating\nK Factor";
    title_map["LoanIntAccrued_Current"          ] = "Curr Loan\nInt\nAccrued";
    title_map["LoanIntAccrued_Guaranteed"       ] = "Guar Loan\nInt\nAccrued";
    title_map["MlyGAIntRate_Current"            ] = "Curr Monthly\nGen Acct\nInt Rate";
    title_map["MlyGAIntRate_Guaranteed"         ] = "Guar Monthly\nGen Acct\nInt Rate";
    title_map["MlyHoneymoonValueRate_Current"   ] = "Curr Monthly\nHoneymoon\nValue Rate";
    title_map["MlyHoneymoonValueRate_Guaranteed"] = "Guar Monthly\nHoneymoon\nValue Rate";
    title_map["MlyPostHoneymoonRate_Current"    ] = "Curr Monthly\nPost\nHoneymoon\nRate";
    title_map["MlyPostHoneymoonRate_Guaranteed" ] = "Guar Monthly\nPost\nHoneymoon\nRate";
    title_map["MlySAIntRate_Current"            ] = "Curr Monthly\nSep Acct\nInt Rate";
    title_map["MlySAIntRate_Guaranteed"         ] = "Guar Monthly\nSep Acct\nInt Rate";
    title_map["ModalMinimumPremium"             ] = "Modal\nMinimum\nPremium";
    title_map["AnnualFlatExtra"                 ] = "Annual\nFlat\nExtra";
//    title_map["NaarForceout"                    ] = "Forced\nWithdrawal\ndue to\nNAAR Limit";
    title_map["NetCOICharge_Current"            ] = "Experience\nRating\nNet COI\nCharge";
    title_map["NetClaims_Current"               ] = "Curr Net\nClaims";
    title_map["NetClaims_Guaranteed"            ] = "Guar Net\nClaims";
    title_map["NetIntCredited_Current"          ] = "Curr Net\nInt\nCredited";
    title_map["NetIntCredited_Guaranteed"       ] = "Guar Net\nInt\nCredited";
    title_map["NetPmt_Current"                  ] = "Curr Net\nPayment";
    title_map["NetPmt_Guaranteed"               ] = "Guar Net\nPayment";
    title_map["NetWD"                           ] = "Withdrawal";
    title_map["NewCashLoan"                     ] = "Annual\nLoan";
    title_map["Outlay"                          ] = "Net Outlay";
    title_map["PartMortTableMult"               ] = "Partial\nMortality\nMuliplier";
    title_map["PolicyFee_Current"               ] = "Curr\nPolicy\nFee";
    title_map["PolicyFee_Guaranteed"            ] = "Guar\nPolicy\nFee";
    title_map["PolicyYear"                      ] = "Policy\nYear";
    title_map["PrefLoanBalance_Current"         ] = "Curr\nPreferred\nLoan Bal";
    title_map["PrefLoanBalance_Guaranteed"      ] = "Guar\nPreferred\nLoan Bal";
    title_map["PremTaxLoad_Current"             ] = "Curr\nPremium\nTax Load";
    title_map["PremTaxLoad_Guaranteed"          ] = "Guar\nPremium\nTax Load";
// Excluded because it's unimplemented:
//    title_map["ProducerCompensation"            ] = "Producer\nCompensation";
    title_map["ProjectedCoiCharge_Current"      ] = "Experience\nRating\nProjected\nCOI Charge";
    title_map["RefundableSalesLoad"             ] = "Refundable\nSales\nLoad";
    title_map["RiderCharges_Current"            ] = "Curr Rider\nCharges";
    title_map["Salary"                          ] = "Salary";
    title_map["SepAcctCharges_Current"          ] = "Curr Sep\nAcct\nCharges";
    title_map["SepAcctCharges_Guaranteed"       ] = "Guar Sep\nAcct\nCharges";
    title_map["SpecAmt"                         ] = "Specified\nAmount";
    title_map["SpecAmtLoad_Current"             ] = "Curr Spec\nAmt Load";
    title_map["SpecAmtLoad_Guaranteed"          ] = "Guar Spec\nAmt Load";
    title_map["SurrChg_Current"                 ] = "Curr Surr\nCharge";
    title_map["SurrChg_Guaranteed"              ] = "Guar Surr\nCharge";
    title_map["TermPurchased_Current"           ] = "Curr Term\nAmt\nPurchased";
    title_map["TermPurchased_Guaranteed"        ] = "Guar Term\nAmt\nPurchased";
    title_map["TermSpecAmt"                     ] = "Term\nSpecified\nAmount";
    title_map["TgtPrem"                         ] = "Target\nPremium";
    title_map["TotalIMF"                        ] = "Total\nInvestment\nMgt Fee";
    title_map["TotalLoanBalance_Current"        ] = "Curr Total\nLoan\nBalance";
    title_map["TotalLoanBalance_Guaranteed"     ] = "Guar Total\nLoan\nBalance";

    // TODO ?? Titles ought to be read from an external file that
    // permits flexible customization. Compliance might require that
    // 'AcctVal_Current' be called "Cash Value" for one policy form,
    // and "Account Value" for another, in order to match the terms
    // used in the contract exactly. Therefore, these titles probably
    // belong in the product database, which permits variation by
    // product--though it does not accommodate strings as this is
    // written in 2006-07. DATABASE !! So consider adding them there
    // when the database is revamped.

    mask_map_t mask_map;

    mask_map ["AVGenAcct_CurrentZero"           ] = "999,999,999";
    mask_map ["AVGenAcct_GuaranteedZero"        ] = "999,999,999";
    mask_map ["AVRelOnDeath_Current"            ] = "999,999,999";
    mask_map ["AVSepAcct_CurrentZero"           ] = "999,999,999";
    mask_map ["AVSepAcct_GuaranteedZero"        ] = "999,999,999";
    mask_map ["AcctVal_Current"                 ] = "999,999,999";
    mask_map ["AcctVal_CurrentZero"             ] = "999,999,999";
    mask_map ["AcctVal_Guaranteed"              ] = "999,999,999";
    mask_map ["AcctVal_GuaranteedZero"          ] = "999,999,999";
    mask_map ["AddonCompOnAssets"               ] = "999,999,999";
    mask_map ["AddonCompOnPremium"              ] = "999,999,999";
    mask_map ["AddonMonthlyFee"                 ] = "999,999,999";
    mask_map ["AnnGAIntRate_Current"            ] =      "99.99%";
    mask_map ["AnnGAIntRate_Guaranteed"         ] =      "99.99%";
    mask_map ["AnnHoneymoonValueRate_Current"   ] =      "99.99%";
    mask_map ["AnnHoneymoonValueRate_Guaranteed"] =      "99.99%";
    mask_map ["AnnLoanDueRate"                  ] =      "99.99%";
    mask_map ["AnnPostHoneymoonRate_Current"    ] =      "99.99%";
    mask_map ["AnnPostHoneymoonRate_Guaranteed" ] =      "99.99%";
    mask_map ["AnnSAIntRate_Current"            ] =      "99.99%";
    mask_map ["AnnSAIntRate_Guaranteed"         ] =      "99.99%";
    mask_map ["AttainedAge"                     ] =         "999";
    mask_map ["AvgDeathBft_Current"             ] = "999,999,999";
    mask_map ["AvgDeathBft_Guaranteed"          ] = "999,999,999";
    mask_map ["BaseDeathBft_Current"            ] = "999,999,999";
    mask_map ["BaseDeathBft_Guaranteed"         ] = "999,999,999";
    mask_map ["COICharge_Current"               ] = "999,999,999";
    mask_map ["COICharge_Guaranteed"            ] = "999,999,999";
    mask_map ["CSVNet_Current"                  ] = "999,999,999";
    mask_map ["CSVNet_CurrentZero"              ] = "999,999,999";
    mask_map ["CSVNet_Guaranteed"               ] = "999,999,999";
    mask_map ["CSVNet_GuaranteedZero"           ] = "999,999,999";
    mask_map ["CV7702_Current"                  ] = "999,999,999";
    mask_map ["CV7702_Guaranteed"               ] = "999,999,999";
    mask_map ["ClaimsPaid_Current"              ] = "999,999,999";
    mask_map ["ClaimsPaid_Guaranteed"           ] = "999,999,999";
    mask_map ["CorpTaxBracket"                  ] =      "99.99%";
    mask_map ["CorridorFactor"                  ] =       "9999%";
    mask_map ["CurrMandE"                       ] =      "99.99%";
    mask_map ["DBOpt"                           ] =         "ROP";
    mask_map ["DacTaxLoad_Current"              ] = "999,999,999";
    mask_map ["DacTaxLoad_Guaranteed"           ] = "999,999,999";
    mask_map ["DacTaxRsv_Current"               ] = "999,999,999";
    mask_map ["DacTaxRsv_Guaranteed"            ] = "999,999,999";
    mask_map ["DeathProceedsPaid_Current"       ] = "999,999,999";
    mask_map ["DeathProceedsPaid_Guaranteed"    ] = "999,999,999";
    mask_map ["EOYDeathBft_Current"             ] = "999,999,999";
    mask_map ["EOYDeathBft_Guaranteed"          ] = "999,999,999";
    mask_map ["EeGrossPmt"                      ] = "999,999,999";
    mask_map ["EeModalMinimumPremium"           ] = "999,999,999";
    mask_map ["EeMode"                          ] =  "Semiannual";
// This can't be a mode. I don't know how it differs from 'EeGrossPmt' above.
    mask_map ["EePmt"                           ] = "999,999,999";
    mask_map ["ErGrossPmt"                      ] = "999,999,999";
    mask_map ["ErModalMinimumPremium"           ] = "999,999,999";
    mask_map ["ErMode"                          ] =  "Semiannual";
// This can't be a mode. I don't know how it differs from 'ErGrossPmt' above.
    mask_map ["ErPmt"                           ] = "999,999,999";
    mask_map ["ExpenseCharges_Current"          ] = "999,999,999";
    mask_map ["ExpenseCharges_Guaranteed"       ] = "999,999,999";
    mask_map ["ExperienceReserve_Current"       ] = "999,999,999";
    mask_map ["GptForceout"                     ] = "999,999,999";
    mask_map ["GrossIntCredited_Current"        ] = "999,999,999";
    mask_map ["GrossIntCredited_Guaranteed"     ] = "999,999,999";
    mask_map ["GrossPmt"                        ] = "999,999,999";
    mask_map ["HoneymoonValueSpread"            ] =      "99.99%";
    mask_map ["IndvTaxBracket"                  ] =      "99.99%";
    mask_map ["InforceLives"                    ] = "999,999,999";
    mask_map ["IrrCsv_Current"                  ] =  "100000.00%";
    mask_map ["IrrCsv_Guaranteed"               ] =  "100000.00%";
    mask_map ["IrrDb_Current"                   ] =  "100000.00%";
    mask_map ["IrrDb_Guaranteed"                ] =  "100000.00%";
    mask_map ["KFactor_Current"                 ] =    "9,999.99";
    mask_map ["LoanIntAccrued_Current"          ] = "999,999,999";
    mask_map ["LoanIntAccrued_Guaranteed"       ] = "999,999,999";
    mask_map ["MlyGAIntRate_Current"            ] =      "99.99%";
    mask_map ["MlyGAIntRate_Guaranteed"         ] =      "99.99%";
    mask_map ["MlyHoneymoonValueRate_Current"   ] =      "99.99%";
    mask_map ["MlyHoneymoonValueRate_Guaranteed"] =      "99.99%";
    mask_map ["MlyPostHoneymoonRate_Current"    ] =      "99.99%";
    mask_map ["MlyPostHoneymoonRate_Guaranteed" ] =      "99.99%";
    mask_map ["MlySAIntRate_Current"            ] =      "99.99%";
    mask_map ["MlySAIntRate_Guaranteed"         ] =      "99.99%";
    mask_map ["ModalMinimumPremium"             ] = "999,999,999";
    mask_map ["AnnualFlatExtra"                 ] = "999,999,999";
//    mask_map ["NaarForceout"                    ] = "999,999,999";
    mask_map ["NetCOICharge_Current"            ] = "999,999,999";
    mask_map ["NetClaims_Current"               ] = "999,999,999";
    mask_map ["NetClaims_Guaranteed"            ] = "999,999,999";
    mask_map ["NetIntCredited_Current"          ] = "999,999,999";
    mask_map ["NetIntCredited_Guaranteed"       ] = "999,999,999";
    mask_map ["NetPmt_Current"                  ] = "999,999,999";
    mask_map ["NetPmt_Guaranteed"               ] = "999,999,999";
    mask_map ["NetWD"                           ] = "999,999,999";
    mask_map ["NewCashLoan"                     ] = "999,999,999";
    mask_map ["Outlay"                          ] = "999,999,999";
    mask_map ["PartMortTableMult"               ] =   "9,999.99%";
    mask_map ["PolicyFee_Current"               ] = "999,999,999";
    mask_map ["PolicyFee_Guaranteed"            ] = "999,999,999";
    mask_map ["PolicyYear"                      ] =         "999";
    mask_map ["PrefLoanBalance_Current"         ] = "999,999,999";
    mask_map ["PrefLoanBalance_Guaranteed"      ] = "999,999,999";
    mask_map ["PremTaxLoad_Current"             ] = "999,999,999";
    mask_map ["PremTaxLoad_Guaranteed"          ] = "999,999,999";
// Excluded because it's unimplemented:
//    mask_map ["ProducerCompensation"            ] = "999,999,999";
    mask_map ["ProjectedCoiCharge_Current"      ] = "999,999,999";
    mask_map ["RefundableSalesLoad"             ] = "999,999,999";
    mask_map ["RiderCharges_Current"            ] = "999,999,999";
    mask_map ["Salary"                          ] = "999,999,999";
    mask_map ["SepAcctCharges_Current"          ] = "999,999,999";
    mask_map ["SepAcctCharges_Guaranteed"       ] = "999,999,999";
    mask_map ["SpecAmt"                         ] = "999,999,999";
    mask_map ["SpecAmtLoad_Current"             ] = "999,999,999";
    mask_map ["SpecAmtLoad_Guaranteed"          ] = "999,999,999";
    mask_map ["SurrChg_Current"                 ] = "999,999,999";
    mask_map ["SurrChg_Guaranteed"              ] = "999,999,999";
    mask_map ["TermPurchased_Current"           ] = "999,999,999";
    mask_map ["TermPurchased_Guaranteed"        ] = "999,999,999";
    mask_map ["TermSpecAmt"                     ] = "999,999,999";
    mask_map ["TgtPrem"                         ] = "999,999,999";
    mask_map ["TotalIMF"                        ] =      "99.99%";
    mask_map ["TotalLoanBalance_Current"        ] = "999,999,999";
    mask_map ["TotalLoanBalance_Guaranteed"     ] = "999,999,999";

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
    format_map["GenAcctAllocation"                 ] = f3;

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
    format_map["SalesLoadRefundAvailable"          ] = f1;
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
    format_map["AnnLoanDueRate"                    ] = f4;
    format_map["AnnPostHoneymoonRate"              ] = f4;
    format_map["AnnSAIntRate"                      ] = f4;
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

    ledger_invariant_->CalculateIrrs(*this);

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
    int issue_age = static_cast<int>(ledger_invariant_->Age);

    std::vector<double> PolicyYear (max_duration);
    std::vector<double> AttainedAge(max_duration);
    std::iota(PolicyYear .begin(), PolicyYear .end(), 1);
    std::iota(AttainedAge.begin(), AttainedAge.end(), 1 + issue_age);
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

    std::vector<double> PremiumLoad(max_duration);
    std::vector<double> MiscCharges(max_duration);
    for(int j = 0; j < max_duration; ++j)
        {
        PremiumLoad[j] = Invar.GrossPmt[j] - Curr_.NetPmt[j];
        MiscCharges[j] = Curr_.SpecAmtLoad[j] + Curr_.PolicyFee[j];
        }

    vectors   ["PremiumLoad"] = &PremiumLoad;
    title_map ["PremiumLoad"] = "Premium\nLoad";
    mask_map  ["PremiumLoad"] = "999,999,999";
    format_map["PremiumLoad"] = f1;

    vectors   ["MiscCharges"] = &MiscCharges;
    title_map ["MiscCharges"] = "Miscellaneous\nCharges";
    mask_map  ["MiscCharges"] = "999,999,999";
    format_map["MiscCharges"] = f1;

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
    title_map ["NetDeathBenefit"] = "Net\nDeath\nBenefit";
    mask_map  ["NetDeathBenefit"] = "999,999,999";
    format_map["NetDeathBenefit"] = f1;

    std::vector<double> SupplDeathBft_Current   (Curr_.TermPurchased);
    std::vector<double> SupplDeathBft_Guaranteed(Guar_.TermPurchased);
    vectors   ["SupplDeathBft_Current"   ] = &SupplDeathBft_Current;
    vectors   ["SupplDeathBft_Guaranteed"] = &SupplDeathBft_Guaranteed;
    title_map ["SupplDeathBft_Current"   ] = "Curr Suppl\nDeath\nBenefit";
    title_map ["SupplDeathBft_Guaranteed"] = "Guar Suppl\nDeath\nBenefit";
    mask_map  ["SupplDeathBft_Current"   ] = "999,999,999";
    mask_map  ["SupplDeathBft_Guaranteed"] = "999,999,999";
    format_map["SupplDeathBft_Current"   ] = f1;
    format_map["SupplDeathBft_Guaranteed"] = f1;

    std::vector<double> SupplSpecAmt(Invar.TermSpecAmt);
    vectors   ["SupplSpecAmt"            ] = &SupplSpecAmt;
    title_map ["SupplSpecAmt"            ] = "Suppl\nSpecified\nAmount";
    mask_map  ["SupplSpecAmt"            ] = "999,999,999";
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

    // PDF !! Sales-load refunds are mentioned on 'mce_ill_reg' PDFs
    // only. Other formats defectively ignore them.
    double SalesLoadRefundAvailable =
        !each_equal(ledger_invariant_->RefundableSalesLoad, 0.0);
    double SalesLoadRefundRate0 = ledger_invariant_->RefundableSalesLoad[0];
    double SalesLoadRefundRate1 = ledger_invariant_->RefundableSalesLoad[1];
    // At present, only the first two durations are used; that's
    // correct only if all others are zero.
    LMI_ASSERT
        (
        each_equal
            (2 + ledger_invariant_->RefundableSalesLoad.begin()
            ,    ledger_invariant_->RefundableSalesLoad.end()
            ,0.0
            )
        );

    scalars["SalesLoadRefundAvailable"] = &SalesLoadRefundAvailable;
    scalars["SalesLoadRefundRate0"    ] = &SalesLoadRefundRate0;
    scalars["SalesLoadRefundRate1"    ] = &SalesLoadRefundRate1;

    double SepAcctAllocation = 1.0 - ledger_invariant_->GenAcctAllocation;
    scalars   ["SepAcctAllocation"] = &SepAcctAllocation;
    format_map["SepAcctAllocation"] = f3;

    std::string ScaleUnit = ledger_invariant_->ScaleUnit();
    strings["ScaleUnit"] = &ScaleUnit;

    double InitTotalSA =
            ledger_invariant_->InitBaseSpecAmt
        +   ledger_invariant_->InitTermSpecAmt
        ;
    scalars["InitTotalSA"] = &InitTotalSA;

    // Maps to hold the results of formatting numeric data.

    std::unordered_map<std::string,std::string> stringscalars;
    std::unordered_map<std::string,std::vector<std::string>> stringvectors;

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

        std::vector<std::string> SupplementalReportColumnsMasks;
        SupplementalReportColumnsMasks.reserve(SupplementalReportColumns.size());

        for(auto const& j : SupplementalReportColumns)
            {
            SupplementalReportColumnsTitles.push_back(title_map[j]);
            SupplementalReportColumnsMasks .push_back(mask_map [j]);
            }

        stringvectors["SupplementalReportColumnsNames"] = std::move(SupplementalReportColumns);
        stringvectors["SupplementalReportColumnsTitles"] = std::move(SupplementalReportColumnsTitles);
        stringvectors["SupplementalReportColumnsMasks" ] = std::move(SupplementalReportColumnsMasks );
        }

    return ledger_evaluator(std::move(stringscalars), std::move(stringvectors));
}

std::string ledger_evaluator::operator()(std::string const& scalar) const
{
    return map_lookup(scalars_, scalar);
}

std::string ledger_evaluator::operator()
    (std::string const& vector
    ,int                index
    ) const
{
    return map_lookup(vectors_, vector).at(index);
}

void ledger_evaluator::write_tsv
    (fs::path const& // pdf_out_file
    ,Ledger   const& ledger
    ) const
{
    if(ledger.is_composite() && contains(global_settings::instance().pyx(), "values_tsv"))
        {
        throw_if_interdicted(ledger);

        configurable_settings const& z = configurable_settings::instance();
        fs::path filepath
            (   z.print_directory()
            +   "/values"
            +   z.spreadsheet_file_extension()
            );
        fs::ofstream ofs(filepath, ios_out_trunc_binary());

        // Copy 'vectors_' to a (sorted) std::map in order to
        // show columns alphabetically. Other, more complicated
        // techniques are faster, but direct copying favors simplicity
        // over speed--appropriately, as this facility is rarely used.
        using map_t = std::map<std::string,std::vector<std::string>> const;
        map_t sorted_stringvectors(vectors_.begin(), vectors_.end());

        for(auto const& j : sorted_stringvectors)
            {
            ofs << j.first << '\t';
            }
        ofs << '\n';

        for(int i = 0; i < ledger.GetMaxLength(); ++i)
            {
            for(auto const& j : sorted_stringvectors)
                {
                std::vector<std::string> const& v = j.second;
                if(i < lmi::ssize(v))
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
            alarum() << "Unable to write '" << filepath << "'." << LMI_FLUSH;
            }
        }
}
