// Loads and expense charges.
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

#include "loads.hpp"
#include "loads_impl.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "et_vector.hpp"
#include "math_functors.hpp"            // assign_midpoint()
#include "mc_enum_types_aux.hpp"        // mc_n_ enumerators
#include "premium_tax.hpp"

/// Ctor for unit testing.

Loads::Loads()
{
}

/// Ctor for production branch.

Loads::Loads(BasicValues& V)
{
    int length = V.GetLength();
    LMI_ASSERT(length == V.Database_->length());
    LMI_ASSERT(!V.yare_input_.AmortizePremiumLoad); // Amortization is untested.
    load_details details
        (length
        ,V.yare_input_.AmortizePremiumLoad
        ,V.PremiumTax_->load_rate()
        ,V.PremiumTax_->maximum_load_rate()
        ,V.PremiumTax_->minimum_load_rate()
        ,V.PremiumTax_->levy_rate()
        ,V.Database_->Query(DB_PremTaxAmortIntRate)
        ,V.Database_->Query(DB_PremTaxAmortPeriod)
        ,V.Database_->Query(DB_AssetChargeType)
        ,V.IsSubjectToIllustrationReg()
        ,V.round_interest_rate()
        ,V.yare_input_.ExtraCompensationOnPremium
        ,V.yare_input_.ExtraCompensationOnAssets
        ,V.yare_input_.ExtraMonthlyCustodialFee
        ,V.GetGuarSpecAmtLoadTable()
        ,V.GetCurrSpecAmtLoadTable()
        );
    Allocate(length);
    Initialize(*V.Database_);
    Calculate(details);
}

/// Reserve required space for vector data members.
///
/// Zero-initializing everything is perhaps unnecessary, but the unit
/// test shows that it costs very little.

void Loads::Allocate(int length)
{
    monthly_policy_fee_    .resize(mc_n_gen_bases);
    annual_policy_fee_     .resize(mc_n_gen_bases);
    specified_amount_load_ .resize(mc_n_gen_bases);
    separate_account_load_ .resize(mc_n_gen_bases);
    target_premium_load_   .resize(mc_n_gen_bases);
    excess_premium_load_   .resize(mc_n_gen_bases);
    target_sales_load_     .resize(mc_n_gen_bases);
    excess_sales_load_     .resize(mc_n_gen_bases);
    target_total_load_     .resize(mc_n_gen_bases);
    excess_total_load_     .resize(mc_n_gen_bases);

    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        monthly_policy_fee_    [j].resize(length);
        annual_policy_fee_     [j].resize(length);
        specified_amount_load_ [j].resize(length);
        separate_account_load_ [j].resize(length);
        target_premium_load_   [j].resize(length);
        excess_premium_load_   [j].resize(length);
        target_sales_load_     [j].resize(length);
        excess_sales_load_     [j].resize(length);

        target_total_load_     [j].resize(length);
        excess_total_load_     [j].resize(length);
        }

    amortized_premium_tax_load_                    .resize(length);

    refundable_sales_load_proportion_              .resize(length);
    premium_tax_load_                              .resize(length);
    dac_tax_load_                                  .resize(length);

    target_premium_load_excluding_premium_tax_.resize(length);
    excess_premium_load_excluding_premium_tax_.resize(length);
    target_premium_load_maximum_premium_tax_  .resize(length);
    excess_premium_load_maximum_premium_tax_  .resize(length);
    target_premium_load_minimum_premium_tax_  .resize(length);
    excess_premium_load_minimum_premium_tax_  .resize(length);
}

/// Set various data members from product database.

