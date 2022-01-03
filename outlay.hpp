// Premiums, loans, and withdrawals.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef outlay_hpp
#define outlay_hpp

#include "config.hpp"

#include "currency.hpp"
#include "mc_enum_type_enums.hpp"
#include "round_to.hpp"

#include <vector>

class yare_input;

class modal_outlay final
{
    friend class AccountValue;

  public:
    explicit modal_outlay
        (yare_input       const&
        ,round_to<double> const& round_gross_premium
        ,round_to<double> const& round_withdrawal
        ,round_to<double> const& round_loan
        );
    ~modal_outlay() = default;

    currency                        dumpin               () const;
    currency                        external_1035_amount () const;
    currency                        internal_1035_amount () const;
    std::vector<currency>    const& ee_modal_premiums    () const;
    std::vector<mcenum_mode> const& ee_premium_modes     () const;
    std::vector<currency>    const& er_modal_premiums    () const;
    std::vector<mcenum_mode> const& er_premium_modes     () const;
    std::vector<currency>    const& withdrawals          () const;
    std::vector<currency>    const& new_cash_loans       () const;

  private:
    modal_outlay(modal_outlay const&) = delete;
    modal_outlay& operator=(modal_outlay const&) = delete;

    void block_dumpin              ();
    void block_external_1035_amount();
    void block_internal_1035_amount();

    void set_ee_modal_premiums(currency z, int from_year, int to_year);
    void set_er_modal_premiums(currency z, int from_year, int to_year);
    void set_withdrawals      (currency z, int from_year, int to_year);
    void set_new_cash_loans   (currency z, int from_year, int to_year);

    round_to<double> round_gross_premium_;
    round_to<double> round_withdrawal_;
    round_to<double> round_loan_;

    currency                 dumpin_;
    currency                 external_1035_amount_;
    currency                 internal_1035_amount_;
    std::vector<currency>    ee_modal_premiums_;
    std::vector<mcenum_mode> ee_premium_modes_;
    std::vector<currency>    er_modal_premiums_;
    std::vector<mcenum_mode> er_premium_modes_;
    std::vector<currency>    withdrawals_;
    std::vector<currency>    new_cash_loans_;
};

inline currency modal_outlay::dumpin() const
{
    return dumpin_;
}

inline currency modal_outlay::external_1035_amount() const
{
    return external_1035_amount_;
}

inline currency modal_outlay::internal_1035_amount() const
{
    return internal_1035_amount_;
}

inline std::vector<currency> const& modal_outlay::ee_modal_premiums() const
{
    return ee_modal_premiums_;
}

inline std::vector<mcenum_mode> const& modal_outlay::ee_premium_modes() const
{
    return ee_premium_modes_;
}

inline std::vector<currency> const& modal_outlay::er_modal_premiums() const
{
    return er_modal_premiums_;
}

inline std::vector<mcenum_mode> const& modal_outlay::er_premium_modes() const
{
    return er_premium_modes_;
}

inline std::vector<currency> const& modal_outlay::withdrawals() const
{
    return withdrawals_;
}

inline std::vector<currency> const& modal_outlay::new_cash_loans() const
{
    return new_cash_loans_;
}

#endif // outlay_hpp
