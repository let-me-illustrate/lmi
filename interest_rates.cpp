// Interest rates.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: interest_rates.cpp,v 1.24 2008-07-23 12:50:09 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "interest_rates.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "math_functors.hpp"
#include "miscellany.hpp" // each_equal()
#include "rounding_rules.hpp"
#include "yare_input.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <utility>
#include <vector>

// TODO ?? Future enhancements:
//
// Use PETE instead of complex, slow std::transform() expressions.
//
// Permit input gross rates.
//
// Permit variable loan rate.
//
// Calculate all 7702 and 7702A interest rates here.
//
// The InterestRates(BasicValues) ctor requires many headers; improve
// physical design by replacing it with a ctor that takes all required
// data as arguments. That redesign is more conducive to unit tests,
// which should be added.
//
// Redesign DynamicMlySepAcctRate(), which is the only member function
// that is not const; then rewrite clients to use only const objects
// of this class.

namespace
{
// Some years have 366 days: but read the documentation below.

    int const days_per_year = 365;

// Transform annual gross rate to net, subtracting spread and
// investment management fee either on an effective annual or a
// nominal daily basis. The nominal daily method the spread as a
// daily rate and the investment management fee as a daily deduction:
//   net_upper_365 = gross_upper_365 - spread_upper_365 - fee
// or, reading '(365)' as 'upper 365',
//   net(365)/365 = gross(365)/365 - spread(365)/365 - fee/365
// Thus, the investment management fee is treated differently from
// 'spread', which aggregates all other subtrahends.
//
// For both annual-effective and daily-nominal methods, the treatment
// of the investment management fee is a simplification. In actual
// practice, fund managers usually assess a pro-rata fee at the end of
// each calendar quarter, based perhaps on average daily assets.
// Modeling this accounting exactly to produce an annual net rate for
// a policy issued on February first in a leap year would require
// assessing
//   335/366 times the current calendar year's fee, plus
//    31/365 times the next year's fee
// with the probably surprising effect that constant inputs would
// produce a net rate that differs by a few thousands of a basis point
// between these two policy years, assuming a typical fund fee of
// twenty basis points. By choosing to ignore that fine point, this
// implementation gives a monthly rate that can be used to approximate
// daily interest accounting closely as
//   (1 + i_upper_12 / 12)^(12 * days_in_month / days_in_year) - 1
//
// The result is constrained by a floor that can be used e.g. to
// ensure that no current general-account rate is less than the
// guaranteed rate.
//
// For the annual-effective method, transformation from annual to
// daily and back again by naive methods would lose considerable
// precision even when the spread and fee are zero, because i is
// small relative to (1 + i). That is why expm1l() and log1pl() are
// used instead of pow().
//
// If both spread and fee are zero, then the net rate should exactly
// equal the gross rate. However, those two rates would differ
// slightly if the annual rate were converted to monthly and back
// again, because of limited numerical precision, so this situation
// is treated as a special case. The annual-effective method requires
// this special treatment for accuracy; the daily-nominal method uses
// it only for speed.
//
// The result is not rounded here so that it can be converted
// elsewhere to a monthly rate without loss of precision.
//
double transform_annual_gross_rate_to_annual_net
    (double               annual_gross_rate
    ,double               spread
    ,mcenum_spread_method spread_method
    ,double               floor
    ,double               fee
    )
{
    double i = annual_gross_rate;
    if(0.0 == spread && 0.0 == fee)
        {
        i = annual_gross_rate;
        }
    else if(mce_spread_is_effective_annual == spread_method)
        {
        i = annual_gross_rate - spread - fee;
        }
    else if(mce_spread_is_nominal_daily == spread_method)
        {
        i = net_i_from_gross<double,days_per_year>()
            (annual_gross_rate
            ,spread
            ,fee
            );
        }
    else
        {
        fatal_error() << "No " << spread_method << " case." << LMI_FLUSH;
        }
    return std::max(floor, i);
}

// Transform vector of annual gross interest rates to monthly net, for
// a single moment only. Use the optimized vector version instead
// wherever possible.
void convert_interest_rates
    (double                  annual_gross_rate
    ,double                & annual_net_rate
    ,double                & monthly_net_rate
    ,round_to<double> const& round_interest_rate
    ,double                  spread
    ,mcenum_spread_method    spread_method
    ,double                  floor
    ,double                  fee
    )
{
    annual_net_rate = transform_annual_gross_rate_to_annual_net
        (annual_gross_rate
        ,spread
        ,spread_method
        ,floor
        ,fee
        );
    monthly_net_rate = i_upper_12_over_12_from_i<double>()(annual_net_rate);
    annual_net_rate  = round_interest_rate(annual_net_rate );
    monthly_net_rate = round_interest_rate(monthly_net_rate);
}

// Transform vector of annual gross interest rates to monthly net.
// Often the rates are the same from one year to the next; when that
// happens, we just replicate the previous value in order to avoid
// costly floating point calculations. The investment management fee
// is a scalar because that seems to be the universal practice.
void convert_interest_rates
    (std::vector<double> const& annual_gross_rate
    ,std::vector<double>      & annual_net_rate
    ,std::vector<double>      & monthly_net_rate
    ,round_to<double>    const& round_interest_rate
    ,std::vector<double> const& spread
    ,mcenum_spread_method       spread_method
    ,std::vector<double> const& floor
    ,double                     fee
    )
{
    unsigned int const length = annual_gross_rate.size();
    annual_net_rate .resize(length);
    monthly_net_rate.resize(length);

    double cached_annual_net_rate;
    double cached_monthly_net_rate;

    double previous_annual_gross_rate = 0.0;
    double previous_spread            = 0.0;
    double previous_floor             = 0.0;
    for(unsigned int j = 0; j < length; j++)
        {
        if
            (
                previous_annual_gross_rate != annual_gross_rate[j]
            ||  previous_spread            != spread           [j]
            ||  previous_floor             != floor            [j]
            )
            {
            previous_annual_gross_rate = annual_gross_rate[j];
            previous_spread            = spread           [j];
            previous_floor             = floor            [j];
            convert_interest_rates
                (annual_gross_rate[j]
                ,cached_annual_net_rate
                ,cached_monthly_net_rate
                ,round_interest_rate
                ,spread           [j]
                ,spread_method
                ,floor            [j]
                ,fee
                );
            }
        annual_net_rate [j] = cached_annual_net_rate;
        monthly_net_rate[j] = cached_monthly_net_rate;
        }
}

/// Determine whether loan rates are needed; else they can be zero.
///
/// Loan rates can potentially affect GPT calculations.

bool need_loan_rates(yare_input const& yi)
{
    return
            mce_gpt                == yi.DefinitionOfLifeInsurance
        ||  mce_solve_loan         == yi.SolveType
        ||  mce_solve_wd_then_loan == yi.SolveType
        ||  true                   == yi.WithdrawToBasisThenLoan
        ||  0.0                    != yi.InforceRegularLoanValue
        ||  0.0                    != yi.InforcePreferredLoanValue
        ||  0.0                    != yi.InforceRegularLoanBalance
        ||  0.0                    != yi.InforcePreferredLoanBalance
        ||  !each_equal(yi.NewLoan.begin(), yi.NewLoan.end(), 0.0)
        ;
}
} // Unnamed namespace.

