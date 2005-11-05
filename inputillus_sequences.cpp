// Set input parameters from sequence strings.
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

// $Id: inputillus_sequences.cpp,v 1.9 2005-11-05 03:57:23 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputillus.hpp"

#include "alert.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "input_seq_helpers.hpp"

#include <algorithm>
#include <sstream>

//============================================================================
// Realize sequence strings with only numeric values.
template<typename T>
std::string realize_sequence_string
    (IllusInputParms  & input
    ,std::vector<T>   & v
    ,std::string const& sequence_string
    ,int                index_origin = 0
    )
{
    InputSequence s
        (sequence_string
        ,input.YearsToMaturity()
        ,static_cast<int>(input.Status[0].IssueAge)
        ,static_cast<int>(input.Status[0].RetAge)
        ,static_cast<int>(input.InforceYear)
        ,calendar_date(input.EffDate).year()
        ,index_origin
        );
    detail::convert_vector(v, s.linear_number_representation());
    return s.formatted_diagnostics(true); // TODO ?? Change default arg?
}

//============================================================================
// Realize sequence strings with only enumerative-string values.
template<typename T>
std::string realize_sequence_string
    (IllusInputParms        & input
    ,std::vector<T>         & v
    ,std::string       const& sequence_string
    ,detail::stringmap const& keyword_dictionary
    ,std::string       const& default_keyword
    ,int                      index_origin = 0
    )
{
    InputSequence s
        (sequence_string
        ,input.YearsToMaturity()
        ,static_cast<int>(input.Status[0].IssueAge)
        ,static_cast<int>(input.Status[0].RetAge)
        ,static_cast<int>(input.InforceYear)
        ,calendar_date(input.EffDate).year()
        ,index_origin
        ,detail::extract_keys_from_string_map(keyword_dictionary)
        ,default_keyword
        ,true
        );
    detail::convert_vector
        (v
        ,s.linear_keyword_representation()
        ,keyword_dictionary
        ,default_keyword
        );
    return s.formatted_diagnostics(true); // TODO ?? Change default arg?
}

//============================================================================
// Realize sequence strings with both numeric and enumerative-string values.
template<typename Numeric, typename Enumerative>
std::string realize_sequence_string
    (IllusInputParms          & input
    ,std::vector<Numeric>     & vn
    ,std::vector<Enumerative> & ve
    ,std::string         const& sequence_string
    ,detail::stringmap   const& keyword_dictionary
    ,std::string         const& default_keyword
    ,int                        index_origin = 0
    )
{
    InputSequence s
        (sequence_string
        ,input.YearsToMaturity()
        ,static_cast<int>(input.Status[0].IssueAge)
        ,static_cast<int>(input.Status[0].RetAge)
        ,static_cast<int>(input.InforceYear)
        ,calendar_date(input.EffDate).year()
        ,index_origin
        ,detail::extract_keys_from_string_map(keyword_dictionary)
        ,default_keyword
        ,false
        );
    detail::convert_vector(vn, s.linear_number_representation());
    detail::convert_vector
        (ve
        ,s.linear_keyword_representation()
        ,keyword_dictionary
        ,default_keyword
        );
    return s.formatted_diagnostics(true); // TODO ?? Change default arg?
}

//============================================================================
IllusInputParms::string_map const
IllusInputParms::permissible_specified_amount_strategy_keywords()
{
    static string_map all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["maximum" ] = "SAMaximum"       ;
        all_keywords["target"  ] = "SATarget"        ;
        all_keywords["sevenpay"] = "SAMEP"           ;
        all_keywords["glp"     ] = "SAGLP"           ;
        all_keywords["gsp"     ] = "SAGSP"           ;
        all_keywords["corridor"] = "SACorridor"      ;
        all_keywords["salary"  ] = "SASalary"        ;
        all_keywords["none"    ] = "SAInputScalar"   ;
        }
