// Loads and expense charges--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: loads_test.cpp,v 1.3 2005-10-05 17:07:52 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "loads.hpp"
#include "loads_impl.hpp"

#include "alert.hpp"
#include "round_to.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/bind.hpp>

// TODO ?? Find a different way of dealing with these dependencies.
// Either refactor 'loads.cpp' to remove them, or provide stubs for
// everything that might get linked.

#include "database.hpp"
void TDatabase::Query(std::vector<double, std::allocator<double> >&, int) const {}
double TDatabase::Query(int) const {return 0.0;}

int test_main(int, char*[])
{
    int length = 50;
    round_to<double> round_interest_rate(0, r_not_at_all);
    std::vector<double> extra_comp_load (length);
    std::vector<double> extra_asset_comp(length);
    std::vector<double> extra_policy_fee(length);

    load_details details
        (length                // length_
        ,false                 // AmortizePremLoad_
        ,0.02                  // LowestPremiumTaxLoadRate_
        ,0.02                  // premium_tax_rate_
        ,0                     // premium_tax_amortization_rate_
        ,0                     // premium_tax_amortization_period_
        ,e_asset_charge_spread // asset_charge_type_
        ,false                 // NeedMidpointRates_
        ,round_interest_rate   // round_interest_rate_
        ,extra_comp_load       // VectorExtraCompLoad_
        ,extra_asset_comp      // VectorExtraAssetComp_
        ,extra_policy_fee      // VectorExtraPolFee_
        );

    Loads loads;
    loads.Allocate(length);
    loads.Calculate(details);

    std::cout
        << "  "
        << aliquot_timer(boost::bind(&Loads::Allocate, &loads, length))
        << '\n'
        ;

    std::cout
        << "  "
        << aliquot_timer(boost::bind(&Loads::Calculate, &loads, details))
        << '\n'
        ;

    BOOST_TEST_EQUAL(0.00, loads.premium_tax_load()[0]);

    return 0;
}