InterestRates::~InterestRates()
{
}

InterestRates::InterestRates(BasicValues const& v)
    :Length_             (v.GetLength())
    ,RoundIntRate_       (v.GetRoundingRules().round_interest_rate())
    ,Round7702Rate_      (v.GetRoundingRules().round_interest_rate_7702())
    ,Zero_               (Length_)
    ,NeedMidpointRates_  (v.IsSubjectToIllustrationReg())
    ,GenAcctRateType_    (v.yare_input_.GeneralAccountRateType)
    ,NeedSepAcctRates_   (v.Database_->Query(DB_AllowSepAcct))
    ,SepAcctRateType_    (v.yare_input_.SeparateAccountRateType)
    ,SepAcctSpreadMethod_(static_cast<mcenum_spread_method>(static_cast<int>(v.Database_->Query(DB_SepAcctSpreadMethod))))
    ,AmortLoad_          (Zero_)
    ,ExtraSepAcctCharge_ (Zero_)
//    ,NeedLoanRates_      (need_loan_rates(v.yare_input_))
    ,NeedLoanRates_      (true) // DEPRECATED
    ,LoanRateType_       (v.yare_input_.LoanRateType)
    ,NeedPrefLoanRates_  (v.Database_->Query(DB_AllowPrefLoan))
    ,NeedHoneymoonRates_ (v.yare_input_.HoneymoonEndorsement)
    ,SpreadFor7702_      (v.SpreadFor7702())
{
    Initialize(v);
}

