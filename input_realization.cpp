// Realize sequence-string input as vectors.
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

#include "input.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "dbo_rules.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "input_sequence_aux.hpp"       // convert_vector()
#include "math_functions.hpp"           // outward_quotient()
#include "miscellany.hpp"               // each_equal()
#include "round_to.hpp"
#include "ssize_lmi.hpp"
#include "unwind.hpp"                   // scoped_unwind_toggler
#include "value_cast.hpp"

#include <algorithm>
#include <exception>
#include <functional>                   // bind()
#include <sstream>
#include <utility>                      // pair

//============================================================================
// Realize sequence strings with only numeric values.
template<typename T>
std::string realize_sequence_string
    (Input          const& input
    ,std::vector<T>      & v
    ,datum_sequence const& sequence_string
    )
{
    try
        {
        scoped_unwind_toggler meaningless_name;
        InputSequence s
            (sequence_string.value()
            ,input.years_to_maturity()
            ,input.issue_age        ()
            ,input.retirement_age   ()
            ,input.inforce_year     ()
            ,input.effective_year   ()
            );
        detail::convert_vector(v, s.seriatim_numbers());
        }
    catch(std::exception const& e)
        {
        return abridge_diagnostics(e.what());
        }
    return std::string();
}

//============================================================================
// Realize sequence strings with only enumerative-string values.
template<typename T>
std::string realize_sequence_string
    (Input             const& input
    ,std::vector<T>         & v
    ,datum_sequence    const& sequence_string
    ,detail::stringmap const& keyword_dictionary
    ,std::string       const& default_keyword
    )
{
    try
        {
        InputSequence s
            (sequence_string.value()
            ,input.years_to_maturity()
            ,input.issue_age        ()
            ,input.retirement_age   ()
            ,input.inforce_year     ()
            ,input.effective_year   ()
            ,detail::extract_keys_from_string_map(keyword_dictionary)
            ,true
            ,default_keyword
            );
        detail::convert_vector
            (v
            ,s.seriatim_keywords()
            ,keyword_dictionary
            ,default_keyword
            );
        }
    catch(std::exception const& e)
        {
        return abridge_diagnostics(e.what());
        }
    return std::string();
}

//============================================================================
// Realize sequence strings with both numeric and enumerative-string values.
template<typename Numeric, typename Enumerative>
std::string realize_sequence_string
    (Input               const& input
    ,std::vector<Numeric>     & vn
    ,std::vector<Enumerative> & ve
    ,datum_sequence      const& sequence_string
    ,detail::stringmap   const& keyword_dictionary
    ,std::string         const& default_keyword
    )
{
    try
        {
        InputSequence s
            (sequence_string.value()
            ,input.years_to_maturity()
            ,input.issue_age        ()
            ,input.retirement_age   ()
            ,input.inforce_year     ()
            ,input.effective_year   ()
            ,detail::extract_keys_from_string_map(keyword_dictionary)
            ,false
            ,default_keyword
            );
        detail::convert_vector(vn, s.seriatim_numbers());
        detail::convert_vector
            (ve
            ,s.seriatim_keywords()
            ,keyword_dictionary
            ,default_keyword
            );
        }
    catch(std::exception const& e)
        {
        return abridge_diagnostics(e.what());
        }
    return std::string();
}

// SpecifiedAmount.allowed_keywords() does more or less the same
// thing, but without the 'specified_amount_indeterminate' test.
// That test isn't actually correct: it's okay to use 'sevenpay'
// for seven years, then solve on the interval [7, maturity).
// However, other restrictions might be applied: e.g., if payment
// strategy is 'sevenpay', then specamt strategy must not also be
// 'sevenpay' in the same year.

