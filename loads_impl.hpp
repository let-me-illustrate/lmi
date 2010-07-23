// Loads and expense charges: arcana.
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

#ifndef loads_impl_hpp
#define loads_impl_hpp

#include "config.hpp"

#include "round_to.hpp"

#include <boost/utility.hpp>

#include <vector>

class BasicValues;
class product_database;

/// Declaration of struct load_details.
///
/// Behaviorless aggregate 'load_details' holds data required for
/// initialization of class 'Loads' that are normally obtained from
/// the input and database classes.
///
/// Data members.
///
/// length_: Number of elements in any vector of loads.
///
/// premium_tax_load_: Rate of passed-through premium tax, assumed to
/// be a scalar.
///
/// LowestPremiumTaxLoadRate_: The lowest rate of premium tax, which
/// products that pass through premium tax need for 7702 calculations.
/// The actual rate may vary if it is tiered.
///
/// asset_charge_type_: Determines whether input extra asset loads
/// should be treated as a load or as an interest-rate decrement.
///
/// NeedMidpointRates_: Nomen est omen.
///
/// round_interest_rate_: Rounding functor generally used for interest
/// rates, used because the separate-account load is similar to an M&E
/// charge--it is expressed annually, converted to monthly in the
/// implementation, and then must be rounded.
///
/// VectorExtraCompLoad_: Input extra load per dollar of premium.
///
/// VectorExtraAssetComp_: Input extra load per dollar of assets.
///
/// VectorExtraPolFee_: Input extra fee per month.
///
/// TabularGuarSpecAmtLoad_, TabularCurrSpecAmtLoad_: Specified-amount
/// loads read from tables--to be combined with those in the database.
///
/// These presently-unused data members
///   AmortizePremLoad
///   premium_tax_rate_
///   premium_tax_amortization_rate_
///   premium_tax_amortization_period_
/// are kept against the day when premium-tax amortization is
/// implemented.

struct load_details
    :private boost::noncopyable
{
    load_details
        (int                        length
        ,bool                       AmortizePremLoad
        ,double                     premium_tax_load
        ,double                     LowestPremiumTaxLoadRate
        ,double                     premium_tax_rate
        ,double                     premium_tax_amortization_rate
        ,double                     premium_tax_amortization_period
        ,double                     asset_charge_type
        ,bool                       NeedMidpointRates
        ,round_to<double>    const& round_interest_rate
        ,std::vector<double> const& VectorExtraCompLoad
        ,std::vector<double> const& VectorExtraAssetComp
        ,std::vector<double> const& VectorExtraPolFee
        ,std::vector<double> const& TabularGuarSpecAmtLoad
        ,std::vector<double> const& TabularCurrSpecAmtLoad
        )
        :length_                          (length)
        ,AmortizePremLoad_                (AmortizePremLoad)
        ,premium_tax_load_                (premium_tax_load)
        ,LowestPremiumTaxLoadRate_        (LowestPremiumTaxLoadRate)
        ,premium_tax_rate_                (premium_tax_rate)
        ,premium_tax_amortization_rate_   (premium_tax_amortization_rate)
        ,premium_tax_amortization_period_ (premium_tax_amortization_period)
        ,asset_charge_type_               (asset_charge_type)
        ,NeedMidpointRates_               (NeedMidpointRates)
        ,round_interest_rate_             (round_interest_rate)
        ,VectorExtraCompLoad_             (VectorExtraCompLoad)
        ,VectorExtraAssetComp_            (VectorExtraAssetComp)
        ,VectorExtraPolFee_               (VectorExtraPolFee)
        ,TabularGuarSpecAmtLoad_          (TabularGuarSpecAmtLoad)
        ,TabularCurrSpecAmtLoad_          (TabularCurrSpecAmtLoad)
        {}

    int                        length_;
    bool                       AmortizePremLoad_;
    double                     premium_tax_load_;
    double                     LowestPremiumTaxLoadRate_;
    double                     premium_tax_rate_;
    double                     premium_tax_amortization_rate_;
    double                     premium_tax_amortization_period_;
    double                     asset_charge_type_;
    bool                       NeedMidpointRates_;
    round_to<double>    const& round_interest_rate_;
    std::vector<double> const& VectorExtraCompLoad_;
    std::vector<double> const& VectorExtraAssetComp_;
    std::vector<double> const& VectorExtraPolFee_;
    std::vector<double> const  TabularGuarSpecAmtLoad_;
    std::vector<double> const  TabularCurrSpecAmtLoad_;
};

#endif // loads_impl_hpp