void Loads::Initialize(product_database const& database)
{
    database.Query(refundable_sales_load_proportion_   , DB_LoadRfdProportion );
    database.Query(dac_tax_load_                       , DB_DacTaxPremLoad    );

    database.Query(monthly_policy_fee_   [mce_gen_guar], DB_GuarMonthlyPolFee );
    database.Query(annual_policy_fee_    [mce_gen_guar], DB_GuarAnnualPolFee  );
    database.Query(specified_amount_load_[mce_gen_guar], DB_GuarSpecAmtLoad   );
    database.Query(separate_account_load_[mce_gen_guar], DB_GuarAcctValLoad   );
    database.Query(target_premium_load_  [mce_gen_guar], DB_GuarPremLoadTgt   );
    database.Query(excess_premium_load_  [mce_gen_guar], DB_GuarPremLoadExc   );
    database.Query(target_sales_load_    [mce_gen_guar], DB_GuarPremLoadTgtRfd);
    database.Query(excess_sales_load_    [mce_gen_guar], DB_GuarPremLoadExcRfd);

    database.Query(monthly_policy_fee_   [mce_gen_curr], DB_CurrMonthlyPolFee );
    database.Query(annual_policy_fee_    [mce_gen_curr], DB_CurrAnnualPolFee  );
    database.Query(specified_amount_load_[mce_gen_curr], DB_CurrSpecAmtLoad   );
    database.Query(separate_account_load_[mce_gen_curr], DB_CurrAcctValLoad   );
    database.Query(target_premium_load_  [mce_gen_curr], DB_CurrPremLoadTgt   );
    database.Query(excess_premium_load_  [mce_gen_curr], DB_CurrPremLoadExc   );
    database.Query(target_sales_load_    [mce_gen_curr], DB_CurrPremLoadTgtRfd);
    database.Query(excess_sales_load_    [mce_gen_curr], DB_CurrPremLoadExcRfd);
}

/// Transform raw input and database data into directly-useful rates.