void InterestRates::Initialize(BasicValues const& v)
{
    // Retrieve general-account data from class BasicValues.

    v.Database_->Query(GenAcctGrossRate_[mce_gen_guar], DB_GuarInt);

    std::copy
        (v.yare_input_.GeneralAccountRate.begin()
        ,v.yare_input_.GeneralAccountRate.end()
        ,std::back_inserter(GenAcctGrossRate_[mce_gen_curr])
        );
    // TODO ?? At least for the antediluvian branch, the vector in
    // the input class has an inappropriate size.
    GenAcctGrossRate_[mce_gen_curr].resize(Length_);

    // General-account interest bonus implemented only as a simple
    // additive adjustment to the annual effective rate. It probably
    // makes no sense to add it to the monthly rate and derive an
    // annual rate from that: this affects a declared rate that is
    // almost certainly quoted as an APR. It is assumed that the
    // interest bonus is not guaranteed.
    std::vector<double> general_account_interest_bonus;
    v.Database_->Query(general_account_interest_bonus, DB_GAIntBonus);
    // ET !! GenAcctGrossRate_ += general_account_interest_bonus;
    // ...and this might be further simplified by implementing e.g.
    //   std::vector<double> TDatabase::QueryVector(int k) const;
    // and replacing 'general_account_interest_bonus' with a
    // temporary:
    //   GenAcctGrossRate_ += v.Database_->QueryVector(DB_GAIntBonus);
    std::transform
        (GenAcctGrossRate_[mce_gen_curr].begin()
        ,GenAcctGrossRate_[mce_gen_curr].end()
        ,general_account_interest_bonus.begin()
        ,GenAcctGrossRate_[mce_gen_curr].begin()
        ,std::plus<double>()
        );

    v.Database_->Query(GenAcctSpread_, DB_CurrIntSpread);

    // Retrieve separate-account data from class BasicValues.

    std::copy
        (v.yare_input_.SeparateAccountRate.begin()
        ,v.yare_input_.SeparateAccountRate.end()
        ,std::back_inserter(SepAcctGrossRate_[mce_sep_full])
        );
    // TODO ?? At least for the antediluvian branch, the vector in
    // the input class has an inappropriate size.
    SepAcctGrossRate_[mce_sep_full].resize(Length_);

    v.Database_->Query(MAndERate_[mce_gen_guar], DB_GuarMandE          );
    v.Database_->Query(MAndERate_[mce_gen_curr], DB_CurrMandE          );

    v.Database_->Query(Stabilizer_,              DB_StableValFundCharge);

    // Deduct miscellaneous fund charges and input extra asset comp in
    // the same way as M&E, iff database entity DB_AssetChargeType has
    // the value 'oe_asset_charge_spread'; otherwise, reflect them
    // elsewhere as an account-value load.
    if(oe_asset_charge_spread == v.Database_->Query(DB_AssetChargeType))
        {
        // TODO ?? At least for the antediluvian branch, the vector in
        // the input class has an inappropriate size. Truncating it
        // with std::transform() here is far too tricky.
        LMI_ASSERT
            (   ExtraSepAcctCharge_.size()
            ==  static_cast<unsigned int>(v.Database_->length())
            );
// Not reliably true:
//        LMI_ASSERT
//            (   ExtraSepAcctCharge_.size()
//            ==  v.yare_input_.ExtraCompensationOnAssets.size()
//            );
        // ET !! ExtraSepAcctCharge_ += v.yare_input_.ExtraCompensationOnAssets;
        // ...but see the problem noted above.
        std::transform
            (ExtraSepAcctCharge_.begin()
            ,ExtraSepAcctCharge_.end()
            ,v.yare_input_.ExtraCompensationOnAssets.begin()
            ,ExtraSepAcctCharge_.begin()
            ,std::plus<double>()
            );
        }

    if(v.yare_input_.AmortizePremiumLoad)
        {
        v.Database_->Query(AmortLoad_, DB_AmortPmLdFundCharge);
        }

    // TODO ?? This was once initialized with 'DB_MgmtFeeFundCharge',
    // which has been deprecated and is ignored. Is it still useful?
    // useful? Here, this vector is simply initialized with a scalar,
    // but later it adds rates passed to DynamicMlySepAcctRate(), which
    // seems ugly. Is it ever accessed externally? Does it really need
    // to be a vector?
    InvestmentManagementFee_.assign(Length_, v.InvestmentManagementFee());

    // Retrieve loan data from class BasicValues.

    switch(LoanRateType_)
        {
        case mce_fixed_loan_rate:
            {
            v.Database_->Query(PublishedLoanRate_, DB_FixedLoanRate);
            }
            break;
        case mce_variable_loan_rate:
            {
            PublishedLoanRate_.assign(Length_, v.yare_input_.LoanRate);
            }
            break;
        default:
            {
            fatal_error() << "No " << LoanRateType_ << " case." << LMI_FLUSH;
            }
        }

    v.Database_->Query(RegLoanSpread_[mce_gen_guar], DB_GuarRegLoanSpread);
    v.Database_->Query(RegLoanSpread_[mce_gen_curr], DB_CurrRegLoanSpread);
    v.Database_->Query(PrfLoanSpread_[mce_gen_guar], DB_GuarPrefLoanSpread);
    v.Database_->Query(PrfLoanSpread_[mce_gen_curr], DB_CurrPrefLoanSpread);

    if(NeedHoneymoonRates_)
        {
        HoneymoonValueSpread_ = v.yare_input_.HoneymoonValueSpread;
        PostHoneymoonSpread_.assign
            (Length_
            ,v.yare_input_.PostHoneymoonSpread
            );
        }

    // Convert interest rates and test.

    InitializeGeneralAccountRates();
    InitializeSeparateAccountRates();
    InitializeLoanRates();
    InitializeHoneymoonRates();
    Initialize7702Rates();

    // Paranoid check.
    unsigned int z = static_cast<unsigned int>(Length_);
    for(int i = mce_annual_rate; i < mc_n_rate_periods; i++)
        {
        for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
            {
            // The next line gets executed more than once with
            // identical semantics, but it's cheap, and writing it
            // to avoid that little problem would make it unclear.
            LMI_ASSERT(z == GenAcctGrossRate_           [j]   .size());
            LMI_ASSERT(z == GenAcctNetRate_          [i][j]   .size());
            for(int k = mce_sep_full; k < mc_n_sep_bases; k++)
                {
                LMI_ASSERT(z == SepAcctGrossRate_          [k].size());
                LMI_ASSERT(z == SepAcctNetRate_      [i][j][k].size());
                }
            LMI_ASSERT(z == RegLnCredRate_           [i][j]   .size());
            LMI_ASSERT(z == RegLnDueRate_            [i][j]   .size());
            LMI_ASSERT(z == PrfLnCredRate_           [i][j]   .size());
            LMI_ASSERT(z == PrfLnDueRate_            [i][j]   .size());
            LMI_ASSERT(z == HoneymoonValueRate_      [i][j]   .size());
            LMI_ASSERT(z == PostHoneymoonGenAcctRate_[i][j]   .size());
            }
        }
}

