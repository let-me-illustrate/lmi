// Loads and expense charges.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005 Gregory W. Chicares.
// Portions marked JLM Copyright (C) 2001 Gregory W. Chicares and Joseph L. Murdzek.
// Author is GWC except where specifically noted otherwise.
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

// $Id: ihs_loads.cpp,v 1.5 2005-07-15 12:40:59 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "loads.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "ihs_rnddata.hpp"
#include "inputs.hpp"
#include "math_functors.hpp"

#include <boost/bind.hpp>

#include <algorithm>
#include <cmath>     // std::pow()
#include <functional>

class load_details
{
  public:
    load_details
        (bool                       a_AmortizePremLoad
        ,double                     a_LowestPremTaxRate
        ,round_to<double>    const& a_round_interest_rate
        ,std::vector<double> const& a_VectorExtraCompLoad
        ,std::vector<double> const& a_VectorExtraAssetComp
        ,std::vector<double> const& a_VectorExtraPolFee
        )
        :AmortizePremLoad     (a_AmortizePremLoad)
        ,LowestPremTaxRate    (a_LowestPremTaxRate)
        ,round_interest_rate      (a_round_interest_rate)
        ,VectorExtraCompLoad  (a_VectorExtraCompLoad)
        ,VectorExtraAssetComp (a_VectorExtraAssetComp)
        ,VectorExtraPolFee    (a_VectorExtraPolFee)
        {}

    bool                       AmortizePremLoad;
    double                     LowestPremTaxRate;
    round_to<double>    const& round_interest_rate;
    std::vector<double> const& VectorExtraCompLoad;
    std::vector<double> const& VectorExtraAssetComp;
    std::vector<double> const& VectorExtraPolFee;
};

//============================================================================
Loads::Loads(BasicValues& V)
{
    load_details details
        (V.Input_->AmortizePremLoad
        ,V.GetLowestPremTaxRate()
        ,V.GetRoundingRules().round_interest_rate()
        ,V.Input_->VectorAddonCompOnPremium
        ,V.Input_->VectorAddonCompOnAssets
        ,V.Input_->VectorAddonMonthlyCustodialFee
        );
    Init(*V.Database_, details);
}

//============================================================================
Loads::Loads
    (bool                       AmortizePremLoad
    ,TDatabase           const& database
    ,round_to<double>    const& round_interest_rate
    ,std::vector<double> const& VectorExtraCompLoad
    ,std::vector<double> const& VectorExtraAssetComp
    ,std::vector<double> const& VectorExtraPolFee
    ,double                     LowestPremTaxRate
    )
{
    load_details details
        (AmortizePremLoad
        ,LowestPremTaxRate
        ,round_interest_rate
        ,VectorExtraCompLoad
        ,VectorExtraAssetComp
        ,VectorExtraPolFee
        );
    Init(database, details);
}

