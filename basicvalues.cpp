// Basic values.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "basic_values.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "death_benefits.hpp"
#include "input.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "premium_tax.hpp"

#include <algorithm>                    // max()
#include <cmath>                        // pow()

// IHS !! Instead of these hardcoded paths, lmi uses a configuration file.
//
char const* CurrentTableFile()    {return "/opt/lmi/data/sample";}
char const* GuaranteedTableFile() {return "/opt/lmi/data/qx_cso";}

// INELEGANT !! Many member variables are initialized not in ctors,
// but rather in common initialization functions.

//============================================================================
BasicValues::BasicValues(Input const& input)
    :yare_input_              {input}
    ,product_                 {}
    ,database_
        ("filename--empty for antediluvian fork"
        ,yare_input_.Gender
        ,yare_input_.UnderwritingClass
        ,yare_input_.Smoking
        ,yare_input_.IssueAge
        ,yare_input_.GroupUnderwritingType
        ,yare_input_.StateOfJurisdiction
        )
    ,StateOfJurisdiction_     {mce_s_CT}
    ,StateOfDomicile_         {mce_s_CT}
    ,PremiumTaxState_         {mce_s_CT}
    ,round_specamt_           {0, r_upward    }
    ,round_death_benefit_     {2, r_to_nearest}
    ,round_naar_              {2, r_to_nearest}
    ,round_coi_rate_          {8, r_downward  }
    ,round_coi_charge_        {2, r_to_nearest}
    ,round_rider_charges_     {2, r_to_nearest}
    ,round_gross_premium_     {2, r_to_nearest}
    ,round_net_premium_       {2, r_to_nearest}
    ,round_interest_rate_     {0, r_not_at_all}
    ,round_interest_credit_   {2, r_to_nearest}
    ,round_withdrawal_        {2, r_to_nearest}
    ,round_loan_              {2, r_to_nearest}
    ,round_interest_rate_7702_{0, r_not_at_all}
    ,round_corridor_factor_   {2, r_to_nearest}
    ,round_surrender_charge_  {2, r_to_nearest}
    ,round_irr_               {4, r_downward  }
    ,round_min_specamt_       {0, r_upward    }
    ,round_max_specamt_       {0, r_downward  }
    ,round_min_premium_       {2, r_upward    }
    ,round_max_premium_       {2, r_downward  }
    ,round_minutiae_          {2, r_to_nearest}
{
    Init();
}

//============================================================================
void BasicValues::Init()
{
    // Bind to input and database representing policy form.

    IssueAge = yare_input_.IssueAge;
    RetAge   = yare_input_.RetirementAge;
    LMI_ASSERT(IssueAge <= RetAge);

    StateOfJurisdiction_ = yare_input_.StateOfJurisdiction;
    PremiumTaxState_     = yare_input_.PremiumTaxState    ;

    // The database class constrains maturity age to be scalar.
    database().query_into(DB_MaturityAge   , EndtAge);
    Length = EndtAge - IssueAge;

    database().query_into(DB_LedgerType    , ledger_type_);
    database().query_into(DB_Nonillustrated, nonillustrated_);
    bool no_longer_issued = database().query<bool>(DB_NoLongerIssued);
    bool is_new_business  = yare_input_.EffectiveDate == yare_input_.InforceAsOfDate;
    no_can_issue_         = no_longer_issued && is_new_business;
    IsSubjectToIllustrationReg_ = is_subject_to_ill_reg(ledger_type());

    // IHS !! Just a dummy initialization here--implemented in lmi.
    SpreadFor7702_.assign(Length, 0.0);

    // Multilife contracts will need a vector of mortality-rate objects.
    MortalityRates_.reset(new MortalityRates (*this));
    InterestRates_ .reset(new InterestRates  (*this));
    DeathBfts_     .reset(new death_benefits (GetLength(), yare_input_, round_specamt_));
    Outlay_        .reset(new modal_outlay   (yare_input_, round_gross_premium_, round_withdrawal_, round_loan_));
    PremiumTax_    .reset(new premium_tax    (PremiumTaxState_, database()));
    Loads_         .reset(new Loads(database(), IsSubjectToIllustrationReg()));

    MinSpecAmt = round_specamt   ().c(database().query<double>(DB_MinSpecAmt));
    MinWD      = round_withdrawal().c(database().query<double>(DB_MinWd     ));
    WDFee      = round_withdrawal().c(database().query<double>(DB_WdFee     ));
    database().query_into(DB_WdFeeRate , WDFeeRate );

// The antediluvian branch leaves FundData_, StratifiedCharges_, and
// ProductData initialized to null pointers.
}