void Loads::Calculate(load_details const& details)
{
    premium_tax_load_.assign(details.length_, details.premium_tax_load_);

    for(int j = mce_gen_curr; j != mc_n_gen_bases; j++)
        {
        // ET !! PETE could support an apply-and-assign operation, e.g.:
        // apply_to_self
        //     (i_upper_12_over_12_from_i<double>()
        //     ,separate_account_load_[j]
        //     );
        assign
            (separate_account_load_[j]
            ,apply_unary
                (i_upper_12_over_12_from_i<double>()
                ,separate_account_load_[j]
                )
            );
        }

    // Deduct input extra asset comp as an account-value load, iff
    // database entity 'DB_AssetChargeType' has the value
    // 'oe_asset_charge_load'; otherwise, reflect it
    // elsewhere as an interest spread.
    if(oe_asset_charge_load == details.asset_charge_type_)
        {
        static long double const units_per_bp = 1.0L / 10000.0L;
        std::vector<double> extra_asset_comp(details.length_);
        assign
            (extra_asset_comp
            ,apply_unary
                (i_upper_12_over_12_from_i<double>()
                ,details.VectorExtraAssetComp_ * units_per_bp
                )
            );

        for(int j = mce_gen_curr; j != mc_n_gen_bases; j++)
            {
#if 0
// ET !! As for rounding, we do want an expression-template library
// to apply a scalar function like rounding to all elements of a
// matrix, ideally with some natural syntax like
//   separate_account_load_ = details.round_interest_rate_(separate_account_load_);
// However, this throws an "Erroneous rounding function" exception,
// because apply_unary() and apply_binary() use a default-constructed
// function object:
//   http://svn.savannah.nongnu.org/viewvc/lmi/trunk/tools/pete-2.1.1/PETE/Tools/PeteOps.in?root=lmi&r1=4151&r2=4150&pathrev=4151
// but round_to<> is stateful, and deliberately throws that exception
// when a default-constructed object is used.
            assign
                (separate_account_load_[j]
                ,apply_unary
                    (details.round_interest_rate_
                    ,separate_account_load_[j] + extra_asset_comp
                    )
                );
#endif // 0
            separate_account_load_[j] += extra_asset_comp;
            std::vector<double>::iterator k;
            for
                (k = separate_account_load_[j].begin()
                ;k != separate_account_load_[j].end()
                ;++k
                )
                {
                *k = details.round_interest_rate_(*k);
                }
            }
        }
    else if(oe_asset_charge_spread == details.asset_charge_type_)
        {
        // Do nothing here: handle in interest-rate class instead.
        }
    else
        {
        fatal_error()
            << "Case '"
            << details.asset_charge_type_
            << "' not found."
            << LMI_FLUSH
            ;
        }

    if(details.AmortizePremLoad_)
        {
        AmortizePremiumTax(details);
        }

    specified_amount_load_[mce_gen_guar] += details.TabularGuarSpecAmtLoad_;
    specified_amount_load_[mce_gen_curr] += details.TabularCurrSpecAmtLoad_;

    // Total load excludes monthly_policy_fee_, annual_policy_fee_,
    // amortized_premium_tax_load_, and specified_amount_load_ because
    // they are charges rather than loads (despite the names of the
    // last two): they're always deducted whether or not any payment
    // is made.

    for(int j = mce_gen_curr; j < mc_n_gen_bases; j++)
        {
        if(mce_gen_mdpt == j)
            {
            continue;
            }
        target_sales_load_[j] += details.VectorExtraCompLoad_;
        target_total_load_[j] = target_sales_load_[j];
        target_total_load_[j] += target_premium_load_[j] + dac_tax_load_;
        if(mce_gen_curr == j)
            {
            target_premium_load_excluding_premium_tax_ = target_total_load_[j];
            target_premium_load_maximum_premium_tax_   = target_total_load_[j];
            target_premium_load_maximum_premium_tax_  += details.maximum_premium_tax_load_rate_;
            target_premium_load_minimum_premium_tax_   = target_total_load_[j];
            target_premium_load_minimum_premium_tax_  += details.minimum_premium_tax_load_rate_;
            }
        target_total_load_[j] += premium_tax_load_;

        excess_sales_load_[j] += details.VectorExtraCompLoad_;
        excess_total_load_[j] = excess_sales_load_[j];
        excess_total_load_[j] += excess_premium_load_[j] + dac_tax_load_;
        if(mce_gen_curr == j)
            {
            excess_premium_load_excluding_premium_tax_ = excess_total_load_[j];
            excess_premium_load_maximum_premium_tax_   = excess_total_load_[j];
            excess_premium_load_maximum_premium_tax_  += details.maximum_premium_tax_load_rate_;
            excess_premium_load_minimum_premium_tax_   = excess_total_load_[j];
            excess_premium_load_minimum_premium_tax_  += details.minimum_premium_tax_load_rate_;
            }
        excess_total_load_[j] += premium_tax_load_;
        }

    // USER !! Explain this in user documentation.
    // Add 'VectorExtraPolFee_' to monthly policy fee on the current
    // basis only. It's added only to the current-basis policy fee
    // because it represents an extra current charge constrained by
    // the difference between the guaranteed and the (otherwise)
    // current policy fee. It was originally conceived as a fund
    // custodial fee, but can be used in any situation that's
    // consistent with this constraint.

    monthly_policy_fee_[mce_gen_curr] += details.VectorExtraPolFee_;
    for(int j = 0; j < details.length_; ++j)
        {
        if
            ( monthly_policy_fee_[mce_gen_guar][j]
            < monthly_policy_fee_[mce_gen_curr][j]
            )
            {
            fatal_error()
                << "Duration "
                << j
                << ": current monthly policy fee "
                << monthly_policy_fee_[mce_gen_curr][j]
                << " (which includes a custodial fee of "
                << details.VectorExtraPolFee_[j]
                << ") improperly exceeds guaranteed maximum of "
                << monthly_policy_fee_[mce_gen_guar][j]
                << " ."
                << LMI_FLUSH
                ;
            }
        }

    if(details.NeedMidpointRates_)
        {
        assign_midpoint(monthly_policy_fee_   [mce_gen_mdpt], monthly_policy_fee_   [mce_gen_guar], monthly_policy_fee_   [mce_gen_curr]);
        assign_midpoint(annual_policy_fee_    [mce_gen_mdpt], annual_policy_fee_    [mce_gen_guar], annual_policy_fee_    [mce_gen_curr]);
        assign_midpoint(specified_amount_load_[mce_gen_mdpt], specified_amount_load_[mce_gen_guar], specified_amount_load_[mce_gen_curr]);
        assign_midpoint(separate_account_load_[mce_gen_mdpt], separate_account_load_[mce_gen_guar], separate_account_load_[mce_gen_curr]);
        assign_midpoint(target_premium_load_  [mce_gen_mdpt], target_premium_load_  [mce_gen_guar], target_premium_load_  [mce_gen_curr]);
        assign_midpoint(excess_premium_load_  [mce_gen_mdpt], excess_premium_load_  [mce_gen_guar], excess_premium_load_  [mce_gen_curr]);
        assign_midpoint(target_sales_load_    [mce_gen_mdpt], target_sales_load_    [mce_gen_guar], target_sales_load_    [mce_gen_curr]);
        assign_midpoint(excess_sales_load_    [mce_gen_mdpt], excess_sales_load_    [mce_gen_guar], excess_sales_load_    [mce_gen_curr]);
        assign_midpoint(target_total_load_    [mce_gen_mdpt], target_total_load_    [mce_gen_guar], target_total_load_    [mce_gen_curr]);
        assign_midpoint(excess_total_load_    [mce_gen_mdpt], excess_total_load_    [mce_gen_guar], excess_total_load_    [mce_gen_curr]);
        }
}

