// Ledger evaluator returning values of all ledger fields.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "ledger.hpp"
#include "ledger_evaluator.hpp"

#include "alert.hpp"
#include "authenticity.hpp"
#include "bourn_cast.hpp"
#include "calendar_date.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "et_vector.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "ledger_invariant.hpp"
#include "ledger_text_formats.hpp"      // ledger_format()
#include "ledger_variant.hpp"
#include "map_lookup.hpp"
#include "mc_enum_aux.hpp"              // mc_e_vector_to_string_vector()
#include "miscellany.hpp"               // each_equal(), ios_out_trunc_binary()
#include "oecumenic_enumerations.hpp"
#include "path.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "ssize_lmi.hpp"
#include "value_cast.hpp"
#include "version.hpp"

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

// PDF !! Three maps, with similar but not necessarily identical
// sets of keys, may not be the best imaginable data structure.
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
        ,"LastCoiReentryDateJdn" // probably not needed
        ,"ListBillDateJdn"       // probably not needed
        ,"InforceAsOfDateJdn"    // probably not needed
        ,"InitDacTaxRate"        // used by PrintRosterTabDelimited(); not cents
        ,"InitPremTaxRate"       // used by PrintRosterTabDelimited(); not cents
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

title_map_t static_titles()
{
//  Here are the columns to be listed in the user interface
//  as well as their corresponding titles.

    // Current and guaranteed variants are generally given for columns
    // that vary by basis. Some offer only a current variant because
    // they are defined only on a current basis--experience-rating
    // columns, e.g.

    static title_map_t const title_map =
    {{"AVGenAcct_CurrentZero"           , "Curr\nCharges\nAccount\nValue\nGen Acct"}
    ,{"AVGenAcct_GuaranteedZero"        , "Guar\nCharges\nAccount\nValue\nGen Acct"}
    ,{"AVRelOnDeath_Current"            , "Account\nValue\nReleased\non Death"}
    ,{"AVSepAcct_CurrentZero"           , "Curr\nCharges\n0% Account\nValue\nSep Acct"}
    ,{"AVSepAcct_GuaranteedZero"        , "Guar\nCharges\n0% Account\nValue\nSep Acct"}
    ,{"AcctVal_Current"                 , "Curr\nAccount\nValue"}
    ,{"AcctVal_CurrentZero"             , "Curr\nCharges\n0% Account\nValue"}
    ,{"AcctVal_Guaranteed"              , "Guar\nAccount\nValue"}
    ,{"AcctVal_GuaranteedZero"          , "Guar\nCharges\n0% Account\nValue"}
    ,{"AddonCompOnAssets"               , "Additional\nComp on\nAssets"}
    ,{"AddonCompOnPremium"              , "Additional\nComp on\nPremium"}
    ,{"AddonMonthlyFee"                 , "Additional\nMonthly\nFee"}
    ,{"AnnGAIntRate_Current"            , "Curr Ann\nGen Acct\nInt Rate"}
    ,{"AnnGAIntRate_Guaranteed"         , "Guar Ann\nGen Acct\nInt Rate"}
    ,{"AnnHoneymoonValueRate_Current"   , "Curr Ann\nHoneymoon\nValue Rate"}
    ,{"AnnHoneymoonValueRate_Guaranteed", "Guar Ann\nHoneymoon\nValue Rate"}
    ,{"AnnLoanDueRate"                  , "Loan\nInt Rate"}
    ,{"AnnPostHoneymoonRate_Current"    , "Curr Post\nHoneymoon\nRate"}
    ,{"AnnPostHoneymoonRate_Guaranteed" , "Guar Post\nHoneymoon\nRate"}
    ,{"AnnSAIntRate_Current"            , "Curr Ann\nSep Acct\nInt Rate"}
    ,{"AnnSAIntRate_Guaranteed"         , "Guar Ann\nSep Acct\nInt Rate"}
    ,{"AnnualFlatExtra"                 , "Annual\nFlat\nExtra"}
    ,{"AttainedAge"                     , "End of\nYear\nAge"}
    ,{"AvgDeathBft_Current"             , "Curr Avg\nDeath\nBenefit"}
    ,{"AvgDeathBft_Guaranteed"          , "Guar Avg\nDeath\nBenefit"}
    ,{"BaseDeathBft_Current"            , "Curr Base\nDeath\nBenefit"}
    ,{"BaseDeathBft_Guaranteed"         , "Guar Base\nDeath\nBenefit"}
    ,{"COICharge_Current"               , "Curr COI\nCharge"}
    ,{"COICharge_Guaranteed"            , "Guar COI\nCharge"}
    ,{"CSVNet_Current"                  , "Curr Net\nCash\nSurr Value"}
    ,{"CSVNet_CurrentZero"              , "Curr\nCharges\n0% Net Cash\nSurr Value"}
    ,{"CSVNet_Guaranteed"               , "Guar Net\nCash\nSurr Value"}
    ,{"CSVNet_GuaranteedZero"           , "Guar\nCharges\n0% Net Cash\nSurr Value"}
    ,{"CV7702_Current"                  , "Curr 7702\nCash Value"}
    ,{"CV7702_Guaranteed"               , "Guar 7702\nCash Value"}
    ,{"ClaimsPaid_Current"              , "Curr\nClaims\nPaid"}
    ,{"ClaimsPaid_Guaranteed"           , "Guar\nClaims\nPaid"}
    ,{"CorpTaxBracket"                  , "Corp Tax\nBracket"}
    ,{"CorridorFactor"                  , "Corridor\nFactor"}
    ,{"CurrMandE"                       , "Mortality\nand\nExpense\nCharge"}
    ,{"DBOpt"                           , "Death\nBenefit\nOption"}
    ,{"DacTaxLoad_Current"              , "Curr DAC\nTax\nLoad"}
    ,{"DacTaxLoad_Guaranteed"           , "Guar DAC\nTax\nLoad"}
    ,{"DacTaxRsv_Current"               , "Curr DAC\nTax\nReserve"}
    ,{"DacTaxRsv_Guaranteed"            , "Guar DAC\nTax\nReserve"}
    ,{"Dcv"                             , "Deemed\nCash\nValue"}
    ,{"DeathProceedsPaid_Current"       , "Curr Death\nProceeds\nPaid"}
    ,{"DeathProceedsPaid_Guaranteed"    , "Guar Death\nProceeds\nPaid"}
    ,{"Duration"                        , "Duration"}
    ,{"EOYDeathBft_Current"             , "Curr EOY\nDeath\nBenefit"}
    ,{"EOYDeathBft_Guaranteed"          , "Guar EOY\nDeath\nBenefit"}
    ,{"EeGrossPmt"                      , "EE Gross\nPayment"}
    ,{"EeModalMinimumPremium"           , "EE Modal\nMinimum\nPremium"}
    ,{"EeMode"                          , "EE\nPayment\nMode"}
    ,{"ErGrossPmt"                      , "ER Gross\nPayment"}
    ,{"ErModalMinimumPremium"           , "ER Modal\nMinimum\nPremium"}
    ,{"ErMode"                          , "ER\nPayment\nMode"}
    ,{"ExpenseCharges_Current"          , "Curr\nExpense\nCharge"}
    ,{"ExpenseCharges_Guaranteed"       , "Guar\nExpense\nCharge"}
    ,{"GptForceout"                     , "Forceout"}
    ,{"GrossIntCredited_Current"        , "Curr Gross\nInt\nCredited"}
    ,{"GrossIntCredited_Guaranteed"     , "Guar Gross\nInt\nCredited"}
    ,{"GrossPmt"                        , "Premium\nOutlay"}
    ,{"HoneymoonValueSpread"            , "Honeymoon\nValue\nSpread"}
    ,{"IndvTaxBracket"                  , "EE Tax\nBracket"}
    ,{"InforceLives"                    , "BOY\nLives\nInforce"}
    ,{"Irc7702ic_usual"                 , "7702 ic\nUsual"}
    ,{"Irc7702ic_glp"                   , "7702 ic\nfor GLP"}
    ,{"Irc7702ic_gsp"                   , "7702 ic\nfor GSP"}
    ,{"Irc7702ig_usual"                 , "7702 ig\nUsual"}
    ,{"Irc7702ig_glp"                   , "7702 ig\nfor GLP"}
    ,{"Irc7702ig_gsp"                   , "7702 ig\nfor GSP"}
    ,{"IrrCsv_Current"                  , "Curr IRR\non CSV"}
    ,{"IrrCsv_Guaranteed"               , "Guar IRR\non CSV"}
    ,{"IrrDb_Current"                   , "Curr IRR\non DB"}
    ,{"IrrDb_Guaranteed"                , "Guar IRR\non DB"}
    ,{"LoanIntAccrued_Current"          , "Curr Loan\nInt\nAccrued"}
    ,{"LoanIntAccrued_Guaranteed"       , "Guar Loan\nInt\nAccrued"}
    ,{"MlyGAIntRate_Current"            , "Curr Monthly\nGen Acct\nInt Rate"}
    ,{"MlyGAIntRate_Guaranteed"         , "Guar Monthly\nGen Acct\nInt Rate"}
    ,{"MlyHoneymoonValueRate_Current"   , "Curr Monthly\nHoneymoon\nValue Rate"}
    ,{"MlyHoneymoonValueRate_Guaranteed", "Guar Monthly\nHoneymoon\nValue Rate"}
    ,{"MlyPostHoneymoonRate_Current"    , "Curr Monthly\nPost\nHoneymoon\nRate"}
    ,{"MlyPostHoneymoonRate_Guaranteed" , "Guar Monthly\nPost\nHoneymoon\nRate"}
    ,{"MlySAIntRate_Current"            , "Curr Monthly\nSep Acct\nInt Rate"}
    ,{"MlySAIntRate_Guaranteed"         , "Guar Monthly\nSep Acct\nInt Rate"}
    ,{"ModalMinimumPremium"             , "Modal\nMinimum\nPremium"}
//  ,{"NaarForceout"                    , "Forced\nWithdrawal\ndue to\nNAAR Limit"}
    ,{"NetClaims_Current"               , "Curr Net\nClaims"}
    ,{"NetClaims_Guaranteed"            , "Guar Net\nClaims"}
    ,{"NetIntCredited_Current"          , "Curr Net\nInt\nCredited"}
    ,{"NetIntCredited_Guaranteed"       , "Guar Net\nInt\nCredited"}
    ,{"NetPmt_Current"                  , "Curr Net\nPayment"}
    ,{"NetPmt_Guaranteed"               , "Guar Net\nPayment"}
    ,{"NetWD"                           , "Withdrawal"}
    ,{"NewCashLoan"                     , "Annual\nLoan"}
    ,{"Outlay"                          , "Net Outlay"}
    ,{"PartMortTableMult"               , "Partial\nMortality\nMuliplier"}
    ,{"PolicyFee_Current"               , "Curr\nPolicy\nFee"}
    ,{"PolicyFee_Guaranteed"            , "Guar\nPolicy\nFee"}
    ,{"PolicyYear"                      , "Policy\nYear"}
    ,{"PrefLoanBalance_Current"         , "Curr\nPreferred\nLoan Bal"}
    ,{"PrefLoanBalance_Guaranteed"      , "Guar\nPreferred\nLoan Bal"}
    ,{"PremTaxLoad_Current"             , "Curr\nPremium\nTax Load"}
    ,{"PremTaxLoad_Guaranteed"          , "Guar\nPremium\nTax Load"}
    ,{"RefundableSalesLoad"             , "Refundable\nSales\nLoad"}
    ,{"RiderCharges_Current"            , "Curr Rider\nCharges"}
    ,{"Salary"                          , "Salary"}
    ,{"SepAcctCharges_Current"          , "Curr Sep\nAcct\nCharges"}
    ,{"SepAcctCharges_Guaranteed"       , "Guar Sep\nAcct\nCharges"}
    ,{"SpecAmt"                         , "Specified\nAmount"}
    ,{"SpecAmtLoad_Current"             , "Curr Spec\nAmt Load"}
    ,{"SpecAmtLoad_Guaranteed"          , "Guar Spec\nAmt Load"}
    ,{"SurrChg_Current"                 , "Curr Surr\nCharge"}
    ,{"SurrChg_Guaranteed"              , "Guar Surr\nCharge"}
    ,{"TermPurchased_Current"           , "Curr Term\nAmt\nPurchased"}
    ,{"TermPurchased_Guaranteed"        , "Guar Term\nAmt\nPurchased"}
    ,{"TermSpecAmt"                     , "Term\nSpecified\nAmount"}
    ,{"TotalIMF"                        , "Total\nInvestment\nMgt Fee"}
    ,{"TotalLoanBalance_Current"        , "Curr Total\nLoan\nBalance"}
    ,{"TotalLoanBalance_Guaranteed"     , "Guar Total\nLoan\nBalance"}
    };

    // TODO ?? Titles ought to be read from an external file that
    // permits flexible customization. Compliance might require that
    // 'AcctVal_Current' be called "Cash Value" for one policy form,
    // and "Account Value" for another, in order to match the terms
    // used in the contract exactly. Therefore, these titles probably
    // belong in the product database, which permits variation by
    // product--though it does not accommodate strings as this is
    // written in 2006-07. DATABASE !! So consider adding them there
    // when the database is revamped.

    return title_map;
}

