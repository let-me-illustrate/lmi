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

// $Id: loads.hpp,v 1.7 2005-10-05 17:07:52 chicares Exp $

#ifndef loads_hpp
#define loads_hpp

#include "config.hpp"

#include "xenumtypes.hpp"

#include <vector>

class BasicValues;
class TDatabase;
class load_details;

/// Declaration of class Loads.

class Loads
{
    // TODO ?? Would a friend class be better?
    friend int test_main(int, char*[]);

  public:
    Loads(BasicValues& values);
    Loads(TDatabase const&, bool NeedMidpointRates); // Antediluvian branch.

    // Accessors.

    // Typically, if a portion of the load is refundable on early
    // termination, the refund applies only to the sales load.
    std::vector<double> const& refundable_sales_load_proportion() const;

    std::vector<double> const& monthly_policy_fee    (e_basis const&) const;
    std::vector<double> const& annual_policy_fee     (e_basis const&) const;
    std::vector<double> const& specified_amount_load (e_basis const&) const;
    std::vector<double> const& separate_account_load (e_basis const&) const;
    std::vector<double> const& target_premium_load   (e_basis const&) const;
    std::vector<double> const& excess_premium_load   (e_basis const&) const;
    std::vector<double> const& target_sales_load     (e_basis const&) const;
    std::vector<double> const& excess_sales_load     (e_basis const&) const;
    std::vector<double> const& target_total_load     (e_basis const&) const;
    std::vector<double> const& excess_total_load     (e_basis const&) const;

    std::vector<double> const& premium_tax_load           () const;
    std::vector<double> const& amortized_premium_tax_load () const;
    std::vector<double> const& dac_tax_load               () const;

    std::vector<double> const& target_premium_load_7702_excluding_premium_tax() const;
    std::vector<double> const& excess_premium_load_7702_excluding_premium_tax() const;
    std::vector<double> const& target_premium_load_7702_lowest_premium_tax() const;
    std::vector<double> const& excess_premium_load_7702_lowest_premium_tax() const;

    // Accessors for antediluvian branch.
    std::vector<double> const& monthly_policy_fee    (enum_basis) const;
    std::vector<double> const& specified_amount_load (enum_basis) const;
    std::vector<double> const& target_premium_load   (enum_basis) const;
    std::vector<double> const& excess_premium_load   (enum_basis) const;

  private:
    Loads(); // Ctor for unit testing.

    void Allocate(int length);
    void Initialize(TDatabase const&);
    void Calculate(load_details const&);

    void AmortizePremiumTax(load_details const& details);

    std::vector<double> refundable_sales_load_proportion_;

    std::vector<std::vector<double> > monthly_policy_fee_;
    std::vector<std::vector<double> > annual_policy_fee_;
    std::vector<std::vector<double> > specified_amount_load_;
    std::vector<std::vector<double> > separate_account_load_;
    std::vector<std::vector<double> > target_premium_load_;
    std::vector<std::vector<double> > excess_premium_load_;
    std::vector<std::vector<double> > target_sales_load_;
    std::vector<std::vector<double> > excess_sales_load_;
    std::vector<std::vector<double> > target_total_load_;
    std::vector<std::vector<double> > excess_total_load_;

    std::vector<double> premium_tax_load_;
    std::vector<double> amortized_premium_tax_load_;
    std::vector<double> dac_tax_load_;

    std::vector<double> target_premium_load_7702_excluding_premium_tax_;
    std::vector<double> excess_premium_load_7702_excluding_premium_tax_;
    std::vector<double> target_premium_load_7702_lowest_premium_tax_;
    std::vector<double> excess_premium_load_7702_lowest_premium_tax_;
};

inline std::vector<double> const&
Loads::refundable_sales_load_proportion() const
{
    return refundable_sales_load_proportion_;
}

inline std::vector<double> const&
Loads::monthly_policy_fee(e_basis const& b) const
{
    return monthly_policy_fee_[b.value()];
}

inline std::vector<double> const&
Loads::annual_policy_fee(e_basis const& b) const
{
    return annual_policy_fee_[b.value()];
}

inline std::vector<double> const&
Loads::specified_amount_load(e_basis const& b) const
{
    return specified_amount_load_[b.value()];
}

inline std::vector<double> const&
Loads::separate_account_load(e_basis const& b) const
{
    return separate_account_load_[b.value()];
}

inline std::vector<double> const&
Loads::target_premium_load(e_basis const& b) const
{
    return target_premium_load_[b.value()];
}

inline std::vector<double> const&
Loads::excess_premium_load(e_basis const& b) const
{
    return excess_premium_load_[b.value()];
}

inline std::vector<double> const&
Loads::target_sales_load(e_basis const& b) const
{
    return target_sales_load_[b.value()];
}

inline std::vector<double> const&
Loads::excess_sales_load(e_basis const& b) const
{
    return excess_sales_load_[b.value()];
}

inline std::vector<double> const&
Loads::target_total_load(e_basis const& b) const
{
    return target_total_load_[b.value()];
}

inline std::vector<double> const&
Loads::excess_total_load(e_basis const& b) const
{
    return excess_total_load_[b.value()];
}

inline std::vector<double> const&
Loads::premium_tax_load() const
{
    return premium_tax_load_;
}

inline std::vector<double> const&
Loads::amortized_premium_tax_load() const
{
    return amortized_premium_tax_load_;
}

inline std::vector<double> const&
Loads::dac_tax_load() const
{
    return dac_tax_load_;
}

inline std::vector<double> const&
Loads::target_premium_load_7702_excluding_premium_tax() const
{
    return target_premium_load_7702_excluding_premium_tax_;
}

inline std::vector<double> const&
Loads::excess_premium_load_7702_excluding_premium_tax() const
{
    return excess_premium_load_7702_excluding_premium_tax_;
}

inline std::vector<double> const&
Loads::target_premium_load_7702_lowest_premium_tax() const
{
    return target_premium_load_7702_lowest_premium_tax_;
}

inline std::vector<double> const&
Loads::excess_premium_load_7702_lowest_premium_tax() const
{
    return excess_premium_load_7702_lowest_premium_tax_;
}

// Accessors for antediluvian branch.

inline std::vector<double> const&
Loads::monthly_policy_fee(enum_basis b) const
{
    return monthly_policy_fee_[b];
}

inline std::vector<double> const&
Loads::specified_amount_load(enum_basis b) const
{
    return specified_amount_load_[b];
}

inline std::vector<double> const&
Loads::target_premium_load(enum_basis b) const
{
    return target_premium_load_[b];
}

inline std::vector<double> const&
Loads::excess_premium_load(enum_basis b) const
{
    return excess_premium_load_[b];
}

#endif // loads_hpp

