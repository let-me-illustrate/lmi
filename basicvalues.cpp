// Basic values.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: basicvalues.cpp,v 1.3 2005-02-14 04:37:51 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "basic_values.hpp"

#include "alert.hpp"
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

// TODO ??  Instead of this hardcoded path, use either the location
// FHS would recommend, or a configuration file.
//
char const* CurrentTableFile()    {return "/lmi-data/sample";}
char const* GuaranteedTableFile() {return "/lmi-data/qx_cso";}

// INELEGANT !! Many member variables are initialized not in ctors,
// but rather in common initialization functions.

//============================================================================
BasicValues::BasicValues()
{
    Input = new InputParms;
    Init();
}

//============================================================================
BasicValues::BasicValues(InputParms const& input)
{
    Input = new InputParms(input);
    Init();
}

//============================================================================
BasicValues::BasicValues(BasicValues const& obj)
{
    Input = new InputParms(*obj.Input);
    Init();
}

//============================================================================
BasicValues& BasicValues::operator=(BasicValues const& obj)
{
    if(this != &obj)
        {
        delete Input;
        Input = new InputParms(*obj.Input);
        Init();
        }
    return *this;
}

//============================================================================
BasicValues::~BasicValues()
{
    delete Input;

    delete Database;
    delete MortalityRates_;
    delete InterestRates_;
    delete SurrChgRates_;
    delete DeathBfts_;
    delete Outlay_;
    delete Loads_;
}

//============================================================================
void BasicValues::Init()
{
    // Bind to input and database representing policy form.

    InputStatus const& S = Input->Status[0]; // TODO ?? Database based on first life only?
    Length = Input->YearsToMaturity();
    IssueAge = S.IssueAge.value();
    RetAge = S.RetAge.value();
    LMI_ASSERT(IssueAge <= RetAge);

    Database = new TDatabase
        ("empty for now" // filename
        ,S.Gender
        ,S.Class
        ,S.Smoking
        ,S.IssueAge
        ,Input->GroupUWType
        ,Input->InsdState
        );

    LedgerType = e_ledger_type(e_ill_reg);

    RoundingRules_ = new rounding_rules;

    // TODO ?? Just a dummy initialization for now.
    SpreadFor7702_.assign(Length, 0.0);

    // Multilife contracts will need a vector of mortality-rate objects.
    MortalityRates_ = new MortalityRates (*this);
    InterestRates_  = new InterestRates  (*this);
    SurrChgRates_   = new SurrChgRates   (*Database);
    DeathBfts_      = new death_benefits (*this);
    Outlay_         = new Outlay         (*this);
    Loads_          = new Loads          (*Database);

    MinSpecAmt = Database->Query(DB_MinSpecAmt);
    MinWD      = Database->Query(DB_MinWD     );
    WDFee      = Database->Query(DB_WDFee     );
    WDFeeRate  = Database->Query(DB_WDFeeRate );

    FundData_ = 0;
    TieredCharges_ = 0;
    ProductData = 0;
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
//    z += ADDrate;
//    z *= 1.0 + WPrate;
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
//    z /= WPRate;
//    z -= ADDRate;
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