mask_map_t static_masks()
{
    static mask_map_t const mask_map =
    {{"AVGenAcct_CurrentZero"           , "999,999,999"}
    ,{"AVGenAcct_GuaranteedZero"        , "999,999,999"}
    ,{"AVRelOnDeath_Current"            , "999,999,999"}
    ,{"AVSepAcct_CurrentZero"           , "999,999,999"}
    ,{"AVSepAcct_GuaranteedZero"        , "999,999,999"}
    ,{"AcctVal_Current"                 , "999,999,999"}
    ,{"AcctVal_CurrentZero"             , "999,999,999"}
    ,{"AcctVal_Guaranteed"              , "999,999,999"}
    ,{"AcctVal_GuaranteedZero"          , "999,999,999"}
    ,{"AddonCompOnAssets"               , "999,999,999"}
    ,{"AddonCompOnPremium"              , "999,999,999"}
    ,{"AddonMonthlyFee"                 , "999,999,999"}
    ,{"AnnGAIntRate_Current"            ,      "99.99%"}
    ,{"AnnGAIntRate_Guaranteed"         ,      "99.99%"}
    ,{"AnnHoneymoonValueRate_Current"   ,      "99.99%"}
    ,{"AnnHoneymoonValueRate_Guaranteed",      "99.99%"}
    ,{"AnnLoanDueRate"                  ,      "99.99%"}
    ,{"AnnPostHoneymoonRate_Current"    ,      "99.99%"}
    ,{"AnnPostHoneymoonRate_Guaranteed" ,      "99.99%"}
    ,{"AnnSAIntRate_Current"            ,      "99.99%"}
    ,{"AnnSAIntRate_Guaranteed"         ,      "99.99%"}
    ,{"AnnualFlatExtra"                 , "999,999,999"}
    ,{"AttainedAge"                     ,         "999"}
    ,{"AvgDeathBft_Current"             , "999,999,999"}
    ,{"AvgDeathBft_Guaranteed"          , "999,999,999"}
    ,{"BaseDeathBft_Current"            , "999,999,999"}
    ,{"BaseDeathBft_Guaranteed"         , "999,999,999"}
    ,{"COICharge_Current"               , "999,999,999"}
    ,{"COICharge_Guaranteed"            , "999,999,999"}
    ,{"CSVNet_Current"                  , "999,999,999"}
    ,{"CSVNet_CurrentZero"              , "999,999,999"}
    ,{"CSVNet_Guaranteed"               , "999,999,999"}
    ,{"CSVNet_GuaranteedZero"           , "999,999,999"}
    ,{"CV7702_Current"                  , "999,999,999"}
    ,{"CV7702_Guaranteed"               , "999,999,999"}
    ,{"ClaimsPaid_Current"              , "999,999,999"}
    ,{"ClaimsPaid_Guaranteed"           , "999,999,999"}
    ,{"CorpTaxBracket"                  ,      "99.99%"}
    ,{"CorridorFactor"                  ,       "9999%"}
    ,{"CurrMandE"                       ,      "99.99%"}
    ,{"DBOpt"                           ,         "ROP"}
    ,{"DacTaxLoad_Current"              , "999,999,999"}
    ,{"DacTaxLoad_Guaranteed"           , "999,999,999"}
    ,{"DacTaxRsv_Current"               , "999,999,999"}
    ,{"DacTaxRsv_Guaranteed"            , "999,999,999"}
    ,{"Dcv"                             , "999,999,999"}
    ,{"DeathProceedsPaid_Current"       , "999,999,999"}
    ,{"DeathProceedsPaid_Guaranteed"    , "999,999,999"}
    ,{"Duration"                        ,         "999"}
    ,{"EOYDeathBft_Current"             , "999,999,999"}
    ,{"EOYDeathBft_Guaranteed"          , "999,999,999"}
    ,{"EeGrossPmt"                      , "999,999,999"}
    ,{"EeModalMinimumPremium"           , "999,999,999"}
    ,{"EeMode"                          ,  "Semiannual"}
    ,{"ErGrossPmt"                      , "999,999,999"}
    ,{"ErModalMinimumPremium"           , "999,999,999"}
    ,{"ErMode"                          ,  "Semiannual"}
    ,{"ExpenseCharges_Current"          , "999,999,999"}
    ,{"ExpenseCharges_Guaranteed"       , "999,999,999"}
    ,{"GptForceout"                     , "999,999,999"}
    ,{"GrossIntCredited_Current"        , "999,999,999"}
    ,{"GrossIntCredited_Guaranteed"     , "999,999,999"}
    ,{"GrossPmt"                        , "999,999,999"}
    ,{"HoneymoonValueSpread"            ,      "99.99%"}
    ,{"IndvTaxBracket"                  ,      "99.99%"}
    ,{"InforceLives"                    , "999,999,999"}
    ,{"Irc7702ic_usual"                 ,      "99.99%"}
    ,{"Irc7702ic_glp"                   ,      "99.99%"}
    ,{"Irc7702ic_gsp"                   ,      "99.99%"}
    ,{"Irc7702ig_usual"                 ,      "99.99%"}
    ,{"Irc7702ig_glp"                   ,      "99.99%"}
    ,{"Irc7702ig_gsp"                   ,      "99.99%"}
    ,{"IrrCsv_Current"                  ,  "100000.00%"}
    ,{"IrrCsv_Guaranteed"               ,  "100000.00%"}
    ,{"IrrDb_Current"                   ,  "100000.00%"}
    ,{"IrrDb_Guaranteed"                ,  "100000.00%"}
    ,{"LoanIntAccrued_Current"          , "999,999,999"}
    ,{"LoanIntAccrued_Guaranteed"       , "999,999,999"}
    ,{"MlyGAIntRate_Current"            ,      "99.99%"}
    ,{"MlyGAIntRate_Guaranteed"         ,      "99.99%"}
    ,{"MlyHoneymoonValueRate_Current"   ,      "99.99%"}
    ,{"MlyHoneymoonValueRate_Guaranteed",      "99.99%"}
    ,{"MlyPostHoneymoonRate_Current"    ,      "99.99%"}
    ,{"MlyPostHoneymoonRate_Guaranteed" ,      "99.99%"}
    ,{"MlySAIntRate_Current"            ,      "99.99%"}
    ,{"MlySAIntRate_Guaranteed"         ,      "99.99%"}
    ,{"ModalMinimumPremium"             , "999,999,999"}
//  ,{"NaarForceout"                    , "999,999,999"}
    ,{"NetClaims_Current"               , "999,999,999"}
    ,{"NetClaims_Guaranteed"            , "999,999,999"}
    ,{"NetIntCredited_Current"          , "999,999,999"}
    ,{"NetIntCredited_Guaranteed"       , "999,999,999"}
    ,{"NetPmt_Current"                  , "999,999,999"}
    ,{"NetPmt_Guaranteed"               , "999,999,999"}
    ,{"NetWD"                           , "999,999,999"}
    ,{"NewCashLoan"                     , "999,999,999"}
    ,{"Outlay"                          , "999,999,999"}
    ,{"PartMortTableMult"               ,   "9,999.99%"}
    ,{"PolicyFee_Current"               , "999,999,999"}
    ,{"PolicyFee_Guaranteed"            , "999,999,999"}
    ,{"PolicyYear"                      ,         "999"}
    ,{"PrefLoanBalance_Current"         , "999,999,999"}
    ,{"PrefLoanBalance_Guaranteed"      , "999,999,999"}
    ,{"PremTaxLoad_Current"             , "999,999,999"}
    ,{"PremTaxLoad_Guaranteed"          , "999,999,999"}
    ,{"RefundableSalesLoad"             , "999,999,999"}
    ,{"RiderCharges_Current"            , "999,999,999"}
    ,{"Salary"                          , "999,999,999"}
    ,{"SepAcctCharges_Current"          , "999,999,999"}
    ,{"SepAcctCharges_Guaranteed"       , "999,999,999"}
    ,{"SpecAmt"                         , "999,999,999"}
    ,{"SpecAmtLoad_Current"             , "999,999,999"}
    ,{"SpecAmtLoad_Guaranteed"          , "999,999,999"}
    ,{"SurrChg_Current"                 , "999,999,999"}
    ,{"SurrChg_Guaranteed"              , "999,999,999"}
    ,{"TermPurchased_Current"           , "999,999,999"}
    ,{"TermPurchased_Guaranteed"        , "999,999,999"}
    ,{"TermSpecAmt"                     , "999,999,999"}
    ,{"TotalIMF"                        ,      "99.99%"}
    ,{"TotalLoanBalance_Current"        , "999,999,999"}
    ,{"TotalLoanBalance_Guaranteed"     , "999,999,999"}
    };

    return mask_map;
}