void InterestRates::InitializeGeneralAccountRates()
{
    std::vector<double> spread[mc_n_gen_bases] = {Zero_, Zero_, Zero_};
    if(mce_gross_rate == GenAcctRateType_)
        {
        fatal_error()
            << "General-account rate is unexpectedly gross."
            << LMI_FLUSH
            ;
        spread[mce_gen_curr] = GenAcctSpread_;
        // ET !! spread[mce_gen_curr] -= spread[mce_gen_curr][0];
        std::transform
            (spread[mce_gen_curr].begin()
            ,spread[mce_gen_curr].end()
            ,spread[mce_gen_curr].begin()
            ,std::bind2nd(std::minus<double>(), spread[mce_gen_curr].front())
            );
        // ET !! spread[mce_gen_mdpt] = 0.5 * spread[mce_gen_curr];
        // ...but writing it that way makes it look wrong.
        std::transform
            (spread[mce_gen_curr].begin()
            ,spread[mce_gen_curr].end()
            ,spread[mce_gen_mdpt].begin()
            ,std::bind1st(std::multiplies<double>(), 0.5)
            );
        }
    else
        {
        LMI_ASSERT(mce_net_rate == GenAcctRateType_);
        }

    GenAcctGrossRate_[mce_gen_mdpt] = Zero_;
    if(NeedMidpointRates_)
        {
        // ET !! GenAcctGrossRate_[mce_gen_mdpt] = mean(GenAcctGrossRate_[mce_gen_guar], GenAcctGrossRate_[mce_gen_curr]);
        std::transform
            (GenAcctGrossRate_[mce_gen_guar].begin()
            ,GenAcctGrossRate_[mce_gen_guar].end()
            ,GenAcctGrossRate_[mce_gen_curr].begin()
            ,GenAcctGrossRate_[mce_gen_mdpt].begin()
            ,mean<double>()
            );
        }
    else
        {
        GenAcctNetRate_[mce_annual_rate ][mce_gen_mdpt] = Zero_;
        GenAcctNetRate_[mce_monthly_rate][mce_gen_mdpt] = Zero_;
        }

    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        convert_interest_rates
            (GenAcctGrossRate_[j]
            ,GenAcctNetRate_[mce_annual_rate ][j]
            ,GenAcctNetRate_[mce_monthly_rate][j]
            ,RoundIntRate_
            ,spread[j]
            ,mce_spread_is_effective_annual
            ,GenAcctGrossRate_[mce_gen_guar]
            ,0.0
            );
        }
}

void InterestRates::InitializeSeparateAccountRates()
{
    SepAcctFloor_.assign(Length_, -.999999999999);
    if(!NeedSepAcctRates_)
        {
        for(int i = mce_annual_rate; i < mc_n_rate_periods; i++)
            {
            for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
                {
                SepAcctGrossRate_[mce_sep_zero] = Zero_;
                SepAcctGrossRate_[mce_sep_half] = Zero_;
                for(int k = mce_sep_full; k < mc_n_sep_bases; k++)
                    {
                    SepAcctNetRate_[i][j][k] = Zero_;
                    }
                }
            }
        return;
        }

// TODO ?? Are tiered M&E, IMF, comp treated correctly?

    std::vector<double> miscellaneous_charges(Stabilizer_);
// TODO ?? Replace these long lines with PETE expressions.
    // ET !! miscellaneous_charges += AmortLoad_ + ExtraSepAcctCharge_;
    std::transform(miscellaneous_charges.begin(), miscellaneous_charges.end(), AmortLoad_         .begin(), miscellaneous_charges.begin(), std::plus<double>());
    std::transform(miscellaneous_charges.begin(), miscellaneous_charges.end(), ExtraSepAcctCharge_.begin(), miscellaneous_charges.begin(), std::plus<double>());

    std::vector<double> total_charges[mc_n_gen_bases];
    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        if(mce_gen_mdpt == j)
            {
            continue;
            }
        // ET !! total_charges[j] = MAndERate_[j] + miscellaneous_charges;
        total_charges[j].reserve(Length_);
        std::transform
            (MAndERate_[j].begin()
            ,MAndERate_[j].end()
            ,miscellaneous_charges.begin()
            ,std::back_inserter(total_charges[j])
            ,std::plus<double>()
            );
        }

    double fee = InvestmentManagementFee_[0];

    // Take input scalar net rate as indicating a scalar gross rate
    // minus the first-year charges. If the charges aren't level, then
    // neither is the implicit net rate.
    if(mce_net_rate == SepAcctRateType_)
        {
        fatal_error()
            << "Separate-account rate is unexpectedly net."
            << LMI_FLUSH
            ;
        for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
            {
            if(mce_gen_mdpt == j)
                {
                continue;
                }
            // ET !! total_charges[j] -= total_charges[j][0];
            std::transform
                (total_charges[j].begin()
                ,total_charges[j].end()
                ,total_charges[j].begin()
                ,std::bind2nd(std::minus<double>(), total_charges[j].front())
                );
            }
        fee = 0.0;
        }
    else
        {
        LMI_ASSERT(mce_gross_rate == SepAcctRateType_);
        }

    SepAcctGrossRate_[mce_sep_zero] = Zero_;
    // ET !! SepAcctGrossRate_[mce_sep_half] = 0.5 * SepAcctGrossRate_[mce_sep_full];
    std::transform
        (SepAcctGrossRate_[mce_sep_full].begin()
        ,SepAcctGrossRate_[mce_sep_full].end()
        ,std::back_inserter(SepAcctGrossRate_[mce_sep_half])
        ,std::bind1st(std::multiplies<double>(), 0.5)
        );

    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        for(int k = mce_sep_full; k < mc_n_sep_bases; k++)
            {
            if(mce_gen_mdpt == j)
                {
                SepAcctNetRate_[mce_annual_rate ][j][k] = Zero_;
                SepAcctNetRate_[mce_monthly_rate][j][k] = Zero_;
                continue;
                }
            convert_interest_rates
                (SepAcctGrossRate_[k]
                ,SepAcctNetRate_[mce_annual_rate ][j][k]
                ,SepAcctNetRate_[mce_monthly_rate][j][k]
                ,RoundIntRate_
                ,total_charges[j]
                ,SepAcctSpreadMethod_
                ,SepAcctFloor_
                ,fee
                );
            }
        }
}

