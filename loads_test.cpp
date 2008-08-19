// Loads and expense charges--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: loads_test.cpp,v 1.14 2008-08-19 17:03:32 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "loads.hpp"
#include "loads_impl.hpp"

#include "oecumenic_enumerations.hpp"
#include "round_to.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/bind.hpp>

#include <cstddef> // std::size_t

namespace
{
int const length = 50;
std::vector<double> const dummy_vector(50);
} // Unnamed namespace.

// TODO ?? Consider factoring out these stubs...or abandoning the
// present experiment and finding a better way of making classes such
// as this one independent of class BasicValues.

#include "basic_values.hpp"
std::vector<double> BasicValues::GetCurrSpecAmtLoadTable() const {return dummy_vector;}
std::vector<double> BasicValues::GetGuarSpecAmtLoadTable() const {return dummy_vector;}

#include "database.hpp"
TDatabase::TDatabase(int length) :length_(length) {}
TDatabase::~TDatabase() {}
int TDatabase::length() const {return length_;}
void TDatabase::Query(std::vector<double>& v, int) const {v.resize(length_);}
double TDatabase::Query(int) const {return 0.0;}

struct LoadsTest
{
    LoadsTest(load_details const& details)
        :details_ (details)
        ,database_(details.length_)
        ,loads_   ()
        {}

    void Allocate  () {loads_.Allocate(details_.length_);}
    void Initialize() {loads_.Initialize(database_);}
    void Calculate () {loads_.Calculate(details_);}

    void TestVectorLengths(char const* file, int line);
    void TestCalculations (char const* file, int line);

    load_details details_;
    TDatabase database_;
    Loads loads_;
};

void LoadsTest::TestVectorLengths(char const* file, int line)
{
    size_t const z = details_.length_;

    INVOKE_BOOST_TEST_EQUAL(z, loads_.refundable_sales_load_proportion              ().size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.premium_tax_load                              ().size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.amortized_premium_tax_load                    ().size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.dac_tax_load                                  ().size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_premium_load_7702_excluding_premium_tax().size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_premium_load_7702_excluding_premium_tax().size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_premium_load_7702_lowest_premium_tax   ().size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_premium_load_7702_lowest_premium_tax   ().size(), file, line);

    INVOKE_BOOST_TEST_EQUAL(z, loads_.monthly_policy_fee    (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.annual_policy_fee     (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.specified_amount_load (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.separate_account_load (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_premium_load   (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_premium_load   (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_sales_load     (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_sales_load     (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_total_load     (mce_gen_curr).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_total_load     (mce_gen_curr).size(), file, line);

    INVOKE_BOOST_TEST_EQUAL(z, loads_.monthly_policy_fee    (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.annual_policy_fee     (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.specified_amount_load (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.separate_account_load (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_premium_load   (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_premium_load   (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_sales_load     (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_sales_load     (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_total_load     (mce_gen_guar).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_total_load     (mce_gen_guar).size(), file, line);

    INVOKE_BOOST_TEST_EQUAL(z, loads_.monthly_policy_fee    (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.annual_policy_fee     (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.specified_amount_load (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.separate_account_load (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_premium_load   (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_premium_load   (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_sales_load     (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_sales_load     (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.target_total_load     (mce_gen_mdpt).size(), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, loads_.excess_total_load     (mce_gen_mdpt).size(), file, line);
}

void LoadsTest::TestCalculations(char const* file, int line)
{
// TODO ?? Really do something here.
    INVOKE_BOOST_TEST_EQUAL(0.00, loads_.premium_tax_load()[0], file, line);
}

int test_main(int, char*[])
{
    round_to<double> round_interest_rate(0, r_not_at_all);
    std::vector<double> extra_comp_load  (length);
    std::vector<double> extra_asset_comp (length);
    std::vector<double> extra_policy_fee (length);
    std::vector<double> guar_specamt_load(length);
    std::vector<double> curr_specamt_load(length);

    load_details details
        (length                 // length_
        ,false                  // AmortizePremLoad_
        ,0.02                   // LowestPremiumTaxLoadRate_
        ,0.02                   // premium_tax_rate_
        ,0                      // premium_tax_amortization_rate_
        ,0                      // premium_tax_amortization_period_
        ,oe_asset_charge_spread // asset_charge_type_
        ,false                  // NeedMidpointRates_
        ,round_interest_rate    // round_interest_rate_
        ,extra_comp_load        // VectorExtraCompLoad_
        ,extra_asset_comp       // VectorExtraAssetComp_
        ,extra_policy_fee       // VectorExtraPolFee_
        ,guar_specamt_load      // TabularGuarSpecAmtLoad_
        ,curr_specamt_load      // TabularCurrSpecAmtLoad_
        );

    LoadsTest t(details);
    t.Allocate();
    t.Initialize();
    t.TestVectorLengths(__FILE__, __LINE__);
    t.Calculate();
    t.TestCalculations (__FILE__, __LINE__);

    std::cout
        << "  Allocate:  "
        << TimeAnAliquot(boost::bind(&LoadsTest::Allocate , &t))
        << '\n'
        ;

    std::cout
        << "  Calculate: "
        << TimeAnAliquot(boost::bind(&LoadsTest::Calculate, &t))
        << '\n'
        ;

    return 0;
}

