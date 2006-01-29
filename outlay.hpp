// Premiums, loans, and withdrawals.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: outlay.hpp,v 1.2 2006-01-29 13:52:00 chicares Exp $

#ifndef outlay_hpp
#define outlay_hpp

#include "config.hpp"

#include "xenumtypes.hpp"

#include <vector>

class BasicValues;

class Outlay
{
    friend class AccountValue;

  public:
    explicit Outlay(BasicValues const&);

    double dumpin() const;
    double external_1035_amount() const;
    double internal_1035_amount() const;

    std::vector<e_mode> const& ee_premium_modes  () const;
    std::vector<e_mode> const& er_premium_modes  () const;
    std::vector<double> const& ee_modal_premiums () const;
    std::vector<double> const& er_modal_premiums () const;
    std::vector<double> const& new_cash_loans    () const;
    std::vector<double> const& withdrawals       () const;

  private:
    Outlay();

    void Init(BasicValues const&);

    double dumpin_;
    double external_1035_amount_;
    double internal_1035_amount_;

    void set_ee_modal_premiums(double z, int from_year, int to_year);
    void set_er_modal_premiums(double z, int from_year, int to_year);
    void set_new_cash_loans   (double z, int from_year, int to_year);
    void set_withdrawals      (double z, int from_year, int to_year);

    // Not yet used, but needed for MEC avoidance.
    void set_external_1035_amount(double z);
    void set_internal_1035_amount(double z);

    std::vector<double> ee_modal_premiums_;
    std::vector<double> er_modal_premiums_;
    std::vector<e_mode> ee_premium_modes_;
    std::vector<e_mode> er_premium_modes_;
    std::vector<double> new_cash_loans_;
    std::vector<double> withdrawals_;
};

inline void Outlay::set_external_1035_amount(double z)
{
    external_1035_amount_ = z;
}

inline void Outlay::set_internal_1035_amount(double z)
{
    internal_1035_amount_ = z;
}

inline double Outlay::dumpin() const
{
    return dumpin_;
}

inline double Outlay::external_1035_amount() const
{
    return external_1035_amount_;
}

inline double Outlay::internal_1035_amount() const
{
    return internal_1035_amount_;
}

inline std::vector<e_mode> const& Outlay::ee_premium_modes() const
{
    return ee_premium_modes_;
}

inline std::vector<e_mode> const& Outlay::er_premium_modes() const
{
    return er_premium_modes_;
}

inline std::vector<double> const& Outlay::ee_modal_premiums() const
{
    return ee_modal_premiums_;
}

inline std::vector<double> const& Outlay::er_modal_premiums() const
{
    return er_modal_premiums_;
}

inline std::vector<double> const& Outlay::new_cash_loans() const
{
    return new_cash_loans_;
}

inline std::vector<double> const& Outlay::withdrawals() const
{
    return withdrawals_;
}

#endif // outlay_hpp

