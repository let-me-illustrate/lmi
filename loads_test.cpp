// Loads and expense charges--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "loads.hpp"
#include "loads_impl.hpp"

#include "materially_equal.hpp"
#include "oecumenic_enumerations.hpp"
#include "round_to.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <functional>                   // bind()

namespace
{
int const length = 50;
std::vector<double> const dummy_vector(length);
} // Unnamed namespace.

// TODO ?? Consider factoring out these stubs...or abandoning the
// present experiment and finding a better way of making classes such
// as this one independent of class BasicValues.

#include "basic_values.hpp"
std::vector<double> BasicValues::GetCurrSpecAmtLoadTable() const {return dummy_vector;}
std::vector<double> BasicValues::GetGuarSpecAmtLoadTable() const {return dummy_vector;}

#include "database.hpp"
int product_database::length() const {return length_;}
void product_database::query_into(e_database_key, std::vector<double>& v) const {v.resize(length_);}
void product_database::query_into(e_database_key, std::vector<currency>& v) const {v.resize(length_);}
double product_database::query(e_database_key, database_index const&) const {return 0.0;}

#include "premium_tax.hpp"
double premium_tax::levy_rate        () const {return 0.0;}
double premium_tax::load_rate        () const {return 0.0;}
double premium_tax::maximum_load_rate() const {return 0.0;}
double premium_tax::minimum_load_rate() const {return 0.0;}

struct LoadsTest
{
    LoadsTest(load_details const& details)
        :details_  {details}
        ,database_ (details.length_)
        ,loads_    {}
        {}

    void Allocate  () {loads_.Allocate(details_.length_);}
    void Initialize() {loads_.Initialize(database_, details_);}
    void Calculate () {loads_.Calculate(details_);}

    void Reinitialize();
    void Recalculate () {Reinitialize(); Calculate();}

    void TestVectorLengths(char const* file, int line);
    void TestCalculations (char const* file, int line);

    load_details const& details_;
    product_database database_;
    Loads loads_;
};

void LoadsTest::Reinitialize()
{
    loads_.refundable_sales_load_proportion_    = std::vector<double>  (length, 0.50000);
    loads_.dac_tax_load_                        = std::vector<double>  (length, 0.00500);

    loads_.monthly_policy_fee_   [mce_gen_guar] = std::vector<currency>(length, from_cents(800));
    loads_.annual_policy_fee_    [mce_gen_guar] = std::vector<currency>(length, from_cents(200));
    loads_.specified_amount_load_[mce_gen_guar] = std::vector<double>  (length, 0.00003);
    loads_.separate_account_load_[mce_gen_guar] = std::vector<double>  (length, 0.00130);
    loads_.target_premium_load_  [mce_gen_guar] = std::vector<double>  (length, 0.04000);
    loads_.excess_premium_load_  [mce_gen_guar] = std::vector<double>  (length, 0.03000);
    loads_.target_sales_load_    [mce_gen_guar] = std::vector<double>  (length, 0.30000);
    loads_.excess_sales_load_    [mce_gen_guar] = std::vector<double>  (length, 0.15000);

    loads_.monthly_policy_fee_   [mce_gen_curr] = std::vector<currency>(length, from_cents(525));
    loads_.annual_policy_fee_    [mce_gen_curr] = std::vector<currency>(length, from_cents(100));
    loads_.specified_amount_load_[mce_gen_curr] = std::vector<double>  (length, 0.00002);
    loads_.separate_account_load_[mce_gen_curr] = std::vector<double>  (length, 0.00110);
    loads_.target_premium_load_  [mce_gen_curr] = std::vector<double>  (length, 0.02000);
    loads_.excess_premium_load_  [mce_gen_curr] = std::vector<double>  (length, 0.01000);
    loads_.target_sales_load_    [mce_gen_curr] = std::vector<double>  (length, 0.10000);
    loads_.excess_sales_load_    [mce_gen_curr] = std::vector<double>  (length, 0.05000);
}