std::map<std::string,std::string> const
Input::permissible_specified_amount_strategy_keywords()
{
    static std::map<std::string,std::string> all_keywords;
    if(all_keywords.empty())
        {
        all_keywords["maximum" ] = "SAMaximum"       ;
        all_keywords["target"  ] = "SATarget"        ;
        all_keywords["sevenpay"] = "SA7PP"           ;
        all_keywords["glp"     ] = "SAGLP"           ;
        all_keywords["gsp"     ] = "SAGSP"           ;
        all_keywords["corridor"] = "SACorridor"      ;
        all_keywords["salary"  ] = "SASalary"        ;
        }
//    std::map<std::string,std::string> permissible_keywords = all_keywords;
    std::map<std::string,std::string> permissible_keywords;
    // Don't use initialization--we want this to happen every time [6.7].
    for(auto const& i : all_keywords)
        {
        permissible_keywords.insert(i);
        }

    bool specified_amount_indeterminate = mce_solve_specamt == SolveType;
    if(specified_amount_indeterminate)
        {
        permissible_keywords.clear();
        }

    return permissible_keywords;
}

//============================================================================
std::vector<std::string> Input::RealizeAllSequenceInput(bool report_errors)
{
    LMI_ASSERT(years_to_maturity() == database_->length());

    // INPUT !! https://savannah.nongnu.org/support/?104481
    // This needs to be reimplemented.
    {
    constexpr int NumberOfFunds {30}; // DEPRECATED
    std::istringstream iss(FundAllocations.value());
    std::vector<tnr_unrestricted_double> v;
    for(;;)
        {
        int i;
        iss >> i;
        if(!iss)
            {
            break;
            }
        v.push_back(tnr_unrestricted_double(i));
        }
    if(v.size() < NumberOfFunds)
        {
        v.insert(v.end(), NumberOfFunds - v.size(), tnr_unrestricted_double(0.0));
        }
    FundAllocationsRealized_ = v;
    }

    std::vector<std::string> s;

#if defined __MINGW32__ && defined __GNUC__ && __GNUC__ == 2 && __GNUC_MINOR__ == 95
    // COMPILER !! Here's a mystery: unit test segfaults without this line.
    s.push_back("");
#endif // mingw gcc 2.95.2-1 compiler.
    s.push_back(RealizeExtraMonthlyCustodialFee   ());
    s.push_back(RealizeExtraCompensationOnAssets  ());
    s.push_back(RealizeExtraCompensationOnPremium ());
    s.push_back(RealizePartialMortalityMultiplier ());
    s.push_back(RealizeCurrentCoiMultiplier       ());
    s.push_back(RealizeCashValueEnhancementRate   ());
    s.push_back(RealizeCorporationTaxBracket      ());
    s.push_back(RealizeTaxBracket                 ());
    s.push_back(RealizeProjectedSalary            ());
    s.push_back(RealizeSpecifiedAmount            ());
    s.push_back(RealizeSupplementalAmount         ());
    s.push_back(RealizeDeathBenefitOption         ());
    s.push_back(RealizePayment                    ());
    s.push_back(RealizePaymentMode                ());
    s.push_back(RealizeCorporationPayment         ());
    s.push_back(RealizeCorporationPaymentMode     ());
    s.push_back(RealizeGeneralAccountRate         ());
    s.push_back(RealizeSeparateAccountRate        ());
    s.push_back(RealizeNewLoan                    ());
    s.push_back(RealizeWithdrawal                 ());
    s.push_back(RealizeFlatExtra                  ());
    s.push_back(RealizeHoneymoonValueSpread       ());
    s.push_back(RealizeAmountsPaidHistory         ());

    if(report_errors)
        {
        for(auto const& i : s)
            {
            std::ostringstream oss;
            bool diagnostics_present = false;
            if(!i.empty())
                {
                diagnostics_present = true;
                oss << i << "\n";
                }
            if(diagnostics_present)
                {
                alarum()
                    << "Input validation problems for '"
                    << InsuredName
                    << "':\n"
                    << oss.str()
                    << LMI_FLUSH
                    ;
                }
            }
        }

    return s;
}

//============================================================================
std::string Input::RealizeExtraMonthlyCustodialFee()
{
    return realize_sequence_string
        (*this
        ,ExtraMonthlyCustodialFeeRealized_
        ,ExtraMonthlyCustodialFee
        );
}

