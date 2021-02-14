// Interest rates.
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

#include "interest_rates.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "irc7702_interest.hpp"         // iglp(), igsp()
#include "math_functions.hpp"           // assign_midpoint()
#include "miscellany.hpp"               // each_equal()
#include "ssize_lmi.hpp"
#include "yare_input.hpp"

#include <algorithm>                    // max(), copy()
#include <functional>
#include <iterator>                     // back_inserter()

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
// small relative to (1 + i). That is why std::expm1() and
// std::log1p() are used instead of std::pow().
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
        alarum() << "No " << spread_method << " case." << LMI_FLUSH;
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
    int const length = lmi::ssize(annual_gross_rate);
    annual_net_rate .resize(length);
    monthly_net_rate.resize(length);

    double cached_annual_net_rate     = 0.0;
    double cached_monthly_net_rate    = 0.0;

    double previous_annual_gross_rate = 0.0;
    double previous_spread            = 0.0;
    double previous_floor             = 0.0;
    for(int j = 0; j < length; ++j)
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

// Transform vector of annual gross interest rates to monthly gross.
// Often the rates are the same from one year to the next; when that
// happens, we just replicate the previous value in order to avoid
// costly floating point calculations.
void convert_interest_rates
    (std::vector<double> const& annual_gross_rate
    ,std::vector<double>      & monthly_gross_rate
    ,round_to<double>    const& round_interest_rate
    )
{
    int const length = lmi::ssize(annual_gross_rate);
    monthly_gross_rate.resize(length);

    double cached_monthly_gross_rate  = 0.0;

    double previous_annual_gross_rate = 0.0;
    for(int j = 0; j < length; ++j)
        {
        if(previous_annual_gross_rate != annual_gross_rate[j])
            {
            previous_annual_gross_rate = annual_gross_rate[j];
            cached_monthly_gross_rate = i_upper_12_over_12_from_i<double>()(annual_gross_rate[j]);
            cached_monthly_gross_rate = round_interest_rate(cached_monthly_gross_rate);
            }
        monthly_gross_rate[j] = cached_monthly_gross_rate;
        }
}

#if 0
/// Determine whether loan rates are needed; else they can be zero.
///
/// Loan rates can potentially affect GPT calculations.
///
/// SOMEDAY !! Is this function worthwhile, or should it be expunged?
/// An error in its implementation could have severe consequences, and
/// the benefit seems slight. At present, it's never called anyway,
/// because BasicValues::Init7702() is called unconditionally--so the
/// test for 'DefinitionOfLifeInsurance' is incorrect.

bool need_loan_rates(yare_input const& yi)
{
    return
            mce_gpt                == yi.DefinitionOfLifeInsurance
        ||  mce_solve_loan         == yi.SolveType
        ||  true                   == yi.WithdrawToBasisThenLoan
        ||  0.0                    != yi.InforceRegularLoanValue
        ||  0.0                    != yi.InforcePreferredLoanValue
        ||  0.0                    != yi.InforceRegularLoanBalance
        ||  0.0                    != yi.InforcePreferredLoanBalance
        ||  !each_equal(yi.NewLoan, 0.0)
        ;
}
#endif // 0
} // Unnamed namespace.

#if 0
instead of passing BasicValues, pass these requirements only:
    v.yare_input_
    v.database()
    v.InvestmentManagementFee()
    v.GetLength()
    v.round_interest_rate()
    v.round_interest_rate_7702()
    v.IsSubjectToIllustrationReg()
    v.SpreadFor7702()
#endif // 0

InterestRates::InterestRates(BasicValues const& v)
    :Length_             {v.GetLength()}
    ,RoundIntRate_       {v.round_interest_rate()}
    ,Round7702Rate_      {v.round_interest_rate_7702()}
    ,Zero_               (Length_)
    ,NeedMidpointRates_  {v.IsSubjectToIllustrationReg()}
    ,GenAcctRateType_    {v.yare_input_.GeneralAccountRateType}
    ,NeedSepAcctRates_   {v.database().query<bool>(DB_AllowSepAcct)}
    ,SepAcctRateType_    {v.yare_input_.SeparateAccountRateType}
    ,SepAcctSpreadMethod_{v.database().query<mcenum_spread_method>(DB_SepAcctSpreadMethod)}
    ,AmortLoad_          {Zero_}
    ,ExtraSepAcctCharge_ {Zero_}
    ,LoanRateType_       {v.yare_input_.LoanRateType}
    ,NeedPrefLoanRates_  {v.database().query<bool>(DB_AllowPrefLoan)}
    ,NeedHoneymoonRates_ {v.yare_input_.HoneymoonEndorsement}
    ,SpreadFor7702_      {v.SpreadFor7702()}
{
    Initialize(v);
}

