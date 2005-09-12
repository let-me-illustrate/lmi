// Loads and expense charges.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: loads.cpp,v 1.6 2005-09-12 01:32:19 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "loads.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "inputs.hpp"
#include "math_functors.hpp"
#include "rounding_rules.hpp"

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>

struct load_details
{
    load_details
        (int                        length
        ,bool                       AmortizePremLoad
        ,double                     LowestPremTaxRate
        ,double                     premium_tax_rate
        ,double                     premium_tax_amortization_rate
        ,double                     premium_tax_amortization_period
        ,double                     asset_charge_type
        ,double                     ledger_type
        ,round_to<double>    const& round_interest_rate
        ,std::vector<double> const& VectorExtraCompLoad
        ,std::vector<double> const& VectorExtraAssetComp
        ,std::vector<double> const& VectorExtraPolFee
        )
        :length_                          (length)
        ,AmortizePremLoad_                (AmortizePremLoad)
        ,LowestPremTaxRate_               (LowestPremTaxRate)
        ,premium_tax_rate_                (premium_tax_rate)
        ,premium_tax_amortization_rate_   (premium_tax_amortization_rate)
        ,premium_tax_amortization_period_ (premium_tax_amortization_period)
        ,asset_charge_type_               (asset_charge_type)
        ,ledger_type_                     (ledger_type)
        ,round_interest_rate_             (round_interest_rate)
        ,VectorExtraCompLoad_             (VectorExtraCompLoad)
        ,VectorExtraAssetComp_            (VectorExtraAssetComp)
        ,VectorExtraPolFee_               (VectorExtraPolFee)
        {}

    int                        length_;
    bool                       AmortizePremLoad_;
    double                     LowestPremTaxRate_;
    double                     premium_tax_rate_;
    double                     premium_tax_amortization_rate_;
    double                     premium_tax_amortization_period_;
    double                     asset_charge_type_;
    double                     ledger_type_;
    round_to<double>    const& round_interest_rate_;
    std::vector<double> const& VectorExtraCompLoad_;
    std::vector<double> const& VectorExtraAssetComp_;
    std::vector<double> const& VectorExtraPolFee_;
};

//============================================================================
Loads::Loads(BasicValues& V)
{
    int length = V.GetLength();
    LMI_ASSERT(length == V.Database_->length());
    load_details details
        (length
        ,V.Input_->AmortizePremLoad
        ,V.GetLowestPremTaxRate()
        ,V.Database_->Query(DB_PremTaxRate)
        ,V.Database_->Query(DB_PmTxAmortIntRate)
        ,V.Database_->Query(DB_PmTxAmortPeriod)
        ,V.Database_->Query(DB_AssetChargeType)
        ,V.Database_->Query(DB_LedgerType)
        ,V.GetRoundingRules().round_interest_rate()
        ,V.Input_->VectorAddonCompOnPremium
        ,V.Input_->VectorAddonCompOnAssets
        ,V.Input_->VectorAddonMonthlyCustodialFee
        );
    Init(*V.Database_, details);
}