//============================================================================
double BasicValues::InvestmentManagementFee() const
{
    // IHS !! Just a stub here--implemented in lmi.
    return 0.0;
}

//============================================================================
// IHS !! Simply calls the target-premium routine for now--see lmi.
currency BasicValues::GetModalMinPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    return GetModalTgtPrem(a_year, a_mode, a_specamt);
}

//============================================================================
currency BasicValues::GetModalTgtPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    // IHS !! Simplistic. Ignores table ratings, flat extras, and
    // riders. The interest rates are arbitrary, and given as repeated
    // floating literals; they should come from the database instead.
    // See lmi for a much better implementation.

    double spread = 0.0;

    switch(a_mode)
        {
        case mce_annual:
            {
            spread = 0.0200;
            }
            break;
        case mce_semiannual:
            {
            spread = 0.0100;
            }
            break;
        case mce_quarterly:
            {
            spread = 0.0050;
            }
            break;
        case mce_monthly:
            {
            spread = 0.0000;
            }
            break;
        }

    double u = 1.0 + std::max
        (.03
        ,    InterestRates_->GenAcctNetRate
                (mce_gen_curr
                ,mce_monthly_rate
                )[a_year]
        -   spread
        );
    // IHS !! Implemented better in lmi.
    double Annuity = (1.0 - std::pow(u, 12 / a_mode)) / (1.0 - u);

    double z = dblize(a_specamt);
    z /=
        (   1.0
        +   InterestRates_->GenAcctNetRate
                (mce_gen_guar
                ,mce_monthly_rate
                )[a_year]
        );
    z *= MortalityRates_->MonthlyCoiRates(mce_gen_curr)[a_year];
    z += dblize(Loads_->monthly_policy_fee(mce_gen_curr)[a_year]);
//    z += AdbRate;
//    z *= 1.0 + WpRate;
    z /= 1.0 - Loads_->target_premium_load(mce_gen_curr)[a_year];
    z *= Annuity;

    // IHS !! Parameterized in lmi.
    static round_to<double> const round_it(2, r_upward);
    return round_it.c(z);
}

//============================================================================
// Simply calls the target-specamt routine for now.
currency BasicValues::GetModalMaxSpecAmt
    (mcenum_mode a_mode
    ,currency    a_pmt
    ) const
{
    return GetModalTgtSpecAmt(a_mode, a_pmt);
}

//============================================================================
currency BasicValues::GetModalTgtSpecAmt
    (mcenum_mode a_mode
    ,currency    a_pmt
    ) const
{
    // IHS !! Factor out the (defectively simplistic) code this
    // shares with GetModalTgtPrem()--see lmi.
    double spread = 0.0;

    switch(a_mode)
        {
        case mce_annual:
            {
            spread = 0.0200;
            }
            break;
        case mce_semiannual:
            {
            spread = 0.0100;
            }
            break;
        case mce_quarterly:
            {
            spread = 0.0050;
            }
            break;
        case mce_monthly:
            {
            spread = 0.0000;
            }
            break;
        }

    double u = 1.0 + std::max
        (.03
        ,   InterestRates_->GenAcctNetRate
                (mce_gen_curr
                ,mce_monthly_rate
                )[0]
            -   spread
        );
    double Annuity = (1.0 - std::pow(u, 12 / a_mode)) / (1.0 - u);

    double z = dblize(a_pmt);
    z /= Annuity;
    z *= 1.0 - Loads_->target_premium_load(mce_gen_curr)[0];
//    z /= WpRate;
//    z -= AdbRate;
    z -= dblize(Loads_->monthly_policy_fee(mce_gen_curr)[0]);
    z /= MortalityRates_->MonthlyCoiRates(mce_gen_curr)[0];
    z *=
        (   1.0
        +   InterestRates_->GenAcctNetRate
                (mce_gen_guar
                ,mce_monthly_rate
                )[0]
        );

    // IHS !! Parameterized in lmi.
    static round_to<double> const round_it(0, r_downward);
    return round_it.c(z);
}

//============================================================================
std::vector<double> const& BasicValues::SpreadFor7702() const
{
    return SpreadFor7702_;
}

//============================================================================
std::vector<double> const& BasicValues::GetCorridorFactor() const
{
    return MortalityRates_->CvatCorridorFactors();
}

//============================================================================
std::vector<double> BasicValues::GetCurrSpecAmtLoadTable() const
{
    return std::vector<double>(GetLength());
}

//============================================================================
std::vector<double> BasicValues::GetGuarSpecAmtLoadTable() const
{
    return std::vector<double>(GetLength());
}