void InterestRates::Initialize(BasicValues const& v)
{
    // Retrieve general-account data from class BasicValues.

    v.database().query_into(DB_GuarInt, GenAcctGrossRate_[mce_gen_guar]);

    GenAcctGrossRate_[mce_gen_curr] = v.yare_input_.GeneralAccountRate;

    // General-account interest bonus implemented only as a simple
    // additive adjustment to the annual effective rate. It probably
    // makes no sense to add it to the monthly rate and derive an
    // annual rate from that: this affects a declared rate that is
    // almost certainly quoted as an APR. It is assumed that the
    // interest bonus is not guaranteed.
    std::vector<double> general_account_interest_bonus;
    v.database().query_into(DB_GenAcctIntBonus, general_account_interest_bonus);
    // ET !! GenAcctGrossRate_ += general_account_interest_bonus;
    // ...and this might be further simplified by implementing e.g.
    //   std::vector<double> product_database::QueryVector(int k) const;
    // and replacing 'general_account_interest_bonus' with a
    // temporary:
    //   GenAcctGrossRate_ += v.database().QueryVector(DB_GenAcctIntBonus);
    std::transform
        (GenAcctGrossRate_[mce_gen_curr].begin()
        ,GenAcctGrossRate_[mce_gen_curr].end()
        ,general_account_interest_bonus.begin()
        ,GenAcctGrossRate_[mce_gen_curr].begin()
        ,std::plus<double>()
        );

    v.database().query_into(DB_CurrIntSpread, GenAcctSpread_);

    // Retrieve separate-account data from class BasicValues.

    SepAcctGrossRate_[mce_annual_rate][mce_sep_full] = v.yare_input_.SeparateAccountRate;

    v.database().query_into(DB_GuarMandE          , MAndERate_[mce_gen_guar]);
    v.database().query_into(DB_CurrMandE          , MAndERate_[mce_gen_curr]);

    // Deduct miscellaneous fund charges and input extra asset comp in
    // the same way as M&E, iff database entity DB_AssetChargeType has
    // the value 'oe_asset_charge_spread'; otherwise, reflect them
    // elsewhere as an account-value load.
    if(oe_asset_charge_spread == v.database().query<oenum_asset_charge_type>(DB_AssetChargeType))
        {
        ExtraSepAcctCharge_ = v.yare_input_.ExtraCompensationOnAssets;
        LMI_ASSERT(lmi::ssize(ExtraSepAcctCharge_) == v.database().length());
        }

    if(v.yare_input_.AmortizePremiumLoad)
        {
        v.database().query_into(DB_LoadAmortFundCharge, AmortLoad_);
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
            v.database().query_into(DB_FixedLoanRate, PublishedLoanRate_);
            }
            break;
        case mce_variable_loan_rate:
            {
            PublishedLoanRate_.assign(Length_, v.yare_input_.LoanRate);
            }
            break;
        default:
            {
            alarum() << "No " << LoanRateType_ << " case." << LMI_FLUSH;
            }
        }

    v.database().query_into(DB_PrefLoanRateDecr  , PrefLoanRateDecr_           );

    v.database().query_into(DB_GuarRegLoanSpread , RegLoanSpread_[mce_gen_guar]);
    v.database().query_into(DB_CurrRegLoanSpread , RegLoanSpread_[mce_gen_curr]);
    v.database().query_into(DB_GuarPrefLoanSpread, PrfLoanSpread_[mce_gen_guar]);
    v.database().query_into(DB_CurrPrefLoanSpread, PrfLoanSpread_[mce_gen_curr]);

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
    for(int i = mce_annual_rate; i < mc_n_rate_periods; ++i)
        {
        for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
            {
            // The next line gets executed more than once with
            // identical semantics, but it's cheap, and writing it
            // to avoid that little problem would make it unclear.
            LMI_ASSERT(Length_ == lmi::ssize(GenAcctGrossRate_           [j]   ));
            LMI_ASSERT(Length_ == lmi::ssize(GenAcctNetRate_          [i][j]   ));
            for(int k = mce_sep_full; k < mc_n_sep_bases; ++k)
                {
                LMI_ASSERT(Length_ == lmi::ssize(SepAcctGrossRate_    [i]   [k]));
                LMI_ASSERT(Length_ == lmi::ssize(SepAcctNetRate_      [i][j][k]));
                }
            LMI_ASSERT(Length_ == lmi::ssize(RegLnCredRate_           [i][j]   ));
            LMI_ASSERT(Length_ == lmi::ssize(RegLnDueRate_            [i][j]   ));
            LMI_ASSERT(Length_ == lmi::ssize(PrfLnCredRate_           [i][j]   ));
            LMI_ASSERT(Length_ == lmi::ssize(PrfLnDueRate_            [i][j]   ));
            LMI_ASSERT(Length_ == lmi::ssize(HoneymoonValueRate_      [i][j]   ));
            LMI_ASSERT(Length_ == lmi::ssize(PostHoneymoonGenAcctRate_[i][j]   ));
            }
        }
}