format_map_t static_formats()
{
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

    static format_map_t const format_map =
// > Special Formatting for Scalar Items
// >
// F4: scaled by 100, two decimals, with '%' at end:
// > Format as percentage "0.00%"
// >
    {{"GuarMaxMandE"                    , f4}
    ,{"InitAnnGenAcctInt"               , f4}
    ,{"InitAnnLoanCredRate"             , f4}
    ,{"InitAnnSepAcctCurrGross0Rate"    , f4}
    ,{"InitAnnSepAcctCurrGrossHalfRate" , f4}
    ,{"InitAnnSepAcctCurrNet0Rate"      , f4}
    ,{"InitAnnSepAcctCurrNetHalfRate"   , f4}
    ,{"InitAnnSepAcctGrossInt"          , f4}
    ,{"InitAnnSepAcctGuarGross0Rate"    , f4}
    ,{"InitAnnSepAcctGuarGrossHalfRate" , f4}
    ,{"InitAnnSepAcctGuarNet0Rate"      , f4}
    ,{"InitAnnSepAcctGuarNetHalfRate"   , f4}
    ,{"InitAnnSepAcctNetInt"            , f4}
    ,{"MaxAnnCurrLoanDueRate"           , f4}
    ,{"MaxAnnGuarLoanSpread"            , f4}
    ,{"PostHoneymoonSpread"             , f4}
    ,{"Preferred"                       , f4}
    ,{"PremTaxRate"                     , f4}

// F3: scaled by 100, zero decimals, with '%' at end:
// > Format as percentage with no decimal places (##0%)
    ,{"GenAcctAllocation"               , f3}
    ,{"SalesLoadRefundRate0"            , f3}
    ,{"SalesLoadRefundRate1"            , f3}

// >
// F2: two decimals, commas
// > Format as a number with thousand separators and two decimal places (#,###,###.00)
// >
    ,{"CurrentCoiMultiplier"            , f2}
    ,{"EeListBillPremium"               , f2}
    ,{"ErListBillPremium"               , f2}
    ,{"GuarPrem"                        , f2}
    ,{"InforceTaxBasis"                 , f2}
    ,{"InforceTotalAV"                  , f2}
    ,{"InitGLP"                         , f2}
    ,{"InitGSP"                         , f2}
    ,{"InitPrem"                        , f2}
    ,{"InitSevenPayPrem"                , f2}
    ,{"InitTgtPrem"                     , f2}
    ,{"ListBillPremium"                 , f2}
// >
// F1: zero decimals, commas
// > Format as a number with thousand separators and no decimal places (#,###,###)
// >
    ,{"Age"                             , f1}
    ,{"AllowGroupQuote"                 , f1}
    ,{"AvgFund"                         , f1}
    ,{"ChildRiderAmount"                , f1}
    ,{"CustomFund"                      , f1}
    ,{"Dumpin"                          , f1}
    ,{"EndtAge"                         , f1}
    ,{"ErNotionallyPaysTerm"            , f1}
    ,{"External1035Amount"              , f1}
    ,{"GenderBlended"                   , f1}
    ,{"GreatestLapseDuration"           , f1}
    ,{"GroupIndivSelection"             , f1}
    ,{"Has1035ExchCharge"               , f1}
    ,{"HasADD"                          , f1}
    ,{"HasChildRider"                   , f1}
    ,{"HasHoneymoon"                    , f1}
    ,{"HasSpouseRider"                  , f1}
    ,{"HasSupplSpecAmt"                 , f1}
    ,{"HasTerm"                         , f1}
    ,{"HasWP"                           , f1}
    ,{"InforceIsMec"                    , f1}
    ,{"InforceMonth"                    , f1}
    ,{"InforceYear"                     , f1}
    ,{"InitBaseSpecAmt"                 , f1}
    ,{"InitTermSpecAmt"                 , f1}
    ,{"InitTotalSA"                     , f1}
    ,{"Internal1035Amount"              , f1}
    ,{"IsInforce"                       , f1}
    ,{"IsMec"                           , f1}
    ,{"IsSinglePremium"                 , f1}
    ,{"LapseMonth"                      , f1}
    ,{"LapseYear"                       , f1}
    ,{"MecMonth"                        , f1}
    ,{"MecYear"                         , f1}
    ,{"NoLapse"                         , f1}
    ,{"NoLapseAlwaysActive"             , f1}
    ,{"NoLapseMinAge"                   , f1}
    ,{"NoLapseMinDur"                   , f1}
    ,{"NoLongerIssued"                  , f1}
    ,{"RetAge"                          , f1}
    ,{"SalesLoadRefundAvailable"        , f1}
    ,{"SmokerBlended"                   , f1}
    ,{"SplitFundAllocation"             , f1}
    ,{"SplitMinPrem"                    , f1}
    ,{"SpouseIssueAge"                  , f1}
    ,{"SupplementalReport"              , f1}
    ,{"SurvivalMaxAge"                  , f1}
    ,{"SurvivalMaxYear"                 , f1}
    ,{"SurviveToAge"                    , f1}
    ,{"SurviveToExpectancy"             , f1}
    ,{"SurviveToYear"                   , f1}
    ,{"TxCallsGuarUwSubstd"             , f1}
    ,{"UsePartialMort"                  , f1}
    ,{"WriteTsvFile"                    , f1}

// > Vector Formatting
// >
// > Here are the vectors enumerated
// >
// F3: scaled by 100, zero decimals, with '%' at end:
// > Format as percentage with no decimal places (##0%)
// >
    ,{"CorridorFactor"                  , f3}
    ,{"FundAllocations"                 , f3}
    ,{"MaleProportion"                  , f3}
    ,{"NonsmokerProportion"             , f3}
    ,{"PartMortTableMult"               , f3}

// >
// F4: scaled by 100, two decimals, with '%' at end:
// > Format as percentage with two decimal places (##0.00%)
// >
    ,{"AnnGAIntRate"                    , f4}
    ,{"AnnHoneymoonValueRate"           , f4}
    ,{"AnnLoanDueRate"                  , f4}
    ,{"AnnPostHoneymoonRate"            , f4}
    ,{"AnnSAIntRate"                    , f4}
    ,{"CorpTaxBracket"                  , f4}
    ,{"CurrMandE"                       , f4}
    ,{"HoneymoonValueSpread"            , f4}
    ,{"IndvTaxBracket"                  , f4}
    ,{"InforceHMVector"                 , f4}

    ,{"Irc7702ic_usual"                 , f4}
    ,{"Irc7702ic_glp"                   , f4}
    ,{"Irc7702ic_gsp"                   , f4}
    ,{"Irc7702ig_usual"                 , f4}
    ,{"Irc7702ig_glp"                   , f4}
    ,{"Irc7702ig_gsp"                   , f4}

    ,{"IrrCsv_Current"                  , f4}
    ,{"IrrCsv_CurrentZero"              , f4}
    ,{"IrrCsv_Guaranteed"               , f4}
    ,{"IrrCsv_GuaranteedZero"           , f4}
    ,{"IrrDb_Current"                   , f4}
    ,{"IrrDb_CurrentZero"               , f4}
    ,{"IrrDb_Guaranteed"                , f4}
    ,{"IrrDb_GuaranteedZero"            , f4}

    ,{"MlyGAIntRate"                    , f4}
    ,{"MlyHoneymoonValueRate"           , f4}
    ,{"MlyPostHoneymoonRate"            , f4}
    ,{"MlySAIntRate"                    , f4}
    ,{"TotalIMF"                        , f4}
// >
// F0: zero decimals
// > Format as a number no thousand separator or decimal point (##0%)
// >
    ,{"AttainedAge"                     , f1}
    ,{"Duration"                        , f1}
    ,{"PolicyYear"                      , f1}
// >
// F2: two decimals, commas
// > Format as a number with thousand separators and two decimal places (#,###,###.00)
// >
    ,{"AddonMonthlyFee"                 , f2}
    ,{"AnnualFlatExtra"                 , f2}
// TODO ?? The precision of 'InforceLives' is inadequate. Is every other format OK?
    ,{"InforceLives"                    , f2}
// >
// F1: zero decimals, commas
// > Format as a number with thousand separators and no decimal places (#,###,##0)
// >
    ,{"AVGenAcct"                       , f1}
    ,{"AVRelOnDeath"                    , f1}
    ,{"AVSepAcct"                       , f1}
    ,{"AcctVal"                         , f1}
    ,{"AccumulatedPremium"              , f1}
    ,{"AddonCompOnAssets"               , f1}
    ,{"AddonCompOnPremium"              , f1}
    ,{"AvgDeathBft"                     , f1}
    ,{"BOYAssets"                       , f1}
    ,{"BaseDeathBft"                    , f1}
    ,{"COICharge"                       , f1}
    ,{"CSVNet"                          , f1}
    ,{"CV7702"                          , f1}
    ,{"ClaimsPaid"                      , f1}
    ,{"Composite"                       , f1}
    ,{"DacTaxLoad"                      , f1}
    ,{"DacTaxRsv"                       , f1}
    ,{"Dcv"                             , f1}
    ,{"DeathProceedsPaid"               , f1}
    ,{"EOYDeathBft"                     , f1}
    ,{"EeGrossPmt"                      , f1}
    ,{"EeModalMinimumPremium"           , f1}
//  ,{"EeMode"                          , f1} // Not numeric.
    ,{"ErGrossPmt"                      , f1}
    ,{"ErModalMinimumPremium"           , f1}
//  ,{"ErMode"                          , f1} // Not numeric.
    ,{"ExpenseCharges"                  , f1}
    ,{"FundNumbers"                     , f1}
    ,{"GptForceout"                     , f1}
    ,{"GrossIntCredited"                , f1}
    ,{"GrossPmt"                        , f1}
    ,{"Loads"                           , f1}
    ,{"LoanInt"                         , f1}
    ,{"LoanIntAccrued"                  , f1}
    ,{"ModalMinimumPremium"             , f1}
    ,{"NaarForceout"                    , f1}
    ,{"NetClaims"                       , f1}
    ,{"NetIntCredited"                  , f1}
    ,{"NetPmt"                          , f1}
    ,{"NetWD"                           , f1}
    ,{"NewCashLoan"                     , f1}
    ,{"Outlay"                          , f1}
    ,{"PolicyFee"                       , f1}
    ,{"PrefLoanBalance"                 , f1}
    ,{"PremTaxLoad"                     , f1}
    ,{"RefundableSalesLoad"             , f1}
    ,{"RiderCharges"                    , f1}
    ,{"Salary"                          , f1}
    ,{"SepAcctCharges"                  , f1}
    ,{"SpecAmt"                         , f1}
    ,{"SpecAmtLoad"                     , f1}
    ,{"SpouseRiderAmount"               , f1}
    ,{"SurrChg"                         , f1}
    ,{"TermPurchased"                   , f1}
    ,{"TermSpecAmt"                     , f1}
    ,{"TotalLoanBalance"                , f1}
    };

    return format_map;
}
} // Unnamed namespace.

