// Premiums, loans, and withdrawals.
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

// $Id: outlay.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "outlay.hpp"

#include "basic_values.hpp"
#include "inputs.hpp"

#include <algorithm>
#include <iterator>

//============================================================================
Outlay::Outlay(BasicValues const& basic_values)
{
    Init(basic_values);
}

//============================================================================
void Outlay::Init(BasicValues const& basic_values)
{
    InputParms const& in = *basic_values.Input;

    dumpin_               = in.Dumpin;
    external_1035_amount_ = in.External1035ExchangeAmount.value();
    internal_1035_amount_ = in.Internal1035ExchangeAmount.value();

    std::copy(in.EePremium.begin(), in.EePremium.end(), std::back_inserter(ee_modal_premiums_));
    std::copy(in.ErPremium.begin(), in.ErPremium.end(), std::back_inserter(er_modal_premiums_));
    std::copy(in.EeMode   .begin(), in.EeMode   .end(), std::back_inserter(ee_premium_modes_ ));
    std::copy(in.ErMode   .begin(), in.ErMode   .end(), std::back_inserter(er_premium_modes_ ));
    std::copy(in.Loan     .begin(), in.Loan     .end(), std::back_inserter(new_cash_loans_   ));
    std::copy(in.WD       .begin(), in.WD       .end(), std::back_inserter(withdrawals_      ));
}

//============================================================================
void Outlay::set_ee_modal_premiums(double z, int from_year, int to_year)
{
    std::fill_n(ee_modal_premiums_.begin() + from_year, to_year - from_year, z);
}

//============================================================================
void Outlay::set_er_modal_premiums(double z, int from_year, int to_year)
{
    std::fill_n(er_modal_premiums_.begin() + from_year, to_year - from_year, z);
}

//============================================================================
void Outlay::set_new_cash_loans(double z, int from_year, int to_year)
{
    std::fill_n(new_cash_loans_.begin() + from_year, to_year - from_year, z);
}

//============================================================================
void Outlay::set_withdrawals(double z, int from_year, int to_year)
{
    std::fill_n(withdrawals_.begin() + from_year, to_year - from_year, z);
}

