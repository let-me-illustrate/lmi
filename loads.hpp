// Loads and expense charges.
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

#ifndef loads_hpp
#define loads_hpp

#include "config.hpp"

#include "currency.hpp"
#include "mc_enum_type_enums.hpp"

#include <vector>

class BasicValues;
class product_database;

struct load_details;

/// Declaration of class Loads.

class Loads
{
    friend class LoadsTest;

  public:
    Loads(BasicValues& values);
    Loads(product_database const&, bool NeedMidpointRates); // Antediluvian branch.

    // Accessors.

    // Typically, if a portion of the load is refundable on early
    // termination, the refund applies only to the sales load.
    std::vector<double> const& refundable_sales_load_proportion() const;

    std::vector<currency> const& monthly_policy_fee    (mcenum_gen_basis) const;
    std::vector<currency> const& annual_policy_fee     (mcenum_gen_basis) const;
    std::vector<double>   const& specified_amount_load (mcenum_gen_basis) const;
    std::vector<double>   const& separate_account_load (mcenum_gen_basis) const;
    std::vector<double>   const& target_premium_load   (mcenum_gen_basis) const;
    std::vector<double>   const& excess_premium_load   (mcenum_gen_basis) const;
    std::vector<double>   const& target_sales_load     (mcenum_gen_basis) const;
    std::vector<double>   const& excess_sales_load     (mcenum_gen_basis) const;
    std::vector<double>   const& target_total_load     (mcenum_gen_basis) const;
    std::vector<double>   const& excess_total_load     (mcenum_gen_basis) const;

    std::vector<double> const& premium_tax_load           () const;
    std::vector<double> const& amortized_premium_tax_load () const;
    std::vector<double> const& dac_tax_load               () const;

    std::vector<double> const& target_premium_load_excluding_premium_tax() const;
    std::vector<double> const& excess_premium_load_excluding_premium_tax() const;
    std::vector<double> const& target_premium_load_maximum_premium_tax() const;
    std::vector<double> const& excess_premium_load_maximum_premium_tax() const;
    std::vector<double> const& target_premium_load_minimum_premium_tax() const;
    std::vector<double> const& excess_premium_load_minimum_premium_tax() const;

  private:
    Loads() = default; // Ctor for unit testing.

    void Allocate(int length);
    void Initialize(product_database const&, load_details const&);
    void Calculate(load_details const&);

    void AmortizePremiumTax(load_details const&);

    std::vector<double> refundable_sales_load_proportion_;

    std::vector<std::vector<currency>> monthly_policy_fee_;
    std::vector<std::vector<currency>> annual_policy_fee_;
    std::vector<std::vector<double>>   specified_amount_load_;
    std::vector<std::vector<double>>   separate_account_load_;
    std::vector<std::vector<double>>   target_premium_load_;
    std::vector<std::vector<double>>   excess_premium_load_;
    std::vector<std::vector<double>>   target_sales_load_;
    std::vector<std::vector<double>>   excess_sales_load_;
    std::vector<std::vector<double>>   target_total_load_;
    std::vector<std::vector<double>>   excess_total_load_;

    std::vector<double> premium_tax_load_;
    std::vector<double> amortized_premium_tax_load_;
    std::vector<double> dac_tax_load_;

    std::vector<double> target_premium_load_excluding_premium_tax_;
    std::vector<double> excess_premium_load_excluding_premium_tax_;
    std::vector<double> target_premium_load_maximum_premium_tax_;
    std::vector<double> excess_premium_load_maximum_premium_tax_;
    std::vector<double> target_premium_load_minimum_premium_tax_;
    std::vector<double> excess_premium_load_minimum_premium_tax_;
};

inline std::vector<double> const&
Loads::refundable_sales_load_proportion() const
{
    return refundable_sales_load_proportion_;
}

inline std::vector<currency> const&
Loads::monthly_policy_fee(mcenum_gen_basis b) const
{
    return monthly_policy_fee_[b];
}

inline std::vector<currency> const&
Loads::annual_policy_fee(mcenum_gen_basis b) const
{
    return annual_policy_fee_[b];
}

inline std::vector<double> const&
Loads::specified_amount_load(mcenum_gen_basis b) const
{
    return specified_amount_load_[b];
}

inline std::vector<double> const&
Loads::separate_account_load(mcenum_gen_basis b) const
{
    return separate_account_load_[b];
}

inline std::vector<double> const&
Loads::target_premium_load(mcenum_gen_basis b) const
{
    return target_premium_load_[b];
}

inline std::vector<double> const&
Loads::excess_premium_load(mcenum_gen_basis b) const
{
    return excess_premium_load_[b];
}

inline std::vector<double> const&
Loads::target_sales_load(mcenum_gen_basis b) const
{
    return target_sales_load_[b];
}

inline std::vector<double> const&
Loads::excess_sales_load(mcenum_gen_basis b) const
{
    return excess_sales_load_[b];
}

inline std::vector<double> const&
Loads::target_total_load(mcenum_gen_basis b) const
{
    return target_total_load_[b];
}

inline std::vector<double> const&
Loads::excess_total_load(mcenum_gen_basis b) const
{
    return excess_total_load_[b];
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
Loads::target_premium_load_excluding_premium_tax() const
{
    return target_premium_load_excluding_premium_tax_;
}

inline std::vector<double> const&
Loads::excess_premium_load_excluding_premium_tax() const
{
    return excess_premium_load_excluding_premium_tax_;
}

inline std::vector<double> const&
Loads::target_premium_load_maximum_premium_tax() const
{
    return target_premium_load_maximum_premium_tax_;
}

inline std::vector<double> const&
Loads::excess_premium_load_maximum_premium_tax() const
{
    return excess_premium_load_maximum_premium_tax_;
}

inline std::vector<double> const&
Loads::target_premium_load_minimum_premium_tax() const
{
    return target_premium_load_minimum_premium_tax_;
}

inline std::vector<double> const&
Loads::excess_premium_load_minimum_premium_tax() const
{
    return excess_premium_load_minimum_premium_tax_;
}

#endif // loads_hpp
