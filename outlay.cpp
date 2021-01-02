// Premiums, loans, and withdrawals.
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

#include "outlay.hpp"

#include "assert_lmi.hpp"
#include "yare_input.hpp"

#include <algorithm>

modal_outlay::modal_outlay
    (yare_input       const& yi
    ,round_to<double> const& round_gross_premium
    ,round_to<double> const& round_withdrawal
    ,round_to<double> const& round_loan
    )
    :round_gross_premium_  {round_gross_premium}
    ,round_withdrawal_     {round_withdrawal   }
    ,round_loan_           {round_loan         }
    ,dumpin_               {round_gross_premium_(yi.Dumpin)                    }
    ,external_1035_amount_ {round_gross_premium_(yi.External1035ExchangeAmount)}
    ,internal_1035_amount_ {round_gross_premium_(yi.Internal1035ExchangeAmount)}
    ,ee_modal_premiums_    {round_gross_premium_(yi.Payment)                   }
    ,ee_premium_modes_     {                     yi.PaymentMode                }
    ,er_modal_premiums_    {round_gross_premium_(yi.CorporationPayment)        }
    ,er_premium_modes_     {                     yi.CorporationPaymentMode     }
    ,withdrawals_          {round_withdrawal_   (yi.Withdrawal)                }
    ,new_cash_loans_       {round_loan_         (yi.NewLoan)                   }
{
}

void modal_outlay::set_external_1035_amount(double z)
{
    external_1035_amount_ = round_gross_premium_(z);
}

void modal_outlay::set_internal_1035_amount(double z)
{
    internal_1035_amount_ = round_gross_premium_(z);
}

void modal_outlay::set_ee_modal_premiums(double z, int from_year, int to_year)
{
    z = round_gross_premium_(z);
    std::fill_n(ee_modal_premiums_.begin() + from_year, to_year - from_year, z);
}

void modal_outlay::set_er_modal_premiums(double z, int from_year, int to_year)
{
    z = round_gross_premium_(z);
    std::fill_n(er_modal_premiums_.begin() + from_year, to_year - from_year, z);
}

void modal_outlay::set_er_modal_premiums(std::vector<double> const& z)
{
    LMI_ASSERT(z.size() == er_modal_premiums_.size());
    er_modal_premiums_ = round_gross_premium_(z);
}

void modal_outlay::set_withdrawals(double z, int from_year, int to_year)
{
    z = round_withdrawal_(z);
    std::fill_n(withdrawals_.begin() + from_year, to_year - from_year, z);
}

void modal_outlay::set_new_cash_loans(double z, int from_year, int to_year)
{
    z = round_loan_(z);
    std::fill_n(new_cash_loans_.begin() + from_year, to_year - from_year, z);
}