//============================================================================
std::string Input::RealizeExtraCompensationOnAssets()
{
    std::string s = realize_sequence_string
        (*this
        ,ExtraCompensationOnAssetsRealized_
        ,ExtraCompensationOnAssets
        );
    if(s.size())
        {
        return s;
        }

    LMI_ASSERT(!ExtraCompensationOnAssetsRealized_.empty());
    double highest = std::max_element
        (ExtraCompensationOnAssetsRealized_.begin()
        ,ExtraCompensationOnAssetsRealized_.end()
        )->value();
    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal bp
    // or
    //   numeric-value: numeric-literal %%
    // then we might say "100%" here.

    if(1.0 < highest)
        {
        return "Add-on compensation cannot exceed 1 (meaning 100% of assets).";
        }

    return "";
}

//============================================================================
std::string Input::RealizeExtraCompensationOnPremium()
{
    std::string s = realize_sequence_string
        (*this
        ,ExtraCompensationOnPremiumRealized_
        ,ExtraCompensationOnPremium
        );
    if(s.size())
        {
        return s;
        }

    LMI_ASSERT(!ExtraCompensationOnPremiumRealized_.empty());
    double highest = std::max_element
        (ExtraCompensationOnPremiumRealized_.begin()
        ,ExtraCompensationOnPremiumRealized_.end()
        )->value();
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
std::string Input::RealizePartialMortalityMultiplier()
{
    return realize_sequence_string
        (*this
        ,PartialMortalityMultiplierRealized_
        ,PartialMortalityMultiplier
        );
}

//============================================================================
std::string Input::RealizeCurrentCoiMultiplier()
{
    if(std::string::npos == CurrentCoiMultiplier.value().find_first_of("123456789"))
        {
        std::ostringstream oss;
        oss
            << "COI multiplier entered is '"
            << CurrentCoiMultiplier
            << "', but it must contain at least one number other than zero."
            ;
        return oss.str();
        }

    std::string s = realize_sequence_string
        (*this
        ,CurrentCoiMultiplierRealized_
        ,CurrentCoiMultiplier
        );
    if(s.size())
        {
        return s;
        }

    if(global_settings::instance().mellon())
        {
        return s;
        }

    double const z = database_->query<double>(DB_MinInputCoiMult);
    LMI_ASSERT(!CurrentCoiMultiplierRealized_.empty());
    double lowest = std::min_element
        (CurrentCoiMultiplierRealized_.begin()
        ,CurrentCoiMultiplierRealized_.end()
        )->value();
    if(lowest < z)
        {
        std::ostringstream oss;
        oss
            << "Lowest COI multiplier entered is "
            << lowest
            << ", but "
            << z
            << " is the lowest multiplier allowed."
            ;
        return oss.str();
        }

    return "";
}

//============================================================================
std::string Input::RealizeCashValueEnhancementRate()
{
    std::string s = realize_sequence_string
        (*this
        ,CashValueEnhancementRateRealized_
        ,CashValueEnhancementRate
        );
    if(s.size())
        {
        return s;
        }

    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "between 0% and 100%." here.
    LMI_ASSERT(!CashValueEnhancementRateRealized_.empty());
    auto const extrema = std::minmax_element
        (CashValueEnhancementRateRealized_.begin()
        ,CashValueEnhancementRateRealized_.end()
        );
    if(!(0.0 <= (*extrema.first).value() && (*extrema.second).value() <= 1.0))
        {
        std::ostringstream oss;
        oss
            << "Cash value enhancement rate as entered ranges from "
            << *extrema.first
            << " to "
            << *extrema.second
            << ", but must be between 0 and 1 inclusive."
            ;
        return oss.str();
        }

    return "";
}

//============================================================================
std::string Input::RealizeCorporationTaxBracket()
{
    std::string s = realize_sequence_string
        (*this
        ,CorporationTaxBracketRealized_
        ,CorporationTaxBracket
        );
    if(s.size())
        {
        return s;
        }

    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "between 0% and 100%." here.
    LMI_ASSERT(!CorporationTaxBracketRealized_.empty());
    auto const extrema = std::minmax_element
        (CorporationTaxBracketRealized_.begin()
        ,CorporationTaxBracketRealized_.end()
        );
    if(!(0.0 <= (*extrema.first).value() && (*extrema.second).value() <= 1.0))
        {
        std::ostringstream oss;
        oss
            << "Corporate tax bracket as entered ranges from "
            << *extrema.first
            << " to "
            << *extrema.second
            << ", but must be between 0 and 1 inclusive."
            ;
        return oss.str();
        }

    return "";
}

//============================================================================
std::string Input::RealizeTaxBracket()
{
    std::string s = realize_sequence_string
        (*this
        ,TaxBracketRealized_
        ,TaxBracket
        );
    if(s.size())
        {
        return s;
        }

    // SOMEDAY !! If we add a production like
    //   numeric-value: numeric-literal %
    // then we might say "between 0% and 100%." here.
    LMI_ASSERT(!TaxBracketRealized_.empty());
    auto const extrema = std::minmax_element
        (TaxBracketRealized_.begin()
        ,TaxBracketRealized_.end()
        );
    if(!(0.0 <= (*extrema.first).value() && (*extrema.second).value() <= 1.0))
        {
        std::ostringstream oss;
        oss
            << "Individual tax bracket as entered ranges from "
            << *extrema.first
            << " to "
            << *extrema.second
            << ", but must be between 0 and 1 inclusive."
            ;
        return oss.str();
        }

    return "";
}

//============================================================================
std::string Input::RealizeProjectedSalary()
{
    return realize_sequence_string
        (*this
        ,ProjectedSalaryRealized_
        ,ProjectedSalary
        );
}

//============================================================================
std::string Input::RealizeSpecifiedAmount()
{
// We could enforce a minimum, using DB_MinSpecAmt from the database.
// But some would think it useful to be able to enter zero and get
// that minimum.
    return realize_sequence_string
        (*this
        ,SpecifiedAmountRealized_
        ,SpecifiedAmountStrategyRealized_
        ,SpecifiedAmount
        ,SpecifiedAmount.allowed_keywords()
        ,SpecifiedAmount.default_keyword()
        );
}

//============================================================================
std::string Input::RealizeSupplementalAmount()
{
    return realize_sequence_string
        (*this
        ,SupplementalAmountRealized_
        ,SupplementalAmountStrategyRealized_
        ,SupplementalAmount
        ,SupplementalAmount.allowed_keywords()
        ,SupplementalAmount.default_keyword()
        );
}

//============================================================================
std::string Input::RealizeDeathBenefitOption()
{
    std::string s = realize_sequence_string
        (*this
        ,DeathBenefitOptionRealized_
        ,DeathBenefitOption
        ,DeathBenefitOption.allowed_keywords()
        ,DeathBenefitOption.default_keyword()
        );
    if(s.size())
        {
        return s;
        }

    if
        (   !database_->query<bool>(DB_AllowChangeToDbo2)
        &&  !std::is_sorted
                (DeathBenefitOptionRealized_.begin()
                ,DeathBenefitOptionRealized_.end()
                ,std::bind
                    (std::logical_and<bool>()
                    ,std::bind(std::equal_to    <mce_dbopt>(), std::placeholders::_1, mce_dbopt("B"))
                    ,std::bind(std::not_equal_to<mce_dbopt>(), std::placeholders::_2, mce_dbopt("B"))
                    )
                )
        )
        {
        return
            ("Policy form forbids change to increasing death benefit option."
            );
        }

    if
        (   !database_->query<bool>(DB_AllowDboRop)
        &&  contains(DeathBenefitOptionRealized_, mce_dbopt("ROP"))
        )
        {
        return
            ("Policy form forbids ROP death benefit option."
            );
        }

    if
        (   !database_->query<bool>(DB_AllowDboMdb)
        &&  contains(DeathBenefitOptionRealized_, mce_dbopt("MDB"))
        // DBO3 !! For testing convenience--remove later.
        &&  !global_settings::instance().ash_nazg()
        )
        {
        return
            ("Policy form forbids MDB death benefit option."
            );
        }

    // DBO3 !! Eventually validate all DBO sequences this way
    // (but using DBO rules from the product database):
    if(database_->query<bool>(DB_AllowDboMdb) && !contains(ProductName.value(), "sample"))
        {
        dbo_sequence_is_allowed(DeathBenefitOptionRealized_);
        }

    return "";
}

//============================================================================
std::string Input::RealizePayment()
{
    return realize_sequence_string
        (*this
        ,PaymentRealized_
        ,PaymentStrategyRealized_
        ,Payment
        ,Payment.allowed_keywords()
        ,Payment.default_keyword()
        );
}

//============================================================================
std::string Input::RealizePaymentMode()
{
    // SOMEDAY !! No universally appropriate default exists: some contracts
    // may forbid monthly mode, while others may require it. We enforce
    // no such restriction because our practice has been confined to
    // products that permit annual mode.
    return realize_sequence_string
        (*this
        ,PaymentModeRealized_
        ,PaymentMode
        ,PaymentMode.allowed_keywords()
        ,PaymentMode.default_keyword()
        );
}

//============================================================================
std::string Input::RealizeCorporationPayment()
{
    return realize_sequence_string
        (*this
        ,CorporationPaymentRealized_
        ,CorporationPaymentStrategyRealized_
        ,CorporationPayment
        ,CorporationPayment.allowed_keywords()
        ,CorporationPayment.default_keyword()
        );
}

//============================================================================
std::string Input::RealizeCorporationPaymentMode()
{
    // SOMEDAY !! No universally appropriate default exists: some contracts
    // may forbid monthly mode, while others may require it. We enforce
    // no such restriction because our practice has been confined to
    // products that permit annual mode.
    return realize_sequence_string
        (*this
        ,CorporationPaymentModeRealized_
        ,CorporationPaymentMode
        ,CorporationPaymentMode.allowed_keywords()
        ,CorporationPaymentMode.default_keyword()
        );
}

//============================================================================
std::string Input::RealizeGeneralAccountRate()
{
    std::string s = realize_sequence_string
        (*this
        ,GeneralAccountRateRealized_
        ,GeneralAccountRate
        );
    if(s.size())
        {
        return s;
        }

    // If the field is disabled, then its old contents aren't invalid.
    if(!database_->query<bool>(DB_AllowGenAcct))
        {
        return "";
        }

    double guar_int = database_->query<double>(DB_GuarInt);
    std::vector<double> general_account_max_rate;
    database_->query_into(DB_MaxGenAcctRate, general_account_max_rate);

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

    for(int j = 0; j < lmi::ssize(general_account_max_rate); ++j)
        {
        if(general_account_max_rate[j] < GeneralAccountRateRealized_[j].value())
            {
            std::ostringstream oss;
            oss
                << "Duration "
                << j
                << ": general-account interest rate entered is "
                << GeneralAccountRateRealized_[j]
                << ", but "
                << general_account_max_rate[j]
                << " is the highest rate allowed."
                ;
            return oss.str();
            }
        }

    // DEPRECATED An empty string is a tricky special case for the
    // obsolete input class, which requires this goofy workaround.
    if(GeneralAccountRate.value().empty())
        {
        return "";
        }
    for(int j = 0; j < lmi::ssize(general_account_max_rate); ++j)
        {
        if(GeneralAccountRateRealized_[j].value() < guar_int)
            {
            std::ostringstream oss;
            oss
                << "Duration "
                << j
                << ": general-account interest rate entered is "
                << GeneralAccountRateRealized_[j]
                << ", but "
                << guar_int
                << " is the lowest rate allowed."
                ;
            return oss.str();
            }
        }

    return "";
}

//============================================================================
std::string Input::RealizeSeparateAccountRate()
{
    std::string s = realize_sequence_string
        (*this
        ,SeparateAccountRateRealized_
        ,SeparateAccountRate
        );
    if(s.size())
        {
        return s;
        }

    // If the field is disabled, then its old contents aren't invalid.
    if(!database_->query<bool>(DB_AllowSepAcct))
        {
        return "";
        }

    // Arguably the minimum gross rate would be -(100% + spread).
    // Such an exquisite refinement would complicate the program by
    // making this field's range depend on gross versus net. The -100%
    // minimum for an eventual tn_range type will be low enough.

    double max_sep_acct_rate = database_->query<double>(DB_MaxSepAcctRate);
    if(global_settings::instance().ash_nazg())
        {
        // We have some regression-test files with rates higher even
        // than twelve percent.
        max_sep_acct_rate = 1.0;
        }
    LMI_ASSERT(!SeparateAccountRateRealized_.empty());
    double highest = std::max_element
        (SeparateAccountRateRealized_.begin()
        ,SeparateAccountRateRealized_.end()
        )->value();
    if(max_sep_acct_rate < highest)
        {
        std::ostringstream oss;
        oss
            << "Highest separate-account interest rate entered is "
            << highest
            << ", but "
            << max_sep_acct_rate
            << " is the highest rate allowed."
            ;
        return oss.str();
        }
    double lowest = std::min_element
        (SeparateAccountRateRealized_.begin()
        ,SeparateAccountRateRealized_.end()
        )->value();
    if(lowest < -1.0)
        {
        std::ostringstream oss;
        oss
            << "Lowest separate-account interest rate entered is "
            << lowest
            << ", but "
            << max_sep_acct_rate
            << " is the lowest rate allowed."
            ;
        return oss.str();
        }

    return "";
}

//============================================================================
std::string Input::RealizeNewLoan()
{
    std::string s = realize_sequence_string
        (*this
        ,NewLoanRealized_
        ,NewLoan
        );
    if(s.size())
        {
        return s;
        }

    if(database_->query<bool>(DB_AllowLoan))
        {
        return "";
        }

    if(!each_equal(NewLoanRealized_, 0.0))
        {
        return "Loans may not be illustrated on this policy form.";
        }

    return "";
}

//============================================================================
std::string Input::RealizeWithdrawal()
{
    std::string s = realize_sequence_string
        (*this
        ,WithdrawalRealized_
        ,Withdrawal
        );
    if(s.size())
        {
        return s;
        }

    int FirstWdMonth;
    database_->query_into(DB_FirstWdMonth, FirstWdMonth);

    if(!database_->query<bool>(DB_AllowWd))
        {
        if(!each_equal(WithdrawalRealized_, 0.0))
            {
            return "Withdrawals may not be illustrated on this policy form.";
            }
        }
    else if(0 != FirstWdMonth)
        {
        int const first_wd_year = outward_quotient(FirstWdMonth, 12);
        auto const first = WithdrawalRealized_.begin();
        auto const nth = first + first_wd_year;
        if(!each_equal(first, nth, 0.0))
            {
            std::ostringstream oss;
            oss
                << "This policy form does not allow withdrawals for the first "
                << FirstWdMonth
                << " months."
                ;
            return oss.str();
            }
        }
    else
        {
        double lowest_allowed_withdrawal = database_->query<double>(DB_MinWd);
        for(auto const& i : WithdrawalRealized_)
            {
            if(0.0 < i.value() && i.value() < lowest_allowed_withdrawal)
                {
                std::ostringstream oss;
                oss
                    << "Minimum withdrawal is "
                    << lowest_allowed_withdrawal
                    << "; "
                    << i
                    << " is too low."
                    ;
                return oss.str();
                }
            }
        }

    return "";
}

//============================================================================
std::string Input::RealizeFlatExtra()
{
// We could enforce a maximum of the monthly equivalent of unity,
// and a minimum of zero; is that worth the bother though?
    std::string s = realize_sequence_string
        (*this
        ,FlatExtraRealized_
        ,FlatExtra
        );
    if(s.size())
        {
        return s;
        }

    if(database_->query<bool>(DB_AllowFlatExtras))
        {
        return "";
        }

    if(!each_equal(FlatExtraRealized_, 0.0))
        {
        return "Flat extras not permitted.";
        }

    return "";
}

//============================================================================
std::string Input::RealizeHoneymoonValueSpread()
{
    return realize_sequence_string
        (*this
        ,HoneymoonValueSpreadRealized_
        ,HoneymoonValueSpread
        );
}

//============================================================================
std::string Input::RealizeAmountsPaidHistory()
{
    return realize_sequence_string
        (*this
        ,AmountsPaidHistoryRealized_
        ,Inforce7702AAmountsPaidHistory
        );
}

/// Determine whether specamt must be overwritten with history.
///
/// 'SpecifiedAmount' gives values for all policy years since issue,
/// so 'SpecamtHistory' ought never to been created. Given that it did
/// exist, it ought to have included only a subset of the values given
/// by 'SpecifiedAmount'; but some extracts provide only a scalar for
/// 'SpecifiedAmount', which must therefore be overwritten with the
/// contents of the obsolete history entity. A warning is given if
/// this backward-compatibility measure would lose any additional
/// information given in 'SpecifiedAmount'--e.g., if a user saved an
/// extract after modifying it to change future specamt without
/// copying history into 'SpecifiedAmount'.
///
/// One of these four values is returned:
///  0 'SpecifiedAmount' already matches 'SpecamtHistory' through the
///    inforce as-of date, so 'SpecamtHistory' can be discarded.
///  1 'SpecifiedAmount' matches 'SpecamtHistory' for future durations
///    but not for historical durations, so 'SpecifiedAmount' should
///    be overwritten with 'SpecamtHistory'.
///  2 Otherwise, they're inconsistent, so a warning is displayed. The
///    warning may also be displayed when 'SpecifiedAmount' contains
///    a keyword, in which case consistency is either difficult or
///    impossible to determine.
///  3 An unexpected exception occurred in parsing an argument string;
///    it's trapped, and passed to report_exception().
/// As the function's name implies, it's used as though it returned a
/// boolean, but returning an int facilitates unit testing, as does
/// the last argument.

int Input::must_overwrite_specamt_with_obsolete_history
    (std::string specamt
    ,std::string history
    ,bool        hide_errors
    ) const
{
    std::vector<tnr_unrestricted_double> u;
    std::vector<tnr_unrestricted_double> v;
    std::string su = realize_sequence_string(*this, u, numeric_sequence(specamt));
    std::string sv = realize_sequence_string(*this, v, numeric_sequence(history));
    if(!su.empty() || !sv.empty())
        {
        if(!hide_errors)
            {
            warning()
                << "Possible conflict between specified amount and history."
                << " Merge them manually into the specified-amount field."
                << "\nSpecified amount: " << specamt
                << "\nErrors: '" << su << "'"
                << "\nHistory: " << history
                << "\nErrors: '" << sv << "'"
                << LMI_FLUSH
                ;
            }
        return 2;
        }

    bool history_differs = false;
    bool future_differs  = false;

    int const years_of_history = InforceYear.value() + (0 != InforceMonth.value());
    LMI_ASSERT(years_of_history <= lmi::ssize(u));
    LMI_ASSERT(years_of_history <= lmi::ssize(v));

    for(int j = 0; j < years_of_history; ++j)
        {
        if(u[j] != v[j])
            {
            history_differs = true;
            break;
            }
        }
    for(int j = years_of_history; j < years_to_maturity(); ++j)
        {
        if(u[j] != v[j])
            {
            future_differs = true;
            break;
            }
        }
    if(history_differs && future_differs && !hide_errors)
        {
        warning()
            << "Possible conflict between specified amount and history."
            << " Merge them manually into the specified-amount field."
            << "\nSpecified amount: " << specamt
            << "\nHistory: " << history
            << LMI_FLUSH
            ;
        }
    return history_differs + (history_differs && future_differs);
}

// TODO ?? More attention could be paid to term-rider rounding.
// This would be preferable:
//
// #include "data_directory.hpp" // AddDataDir(), needed to access product data.
// #include "product_data.hpp"   // Product data, needed to access rounding rules.
// #include "rounding_rules.hpp" // Rounding.
//
//        term_spec_amt = rounding_rules
//            (AddDataDir(product_data(ProductName).GetRoundingFilename())
//            ).round_specamt()(term_spec_amt)
//            ;
//
// except that it wouldn't work on the antediluvian branch.

namespace
{
round_to<double> const& specamt_rounder()
{
    static round_to<double> z(0, r_upward);
    return z;
}
} // Unnamed namespace.

/// Special handling for proportional term rider.
///
/// See the reference in AccountValue::ChangeSpecAmtBy() to issues
/// that affect this function as well.
///
/// Term rider can be specified either as a scalar or as a proportion
/// of a given aggregate (base + term) specified amount. In the latter
/// case, base specified amount generally needs to be adjusted here to
/// conserve the aggregate amount. However, transforming values from
/// one formula to the other and back:
///   aggregate = base + term
/// vs.
///   base = (1-P) * aggregate
///   term =    P  * aggregate
/// does not necessarily preserve the original values, due to rounding
/// and enforcement of contractual minimums. The legacy system from
/// which this code descends attempted to avoid that issue by passing
/// a 'false' argument when it needed to display a proportional term
/// specified amount while graying out and not changing the control
/// depicting the base specified amount.
///
/// Probably it is desirable to update and display these resultant
/// amounts within the GUI. The present implementation doesn't do so,
/// because:
///  - A call to RealizeSpecifiedAmount() would be required. Probably
///    every input sequence should be "realized" as its corresponding
///    control loses focus. That would let exceptions be trapped and
///    error messages displayed without leaving the dialog, but it's a
///    potentially slow operation that should be performed only when
///    actually necessary. Once that's done, this function could be
///    called by Input::DoTransmogrify() without impairing the GUI's
///    responsiveness.
///  - It's not obvious that the legacy implementation was ideal. For
///    example, in the "proportionate" case, it constrained both term
///    and base to be scalar, because the aggregate specified amount
///    was a scalar field.
/// A future implementation should probably either:
///  - add a sequence field for the aggregate specified amount; or
///  - overload the existing specified-amount field to mean aggregate
///    iff mce_yes == TermRiderUseProportion.

void Input::make_term_rider_consistent(bool aggressively)
{
    LMI_ASSERT(!SpecifiedAmountRealized_.empty());
    if(mce_no == TermRider)
        {
        return;
        }

    if(mce_no == TermRiderUseProportion)
        {
        double term_spec_amt   = TermRiderAmount.value();
        double base_spec_amt   = SpecifiedAmountRealized_[0].value();
        double total_spec_amt  = term_spec_amt + base_spec_amt;
        double term_proportion = 0.0;
        if(0.0 != total_spec_amt)
            {
            term_proportion = term_spec_amt / total_spec_amt;
            }

        TotalSpecifiedAmount = total_spec_amt;
        TermRiderProportion = term_proportion;
        }
    else if(mce_yes == TermRiderUseProportion)
        {
        double total_spec_amt  = TotalSpecifiedAmount.value();
        double term_proportion = TermRiderProportion.value();
        double term_spec_amt   = total_spec_amt * term_proportion;
        term_spec_amt = specamt_rounder()(term_spec_amt);
        TermRiderAmount = term_spec_amt;

        if(aggressively)
            {
            double base_spec_amt = total_spec_amt - term_spec_amt;
            SpecifiedAmount = value_cast<std::string>(base_spec_amt);
            RealizeSpecifiedAmount();
            }
        }
    else
        {
        alarum() << "Term is neither proportional nor absolute." << LMI_FLUSH;
        }

    if
        (  mce_adjust_base != TermAdjustmentMethod
        && 0.0 != TermRiderAmount.value()
        && !global_settings::instance().ash_nazg()
        && !global_settings::instance().regression_testing()
        )
        {
        alarum()
            << "Method '"
            << TermAdjustmentMethod.str()
            << "' is unreliable."
            << LMI_FLUSH
            ;
        }
}
