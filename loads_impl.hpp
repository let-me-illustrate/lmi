// Loads and expense charges: arcana.
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

// $Id: loads_impl.hpp,v 1.1 2005-10-04 05:36:35 chicares Exp $

#ifndef loads_impl_hpp
#define loads_impl_hpp

#include "config.hpp"

#include "round_to.hpp"

#include <vector>

class BasicValues;
class TDatabase;

/// Declaration of struct load_details.
///
/// Behaviorless aggregate 'load_details' holds data required for
/// initialization of class 'Loads' that are normally obtained from
/// the input and database classes.

struct load_details
{
    load_details
        (int                        length
        ,bool                       AmortizePremLoad
        ,double                     LowestPremiumTaxLoadRate
        ,double                     premium_tax_rate
        ,double                     premium_tax_amortization_rate
        ,double                     premium_tax_amortization_period
        ,double                     asset_charge_type
        ,double                     ledger_type
        ,round_to<double>    const& round_interest_rate
        ,std::vector<double> const& VectorExtraCompLoad
        ,std::vector<double> const& VectorExtraAssetComp
        ,std::vector<double> const& VectorExtraPolFee
        )
        :length_                          (length)
        ,AmortizePremLoad_                (AmortizePremLoad)
        ,LowestPremiumTaxLoadRate_        (LowestPremiumTaxLoadRate)
        ,premium_tax_rate_                (premium_tax_rate)
        ,premium_tax_amortization_rate_   (premium_tax_amortization_rate)
        ,premium_tax_amortization_period_ (premium_tax_amortization_period)
        ,asset_charge_type_               (asset_charge_type)
        ,ledger_type_                     (ledger_type)
        ,round_interest_rate_             (round_interest_rate)
        ,VectorExtraCompLoad_             (VectorExtraCompLoad)
        ,VectorExtraAssetComp_            (VectorExtraAssetComp)
        ,VectorExtraPolFee_               (VectorExtraPolFee)
        {}

    int                        length_;
    bool                       AmortizePremLoad_;
    double                     LowestPremiumTaxLoadRate_;
    double                     premium_tax_rate_; // TODO ?? Unused?
    double                     premium_tax_amortization_rate_;
    double                     premium_tax_amortization_period_;
    double                     asset_charge_type_;
    double                     ledger_type_;
    round_to<double>    const& round_interest_rate_;
    std::vector<double> const& VectorExtraCompLoad_;
    std::vector<double> const& VectorExtraAssetComp_;
    std::vector<double> const& VectorExtraPolFee_;
};

#endif // loads_impl_hpp