/// Amortize premium tax.
///
/// Not implemented. The idea is to amortize premium tax as an
/// annuity-certain. Sketch:
///
/// Test parameters for sanity.
///
/// Calculate a monthly annuity factor, reflecting the amortization
/// rate and period.
///
/// Apply the annuity factor to the appropriate charge: probably
/// the rate the state actually charges, though the product's
/// premium-tax load might be taken into account. It's also
/// conceivable to amortize DAC tax similarly.
///
/// Store the result in 'amortized_premium_tax_load_'. Modify
/// any corresponding loads previously stored.

void Loads::AmortizePremiumTax(load_details const&)
{
    fatal_error() << "Premium-tax amortization not implemented." << LMI_FLUSH;
}

/// Ctor for antediluvian branch.

Loads::Loads(product_database const& database, bool NeedMidpointRates)
{
    monthly_policy_fee_   .resize(mc_n_gen_bases);
    target_premium_load_  .resize(mc_n_gen_bases);
    excess_premium_load_  .resize(mc_n_gen_bases);
    specified_amount_load_.resize(mc_n_gen_bases);

    database.Query(monthly_policy_fee_   [mce_gen_guar], DB_GuarMonthlyPolFee);
    database.Query(target_premium_load_  [mce_gen_guar], DB_GuarPremLoadTgt  );
    database.Query(excess_premium_load_  [mce_gen_guar], DB_GuarPremLoadExc  );
    database.Query(specified_amount_load_[mce_gen_guar], DB_GuarSpecAmtLoad  );

    database.Query(monthly_policy_fee_   [mce_gen_curr], DB_CurrMonthlyPolFee);
    database.Query(target_premium_load_  [mce_gen_curr], DB_CurrPremLoadTgt  );
    database.Query(excess_premium_load_  [mce_gen_curr], DB_CurrPremLoadExc  );
    database.Query(specified_amount_load_[mce_gen_curr], DB_CurrSpecAmtLoad  );

    // This ctor ignores tabular specified-amount loads.

    if(NeedMidpointRates)
        {
        assign_midpoint(monthly_policy_fee_   [mce_gen_mdpt], monthly_policy_fee_   [mce_gen_guar], monthly_policy_fee_   [mce_gen_curr]);
        assign_midpoint(target_premium_load_  [mce_gen_mdpt], target_premium_load_  [mce_gen_guar], target_premium_load_  [mce_gen_curr]);
        assign_midpoint(excess_premium_load_  [mce_gen_mdpt], excess_premium_load_  [mce_gen_guar], excess_premium_load_  [mce_gen_curr]);
        assign_midpoint(specified_amount_load_[mce_gen_mdpt], specified_amount_load_[mce_gen_guar], specified_amount_load_[mce_gen_curr]);
        }

    premium_tax_load_                       .resize(database.length());
    dac_tax_load_                           .resize(database.length());
    target_premium_load_maximum_premium_tax_.resize(database.length());
}