//============================================================================
void Loads::Init
    (TDatabase    const& database
    ,load_details const& details
    )
{
    monthly_policy_fee_                  .resize(n_illreg_bases);
    annual_policy_fee_                   .resize(n_illreg_bases);
    target_premium_load_                 .resize(n_illreg_bases);
    excess_premium_load_                 .resize(n_illreg_bases);
    target_sales_load_                   .resize(n_illreg_bases);
    excess_sales_load_                   .resize(n_illreg_bases);
    target_total_load_                   .resize(n_illreg_bases);
    excess_total_load_                   .resize(n_illreg_bases);
    specified_amount_load_               .resize(n_illreg_bases);
    account_value_load_before_deduction_ .resize(n_illreg_bases);
    account_value_load_after_deduction_  .resize(n_illreg_bases);

    int length = database.length();
    for(int j = e_currbasis; j < n_illreg_bases; j++)
        {
        // TODO ?? Consider skipping midpoint basis if unneeded.
        monthly_policy_fee_                  [j].resize(length);
        annual_policy_fee_                   [j].resize(length);
        target_premium_load_                 [j].resize(length);
        excess_premium_load_                 [j].resize(length);
        target_sales_load_                   [j].resize(length);
        excess_sales_load_                   [j].resize(length);
        target_total_load_                   [j].resize(length);
        excess_total_load_                   [j].resize(length);
        specified_amount_load_               [j].resize(length);
        account_value_load_before_deduction_ [j].resize(length);
        account_value_load_after_deduction_  [j].resize(length);
        }
    target_premium_load_7702_excluding_premium_tax_.resize(length);
    excess_premium_load_7702_excluding_premium_tax_.resize(length);
    target_premium_load_7702_lowest_premium_tax_   .resize(length);
    excess_premium_load_7702_lowest_premium_tax_   .resize(length);
    premium_tax_load_                              .resize(length);
    amortized_premium_tax_load_                    .resize(length);
    dac_tax_load_                                  .resize(length);

    database.Query(refundable_sales_load_proportion_, DB_PremRefund);

    database.Query(monthly_policy_fee_                  [e_guarbasis]   ,DB_GuarPolFee          );
    database.Query(annual_policy_fee_                   [e_guarbasis]   ,DB_GuarIssueFee        );
    database.Query(specified_amount_load_               [e_guarbasis]   ,DB_GuarSpecAmtLoad     );
    database.Query(account_value_load_before_deduction_ [e_guarbasis]   ,DB_GuarAcctValLoadBOM  );
    database.Query(account_value_load_after_deduction_  [e_guarbasis]   ,DB_GuarAcctValLoadAMD  );

    database.Query(monthly_policy_fee_                  [e_currbasis]   ,DB_CurrPolFee          );
    database.Query(annual_policy_fee_                   [e_currbasis]   ,DB_CurrIssueFee        );
    database.Query(specified_amount_load_               [e_currbasis]   ,DB_CurrSpecAmtLoad     );
    database.Query(account_value_load_before_deduction_ [e_currbasis]   ,DB_CurrAcctValLoadBOM  );
    database.Query(account_value_load_after_deduction_  [e_currbasis]   ,DB_CurrAcctValLoadAMD  );

    // put account_value_load_after_deduction_ on a monthly basis
    for(int j = e_currbasis; j != n_illreg_bases; j++)
        {
// Authors of this block: GWC and JLM.
        // TODO ?? JOE Here and below, don't you really want transform()
        // with the functor? See the example below.
        std::vector<double>::iterator i;
        for
            (i  = account_value_load_after_deduction_[j].begin()
            ;i != account_value_load_after_deduction_[j].end()
            ;++i
            )
            {
            *i = i_upper_12_over_12_from_i<double>()(*i);
            }
        }

    // Iff asset comp type is e_extra_comp_load_after_ded,
    // then add in miscellaneous fund charge and input extra asset comp.
    if(e_asset_charge_load_after_ded == database.Query(DB_AssetChargeType))
        {
        std::vector<double> miscellaneous_fund_charge;
        database.Query(miscellaneous_fund_charge, DB_MiscFundCharge);
        std::vector<double>::iterator i;
        // TODO ?? JOE See note above.
        for(i = miscellaneous_fund_charge.begin(); i != miscellaneous_fund_charge.end(); ++i)
            {
// Authors of this block: GWC and JLM.
            *i = i_upper_12_over_12_from_i<double>()(*i);
            }

        // get input extra asset comp, and put it on a monthly basis
        //
        std::vector<double> extra_asset_comp = details.VectorExtraAssetComp;

        std::transform
            (extra_asset_comp.begin()
            ,extra_asset_comp.end()
            ,extra_asset_comp.begin()
            ,boost::bind
                (i_upper_12_over_12_from_i<double>()
                ,boost::bind(std::multiplies<double>(), _1, 1.0 / 10000.0L)
                )
            );

        for(int j = e_currbasis; j != n_illreg_bases; j++)
            {
            std::transform
                (account_value_load_after_deduction_[j].begin()
                ,account_value_load_after_deduction_[j].end()
                ,miscellaneous_fund_charge.begin()
                ,account_value_load_after_deduction_[j].begin()
                ,std::plus<double>()
                );
            std::transform
                (account_value_load_after_deduction_[j].begin()
                ,account_value_load_after_deduction_[j].end()
                ,extra_asset_comp.begin()
                ,account_value_load_after_deduction_[j].begin()
                ,std::plus<double>()
                );
            std::vector<double>::iterator k;
            // TODO ?? Who wrote this, and did it ever work?
            for
                (k = account_value_load_after_deduction_[j].begin()
                ;k != account_value_load_after_deduction_[j].end()
                ;++k
                )
                {
                *k = details.round_interest_rate(*k);
                }
            }
        }
    else if(e_asset_charge_spread == database.Query(DB_AssetChargeType))
        {
        // Do nothing here: handle in interest-rate class instead.
        }
    else
        {
        fatal_error()
            << "Case '"
            << database.Query(DB_AssetChargeType)
            << "' not found."
            << LMI_FLUSH
            ;
        }

    if(details.AmortizePremLoad)
        {
        AmortizePremiumTax(database);
        }
    else
        {
        database.Query(target_premium_load_ [e_guarbasis]   ,DB_GuarPremLoadTgt   );
        database.Query(excess_premium_load_ [e_guarbasis]   ,DB_GuarPremLoadExc   );
        database.Query(target_sales_load_   [e_guarbasis]   ,DB_GuarPremLoadTgtRfd);
        database.Query(excess_sales_load_   [e_guarbasis]   ,DB_GuarPremLoadExcRfd);

        database.Query(target_premium_load_ [e_currbasis]   ,DB_CurrPremLoadTgt   );
        database.Query(excess_premium_load_ [e_currbasis]   ,DB_CurrPremLoadExc   );
        database.Query(target_sales_load_   [e_currbasis]   ,DB_CurrPremLoadTgtRfd);
        database.Query(excess_sales_load_   [e_currbasis]   ,DB_CurrPremLoadExcRfd);

        database.Query(premium_tax_load_ ,DB_PremTaxLoad   );
        database.Query(dac_tax_load_     ,DB_DACTaxPremLoad);
        }

    // Total load excludes monthly_policy_fee_, annual_policy_fee_, and
    // amortized_premium_tax_load_ because they are charges rather than loads.

    // TODO ?? Clearly the common functionality should be factored out here.
    // TODO ?? It is probably unnecessary to handle the midpoint basis here.
    for(int j = e_currbasis; j < n_illreg_bases; j++)
        {
        std::transform
            (target_sales_load_[j].begin()
            ,target_sales_load_[j].end()
            ,details.VectorExtraCompLoad.begin()
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
                ,std::bind2nd(std::plus<double>(), details.LowestPremTaxRate)
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
            ,details.VectorExtraCompLoad.begin()
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
                ,std::bind2nd(std::plus<double>(), details.LowestPremTaxRate)
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

    // Add 'VectorExtraPolFee' to monthly policy fee on the current
    // basis only. It's added only to the current-basis policy fee
    // because it represents an extra current charge constrained by
    // the difference between the guaranteed and the (otherwise)
    // current policy fee. It was originally conceived as a fund
    // custodial fee, but can be used in any situation that's
    // consistent with this constraint.

    std::transform
        (monthly_policy_fee_[e_currbasis].begin()
        ,monthly_policy_fee_[e_currbasis].end()
        ,details.VectorExtraPolFee.begin()
        ,monthly_policy_fee_[e_currbasis].begin()
        ,std::plus<double>()
        );
    for(int j = 0; j < length; ++j)
        {
        if
            ( monthly_policy_fee_[e_guarbasis][j]
            < monthly_policy_fee_[e_currbasis][j]
            )
            {
            hobsons_choice()
                << "Duration "
                << j
                << ": current monthly policy fee "
                << monthly_policy_fee_[e_currbasis][j]
                << " (which includes a custodial fee of "
                << details.VectorExtraPolFee[j]
                << ") improperly exceeds guaranteed maximum of "
                << monthly_policy_fee_[e_guarbasis][j]
                << " so the illustration will be incorrect."
                << LMI_FLUSH
                ;
            }
        }

    // Calculate midpoint as mean of current and guaranteed.
    // A different average might be used instead.
    if(is_subject_to_ill_reg(database.Query(DB_LedgerType)))
        {
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
        }
}

//============================================================================
// Premium tax amortization as dollar certain charge.
// Charge is a function of amortization period and interest rate.
// The interest rate is specified in the database and can differ
// from all other interest rates.
//
// TODO ?? These preconditions should be asserted:
//   For now, correct only for single premium paid annually
//   For now, correct only for scalar amortization rates
void Loads::AmortizePremiumTax(TDatabase const& database)
{
    int const period = static_cast<int>(
        database.Query(DB_PmTxAmortPeriod)
        );
    double const i = database.Query(DB_PmTxAmortIntRate);
    double z = 0.0;

    if(0.0 != i)
// TODO ?? What if 'i' is zero?
        {
// Authors of this block: GWC and JLM.
        // TODO ?? Write a functor to do this.
        double const u = 1.0 + i;
        double const mly_annuity_factor =
                i
            /   ((1.0 - std::pow(u, -period)) * u)
            *   d_upper_12_from_i<double>()(i)
            /   (i / u)
            ;

        // JOE Now premium tax rate payable by ins co is distinct
        // from premium tax load. I based this on the first, so
        // you can zero out the second and still get this.
        z =
                database.Query(DB_PremTaxRate)
            *   mly_annuity_factor
            *   (1.0 / 12.0)    // monthly
            ;
        }

    amortized_premium_tax_load_.assign(database.length(), z);
}

