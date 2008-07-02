// Basic values.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: basicvalues.cpp,v 1.19 2008-07-02 22:39:16 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "basic_values.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "rounding_rules.hpp"
#include "surrchg_rates.hpp"

#include <algorithm> // std::max()
#include <cmath>     // std::pow()

// TODO ??  Instead of this hardcoded path, use a configuration file.
//
char const* CurrentTableFile()    {return "/opt/lmi/data/sample";}
char const* GuaranteedTableFile() {return "/opt/lmi/data/qx_cso";}

// INELEGANT !! Many member variables are initialized not in ctors,
// but rather in common initialization functions.

//============================================================================
BasicValues::BasicValues()
    :Input_(new InputParms)
    ,yare_input_(*Input_)
{
    Init();
}

//============================================================================
BasicValues::BasicValues(InputParms const& input)
    :Input_(new InputParms(input))
    ,yare_input_(input)
{
    Init();
}

//============================================================================
BasicValues::~BasicValues()
{
}

//============================================================================
void BasicValues::Init()
{
    PremiumTaxLoadIsTieredInStateOfJurisdiction = false;

    // Bind to input and database representing policy form.

    Length   = Input_->YearsToMaturity();
    IssueAge = Input_->Status[0].IssueAge.value();
    RetAge   = Input_->Status[0].RetAge.value();
    LMI_ASSERT(IssueAge <= RetAge);

    Database_.reset
        (new TDatabase
            ("empty for now" // filename
            ,yare_input_.Gender
            ,yare_input_.UnderwritingClass
            ,yare_input_.Smoking
            ,yare_input_.IssueAge
            ,yare_input_.GroupUnderwritingType
            ,yare_input_.State
            )
        );

    LedgerType = Input_->LedgerType();
    IsSubjectToIllustrationReg_ = is_subject_to_ill_reg(LedgerType);

    RoundingRules_.reset(new rounding_rules);

    // TODO ?? Just a dummy initialization for now.
    SpreadFor7702_.assign(Length, 0.0);

    // Multilife contracts will need a vector of mortality-rate objects.
    MortalityRates_.reset(new MortalityRates (*this));
    InterestRates_ .reset(new InterestRates  (*this));
    SurrChgRates_  .reset(new SurrChgRates   (*Database_));
    DeathBfts_     .reset(new death_benefits (*this));
    Outlay_        .reset(new Outlay         (*this));
    Loads_         .reset(new Loads(*Database_, IsSubjectToIllustrationReg()));

    MinSpecAmt = Database_->Query(DB_MinSpecAmt);
    MinWD      = Database_->Query(DB_MinWD     );
    WDFee      = Database_->Query(DB_WDFee     );
    WDFeeRate  = Database_->Query(DB_WDFeeRate );

// The antediluvian branch leaves FundData_, StratifiedCharges_, and
// ProductData initialized to null pointers.
}

//============================================================================
double BasicValues::InvestmentManagementFee() const
{
    // TODO ?? Just a stub for now.
    return 0.0;
}

//============================================================================
// TODO ?? Simply calls the target-premium routine for now.
double BasicValues::GetModalMinPrem
    (int           a_Year
    ,e_mode const& a_Mode
    ,double        a_SpecAmt
    ) const
{
    return GetModalTgtPrem(a_Year, a_Mode, a_SpecAmt);
}

//============================================================================
double BasicValues::GetModalTgtPrem
    (int           a_Year
    ,e_mode const& a_Mode
    ,double        a_SpecAmt
    ) const
{
    // TODO ?? Simplistic. Ignores table ratings, flat extras, and
    // riders. The interest rates are arbitrary, and given as repeated
    // floating literals; they should come from the database instead.

    double spread = 0.0;

    switch(a_Mode.value())
        {
        case e_annual:
            {
            spread = 0.0200;
            }
            break;
        case e_semiannual:
            {
            spread = 0.0100;
            }
            break;
        case e_quarterly:
            {
            spread = 0.0050;
            }
            break;
        case e_monthly:
            {
            spread = 0.0000;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << a_Mode.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    double u = 1.0 + std::max
        (.03
        ,    InterestRates_->GenAcctNetRate
                (e_basis(e_currbasis)
                ,e_rate_period(e_monthly_rate)
                )[a_Year]
        -   spread
        );
    // TODO ?? Write a functor to do this.
    double Annuity = (1.0 - std::pow(u, 12 / a_Mode.value())) / (1.0 - u);

    double z = a_SpecAmt;
    z /=
        (   1.0
        +   InterestRates_->GenAcctNetRate
                (e_basis(e_guarbasis)
                ,e_rate_period(e_monthly_rate)
                )[a_Year]
        );
    z *= MortalityRates_->MonthlyCoiRates(e_currbasis)[a_Year];
    z += Loads_->monthly_policy_fee(e_currbasis)[a_Year];
// TODO ?? Would rider charges depend on month?
//    z += AdbRate;
//    z *= 1.0 + WpRate;
    z /= 1.0 - Loads_->target_premium_load(e_currbasis)[a_Year];
    z *= Annuity;

    // TODO ?? Parameterize this.
    static round_to<double> const round_it(2, r_upward);
    return round_it(z);
}

//============================================================================
// Simply calls the target-specamt routine for now.
double BasicValues::GetModalMaxSpecAmt
    (e_mode const& Mode
    ,double Pmt
    ) const
{
    return GetModalTgtSpecAmt(Mode, Pmt);
}

//============================================================================
double BasicValues::GetModalTgtSpecAmt
    (e_mode const& Mode
    ,double Pmt
    ) const
{
    // TODO ?? Factor out the (defectively simplistic) code this
    // shares with GetModalTgtPrem().
    double spread = 0.0;

    switch(Mode.value())
        {
        case e_annual:
            {
            spread = 0.0200;
            }
            break;
        case e_semiannual:
            {
            spread = 0.0100;
            }
            break;
        case e_quarterly:
            {
            spread = 0.0050;
            }
            break;
        case e_monthly:
            {
            spread = 0.0000;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Mode.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    double u = 1.0 + std::max
        (.03
        ,   InterestRates_->GenAcctNetRate
                (e_basis(e_currbasis)
                ,e_rate_period(e_monthly_rate)
                )[0]
            -   spread
        );
    double Annuity = (1.0 - std::pow(u, 12 / Mode.value())) / (1.0 - u);

    double z = Pmt;
    z /= Annuity;
    z *= 1.0 - Loads_->target_premium_load(e_currbasis)[0];
//    z /= WpRate;
//    z -= AdbRate;
    z -= Loads_->monthly_policy_fee(e_currbasis)[0];
    z /= MortalityRates_->MonthlyCoiRates(e_currbasis)[0];
    z *=
        (   1.0
        +   InterestRates_->GenAcctNetRate
                (e_basis(e_guarbasis)
                ,e_rate_period(e_monthly_rate)
                )[0]
        );

    // TODO ?? Parameterize this.
    static round_to<double> const round_it(0, r_downward);
    return round_it(z);
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