void InterestRates::InitializeLoanRates()
{
    if(!NeedLoanRates_)
        {
        for(int i = mce_annual_rate; i < mc_n_rate_periods; i++)
            {
            for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
                {
                RegLnCredRate_[i][j] = Zero_;
                RegLnDueRate_ [i][j] = Zero_;
                PrfLnCredRate_[i][j] = Zero_;
                PrfLnDueRate_ [i][j] = Zero_;
                }
            }
        return;
        }

    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        RegLnDueRate_[mce_annual_rate][j] = PublishedLoanRate_;
        PrfLnDueRate_[mce_annual_rate][j] = PublishedLoanRate_;
        }

    RegLoanSpread_[mce_gen_mdpt] = Zero_;
    PrfLoanSpread_[mce_gen_mdpt] = Zero_;
    if(NeedMidpointRates_)
        {
        // ET !! RegLoanSpread_[mce_gen_mdpt] = mean(RegLoanSpread_[mce_gen_guar], RegLoanSpread_[mce_gen_curr]);
        std::transform
            (RegLoanSpread_[mce_gen_guar].begin()
            ,RegLoanSpread_[mce_gen_guar].end()
            ,RegLoanSpread_[mce_gen_curr].begin()
            ,RegLoanSpread_[mce_gen_mdpt].begin()
            ,mean<double>()
            );
        // ET !! PrfLoanSpread_[mce_gen_mdpt] = mean(PrfLoanSpread_[mce_gen_guar], PrfLoanSpread_[mce_gen_curr]);
        std::transform
            (PrfLoanSpread_[mce_gen_guar].begin()
            ,PrfLoanSpread_[mce_gen_guar].end()
            ,PrfLoanSpread_[mce_gen_curr].begin()
            ,PrfLoanSpread_[mce_gen_mdpt].begin()
            ,mean<double>()
            );
        }

    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        convert_interest_rates
            (RegLnDueRate_[mce_annual_rate ][j]
            ,RegLnDueRate_[mce_annual_rate ][j]
            ,RegLnDueRate_[mce_monthly_rate][j]
            ,RoundIntRate_
            ,Zero_
            ,mce_spread_is_effective_annual
            ,GenAcctGrossRate_[mce_gen_guar]
            ,0.0
            );
        convert_interest_rates
            (RegLnDueRate_ [mce_annual_rate ][j]
            ,RegLnCredRate_[mce_annual_rate ][j]
            ,RegLnCredRate_[mce_monthly_rate][j]
            ,RoundIntRate_
            ,RegLoanSpread_[j]
            ,mce_spread_is_effective_annual
            ,GenAcctGrossRate_[mce_gen_guar]
            ,0.0
            );
        if(NeedPrefLoanRates_)
            {
            fatal_error() << "Preferred loans not implemented." << LMI_FLUSH;
            convert_interest_rates
                (PrfLnDueRate_[mce_annual_rate ][j]
                ,PrfLnDueRate_[mce_annual_rate ][j]
                ,PrfLnDueRate_[mce_monthly_rate][j]
                ,RoundIntRate_
                ,PrfLoanSpread_[j]
                ,mce_spread_is_effective_annual
                ,Zero_
                ,0.0
                );
            convert_interest_rates
                (PrfLnDueRate_ [mce_annual_rate ][j]
                ,PrfLnCredRate_[mce_annual_rate ][j]
                ,PrfLnCredRate_[mce_monthly_rate][j]
                ,RoundIntRate_
                ,PrfLoanSpread_[j]
                ,mce_spread_is_effective_annual
                ,GenAcctGrossRate_[mce_gen_guar]
                ,0.0
                );
            }
        else
            {
            PrfLnDueRate_ [mce_annual_rate ][j] = Zero_;
            PrfLnDueRate_ [mce_monthly_rate][j] = Zero_;
            PrfLnCredRate_[mce_annual_rate ][j] = Zero_;
            PrfLnCredRate_[mce_monthly_rate][j] = Zero_;
            }
        }

        // RegLnDueRate_ cannot vary by basis for fixed loan rate, but
        // may for VLR--in which case these assertions will fire,
        // indicating that this code should be reviewed.
        //
        LMI_ASSERT
            (   RegLnDueRate_[mce_annual_rate ][mce_gen_guar]
            ==  RegLnDueRate_[mce_annual_rate ][mce_gen_curr]
            );
        LMI_ASSERT
            (   RegLnDueRate_[mce_monthly_rate][mce_gen_guar]
            ==  RegLnDueRate_[mce_monthly_rate][mce_gen_curr]
            );
}