//    string_map permissible_keywords = all_keywords;
    string_map permissible_keywords;
    // Don't use initialization--we want this to happen every time [6.7].
    for
        (string_map_iterator i = all_keywords.begin()
        ;i != all_keywords.end()
        ;++i
        )
        {
        permissible_keywords.insert(*i);
        }
    permissible_keywords.erase("none");

    bool specified_amount_indeterminate =
           e_solve_specamt == SolveType
    // TODO ?? Further conditions to disallow improper input:
    // need to compare corresponding years.
    //  || payment strategy is anything but 'none'
        ;
    if(specified_amount_indeterminate)
        {
        permissible_keywords.clear();
        }

    return permissible_keywords;
}

//============================================================================
IllusInputParms::string_map const
IllusInputParms::permissible_death_benefit_option_keywords()
{
    static string_map all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["a"  ] = "A"  ;
        all_keywords["b"  ] = "B"  ;
        all_keywords["rop"] = "ROP";
        }
    string_map permissible_keywords = all_keywords;
    return permissible_keywords;
}

//============================================================================
IllusInputParms::string_map const
IllusInputParms::permissible_payment_strategy_keywords()
{
    static string_map all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["minimum" ] = "PmtMinimum"      ;
        all_keywords["target"  ] = "PmtTarget"       ;
        all_keywords["sevenpay"] = "PmtMEP"          ;
        all_keywords["glp"     ] = "PmtGLP"          ;
        all_keywords["gsp"     ] = "PmtGSP"          ;
        all_keywords["corridor"] = "PmtCorridor"     ;
        all_keywords["table"   ] = "PmtTable"        ;
        all_keywords["none"    ] = "PmtInputScalar"  ;
        }
    string_map permissible_keywords = all_keywords;
    permissible_keywords.erase("none");

    bool payment_indeterminate =
        (
           e_solve_ee_prem     == SolveType
        || e_solve_er_prem     == SolveType
        || e_solve_ee_prem_dur == SolveType
        || e_solve_er_prem_dur == SolveType
    // TODO ?? Further conditions to disallow improper input:
    // need to compare corresponding years.
    //  || specamt strategy is neither 'none' nor 'salary-based'
        );

    if(payment_indeterminate)
        {
        permissible_keywords.clear();
        }

    return permissible_keywords;
}

//============================================================================
IllusInputParms::string_map const
IllusInputParms::permissible_payment_mode_keywords()
{
    static string_map all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["annual"    ] = "Annual";
        all_keywords["semiannual"] = "Semiannual";
        all_keywords["quarterly" ] = "Quarterly";
        all_keywords["monthly"   ] = "Monthly";
        }
    string_map permissible_keywords = all_keywords;
    return permissible_keywords;
}

