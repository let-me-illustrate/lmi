// Interest rates.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
#include "et_vector.hpp"
#include "math_functions.hpp"           // assign_midpoint()
#include "miscellany.hpp"               // each_equal()
#include "ssize_lmi.hpp"
#include "yare_input.hpp"

#include <algorithm>                    // max()

// TODO ?? Future enhancements:
//
// Permit input gross rates.
//
// Permit variable loan rate.
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
// small relative to (1 + i); therefore, i_upper_12_over_12_from_i<>()
// is used instead of pow().
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
} // Unnamed namespace.

#if 0
instead of passing BasicValues, pass these requirements only:
    v.yare_input_
    v.database()
    v.InvestmentManagementFee()
    v.GetLength()
    v.round_interest_rate()
    v.IsSubjectToIllustrationReg()
#endif // 0

InterestRates::InterestRates(BasicValues const& v)
    :Length_             {v.GetLength()}
    ,RoundIntRate_       {v.round_interest_rate()}
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
    GenAcctGrossRate_[mce_gen_curr] += general_account_interest_bonus;
    // DATABASE !! This might be further simplified by implementing e.g.
    //   std::vector<double> product_database::QueryVector(int k) const;
    // and replacing 'general_account_interest_bonus' with a temporary:
    //   GenAcctGrossRate_[...] += v.database().QueryVector(DB_GenAcctIntBonus);

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
        spread[mce_gen_curr] -= spread[mce_gen_curr][0];
        spread[mce_gen_mdpt] = spread[mce_gen_curr];
        spread[mce_gen_mdpt] *= 0.5;
        // (though it should be...
        //   spread[mce_gen_mdpt] = 0.5 * spread[mce_gen_curr] + 0.5 * spread[mce_gen_guar];
        // ...if DB_GuarIntSpread is ever implemented)
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
    miscellaneous_charges += AmortLoad_ + ExtraSepAcctCharge_;

    std::vector<double> total_charges[mc_n_gen_bases];
    for(int j = mce_gen_curr; j < mc_n_gen_bases; ++j)
        {
        if(mce_gen_mdpt == j)
            {
            continue;
            }
        total_charges[j] = MAndERate_[j];
        total_charges[j] += miscellaneous_charges;
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
            total_charges[j] -= total_charges[j][0];
            }
        fee = 0.0;
        }
    else
        {
        LMI_ASSERT(mce_gross_rate == SepAcctRateType_);
        }

    SepAcctGrossRate_[mce_annual_rate][mce_sep_zero] = Zero_;
    // ET !! It would be most useful to have an assignment operator,
    // so that this could be written as
    //   X = 0.5 * Y;
    // instead of
    //   X = zero_vector_of_desired_length;
    //   X += 0.5 * Y;
    // but it's forbidden to add such an operator to std::vector.
    // See test_pete_assignment() in 'expression_template_0_test.cpp'.
    SepAcctGrossRate_[mce_annual_rate][mce_sep_half] = Zero_;
    SepAcctGrossRate_[mce_annual_rate][mce_sep_half] +=
        0.5 * SepAcctGrossRate_[mce_annual_rate][mce_sep_full];

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
        PrfLnDueRate_[mce_annual_rate][j] -= PrefLoanRateDecr_;
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

    LMI_ASSERT
        (  RegLnDueRate_[mce_annual_rate ][mce_gen_guar]
        == RegLnDueRate_[mce_annual_rate ][mce_gen_curr]
        );
    LMI_ASSERT
        (  RegLnDueRate_[mce_monthly_rate][mce_gen_guar]
        == RegLnDueRate_[mce_monthly_rate][mce_gen_curr]
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
    // SOMEDAY !! After we've implemented and tested the
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
                ,AnnualSepAcctIMFRate
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