//============================================================================
void Loads::Init
    (TDatabase    const& database
    ,load_details const& details
    )
{
    monthly_policy_fee_                  .resize(n_illreg_bases);
    annual_policy_fee_                   .resize(n_illreg_bases);
    specified_amount_load_               .resize(n_illreg_bases);
    account_value_load_before_deduction_ .resize(n_illreg_bases);
    account_value_load_after_deduction_  .resize(n_illreg_bases);
    target_premium_load_                 .resize(n_illreg_bases);
    excess_premium_load_                 .resize(n_illreg_bases);
    target_sales_load_                   .resize(n_illreg_bases);
    excess_sales_load_                   .resize(n_illreg_bases);
    target_total_load_                   .resize(n_illreg_bases);
    excess_total_load_                   .resize(n_illreg_bases);

    for(int j = e_currbasis; j < n_illreg_bases; j++)
        {
        // TODO ?? Consider skipping midpoint basis if unneeded.

        // TODO ?? Why is this necessary? Why aren't all of these
        // initialized by database lookup?
        monthly_policy_fee_                  [j].resize(details.length_);
        annual_policy_fee_                   [j].resize(details.length_);
        specified_amount_load_               [j].resize(details.length_);
        account_value_load_before_deduction_ [j].resize(details.length_);
        account_value_load_after_deduction_  [j].resize(details.length_);
        target_premium_load_                 [j].resize(details.length_);
        excess_premium_load_                 [j].resize(details.length_);
        target_sales_load_                   [j].resize(details.length_);
        excess_sales_load_                   [j].resize(details.length_);

        // TODO ?? Not retrieved from database. Initialize elsewhere?
        target_total_load_                   [j].resize(details.length_);
        excess_total_load_                   [j].resize(details.length_);
        }

    premium_tax_load_                              .resize(details.length_);
    amortized_premium_tax_load_                    .resize(details.length_);
    dac_tax_load_                                  .resize(details.length_);

    target_premium_load_7702_excluding_premium_tax_.resize(details.length_);
    excess_premium_load_7702_excluding_premium_tax_.resize(details.length_);
    target_premium_load_7702_lowest_premium_tax_   .resize(details.length_);
    excess_premium_load_7702_lowest_premium_tax_   .resize(details.length_);

    // SOMEDAY !! Database lookups have been consolidated here, to
    // facilitate the long-term goal of factoring them into a separate
    // module so that unit tests can be more easily written for the
    // rest of this class's implementation.

    database.Query(refundable_sales_load_proportion_, DB_PremRefund    );

    database.Query(monthly_policy_fee_                  [e_guarbasis], DB_GuarPolFee          );
    database.Query(annual_policy_fee_                   [e_guarbasis], DB_GuarIssueFee        );
    database.Query(specified_amount_load_               [e_guarbasis], DB_GuarSpecAmtLoad     );
    database.Query(account_value_load_before_deduction_ [e_guarbasis], DB_GuarAcctValLoadBOM  );
    database.Query(account_value_load_after_deduction_  [e_guarbasis], DB_GuarAcctValLoadAMD  );
    database.Query(target_premium_load_                 [e_guarbasis], DB_GuarPremLoadTgt     );
    database.Query(excess_premium_load_                 [e_guarbasis], DB_GuarPremLoadExc     );
    database.Query(target_sales_load_                   [e_guarbasis], DB_GuarPremLoadTgtRfd  );
    database.Query(excess_sales_load_                   [e_guarbasis], DB_GuarPremLoadExcRfd  );

    database.Query(monthly_policy_fee_                  [e_currbasis], DB_CurrPolFee          );
    database.Query(annual_policy_fee_                   [e_currbasis], DB_CurrIssueFee        );
    database.Query(specified_amount_load_               [e_currbasis], DB_CurrSpecAmtLoad     );
    database.Query(account_value_load_before_deduction_ [e_currbasis], DB_CurrAcctValLoadBOM  );
    database.Query(account_value_load_after_deduction_  [e_currbasis], DB_CurrAcctValLoadAMD  );
    database.Query(target_premium_load_                 [e_currbasis], DB_CurrPremLoadTgt     );
    database.Query(excess_premium_load_                 [e_currbasis], DB_CurrPremLoadExc     );
    database.Query(target_sales_load_                   [e_currbasis], DB_CurrPremLoadTgtRfd  );
    database.Query(excess_sales_load_                   [e_currbasis], DB_CurrPremLoadExcRfd  );

    database.Query(premium_tax_load_                , DB_PremTaxLoad   );
    database.Query(dac_tax_load_                    , DB_DACTaxPremLoad);

    // ET !! The loop and the std::transform call should both be
    // unnecessary: it should be possible to write simply
    //   account_value_load_after_deduction_ = i_upper_12_over_12_from_i(account_value_load_after_deduction_);
    for(int j = e_currbasis; j != n_illreg_bases; j++)
        {
        // ET !! Rewrite [but see above comment]
        // account_value_load_after_deduction_[j] = i_upper_12_over_12_from_i
        //    (account_value_load_after_deduction_[j]
        //    );
        std::transform
            (account_value_load_after_deduction_[j].begin()
            ,account_value_load_after_deduction_[j].end()
            ,account_value_load_after_deduction_[j].begin()
            ,i_upper_12_over_12_from_i<double>()
            );
        }

    // Deduct input extra asset comp as an account-value load, iff
    // database entity 'DB_AssetChargeType' has the value
    // 'e_asset_charge_load_after_ded'; otherwise, reflect it
    // elsewhere as an interest spread.
    if(e_asset_charge_load_after_ded == details.asset_charge_type_)
        {
        // ET !! Rewrite:
        // std::vector<double> extra_asset_comp = i_upper_12_over_12_from_i
        //   ((1.0L / 10000.0L) * details.VectorExtraAssetComp_
        //   );
        std::vector<double> extra_asset_comp = details.VectorExtraAssetComp_;
        std::transform
            (extra_asset_comp.begin()
            ,extra_asset_comp.end()
            ,extra_asset_comp.begin()
            ,boost::bind
                (i_upper_12_over_12_from_i<double>()
                ,boost::bind(std::multiplies<double>(), _1, 1.0 / 10000.0L)
                )
            );

        // ET !! Rewrite: inside the loop, it could be:
        //   account_value_load_after_deduction_[j] += extra_asset_comp;
        // ...yet OTOH any APL programmer would just write
        //   account_value_load_after_deduction_ +=
        //     reshape(extra_asset_comp, shape_of(account_value_load_after_deduction_));
        // and would that be sensible with expression templates?
        // No, probably not; optimized APL interpreters wouldn't allocate
        // any extra storage for this, but that's probably too much to ask
        // of an expression-template library. And if we were going to
        // emulate APL, we'd want RPN, too....
        //
        // ET !! As for rounding, we do want an expression-template library
        // to apply a scalar function like rounding to all elements of a
        // matrix, with some natural syntax like
        //   account_value_load_after_deduction_ = details.round_interest_rate_(account_value_load_after_deduction_);
        for(int j = e_currbasis; j != n_illreg_bases; j++)
            {
            // ET !! account_value_load_after_deduction_[j] += extra_asset_comp;
            std::transform
                (account_value_load_after_deduction_[j].begin()
                ,account_value_load_after_deduction_[j].end()
                ,extra_asset_comp.begin()
                ,account_value_load_after_deduction_[j].begin()
                ,std::plus<double>()
                );
            std::vector<double>::iterator k;
            // TODO ?? Test this. Why not use std::transform()?
            for
                (k = account_value_load_after_deduction_[j].begin()
                ;k != account_value_load_after_deduction_[j].end()
                ;++k
                )
                {
                *k = details.round_interest_rate_(*k);
                }
            }
        }
    else if(e_asset_charge_spread == details.asset_charge_type_)
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

    // Total load excludes monthly_policy_fee_, annual_policy_fee_, and
    // amortized_premium_tax_load_ because they are charges rather than loads.
    //
    // TODO ?? Is the specified-amount 'load' not a charge?

    // TODO ?? Clearly the common functionality should be factored out here.
    // TODO ?? It is probably unnecessary to handle the midpoint basis here.
    for(int j = e_currbasis; j < n_illreg_bases; j++)
        {
        // ET !! Naively, rewrite this whole loop body as:
        //
        //   std::vector target_total_load_before_premium_tax; [Declare outside loop.]
        //
        //   target_sales_load_[j] += details.VectorExtraCompLoad_;
        //   target_total_load_[j] = target_sales_load_[j];
        //   target_total_load_[j] += target_premium_load_[j] + dac_tax_load_.begin();
        //   if(e_currbasis == j)
        //     {
        //     target_total_load_before_premium_tax = target_total_load_[j];
        //     }
        //   target_total_load_[j] += premium_tax_load_;
        //
        //   excess_sales_load_[j] += details.VectorExtraCompLoad_;
        //   excess_total_load_[j] = excess_sales_load_[j];
        //   excess_total_load_[j] += excess_premium_load_[j] + dac_tax_load_.begin();
        //   if(e_currbasis == j)
        //     {
        //     excess_total_load_before_premium_tax = excess_total_load_[j];
        //     }
        //   excess_total_load_[j] += premium_tax_load_;
        //
        // Then, after the loop ends:
        //
        //  target_premium_load_7702_excluding_premium_tax_ = target_total_load_before_premium_tax;
        //  target_premium_load_7702_lowest_premium_tax_    = target_load_before_premium_tax + details.LowestPremTaxRate_;
        //
        //  excess_premium_load_7702_excluding_premium_tax_ = excess_total_load_before_premium_tax;
        //  excess_premium_load_7702_lowest_premium_tax_    = excess_load_before_premium_tax + details.LowestPremTaxRate_;
        //
        // Then go back and look at the ET version to see whether it can
        // be rewritten more clearly, now that it can be comprehended.
        //
        std::transform
            (target_sales_load_[j].begin()
            ,target_sales_load_[j].end()
            ,details.VectorExtraCompLoad_.begin()
            ,target_sales_load_[j].begin()
            ,std::plus<double>()
            );
        target_total_load_[j] = target_sales_load_[j];
        std::transform
            (target_total_load_[j].begin()
            ,target_total_load_[j].end()
            ,target_premium_load_[j].begin()
            ,target_total_load_[j].begin()
            ,std::plus<double>()
            );
        std::transform
            (target_total_load_[j].begin()
            ,target_total_load_[j].end()
            ,dac_tax_load_.begin()
            ,target_total_load_[j].begin()
            ,std::plus<double>()
            );
        if(e_currbasis == j)
            {
            target_premium_load_7702_excluding_premium_tax_ = target_total_load_[j];
            target_premium_load_7702_lowest_premium_tax_    = target_total_load_[j];
            std::transform
                (target_premium_load_7702_lowest_premium_tax_.begin()
                ,target_premium_load_7702_lowest_premium_tax_.end()
                ,target_premium_load_7702_lowest_premium_tax_.begin()
                ,std::bind2nd(std::plus<double>(), details.LowestPremTaxRate_)
                );
            }
        std::transform
            (target_total_load_[j].begin()
            ,target_total_load_[j].end()
            ,premium_tax_load_.begin()
            ,target_total_load_[j].begin()
            ,std::plus<double>()
            );

        std::transform
            (excess_sales_load_[j].begin()
            ,excess_sales_load_[j].end()
            ,details.VectorExtraCompLoad_.begin()
            ,excess_sales_load_[j].begin()
            ,std::plus<double>()
            );
        excess_total_load_[j] = excess_sales_load_[j];
        std::transform
            (excess_total_load_[j].begin()
            ,excess_total_load_[j].end()
            ,excess_premium_load_[j].begin()
            ,excess_total_load_[j].begin()
            ,std::plus<double>()
            );
        std::transform
            (excess_total_load_[j].begin()
            ,excess_total_load_[j].end()
            ,dac_tax_load_.begin()
            ,excess_total_load_[j].begin()
            ,std::plus<double>()
            );
        if(e_currbasis == j)
            {
            excess_premium_load_7702_excluding_premium_tax_ = excess_total_load_[j];
            excess_premium_load_7702_lowest_premium_tax_    = excess_total_load_[j];
            std::transform
                (excess_premium_load_7702_lowest_premium_tax_.begin()
                ,excess_premium_load_7702_lowest_premium_tax_.end()
                ,excess_premium_load_7702_lowest_premium_tax_.begin()
                ,std::bind2nd(std::plus<double>(), details.LowestPremTaxRate_)
                );
            }
        std::transform
            (excess_total_load_[j].begin()
            ,excess_total_load_[j].end()
            ,premium_tax_load_.begin()
            ,excess_total_load_[j].begin()
            ,std::plus<double>()
            );
        }

    // USER !! Explain this in user documentation.
    // Add 'VectorExtraPolFee_' to monthly policy fee on the current
    // basis only. It's added only to the current-basis policy fee
    // because it represents an extra current charge constrained by
    // the difference between the guaranteed and the (otherwise)
    // current policy fee. It was originally conceived as a fund
    // custodial fee, but can be used in any situation that's
    // consistent with this constraint.

    // ET !! monthly_policy_fee_[e_currbasis] += details.VectorExtraPolFee_;
    std::transform
        (monthly_policy_fee_[e_currbasis].begin()
        ,monthly_policy_fee_[e_currbasis].end()
        ,details.VectorExtraPolFee_.begin()
        ,monthly_policy_fee_[e_currbasis].begin()
        ,std::plus<double>()
        );
    for(int j = 0; j < details.length_; ++j)
        {
        if
            ( monthly_policy_fee_[e_guarbasis][j]
            < monthly_policy_fee_[e_currbasis][j]
            )
            {
            fatal_error()
                << "Duration "
                << j
                << ": current monthly policy fee "
                << monthly_policy_fee_[e_currbasis][j]
                << " (which includes a custodial fee of "
                << details.VectorExtraPolFee_[j]
                << ") improperly exceeds guaranteed maximum of "
                << monthly_policy_fee_[e_guarbasis][j]
                << " ."
                << LMI_FLUSH
                ;
            }
        }

    // Calculate midpoint as mean of current and guaranteed.
    // A different average might be used instead.
    if(is_subject_to_ill_reg(details.ledger_type_))
        {
        // ET !! Matrix operations are most welcome here:
        //   monthly_policy_fee_[e_mdptbasis] = mean(monthly_policy_fee_[e_guarbasis], monthly_policy_fee_[e_currbasis]);
        //   and so on, reiterating that line for every other name given here.
        std::transform
            (monthly_policy_fee_[e_guarbasis].begin()
            ,monthly_policy_fee_[e_guarbasis].end()
            ,monthly_policy_fee_[e_currbasis].begin()
            ,monthly_policy_fee_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (annual_policy_fee_[e_guarbasis].begin()
            ,annual_policy_fee_[e_guarbasis].end()
            ,annual_policy_fee_[e_currbasis].begin()
            ,annual_policy_fee_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (specified_amount_load_[e_guarbasis].begin()
            ,specified_amount_load_[e_guarbasis].end()
            ,specified_amount_load_[e_currbasis].begin()
            ,specified_amount_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (account_value_load_before_deduction_[e_guarbasis].begin()
            ,account_value_load_before_deduction_[e_guarbasis].end()
            ,account_value_load_before_deduction_[e_currbasis].begin()
            ,account_value_load_before_deduction_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (account_value_load_after_deduction_[e_guarbasis].begin()
            ,account_value_load_after_deduction_[e_guarbasis].end()
            ,account_value_load_after_deduction_[e_currbasis].begin()
            ,account_value_load_after_deduction_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (target_premium_load_[e_guarbasis].begin()
            ,target_premium_load_[e_guarbasis].end()
            ,target_premium_load_[e_currbasis].begin()
            ,target_premium_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (excess_premium_load_[e_guarbasis].begin()
            ,excess_premium_load_[e_guarbasis].end()
            ,excess_premium_load_[e_currbasis].begin()
            ,excess_premium_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (target_sales_load_[e_guarbasis].begin()
            ,target_sales_load_[e_guarbasis].end()
            ,target_sales_load_[e_currbasis].begin()
            ,target_sales_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (excess_sales_load_[e_guarbasis].begin()
            ,excess_sales_load_[e_guarbasis].end()
            ,excess_sales_load_[e_currbasis].begin()
            ,excess_sales_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (target_total_load_[e_guarbasis].begin()
            ,target_total_load_[e_guarbasis].end()
            ,target_total_load_[e_currbasis].begin()
            ,target_total_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        std::transform
            (excess_total_load_[e_guarbasis].begin()
            ,excess_total_load_[e_guarbasis].end()
            ,excess_total_load_[e_currbasis].begin()
            ,excess_total_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        }
}

//============================================================================
// Not implemented. The idea is to amortize premium tax as an
// annuity-certain. Sketch:
//
// Test parameters for sanity.
//
// Calculate a monthly annuity factor, reflecting the amortization
// rate and period.
//
// Apply the annuity factor to the appropriate charge: probably
// the rate the state actually charges, though the product's
// premium-tax load might be taken into account. It's also
// conceivable to amortize DAC tax similarly.
//
// Store the result in 'amortized_premium_tax_load_'. Modify
// any corresponding loads previously stored.
//
void Loads::AmortizePremiumTax(load_details const&)
{
    fatal_error() << "Premium-tax amortization not implemented." << LMI_FLUSH;
}

//============================================================================
// Antediluvian branch.
Loads::Loads(TDatabase const& database)
{
    monthly_policy_fee_   .resize(n_illreg_bases);
    target_premium_load_  .resize(n_illreg_bases);
    excess_premium_load_  .resize(n_illreg_bases);
    specified_amount_load_.resize(n_illreg_bases);

    database.Query(monthly_policy_fee_   [e_guarbasis], DB_GuarPolFee     );
    database.Query(target_premium_load_  [e_guarbasis], DB_GuarPremLoadTgt);
    database.Query(excess_premium_load_  [e_guarbasis], DB_GuarPremLoadExc);
    database.Query(specified_amount_load_[e_guarbasis], DB_GuarSpecAmtLoad);

    database.Query(monthly_policy_fee_   [e_currbasis], DB_CurrPolFee     );
    database.Query(target_premium_load_  [e_currbasis], DB_CurrPremLoadTgt);
    database.Query(excess_premium_load_  [e_currbasis], DB_CurrPremLoadExc);
    database.Query(specified_amount_load_[e_currbasis], DB_CurrSpecAmtLoad);

    // Calculate midpoint as mean of current and guaranteed.
    // A different average might be used instead.
    if(is_subject_to_ill_reg(database.Query(DB_LedgerType)))
        {
        monthly_policy_fee_   [e_mdptbasis].resize(database.length());
        // ET !! Matrix operations are most welcome here:
        //   monthly_policy_fee_[e_mdptbasis] = mean(monthly_policy_fee_[e_guarbasis], monthly_policy_fee_[e_currbasis]);
        //   and so on, reiterating that line for every other name given here.
        std::transform
            (monthly_policy_fee_[e_guarbasis].begin()
            ,monthly_policy_fee_[e_guarbasis].end()
            ,monthly_policy_fee_[e_currbasis].begin()
            ,monthly_policy_fee_[e_mdptbasis].begin()
            ,mean<double>()
            );
        target_premium_load_  [e_mdptbasis].resize(database.length());
        std::transform
            (target_premium_load_[e_guarbasis].begin()
            ,target_premium_load_[e_guarbasis].end()
            ,target_premium_load_[e_currbasis].begin()
            ,target_premium_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        excess_premium_load_  [e_mdptbasis].resize(database.length());
        std::transform
            (excess_premium_load_[e_guarbasis].begin()
            ,excess_premium_load_[e_guarbasis].end()
            ,excess_premium_load_[e_currbasis].begin()
            ,excess_premium_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        specified_amount_load_[e_mdptbasis].resize(database.length());
        std::transform
            (specified_amount_load_[e_guarbasis].begin()
            ,specified_amount_load_[e_guarbasis].end()
            ,specified_amount_load_[e_currbasis].begin()
            ,specified_amount_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        }

    premium_tax_load_.push_back(0.0);
    dac_tax_load_    .push_back(0.0);
}