//============================================================================
std::vector<std::string> IllusInputParms::realize_all_sequence_strings
    (bool report_errors
    )
{
    std::vector<std::string> s;

#if defined __MINGW32__ && defined __GNUC__ && __GNUC__ == 2 && __GNUC_MINOR__ == 95
    // COMPILER !! Here's a mystery: unit test segfaults without this line.
    s.push_back("");
#endif // mingw gcc 2.95.2-1 compiler.
    s.push_back(realize_sequence_string_for_add_on_monthly_custodial_fee());
    s.push_back(realize_sequence_string_for_add_on_comp_on_assets       ());
    s.push_back(realize_sequence_string_for_add_on_comp_on_premium      ());
    s.push_back(realize_sequence_string_for_non_us_corridor_factor      ());
    s.push_back(realize_sequence_string_for_partial_mortality_multiplier());
    s.push_back(realize_sequence_string_for_current_coi_multiplier      ());
    s.push_back(realize_sequence_string_for_current_coi_grading         ());
    s.push_back(realize_sequence_string_for_cash_value_enhancement_rate ());
    s.push_back(realize_sequence_string_for_corp_tax_bracket            ());
    s.push_back(realize_sequence_string_for_indv_tax_bracket            ());
    s.push_back(realize_sequence_string_for_projected_salary            ());
    s.push_back(realize_sequence_string_for_specified_amount            ());
    s.push_back(realize_sequence_string_for_death_benefit_option        ());
    s.push_back(realize_sequence_string_for_indv_payment                ());
    s.push_back(realize_sequence_string_for_indv_payment_mode           ());
    s.push_back(realize_sequence_string_for_corp_payment                ());
    s.push_back(realize_sequence_string_for_corp_payment_mode           ());
    s.push_back(realize_sequence_string_for_gen_acct_int_rate           ());
    s.push_back(realize_sequence_string_for_sep_acct_int_rate           ());
    s.push_back(realize_sequence_string_for_new_loan                    ());
    s.push_back(realize_sequence_string_for_withdrawal                  ());
    s.push_back(realize_sequence_string_for_flat_extra                  ());
    s.push_back(realize_sequence_string_for_policy_level_flat_extra     ());
    s.push_back(realize_sequence_string_for_honeymoon_value_spread      ());
    s.push_back(realize_sequence_string_for_premium_history             ());
    s.push_back(realize_sequence_string_for_specamt_history             ());

    if(report_errors)
        {
        for
            (std::vector<std::string>::iterator i = s.begin()
            ;i != s.end()
            ;++i
            )
            {
            std::ostringstream oss;
            bool diagnostics_present = false;
            if(!i->empty())
                {
                diagnostics_present = true;
                oss << (*i) << "\r\n";
                }
            if(diagnostics_present)
                {
                fatal_error()
                    << "Input validation problems: "
                    << oss.str() << '\n'
                    << LMI_FLUSH
                    ;
                }
            }
        }

    return s;
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_add_on_monthly_custodial_fee()
{
    return realize_sequence_string
        (*this
        ,VectorAddonMonthlyCustodialFee
        ,AddonMonthlyCustodialFee
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_add_on_comp_on_assets()
{
    std::string s = realize_sequence_string
        (*this
        ,VectorAddonCompOnAssets
        ,AddonCompOnAssets
        );
    if(s.size())
        {
        return s;
        }

    double highest = *std::max_element
        (VectorAddonCompOnAssets.begin()
        ,VectorAddonCompOnAssets.end()
        );
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal bp
    // or
    //   numeric-value: numeric-literal %%
    // then we might say "100%" here.

    // TODO ?? Defect: currently entered in basis points.
    if(10000.0 < highest)
        {
        return "Add-on compensation cannot exceed 10000 basis points.";
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_add_on_comp_on_premium()
{
    std::string s = realize_sequence_string
        (*this
        ,VectorAddonCompOnPremium
        ,AddonCompOnPremium
        );
    if(s.size())
        {
        return s;
        }

    double highest = *std::max_element
        (VectorAddonCompOnPremium.begin()
        ,VectorAddonCompOnPremium.end()
        );
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "100%" here.
    if(1.0 < highest)
        {
        return "Add-on compensation cannot exceed one times premium.";
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_non_us_corridor_factor()
{
    std::string s = realize_sequence_string
        (*this
        ,VectorNonUsCorridorFactor
        ,NonUsCorridorFactor
        );
    if(s.size())
        {
        return s;
        }

//    double lowest = *std::min_element
//        (VectorNonUsCorridorFactor.begin()
//        ,VectorNonUsCorridorFactor.end()
//        );
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "at least 100%." here.
    //
    // TODO ?? Temporarily suppressed to let an old hack work:
    // this was used to suppress COIs or something like that.
    // Need to fix that, then restore this.
//    if(1.0 < lowest)
//        {
//        return "Corridor factor must be at least 1.";
//        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_partial_mortality_multiplier()
{
    return realize_sequence_string
        (*this
        ,VectorPartialMortalityMultiplier
        ,PartialMortalityMultiplier
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_current_coi_multiplier()
{
    return realize_sequence_string
        (*this
        ,VectorCurrentCoiMultiplier
        ,CurrentCoiMultiplier
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_current_coi_grading()
{
    std::string s = realize_sequence_string
        (*this
        ,VectorCurrentCoiGrading
        ,CurrentCoiGrading
        );
    if(s.size())
        {
        return s;
        }

    double highest = *std::max_element
        (VectorCurrentCoiGrading.begin()
        ,VectorCurrentCoiGrading.end()
        );
    double lowest = *std::min_element
        (VectorCurrentCoiGrading.begin()
        ,VectorCurrentCoiGrading.end()
        );
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "between 0% and 100%." here.
    //
    // There's no particular reason to use 100% as the maximum.
    // Anything over that seems extreme.
    //
    if(!(0.0 <= lowest && highest <= 1.0))
        {
        std::ostringstream oss;
        oss
            << "Current COI grading entered ranges from "
            << lowest
            << " to "
            << highest
            << " but must be between 0 and 1 inclusive."
            ;
        return oss.str();
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_cash_value_enhancement_rate()
{
    std::string s = realize_sequence_string
        (*this
        ,VectorCashValueEnhancementRate
        ,CashValueEnhancementRate
        );
    if(s.size())
        {
        return s;
        }

    double highest = *std::max_element
        (VectorCashValueEnhancementRate.begin()
        ,VectorCashValueEnhancementRate.end()
        );
    double lowest = *std::min_element
        (VectorCashValueEnhancementRate.begin()
        ,VectorCashValueEnhancementRate.end()
        );
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "between 0% and 100%." here.
    if(!(0.0 <= lowest && highest <= 1.0))
        {
        std::ostringstream oss;
        oss
            << "Current COI grading entered ranges from "
            << lowest
            << " to "
            << highest
            << " but must be between 0 and 1 inclusive."
            ;
        return oss.str();
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_corp_tax_bracket()
{
    std::string s = realize_sequence_string
        (*this
        ,VectorCorpTaxBracket
        ,CorpTaxBracket
        );
    if(s.size())
        {
        return s;
        }

    double highest = *std::max_element
        (VectorCorpTaxBracket.begin()
        ,VectorCorpTaxBracket.end()
        );
    double lowest = *std::min_element
        (VectorCorpTaxBracket.begin()
        ,VectorCorpTaxBracket.end()
        );
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "between 0% and 100%." here.
    if(!(0.0 <= lowest && highest <= 1.0))
        {
        std::ostringstream oss;
        oss
            << "Corporate tax bracket entered ranges from "
            << lowest
            << " to "
            << highest
            << " but must be between 0 and 1 inclusive."
            ;
        return oss.str();
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_indv_tax_bracket()
{
    std::string s = realize_sequence_string
        (*this
        ,VectorIndvTaxBracket
        ,IndvTaxBracket
        );
    if(s.size())
        {
        return s;
        }

    double highest = *std::max_element
        (VectorIndvTaxBracket.begin()
        ,VectorIndvTaxBracket.end()
        );
    double lowest = *std::min_element
        (VectorIndvTaxBracket.begin()
        ,VectorIndvTaxBracket.end()
        );
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "between 0% and 100%." here.
    if(!(0.0 <= lowest && highest <= 1.0))
        {
        std::ostringstream oss;
        oss
            << "Individual tax bracket entered ranges from "
            << lowest
            << " to "
            << highest
            << " but must be between 0 and 1 inclusive."
            ;
        return oss.str();
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_projected_salary()
{
    return realize_sequence_string
        (*this
        ,Salary
        ,ProjectedSalary
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_specified_amount()
{
// We could enforce a minimum, using DB_MinSpecAmt from the database.
// But some would think it useful to be able to enter zero and get
// that minimum.
    return realize_sequence_string
        (*this
        ,SpecAmt
        ,VectorSpecifiedAmountStrategy
        ,SpecifiedAmount
        ,permissible_specified_amount_strategy_keywords()
        ,std::string("none")
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_death_benefit_option()
{
    std::string s = realize_sequence_string
        (*this
        ,DBOpt
        ,DeathBenefitOption
        ,permissible_death_benefit_option_keywords()
        ,std::string("a")
        );
    if(s.size())
        {
        return s;
        }

    // TODO ?? Move the bodies of functions
    //   CheckAllowChangeToDBO2()
    //   CheckAllowDBO3()
    // here for consistency, then expunge those functions.
    if(!CheckAllowChangeToDBO2())
        {
        return
            ("Policy form forbids change to increasing death benefit option."
            );
        }
    if(!CheckAllowDBO3())
        {
        return
            ("Policy form forbids ROP death benefit option."
            );
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_indv_payment()
{
    return realize_sequence_string
        (*this
        ,EePremium
        ,VectorIndvPaymentStrategy
        ,IndvPayment
        ,permissible_payment_strategy_keywords()
        ,std::string("none")
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_indv_payment_mode()
{
    // TODO ?? No universally appropriate default exists: some contracts
    // may forbid monthly mode, while others may require it. We enforce
    // no such restriction because our practice has been confined to
    // products that permit annual mode.
    return realize_sequence_string
        (*this
        ,EeMode
        ,IndvPaymentMode
        ,permissible_payment_mode_keywords()
        ,std::string("annual")
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_corp_payment()
{
    return realize_sequence_string
        (*this
        ,ErPremium
        ,VectorCorpPaymentStrategy
        ,CorpPayment
        ,permissible_payment_strategy_keywords()
        ,std::string("none")
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_corp_payment_mode()
{
    // TODO ?? No universally appropriate default exists: some contracts
    // may forbid monthly mode, while others may require it. We enforce
    // no such restriction because our practice has been confined to
    // products that permit annual mode.
    return realize_sequence_string
        (*this
        ,ErMode
        ,CorpPaymentMode
        ,permissible_payment_mode_keywords()
        ,std::string("annual")
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_gen_acct_int_rate()
{
    // TODO ?? How to handle DB_AllowGenAcct?
    // TODO ?? How to handle ukase against illustrating varying rates?
    std::string s = realize_sequence_string
        (*this
        ,GenAcctRate
        ,GenAcctIntRate
        );
    if(s.size())
        {
        return s;
        }

    TDatabase temp_database
        (ProductName
        ,Status[0].Gender
        ,Status[0].Class
        ,Status[0].Smoking
        ,Status[0].IssueAge
        ,GroupUWType
        ,InsdState
        );

    // TODO ?? Should enforce minimum = guar rate?
    std::vector<double> general_account_max_rate;
    temp_database.Query(general_account_max_rate, DB_MaxGenAcctRate);

    if(global_settings::instance().ash_nazg())
        {
        // We have some regression-test files with rates higher even
        // than twelve percent.
        general_account_max_rate.assign(general_account_max_rate.size(), 1.0);
        }
    else if
        (   global_settings::instance().mellon()
        ||  global_settings::instance().custom_io_0()
        )
        {
        general_account_max_rate.assign(general_account_max_rate.size(), 0.12);
        }

    for(unsigned int j = 0; j < general_account_max_rate.size(); ++j)
        {
        if(general_account_max_rate[j] < GenAcctRate[j])
            {
            std::ostringstream oss;
            oss
                << "Duration "
                << j
                << ": general-account interest rate entered is "
                << GenAcctRate[j]
                << ", but highest rate allowed is "
                << general_account_max_rate[j]
                ;
            return oss.str();
            }
        }

    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_sep_acct_int_rate()
{
    // TODO ?? How to handle DB_AllowSepAcct?
    // TODO ?? How to handle ukase against illustrating varying rates?
    std::string s = realize_sequence_string
        (*this
        ,SepAcctRate
        ,SepAcctIntRate
        );
    if(s.size())
        {
        return s;
        }

    TDatabase temp_database
        (ProductName
        ,Status[0].Gender
        ,Status[0].Class
        ,Status[0].Smoking
        ,Status[0].IssueAge
        ,GroupUWType
        ,InsdState
        );

    // TODO ?? Should enforce minimum = (0.0 - spread)?
    double max_sep_acct_rate = temp_database.Query(DB_MaxSepAcctRate);
    if(global_settings::instance().ash_nazg())
        {
        // We have some regression-test files with rates higher even
        // than twelve percent.
        max_sep_acct_rate = 1.0;
        }
    double highest = *std::max_element
        (SepAcctRate.begin()
        ,SepAcctRate.end()
        );
    if(max_sep_acct_rate < highest)
        {
        std::ostringstream oss;
        oss
            << "Highest separate-account interest rate entered is "
            << highest
            << " but highest rate allowed is "
            << max_sep_acct_rate
            ;
        return oss.str();
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_new_loan()
{
    std::string s = realize_sequence_string
        (*this
        ,Loan
        ,NewLoan
        );
    if(s.size())
        {
        return s;
        }

    TDatabase temp_database
        (ProductName
        ,Status[0].Gender
        ,Status[0].Class
        ,Status[0].Smoking
        ,Status[0].IssueAge
        ,GroupUWType
        ,InsdState
        );
    if(temp_database.Query(DB_AllowLoan))
        {
        return "";
        }

    r_loan highest = *std::max_element
        (Loan.begin()
        ,Loan.end()
        );
    r_loan lowest = *std::min_element
        (Loan.begin()
        ,Loan.end()
        );
    if(0.0 != highest || 0.0 != lowest)
        {
        return "Loans may not be illustrated on this policy form.";
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_withdrawal()
{
    std::string s = realize_sequence_string
        (*this
        ,WD
        ,Withdrawal
        );
    if(s.size())
        {
        return s;
        }

    TDatabase temp_database
        (ProductName
        ,Status[0].Gender
        ,Status[0].Class
        ,Status[0].Smoking
        ,Status[0].IssueAge
        ,GroupUWType
        ,InsdState
        );

    r_wd highest = *std::max_element
        (WD.begin()
        ,WD.end()
        );
    r_wd lowest = *std::min_element
        (WD.begin()
        ,WD.end()
        );

    if(!temp_database.Query(DB_AllowWD))
        {
        if(0.0 != highest || 0.0 != lowest)
            {
            return "Withdrawals may not be illustrated on this policy form.";
            }
        }
    else
        {
        double lowest_allowed_withdrawal = temp_database.Query(DB_MinWD);
        for
            (std::vector<r_wd>::iterator i = WD.begin()
            ;i < WD.end()
            ;++i
            )
// TODO ?? Not sure...should we report all, or just the first?
            {
            if(0.0 < *i && *i < lowest_allowed_withdrawal)
                {
                std::ostringstream oss;
                oss
                    << "Minimum withdrawal is "
                    << lowest_allowed_withdrawal
                    << "; "
                    << *i
                    << " is too low."
                    ;
                return oss.str();
                }
            }
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_flat_extra()
{
// We could enforce a maximum of the monthly equivalent of unity,
// and a minimum of zero; is that worth the bother though?
    std::string s = realize_sequence_string
        (*this
        ,Status[0].VectorMonthlyFlatExtra
        ,FlatExtra
        );
    if(s.size())
        {
        return s;
        }

    TDatabase temp_database
        (ProductName
        ,Status[0].Gender
        ,Status[0].Class
        ,Status[0].Smoking
        ,Status[0].IssueAge
        ,GroupUWType
        ,InsdState
        );
    if(temp_database.Query(DB_AllowFlatExtras))
        {
        return "";
        }

    double highest = *std::max_element
        (Status[0].VectorMonthlyFlatExtra.begin()
        ,Status[0].VectorMonthlyFlatExtra.end()
        );
    double lowest = *std::min_element
        (Status[0].VectorMonthlyFlatExtra.begin()
        ,Status[0].VectorMonthlyFlatExtra.end()
        );
    if(0.0 != highest || 0.0 != lowest)
        {
        return "Flat extras may not be illustrated on this policy form.";
        }
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_policy_level_flat_extra()
{
// Not yet implemented. May be useful for survivorship.
    return "";
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_honeymoon_value_spread()
{
    return realize_sequence_string
        (*this
        ,VectorHoneymoonValueSpread
        ,HoneymoonValueSpread
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_premium_history()
{
    return realize_sequence_string
        (*this
        ,VectorPremiumHistory
        ,PremiumHistory
        );
}

//============================================================================
std::string IllusInputParms::realize_sequence_string_for_specamt_history()
{
    return realize_sequence_string
        (*this
        ,VectorSpecamtHistory
        ,SpecamtHistory
        );
}