void InterestRates::InitializeHoneymoonRates()
{
    if(!NeedHoneymoonRates_)
        {
        for(int i = mce_annual_rate; i < mc_n_rate_periods; i++)
            {
            for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
                {
                HoneymoonValueRate_      [i][j] = Zero_;
                PostHoneymoonGenAcctRate_[i][j] = Zero_;
                }
            }
        return;
        }

    // Honeymoon rates are taken as annual effective rates only,
    // because they are declared rates and therefore APRs.
    //
    // TODO ?? Someday, after we've implemented and tested the
    // alternative for the general account rate, we can aspire to
    // implement it for honeymoon rates too.
    LMI_ASSERT(mce_net_rate == GenAcctRateType_);

    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        convert_interest_rates
            (GenAcctNetRate_    [mce_annual_rate ][j]
            ,HoneymoonValueRate_[mce_annual_rate ][j]
            ,HoneymoonValueRate_[mce_monthly_rate][j]
            ,RoundIntRate_
            ,HoneymoonValueSpread_
            ,mce_spread_is_effective_annual
            ,Zero_
            ,0.0
            );
        convert_interest_rates
            (GenAcctNetRate_          [mce_annual_rate ][j]
            ,PostHoneymoonGenAcctRate_[mce_annual_rate ][j]
            ,PostHoneymoonGenAcctRate_[mce_monthly_rate][j]
            ,RoundIntRate_
            ,PostHoneymoonSpread_
            ,mce_spread_is_effective_annual
            ,GenAcctGrossRate_[mce_gen_guar]
            ,0.0
            );
        }
}

// TODO ?? Still needs a bit of work.
//
// When the M&E charge depends on monthly total case assets, the
// separate-account rate is no longer an annual invariant and must be
// recalculated here each month. There is no corresponding general-
// account adjustment because we don't anticipate needing it, though
// perhaps that is shortsighted.
//
// At entry, non-const reference inputs hold tiered annual values.
// This function adds non-tiered complements to each of these values,
// except that (bogusly) it adds the tiered IMF into the non-tiered
// IMF held in this class and doesn't add non-tiered M&E to tiered M&E.
void InterestRates::DynamicMlySepAcctRate
    (mcenum_gen_basis gen_basis
    ,mcenum_sep_basis sep_basis
    ,int              year
    ,double&          MonthlySepAcctGrossRate
    ,double&          AnnualSepAcctMandERate
    ,double&          AnnualSepAcctIMFRate
    ,double&          AnnualSepAcctMiscChargeRate
    ,double&          AnnualSepAcctSVRate
    )
{
//    AnnualSepAcctIMFRate    += TieredInvestmentManagementFee_[year]; // TODO ?? BOGUS
    InvestmentManagementFee_[year] += AnnualSepAcctIMFRate;
    AnnualSepAcctMiscChargeRate    += ExtraSepAcctCharge_    [year];
    AnnualSepAcctSVRate            += Stabilizer_            [year];
// TODO ?? Reference argument 'AnnualSepAcctMandERate' is not modified.
// Shouldn't it be?

    double dynamic_spread =
            AnnualSepAcctMandERate
        +   AnnualSepAcctSVRate
        +   AmortLoad_[year]
        +   AnnualSepAcctMiscChargeRate
        ;

    switch(SepAcctRateType_)
        {
        case mce_gross_rate:
            {
            if(mce_gen_mdpt == gen_basis)
                {
                fatal_error()
                    << "Midpoint separate-account rate not supported."
                    << LMI_FLUSH
                    ;
                }
// TODO ?? Is what follows appropriate for both current and guaranteed
// bases?

// TODO ?? What if it's not 'full'--what if we want 'half' or 'zero'?
            MonthlySepAcctGrossRate = i_upper_12_over_12_from_i<double>()
                (SepAcctGrossRate_[mce_sep_full][year]
                );

            convert_interest_rates
                (SepAcctGrossRate_[sep_basis][year]
                ,SepAcctNetRate_[mce_annual_rate ][gen_basis][sep_basis][year]
                ,SepAcctNetRate_[mce_monthly_rate][gen_basis][sep_basis][year]
                ,RoundIntRate_
                ,dynamic_spread
                ,SepAcctSpreadMethod_
                ,SepAcctFloor_[year]
                ,InvestmentManagementFee_[year]
                );
            }
            break;
        case mce_net_rate:
            {
            fatal_error() << "Net rate not supported." << LMI_FLUSH;
            }
            break;
        default:
            {
            fatal_error() << "No " << SepAcctRateType_ << " case." << LMI_FLUSH;
            }
        }
}