void LoadsTest::TestVectorLengths(char const* file, int line)
{
    int const z = details_.length_;

    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.refundable_sales_load_proportion         ()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.premium_tax_load                         ()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.amortized_premium_tax_load               ()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.dac_tax_load                             ()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_premium_load_excluding_premium_tax()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_premium_load_excluding_premium_tax()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_premium_load_maximum_premium_tax  ()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_premium_load_maximum_premium_tax  ()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_premium_load_minimum_premium_tax  ()), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_premium_load_minimum_premium_tax  ()), file, line);

    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.monthly_policy_fee    (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.annual_policy_fee     (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.specified_amount_load (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.separate_account_load (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_premium_load   (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_premium_load   (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_sales_load     (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_sales_load     (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_total_load     (mce_gen_curr)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_total_load     (mce_gen_curr)), file, line);

    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.monthly_policy_fee    (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.annual_policy_fee     (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.specified_amount_load (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.separate_account_load (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_premium_load   (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_premium_load   (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_sales_load     (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_sales_load     (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_total_load     (mce_gen_guar)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_total_load     (mce_gen_guar)), file, line);

    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.monthly_policy_fee    (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.annual_policy_fee     (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.specified_amount_load (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.separate_account_load (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_premium_load   (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_premium_load   (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_sales_load     (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_sales_load     (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.target_total_load     (mce_gen_mdpt)), file, line);
    INVOKE_BOOST_TEST_EQUAL(z, lmi::ssize(loads_.excess_total_load     (mce_gen_mdpt)), file, line);
}

void LoadsTest::TestCalculations(char const* file, int line)
{
    INVOKE_BOOST_TEST(materially_equal(0.500000, loads_.refundable_sales_load_proportion()[0]), file, line);

    // (8.00 + 5.25 + 0.50) / 2 = 13.75 / 2 = 6.875, rounded to cents
#   if defined CURRENCY_UNIT_IS_CENTS
    INVOKE_BOOST_TEST(from_cents(688) == loads_.monthly_policy_fee (mce_gen_mdpt)[0] , file, line);
#   else  // !defined CURRENCY_UNIT_IS_CENTS
    INVOKE_BOOST_TEST(materially_equal(6.88, dblize(loads_.monthly_policy_fee (mce_gen_mdpt)[0])), file, line);
#   endif // !defined CURRENCY_UNIT_IS_CENTS
    INVOKE_BOOST_TEST(from_cents(150) == loads_.annual_policy_fee  (mce_gen_mdpt)[0] , file, line);
    INVOKE_BOOST_TEST(materially_equal(0.000625, loads_.specified_amount_load (mce_gen_mdpt)[0]), file, line);
    // 12 bp and 19 bp, both converted to monthly, then added together.
    INVOKE_BOOST_TEST(materially_equal(0.0002581402795930, loads_.separate_account_load (mce_gen_mdpt)[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.030000, loads_.target_premium_load   (mce_gen_mdpt)[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.020000, loads_.excess_premium_load   (mce_gen_mdpt)[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.217000, loads_.target_sales_load     (mce_gen_mdpt)[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.117000, loads_.excess_sales_load     (mce_gen_mdpt)[0]), file, line);

    INVOKE_BOOST_TEST(materially_equal(0.383000, loads_.target_total_load     (mce_gen_guar)[0]), file, line); // 0.30 + 0.04 + 0.021 + 0.005 + 0.017
    INVOKE_BOOST_TEST(materially_equal(0.223000, loads_.excess_total_load     (mce_gen_guar)[0]), file, line); // 0.15 + 0.03 + 0.021 + 0.005 + 0.017
    INVOKE_BOOST_TEST(materially_equal(0.163000, loads_.target_total_load     (mce_gen_curr)[0]), file, line); // 0.10 + 0.02 + 0.021 + 0.005 + 0.017
    INVOKE_BOOST_TEST(materially_equal(0.103000, loads_.excess_total_load     (mce_gen_curr)[0]), file, line); // 0.05 + 0.01 + 0.021 + 0.005 + 0.017
    INVOKE_BOOST_TEST(materially_equal(0.273000, loads_.target_total_load     (mce_gen_mdpt)[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.163000, loads_.excess_total_load     (mce_gen_mdpt)[0]), file, line);

    INVOKE_BOOST_TEST(materially_equal(0.021000, loads_.premium_tax_load           ()[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.000000, loads_.amortized_premium_tax_load ()[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.005000, loads_.dac_tax_load               ()[0]), file, line);

    INVOKE_BOOST_TEST(materially_equal(0.163000, loads_.target_total_load     (mce_gen_curr)[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.103000, loads_.excess_total_load     (mce_gen_curr)[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.142000, loads_.target_premium_load_excluding_premium_tax()[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.082000, loads_.excess_premium_load_excluding_premium_tax()[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.164000, loads_.target_premium_load_maximum_premium_tax  ()[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.104000, loads_.excess_premium_load_maximum_premium_tax  ()[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.162000, loads_.target_premium_load_minimum_premium_tax  ()[0]), file, line);
    INVOKE_BOOST_TEST(materially_equal(0.102000, loads_.excess_premium_load_minimum_premium_tax  ()[0]), file, line);
}

int test_main(int, char*[])
{
    round_to<double> round_interest_rate(0, r_not_at_all);
    round_to<double> round_minutiae     (2, r_to_nearest);
    std::vector<double>   extra_comp_load  (length, 0.0170);
    std::vector<double>   extra_asset_comp (length, 0.0019);
    std::vector<currency> extra_policy_fee (length, from_cents(50));
    std::vector<double>   guar_specamt_load(length, 0.0007);
    std::vector<double>   curr_specamt_load(length, 0.0005);

    load_details details
        (length                 // length_
        ,false                  // AmortizePremLoad_
        ,0.021                  // premium_tax_load_
        ,0.022                  // maximum_premium_tax_load_rate_
        ,0.02                   // minimum_premium_tax_load_rate_
        ,999.999                // premium_tax_rate_                [unused]
        ,999.999                // premium_tax_amortization_rate_   [unused]
        ,999                    // premium_tax_amortization_period_ [unused]
        ,oe_asset_charge_load   // asset_charge_type_
        ,true                   // NeedMidpointRates_
        ,round_interest_rate    // round_interest_rate_
        ,round_minutiae         // round_minutiae_
        ,extra_comp_load        // VectorExtraCompLoad_
        ,extra_asset_comp       // VectorExtraAssetComp_
        ,extra_policy_fee       // VectorExtraPolFee_
        ,guar_specamt_load      // TabularGuarSpecAmtLoad_
        ,curr_specamt_load      // TabularCurrSpecAmtLoad_
        );

    LoadsTest t(details);
    t.Allocate();
    t.Initialize();
    t.Reinitialize();
    t.TestVectorLengths(__FILE__, __LINE__);
    t.Calculate();
    t.TestCalculations (__FILE__, __LINE__);

    std::cout
        << "  Allocate:     "
        << TimeAnAliquot(std::bind(&LoadsTest::Allocate , &t))
        << '\n'
        ;

    std::cout
        << "  Initialize:   "
        << TimeAnAliquot(std::bind(&LoadsTest::Initialize , &t))
        << '\n'
        ;

    std::cout
        << "  Reinitialize: "
        << TimeAnAliquot(std::bind(&LoadsTest::Reinitialize , &t))
        << '\n'
        ;

    std::cout
        << "  Recalculate:  "
        << TimeAnAliquot(std::bind(&LoadsTest::Recalculate, &t))
        << '\n'
        ;

    return 0;
}