ledger_evaluator Ledger::make_evaluator() const
{
    throw_if_interdicted(*this);

    LedgerInvariant const& invar = GetLedgerInvariant();
    LedgerVariant   const& curr  = GetCurrFull();
    LedgerVariant   const& guar  = GetGuarFull();

    title_map_t title_map {static_titles()};

    mask_map_t mask_map {static_masks()};

    std::pair<int,oenum_format_style> f1(0, oe_format_normal);
    std::pair<int,oenum_format_style> f2(2, oe_format_normal);
    std::pair<int,oenum_format_style> f3(0, oe_format_percentage);
    std::pair<int,oenum_format_style> f4(2, oe_format_percentage);

    format_map_t format_map {static_formats()};

    // This is a little tricky. We have some stuff that
    // isn't in the maps inside the ledger classes. We're going to
    // stuff it into a copy of the invariant-ledger class's data.
    // To avoid copying, we'll use pointers to the data. Most of
    // this stuff is invariant anyway, so that's a reasonable
    // place to put it.
    //
    // First we make a copy of the invariant ledger's maps:

    double_vector_map   vectors = invar.AllVectors;
    scalar_map          scalars = invar.AllScalars;
    string_map          strings = invar.Strings;

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

    double GreatestLapseDuration = greatest_lapse_dur();
    scalars["GreatestLapseDuration"] = &GreatestLapseDuration;

    int max_duration = bourn_cast<int>(invar.EndtAge - invar.Age);
    int issue_age = bourn_cast<int>(invar.Age);

    std::vector<double> AttainedAge(max_duration);
    std::vector<double> Duration   (max_duration);
    std::vector<double> PolicyYear (max_duration);
    std::iota(AttainedAge.begin(), AttainedAge.end(), 1 + issue_age);
    std::iota(Duration   .begin(), Duration   .end(), 0);
    std::iota(PolicyYear .begin(), PolicyYear .end(), 1);
// TODO ?? An attained-age column is meaningless in a composite. So
// are several others--notably those affected by partial mortaility.
    vectors["AttainedAge"] = &AttainedAge;
    vectors["Duration"   ] = &Duration   ;
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

    std::vector<double> PremiumLoad(max_duration);
    std::vector<double> MiscCharges(max_duration);
    for(int j = 0; j < max_duration; ++j)
        {
        PremiumLoad[j] = invar.GrossPmt[j] - curr.NetPmt[j];
        MiscCharges[j] = curr.SpecAmtLoad[j] + curr.PolicyFee[j];
        }

    vectors   ["PremiumLoad"] = &PremiumLoad;
    title_map ["PremiumLoad"] = "Premium\nLoad";
    mask_map  ["PremiumLoad"] = "999,999,999";
    format_map["PremiumLoad"] = f1;

    vectors   ["MiscCharges"] = &MiscCharges;
    title_map ["MiscCharges"] = "Miscellaneous\nCharges";
    mask_map  ["MiscCharges"] = "999,999,999";
    format_map["MiscCharges"] = f1;

    std::vector<double> NetDeathBenefit(curr.EOYDeathBft);
    NetDeathBenefit -= curr.TotalLoanBalance;
    vectors   ["NetDeathBenefit"] = &NetDeathBenefit;
    title_map ["NetDeathBenefit"] = "Net\nDeath\nBenefit";
    mask_map  ["NetDeathBenefit"] = "999,999,999";
    format_map["NetDeathBenefit"] = f1;

    std::vector<double> SupplDeathBft_Current   (curr.TermPurchased);
    std::vector<double> SupplDeathBft_Guaranteed(guar.TermPurchased);
    vectors   ["SupplDeathBft_Current"   ] = &SupplDeathBft_Current;
    vectors   ["SupplDeathBft_Guaranteed"] = &SupplDeathBft_Guaranteed;
    title_map ["SupplDeathBft_Current"   ] = "Curr Suppl\nDeath\nBenefit";
    title_map ["SupplDeathBft_Guaranteed"] = "Guar Suppl\nDeath\nBenefit";
    mask_map  ["SupplDeathBft_Current"   ] = "999,999,999";
    mask_map  ["SupplDeathBft_Guaranteed"] = "999,999,999";
    format_map["SupplDeathBft_Current"   ] = f1;
    format_map["SupplDeathBft_Guaranteed"] = f1;

    std::vector<double> SupplSpecAmt(invar.TermSpecAmt);
    vectors   ["SupplSpecAmt"            ] = &SupplSpecAmt;
    title_map ["SupplSpecAmt"            ] = "Suppl\nSpecified\nAmount";
    mask_map  ["SupplSpecAmt"            ] = "999,999,999";
    format_map["SupplSpecAmt"            ] = f1;

    // [End of derived columns.]

    double Composite = is_composite();
    scalars["Composite"] = &Composite;

    double NoLapse =
            0 != invar.NoLapseMinDur
        ||  0 != invar.NoLapseMinAge
        ;
    scalars["NoLapse"] = &NoLapse;

    std::string LmiVersion(LMI_VERSION);
    calendar_date date_prepared;

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
        date_prepared.julian_day_number(bourn_cast<int>(invar.EffDateJdn));
        }

    strings["LmiVersion"] = &LmiVersion;

    std::string DatePrepared =
          month_name(date_prepared.month())
        + " "
        + value_cast<std::string>(date_prepared.day())
        + ", "
        + value_cast<std::string>(date_prepared.year())
        ;
    strings["DatePrepared" ] = &DatePrepared;

    calendar_date inforce_as_of_date;
    inforce_as_of_date.julian_day_number(bourn_cast<int>(invar.InforceAsOfDateJdn));
    std::string InforceAsOfDate =
          month_name(inforce_as_of_date.month())
        + " "
        + value_cast<std::string>(inforce_as_of_date.day())
        + ", "
        + value_cast<std::string>(inforce_as_of_date.year())
        ;
    strings["InforceAsOfDate" ] = &InforceAsOfDate;

    // PDF !! Sales-load refunds are mentioned on 'mce_ill_reg' PDFs
    // only. Other formats defectively ignore them.
    double SalesLoadRefundAvailable =
        !each_equal(invar.RefundableSalesLoad, 0.0);
    double SalesLoadRefundRate0 = invar.RefundableSalesLoad[0];
    double SalesLoadRefundRate1 = invar.RefundableSalesLoad[1];
    // At present, only the first two durations are used; that's
    // correct only if all others are zero.
    LMI_ASSERT
        (
        each_equal
            (2 + invar.RefundableSalesLoad.begin()
            ,    invar.RefundableSalesLoad.end()
            ,0.0
            )
        );

    scalars["SalesLoadRefundAvailable"] = &SalesLoadRefundAvailable;
    scalars["SalesLoadRefundRate0"    ] = &SalesLoadRefundRate0;
    scalars["SalesLoadRefundRate1"    ] = &SalesLoadRefundRate1;

    double SepAcctAllocation = 1.0 - invar.GenAcctAllocation;
    scalars   ["SepAcctAllocation"] = &SepAcctAllocation;
    format_map["SepAcctAllocation"] = f3;

    std::string ScaleUnit = invar.scale_unit();
    strings["ScaleUnit"] = &ScaleUnit;

    double InitTotalSA =
            invar.InitBaseSpecAmt
        +   invar.InitTermSpecAmt
        ;
    scalars["InitTotalSA"] = &InitTotalSA;

    // Maps to hold the results of formatting numeric data.

    std::unordered_map<std::string,std::string> stringscalars;
    std::unordered_map<std::string,std::vector<std::string>> stringvectors;

    stringvectors["FundNames"] = invar.FundNames;

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

    stringvectors["EeMode"] = mc_e_vector_to_string_vector(invar.EeMode);
    stringvectors["ErMode"] = mc_e_vector_to_string_vector(invar.ErMode);
    stringvectors["DBOpt"]  = mc_e_vector_to_string_vector(invar.DBOpt );

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
// Variant
//
// [None of these are stored, and I think none is wanted.]
//
//    // special cases
//    int              Length;
//    mcenum_gen_basis GenBasis_;
//    mcenum_sep_basis SepBasis_;
//    bool             FullyInitialized;   // I.e. by Init(BasicValues const* b)

    if(invar.SupplementalReport)
        {
        std::vector<std::string> SupplementalReportColumns;
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn00);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn01);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn02);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn03);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn04);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn05);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn06);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn07);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn08);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn09);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn10);
        SupplementalReportColumns.push_back(invar.SupplementalReportColumn11);

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

