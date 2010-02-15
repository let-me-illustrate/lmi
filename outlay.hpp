// Premiums, loans, and withdrawals.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef outlay_hpp
#define outlay_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <vector>

class yare_input;

class modal_outlay
    :private boost::noncopyable
    ,virtual private obstruct_slicing<modal_outlay>
{
    friend class AccountValue;

  public:
    explicit modal_outlay(yare_input const&);
    ~modal_outlay();

    double                          dumpin               () const;
    double                          external_1035_amount () const;
    double                          internal_1035_amount () const;
    std::vector<double>      const& ee_modal_premiums    () const;
    std::vector<mcenum_mode> const& ee_premium_modes     () const;
    std::vector<double>      const& er_modal_premiums    () const;
    std::vector<mcenum_mode> const& er_premium_modes     () const;
    std::vector<double>      const& new_cash_loans       () const;
    std::vector<double>      const& withdrawals          () const;

  private:
    modal_outlay();

    // Not yet used, but needed for MEC avoidance.
    void set_external_1035_amount(double z);
    void set_internal_1035_amount(double z);

    void set_ee_modal_premiums(double z, int from_year, int to_year);
    void set_er_modal_premiums(double z, int from_year, int to_year);
    void set_er_modal_premiums(std::vector<double> const&);
    void set_new_cash_loans   (double z, int from_year, int to_year);
    void set_withdrawals      (double z, int from_year, int to_year);

    double                   dumpin_;
    double                   external_1035_amount_;
    double                   internal_1035_amount_;
    std::vector<double>      ee_modal_premiums_;
    std::vector<mcenum_mode> ee_premium_modes_;
    std::vector<double>      er_modal_premiums_;
    std::vector<mcenum_mode> er_premium_modes_;
    std::vector<double>      new_cash_loans_;
    std::vector<double>      withdrawals_;
};

inline double modal_outlay::dumpin() const
{
    return dumpin_;
}

inline double modal_outlay::external_1035_amount() const
{
    return external_1035_amount_;
}

inline double modal_outlay::internal_1035_amount() const
{
    return internal_1035_amount_;
}

inline std::vector<double> const& modal_outlay::ee_modal_premiums() const
{
    return ee_modal_premiums_;
}

inline std::vector<mcenum_mode> const& modal_outlay::ee_premium_modes() const
{
    return ee_premium_modes_;
}

inline std::vector<double> const& modal_outlay::er_modal_premiums() const
{
    return er_modal_premiums_;
}

inline std::vector<mcenum_mode> const& modal_outlay::er_premium_modes() const
{
    return er_premium_modes_;
}

inline std::vector<double> const& modal_outlay::new_cash_loans() const
{
    return new_cash_loans_;
}

inline std::vector<double> const& modal_outlay::withdrawals() const
{
    return withdrawals_;
}

inline void modal_outlay::set_external_1035_amount(double z)
{
    external_1035_amount_ = z;
}

inline void modal_outlay::set_internal_1035_amount(double z)
{
    internal_1035_amount_ = z;
}

#endif // outlay_hpp