void InterestRates::Initialize7702Rates()
{

// �7702 prescribes the interest basis for all �7702 and �7702A
// calculations as the interest rate actually guaranteed in the
// contract, or a statutory rate if greater. The statutory rate is 4%
// for GLP and 6% for GSP. It is 4% for all �7702A calculations,
// except that the necessary premium for guideline contracts is
// defined in terms of the guideline limit.

// The �7702 net rate is determined in two steps. First, the
// guaranteed interest rate is determined from the contract, and the
// statutory rate is used instead if it is greater. This operation is
// performed separately for all periods with different guaranteed
// rates [DEFRA Blue Book, page 648]. For example, if the guaranteed
// rate is 4.5% for five years and 3.5% thereafter, then the GLP
// interest rate is 4.5% for five years and 4.0% thereafter, while the
// GSP rate is always 6.0%. For products such as pure variable UL that
// offer no explicit guarantee, the statutory rate is used. For
// variable products that offer a general account option, the
// guaranteed gross rate must be no less than the general account
// guaranteed rate.

// Even short-term guarantees at issue must be reflected in the GSP,
// the CVAT NSP, and the �7702A NSP, seven-pay premium, and DCV. They
// may be ignored as de minimis in calculating the �7702 GLP [DEFRA
// Blue Book, page 649], but only as long as they last no longer than
// one year. Only guarantees that either last longer than one year or
// are present on the issue date are taken into account: a guarantee
// subsequently added for a future period lasting no longer than one
// year is a dividend, not an adjustment event. Here, "issue" excludes
// cases where the contract is merely deemed by statute to be reissued
// [for example, by �7702A(c)(3)(A)(i)].

// Second, any current asset based charges specified in the contract
// are deducted if we wish. The interest rate remains what it is; the
// net rate that results from subtracting asset-based charges is
// merely a computational convenience that simplifies the formulas.
// In fact, the full interest rate (never less than statutory) is
// credited, and then asset based charges are subtracted from the
// account value. Therefore, this adjustment affects only the �7702
// guideline premiums and the �7702 DCV, because those quantities
// reflect expenses. It must not be taken into account when
// calculating the �7702 CVAT NSP or CVAT corridor factors, or the
// �7702A NSP or seven-pay premium, because those quantities do not
// reflect expenses.

// Asset based charges can be deducted only if they are specified in
// the contract itself: charges imposed by separate accounts cannot be
// deducted unless they are specified in the life insurance contract
// proper, since any charge not so specified is deemed to be zero
// [�7702(c)(3)(D)(i)]. They also must not exceed the charges
// reasonably expected to be actually imposed [�7702(c)(3)(B)(ii)].
// If the schedule page announces a charge of "up to 100 basis points"
// and we actually charge 50 bp and expect to keep charging that, then
// we can use 50 bp; but if we ever charge less than 50 bp, an
// adjustment event results.

// It is critical that the result be rounded up if at all, and never
// rounded down or truncated. The GPT is a bright-line test, and
// truncation at, say, eight decimal places may have an effect of more
// than a dollar per thousand [which would not be a "reasonable"
// approximation: DEFRA Blue Book, page 653] at a later duration.
// Special attention must be paid to the exact method the
// administration system uses (e.g. beginning of period versus end of
// period), to be sure that the resulting charge is what will actually
// be imposed. A �7702(f)(8) waiver granted in one actual case that
// was pennies over the limit cost tens of thousands of dollars in
// filing and attorney's fees.

// Thus, an account-value load that is deducted from the account value
// at the beginning of each month, before interest is credited, may be
// reflected in GPT calculations. We could calculate it as a monthly
// load in order to follow the precise contract mechanics, but that
// would require a significant modification of Eckley's formulas,
// which do not contemplate a load on AV. Instead, we net the account
// value load against the �7702 interest rate; as explained above,
// this is a mere computational convenience that does not change the
// actual interest rate.

// On the other hand, it is not clear that a conventional mortality
// and expense charge (M&E) can be reflected, because it is part of
// the daily unit value calculation. The effect of this M&E on monthly
// interest is a function of the ratio of successive unit values, and
// the actual charge approaches zero when the unit values decrease
// quickly. If it were clearly deducted at the beginning of each day,
// before crediting interest, then we might take it into account by
// adding daily commutation functions to the Formulas section. This
// implementation ignores such charges.

// Multiple guaranteed rates may result, for instance in the case of a
// variable contract with a general account option and a distinct
// guarantee for loaned funds. The highest such rate is used, because
// that produces the most conservative guideline premium limits.

// A higher rate guaranteed in a side letter must be reflected as
// described above, as though it were written in the contract. For
// products that guarantee a rate tied to an index, the �7702 interest
// rates in the first guarantee period must be at least as high as the
// rate determined by the index when the contract is issued. Such
// guarantees must be taken into account even if they arise indirectly
// or contingently, for instance in the case of an unloaned credited
// rate that is guaranteed to be no less than 50 bp below an indexed
// loan rate. No product we've implemented provides any such interest
// guarantee except in connection with a variable loan rate. This
// implementation therefore ignores initial guarantees.

// For calculating mortality charges, most UL products discount the
// NAAR for one month's interest at a rate specified in the contract.
// �7702 and �7702A calculations must use the �7702 rate instead
// whenever that is higher than the contractual rate. This affects all
// premium rates and also the CVAT DCV and corridor factors. Whenever
// this rate is converted to a monthly equivalent, the result must be
// rounded up if at all. If the contract specifies no such discount
// and none is actually applied, then a discount rate of zero may be
// used.

// The interest rate guaranteed by the contract is the greater at each
// duration of the guaranteed loan credited rate or the rate otherwise
// guaranteed. If a fixed rate is elected, then the guaranteed loan
// credited rate, if not stated explicitly, is the fixed rate charged
// on loans minus the guaranteed loan spread if any. If the contract
// guarantees neither the loan credited rate nor the loan spread, then
// a fixed loan rate has no �7702 or �7702A effect.

// There is a concern if a variable loan rate (VLR) is elected.
// Section 3.D of the VLR model regulation provides that "the maximum
// rate...must be determined at regular intervals at least once every
// twelve (12) months, but not more frequently than once in any
// three-month period." There is no rate guarantee after the first
// anniversary, because the VLR rate may change by that time. However,
// since the maximum VLR is fixed for at least three months at issue,
// there is a short-term guarantee that must be reflected as explained
// above if the rate actually credited on loans is too high. The
// complications that ensue may be avoided by actually crediting a
// loan rate no higher than �7702 otherwise requires during the first
// loan rate determination period, or simply by forbidding loans
// during that period.

// TODO ?? Calculate both:
//    std::vector<double> MlyGlpRate_;
//    std::vector<double> MlyGspRate_;

    std::vector<double> const& annual_guar_rate = GenAcctGrossRate_[mce_gen_guar];

    MlyGlpRate_.resize(Length_);
    // ET !! MlyGlpRate_ = max(0.04, annual_guar_rate);
    std::transform(annual_guar_rate.begin(), annual_guar_rate.end(), MlyGlpRate_.begin(), std::bind1st(greater_of<double>(), 0.04));
    // ET !! This ought to be implicit, at least in some 'safe' mode:
    LMI_ASSERT(MlyGlpRate_.size() == SpreadFor7702_.size());
    // ET !! MlyGlpRate_ = i_upper_12_over_12_from_i(MlyGlpRate_ - SpreadFor7702_);
    std::transform(MlyGlpRate_.begin(), MlyGlpRate_.end(), SpreadFor7702_.begin(), MlyGlpRate_.begin(), std::minus<double>());
    std::transform(MlyGlpRate_.begin(), MlyGlpRate_.end(), MlyGlpRate_.begin(), i_upper_12_over_12_from_i<double>());
}