void InterestRates::InitializeGeneralAccountRates()
{
    std::vector<double> spread[mc_n_gen_bases] = {Zero_, Zero_, Zero_};
    if(mce_earned_rate == GenAcctRateType_)
        {
        alarum()
            << "General-account rate is unexpectedly an earned rate."
            << LMI_FLUSH
            ;
        spread[mce_gen_curr] = GenAcctSpread_;
        // ET !! spread[mce_gen_curr] -= spread[mce_gen_curr][0];
        std::transform
            (spread[mce_gen_curr].begin()
            ,spread[mce_gen_curr].end()
            ,spread[mce_gen_curr].begin()
            ,[&spread](double x) { return x - spread[mce_gen_curr].front(); }
            );
        // ET !! spread[mce_gen_mdpt] = 0.5 * spread[mce_gen_curr];
        // (though it should be...
        // ET !! spread[mce_gen_mdpt] = 0.5 * spread[mce_gen_curr] + 0.5 * spread[mce_gen_guar];
        // ...if DB_GuarIntSpread is ever implemented)
        std::transform
            (spread[mce_gen_curr].begin()
            ,spread[mce_gen_curr].end()
            ,spread[mce_gen_mdpt].begin()
            ,[](double x) { return 0.5 * x; }
            );
        }
    else
        {
        LMI_ASSERT(mce_credited_rate == GenAcctRateType_);
        }

    GenAcctGrossRate_[mce_gen_mdpt] = Zero_;
    if(NeedMidpointRates_)
        {
        assign_midpoint(GenAcctGrossRate_[mce_gen_mdpt], GenAcctGrossRate_[mce_gen_guar], GenAcctGrossRate_[mce_gen_curr]);
        }
    else
        {
        GenAcctNetRate_[mce_annual_rate ][mce_gen_mdpt] = Zero_;
        GenAcctNetRate_[mce_monthly_rate][mce_gen_mdpt] = Zero_;
        }

    for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
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
    SepAcctFloor_.assign(Length_, -1.0);
    if(!NeedSepAcctRates_)
        {
        for(int i = mce_annual_rate; i < mc_n_rate_periods; ++i)
            {
            for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
                {
                for(int k = mce_sep_full; k < mc_n_sep_bases; ++k)
                    {
                    SepAcctNetRate_  [i][j][k] = Zero_;
                    // Don't zero out input (gross, annual, full).
                    // SOMEDAY !! That seems senseless: in this
                    //   if(!NeedSepAcctRates_)
                    // block, it should be perfectly all right to
                    // zero out rates that aren't needed.
                    if(mce_annual_rate == i && mce_sep_full == k) continue;
                    SepAcctGrossRate_[i]   [k] = Zero_;
                    }
                }
            }
        return;
        }

// TODO ?? Are tiered M&E, IMF, comp treated correctly?

    std::vector<double> miscellaneous_charges(Zero_);
// TODO ?? Replace these long lines with PETE expressions.
    // ET !! miscellaneous_charges += AmortLoad_ + ExtraSepAcctCharge_;
    std::transform(miscellaneous_charges.begin(), miscellaneous_charges.end(), AmortLoad_         .begin(), miscellaneous_charges.begin(), std::plus<double>());
    std::transform(miscellaneous_charges.begin(), miscellaneous_charges.end(), ExtraSepAcctCharge_.begin(), miscellaneous_charges.begin(), std::plus<double>());

    std::vector<double> total_charges[mc_n_gen_bases];
    for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
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
        alarum()
            << "Separate-account rate is unexpectedly a net rate."
            << LMI_FLUSH
            ;
        for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
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
                ,[&total_charges, j](double x) { return x - total_charges[j].front(); }
                );
            }
        fee = 0.0;
        }
    else
        {
        LMI_ASSERT(mce_gross_rate == SepAcctRateType_);
        }

    SepAcctGrossRate_[mce_annual_rate][mce_sep_zero] = Zero_;
    // ET !! SepAcctGrossRate_[mce_sep_half] = 0.5 * SepAcctGrossRate_[mce_sep_full];
    std::transform
        (SepAcctGrossRate_[mce_annual_rate][mce_sep_full].begin()
        ,SepAcctGrossRate_[mce_annual_rate][mce_sep_full].end()
        ,std::back_inserter(SepAcctGrossRate_[mce_annual_rate][mce_sep_half])
        ,[](double x) { return 0.5 * x; }
        );

    for(int k = mce_sep_full; k < mc_n_sep_bases; ++k)
        {
        convert_interest_rates
            (SepAcctGrossRate_[mce_annual_rate ][k]
            ,SepAcctGrossRate_[mce_monthly_rate][k]
            ,RoundIntRate_
            );
        }

    for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
        {
        for(int k = mce_sep_full; k < mc_n_sep_bases; ++k)
            {
            if(mce_gen_mdpt == j)
                {
                SepAcctNetRate_[mce_annual_rate ][j][k] = Zero_;
                SepAcctNetRate_[mce_monthly_rate][j][k] = Zero_;
                continue;
                }
            convert_interest_rates
                (SepAcctGrossRate_[mce_annual_rate ]   [k]
                ,SepAcctNetRate_  [mce_annual_rate ][j][k]
                ,SepAcctNetRate_  [mce_monthly_rate][j][k]
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
    // Historically, lmi has charged a singular published rate on all
    // loans, which necessarily differs for fixed vs. variable loans,
    // but was the same for both regular and preferred. In 2015, it
    // became necessary to support a new product that charges a lower
    // (fixed) rate on preferred loans (and doesn't offer VLR). To
    // accommodate this with minimal violence to existing code, a new
    // 'PrefLoanRateDecr' entity has been added to the database.
    // SOMEDAY !! It might be better to replace the current paradigm
    // (due-rate minus spread) with a new set of database entities
    // that map directly to the present class's public accessors:
    //   (Reg|Reg)Ln(Due|Cred)Rate
    // For now, it is uncertain whether a nonzero 'PrefLoanRateDecr'
    // should be deducted for VLR, so that case is asserted not to
    // arise:
    LMI_ASSERT
        (  mce_fixed_loan_rate == LoanRateType_
        || each_equal(PrefLoanRateDecr_, 0.0)
        );
    for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
        {
        RegLnDueRate_[mce_annual_rate][j] = PublishedLoanRate_;
        PrfLnDueRate_[mce_annual_rate][j] = PublishedLoanRate_;
        // ET !! PrfLnDueRate_[...] = PublishedLoanRate_ - PrefLoanRateDecr_;
        std::transform
            (PrfLnDueRate_[mce_annual_rate][j].begin()
            ,PrfLnDueRate_[mce_annual_rate][j].end()
            ,PrefLoanRateDecr_.begin()
            ,PrfLnDueRate_[mce_annual_rate][j].begin()
            ,std::minus<double>()
            );
        }

    RegLoanSpread_[mce_gen_mdpt] = Zero_;
    PrfLoanSpread_[mce_gen_mdpt] = Zero_;
    if(NeedMidpointRates_)
        {
        assign_midpoint(RegLoanSpread_[mce_gen_mdpt], RegLoanSpread_[mce_gen_guar], RegLoanSpread_[mce_gen_curr]);
        assign_midpoint(PrfLoanSpread_[mce_gen_mdpt], PrfLoanSpread_[mce_gen_guar], PrfLoanSpread_[mce_gen_curr]);
        }

    for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
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
            convert_interest_rates
                (PrfLnDueRate_[mce_annual_rate ][j]
                ,PrfLnDueRate_[mce_annual_rate ][j]
                ,PrfLnDueRate_[mce_monthly_rate][j]
                ,RoundIntRate_
                ,Zero_
                ,mce_spread_is_effective_annual
                ,GenAcctGrossRate_[mce_gen_guar]
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
    // PETE causes an 'ambiguous overload' error for vector 'v0==v1',
    // but not if std::operator==(v0,v1) is called explicitly.

    LMI_ASSERT
        (std::operator==
            (RegLnDueRate_[mce_annual_rate ][mce_gen_guar]
            ,RegLnDueRate_[mce_annual_rate ][mce_gen_curr]
            )
        );
    LMI_ASSERT
        (std::operator==
            (RegLnDueRate_[mce_monthly_rate][mce_gen_guar]
            ,RegLnDueRate_[mce_monthly_rate][mce_gen_curr]
            )
        );
}

void InterestRates::InitializeHoneymoonRates()
{
    if(!NeedHoneymoonRates_)
        {
        for(int i = mce_annual_rate; i < mc_n_rate_periods; ++i)
            {
            for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
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
    LMI_ASSERT(mce_credited_rate == GenAcctRateType_);

    for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
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

/// Recalculate sepacct net rate if it depends on assets.
///
/// When separate-account charges depend on monthly total case assets,
/// the separate-account rate is no longer an annual invariant and
/// must be recalculated each month. No corresponding general-account
/// adjustment has ever been wanted.
///
/// Non-tiered complements are added to each argument as needed.

void InterestRates::DynamicMlySepAcctRate
    (mcenum_gen_basis gen_basis
    ,mcenum_sep_basis sep_basis
    ,int              year
    ,double           AnnualSepAcctMandERate
    ,double           AnnualSepAcctIMFRate
    ,double           AnnualSepAcctMiscChargeRate
    )
{
    AnnualSepAcctMandERate      += MAndERate_[gen_basis]   [year];
    AnnualSepAcctIMFRate        += InvestmentManagementFee_[year];
    AnnualSepAcctMiscChargeRate += ExtraSepAcctCharge_     [year];

    double dynamic_spread =
            AnnualSepAcctMandERate
        +   AmortLoad_[year]
        +   AnnualSepAcctMiscChargeRate
        ;

    switch(SepAcctRateType_)
        {
        case mce_gross_rate:
            {
            if(mce_gen_mdpt == gen_basis)
                {
                alarum()
                    << "Midpoint separate-account rate not supported."
                    << LMI_FLUSH
                    ;
                }
            convert_interest_rates
                (SepAcctGrossRate_[mce_annual_rate][sep_basis][year]
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
            alarum() << "Net rate not supported." << LMI_FLUSH;
            }
            break;
        default:
            {
            alarum() << "No " << SepAcctRateType_ << " case." << LMI_FLUSH;
            }
        }
}

void InterestRates::Initialize7702Rates()
{

// §7702 prescribes the interest basis for all §7702 and §7702A
// calculations as the interest rate actually guaranteed in the
// contract, or a statutory rate if greater. The statutory rate is
// iglp() and igsp() for GLP and GSP, and iglp() for all §7702A
// calculations except that the necessary premium for guideline
// contracts is defined in terms of the guideline limit.

// The §7702 net rate is determined in two steps. First, the
// guaranteed interest rate is determined from the contract, and the
// statutory rate is used instead if it is greater. This operation is
// performed separately for all periods with different guaranteed
// rates [DEFRA Blue Book, page 648]. For example, using the original
// 1984 statutory rates (4% for GLP and 6% for GSP), if the guaranteed
// rate is 4.5% for five years and 3.5% thereafter, then the GLP
// interest rate is 4.5% for five years and 4.0% thereafter, while the
// GSP rate is always 6.0%. For products such as pure variable UL that
// offer no explicit guarantee, the statutory rate is used. For
// variable products that offer a general account option, the
// guaranteed gross rate must be no less than the general account
// guaranteed rate.

// Even short-term guarantees at issue must be reflected in the GSP,
// the CVAT NSP, and the §7702A NSP, seven-pay premium, and DCV. They
// may be ignored as de minimis in calculating the §7702 GLP [DEFRA
// Blue Book, page 649], but only as long as they last no longer than
// one year. Only guarantees that either last longer than one year or
// are present on the issue date are taken into account: a guarantee
// subsequently added for a future period lasting no longer than one
// year is a dividend, not an adjustment event. Here, "issue" excludes
// cases where the contract is merely deemed by statute to be reissued
// [for example, by §7702A(c)(3)(A)(i)].

// Second, any current asset based charges specified in the contract
// are deducted if we wish. The interest rate remains what it is; the
// net rate that results from subtracting asset-based charges is
// merely a computational convenience that simplifies the formulas.
// In fact, the full interest rate (never less than statutory) is
// credited, and then asset based charges are subtracted from the
// account value. Therefore, this adjustment affects only the §7702
// guideline premiums and the §7702 DCV, because those quantities
// reflect expenses. It must not be taken into account when
// calculating the §7702 CVAT NSP or CVAT corridor factors, or the
// §7702A NSP or seven-pay premium, because those quantities do not
// reflect expenses.

// Asset based charges can be deducted only if they are specified in
// the contract itself: charges imposed by separate accounts cannot be
// deducted unless they are specified in the life insurance contract
// proper, since any charge not so specified is deemed to be zero
// [§7702(c)(3)(D)(i)]. They also must not exceed the charges
// reasonably expected to be actually imposed [§7702(c)(3)(B)(ii)].
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
// be imposed. A §7702(f)(8) waiver granted in one actual case that
// was pennies over the limit cost tens of thousands of dollars in
// filing and attorney's fees.

// Thus, an account-value load that is deducted from the account value
// at the beginning of each month, before interest is credited, may be
// reflected in GPT calculations. We could calculate it as a monthly
// load in order to follow the precise contract mechanics, but that
// would require a significant modification of Eckley's formulas,
// which do not contemplate a load on AV. Instead, we net the account
// value load against the §7702 interest rate; as explained above,
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
// products that guarantee a rate tied to an index, the §7702 interest
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
// §7702 and §7702A calculations must use the §7702 rate instead
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
// a fixed loan rate has no §7702 or §7702A effect.

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
// loan rate no higher than §7702 otherwise requires during the first
// loan rate determination period, or simply by forbidding loans
// during that period.

// TODO ?? TAXATION !! Calculate both:
//    std::vector<double> MlyGlpRate_;
//    std::vector<double> MlyGspRate_;

    std::vector<double> const& annual_guar_rate = GenAcctGrossRate_[mce_gen_guar];

    MlyGlpRate_.resize(Length_);
    // ET !! MlyGlpRate_ = max(iglp(), annual_guar_rate);
    std::transform
        (annual_guar_rate.begin()
        ,annual_guar_rate.end()
        ,MlyGlpRate_.begin()
        ,[](double x) { return std::max(iglp(), x); }
        );
    // ET !! This ought to be implicit, at least in some 'safe' mode:
    LMI_ASSERT(MlyGlpRate_.size() == SpreadFor7702_.size());
    // ET !! MlyGlpRate_ = i_upper_12_over_12_from_i(MlyGlpRate_ - SpreadFor7702_);
    std::transform(MlyGlpRate_.begin(), MlyGlpRate_.end(), SpreadFor7702_.begin(), MlyGlpRate_.begin(), std::minus<double>());
    std::transform(MlyGlpRate_.begin(), MlyGlpRate_.end(), MlyGlpRate_.begin(), i_upper_12_over_12_from_i<double>());
}

#if 0
// TODO ?? Here's the implementation actually used, elsewhere--it needs work.
// Eventually this should be rewritten. It still lives here because it
// really belongs here, not in class BasicValues. TAXATION !! Resolve this.

{
    // Monthly guar net int for 7702 is
    //   greater of {iglp(), igsp()} and annual guar int rate
    //   less 7702 spread
    // TAXATION !! Resolve this:
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
            // TODO ?? TAXATION !! But that looks incorrect when written clearly!
            // Perhaps this old comment:
            //   APL: guar_int gets guar_int max gross_loan_rate - guar_loan_spread
            // suggests the actual intention.

            // TODO ?? TAXATION !! Need loan rates for 7702 whenever loans are allowed.
            std::vector<double> gross_loan_rate = PublishedLoanRate_;
            // TODO ?? TAXATION !! Should at least assert that preferred <= regular spread.
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
            alarum() << "No " << LoanRateType_ << " case." << LMI_FLUSH;
            }
        }
*/

    // ET !! Mly7702iGlp = i_upper_12_over_12_from_i(max(iglp(), guar_int) - SpreadFor7702_);
    Mly7702iGlp.assign(Length, iglp());
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

    // ET !! Mly7702iGlp = i_upper_12_over_12_from_i(max(igsp(), guar_int) - SpreadFor7702_);
    Mly7702iGsp.assign(Length, igsp());
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
    std::transform
        (Mly7702ig.begin()
        ,Mly7702ig.end()
        ,Mly7702ig.begin()
        ,[](double x) { return 1.0 / x; }
        );
    std::transform
        (Mly7702ig.begin()
        ,Mly7702ig.end()
        ,Mly7702ig.begin()
        ,[](double x) { return x - 1.0; }
        );
}
#endif // 0