std::string ledger_evaluator::value(std::string const& scalar_name) const
{
    if(!contains(scalars_, scalar_name))
        alarum() << "Key '" << scalar_name << "' not found." << LMI_FLUSH;
    return map_lookup(scalars_, scalar_name);
}

std::string ledger_evaluator::value
    (std::string const& vector_name
    ,int                index
    ) const
{
    if(!contains(vectors_, vector_name))
        alarum() << "Key '" << vector_name << "' not found." << LMI_FLUSH;
    return map_lookup(vectors_, vector_name).at(index);
}

/// Write values to a TSV file as a side effect of writing a PDF.
///
/// Copy 'vectors_' to a (sorted) std::map in order to show columns
/// alphabetically; 'scalars_', likewise. Other, more complicated
/// techniques are faster, but direct copying favors simplicity over
/// speed--appropriately, as this facility is rarely used.

void ledger_evaluator::write_tsv(fs::path const& pdf_out_file) const
{
    if("1" != value("WriteTsvFile")) return;

    configurable_settings const& c = configurable_settings::instance();
    std::string const& z = c.spreadsheet_file_extension();
    fs::path filepath = unique_filepath(pdf_out_file, ".values" + z);
    fs::ofstream ofs(filepath, ios_out_trunc_binary());

    using v_map_t = std::map<std::string,std::vector<std::string>> const;
    v_map_t sorted_vectors(vectors_.begin(), vectors_.end());

    for(auto const& j : sorted_vectors)
        {
        ofs << j.first << '\t';
        }
    ofs << '\n';

    int const length = value_cast<int>(value("GreatestLapseDuration"));
    for(int i = 0; i < length; ++i)
        {
        for(auto const& j : sorted_vectors)
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

    ofs << '\n';

    using s_map_t = std::map<std::string,std::string> const;
    s_map_t sorted_scalars(scalars_.begin(), scalars_.end());

    for(auto const& j : sorted_scalars)
        {
        ofs << j.first << '\t' << j.second << '\n';
        }

    if(!ofs)
        {
        alarum() << "Unable to write '" << filepath << "'." << LMI_FLUSH;
        }
}