#if 0
// TODO ?? Here's the implementation actually used, elsewhere--it needs work.
// Eventually this should be rewritten. It still lives here because it
// really belongs here, not in class BasicValues.

{
    // Monthly guar net int for 7702, with 4 or 6% min, is
    //   greater of {4%, 6%} and annual guar int rate
    //   less 7702 spread
    // TODO ?? We need to subtract other things too, e.g. comp (sometimes)...
    //   transformed to monthly (simple subtraction?).
    // These interest rates belong here because they're used by
    // DCV calculations in the account value class as well as
    // GPT calculations in the 7702 class.

    std::vector<double> guar_int = GenAcctGrossRate_[mce_gen_guar];
/*
    switch(LoanRateType_)
        {
        case mce_fixed_loan_rate:
            {
            // ET !! std::vector<double> guar_loan_rate = PublishedLoanRate_ - RegLoanSpread_[mce_gen_guar];
            // ET !! guar_int = max(guar_int, RegLoanSpread_[mce_gen_guar]);
            // TODO ?? But that looks incorrect when written clearly!
            // Perhaps this old comment:
            //   APL: guar_int gets guar_int max gross_loan_rate - guar_loan_spread
            // suggests the actual intention.

            // TODO ?? Need loan rates for 7702 whenever loans are allowed.
            std::vector<double> gross_loan_rate = PublishedLoanRate_;
            // TODO ?? Should at least assert that preferred <= regular spread.
            std::vector<double> guar_loan_spread = RegLoanSpread_[mce_gen_guar];
            std::vector<double> guar_loan_rate(Length);
            std::transform
                (gross_loan_rate.begin()
                ,gross_loan_rate.end()
                ,guar_loan_spread.begin()
                ,guar_loan_rate.begin()
                ,std::minus<double>()
                );
            std::transform
                (guar_int.begin()
                ,guar_int.end()
                ,guar_loan_spread.begin()
                ,guar_int.begin()
                ,greater_of<double>()
                );
            }
            break;
        case mce_variable_loan_rate:
            {
            // do nothing
            }
            break;
        default:
            {
            fatal_error() << "No " << LoanRateType_ << " case." << LMI_FLUSH;
            }
        }
*/

    // ET !! Mly7702iGlp = i_upper_12_over_12_from_i(max(.04, guar_int) - SpreadFor7702_);
    Mly7702iGlp.assign(Length, 0.04);
    std::transform
        (guar_int.begin()
        ,guar_int.end()
        ,Mly7702iGlp.begin()
        ,Mly7702iGlp.begin()
        ,greater_of<double>()
        );
    std::transform
        (Mly7702iGlp.begin()
        ,Mly7702iGlp.end()
        ,SpreadFor7702_.begin()
        ,Mly7702iGlp.begin()
        ,std::minus<double>()
        );
    std::transform
        (Mly7702iGlp.begin()
        ,Mly7702iGlp.end()
        ,Mly7702iGlp.begin()
        ,i_upper_12_over_12_from_i<double>()
        );

    // ET !! Mly7702iGlp = i_upper_12_over_12_from_i(max(.06, guar_int) - SpreadFor7702_);
    Mly7702iGsp.assign(Length, 0.06);
    std::transform
        (guar_int.begin()
        ,guar_int.end()
        ,Mly7702iGsp.begin()
        ,Mly7702iGsp.begin()
        ,greater_of<double>()
        );
    std::transform
        (Mly7702iGsp.begin()
        ,Mly7702iGsp.end()
        ,SpreadFor7702_.begin()
        ,Mly7702iGsp.begin()
        ,std::minus<double>()
        );
    std::transform
        (Mly7702iGsp.begin()
        ,Mly7702iGsp.end()
        ,Mly7702iGsp.begin()
        ,i_upper_12_over_12_from_i<double>()
        );

    // ET !! Mly7702ig = -1.0 + 1.0 / DBDiscountRate;
    Mly7702ig = DBDiscountRate;
    std::transform(Mly7702ig.begin(), Mly7702ig.end(), Mly7702ig.begin(),
          std::bind1st(std::divides<double>(), 1.0)
          );
    std::transform(Mly7702ig.begin(), Mly7702ig.end(), Mly7702ig.begin(),
          std::bind2nd(std::minus<double>(), 1.0)
          );
}
#endif // 0

