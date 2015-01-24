// Premium tax--unit test.
//
// Copyright (C) 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "premium_tax.hpp"

#include "database.hpp"
#include "dbdict.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "product_data.hpp"
#include "stratified_charges.hpp"
#include "test_tools.hpp"

class premium_tax_test
{
  public:
    static void test()
        {
        write_prerequisite_files();
        test_rates();
        }

  private:
    static void write_prerequisite_files();
    static void test_rates();
};

void premium_tax_test::write_prerequisite_files()
{
    DBDictionary::instance() .WriteSampleDBFile      ();
    // product_database::initialize() requires a real '.product' file,
    // even though it's not otherwise used in this TU.
    product_data            ::WritePolFiles          ();
    stratified_charges      ::write_stratified_files ();
}

/// Test premium-tax rates.

void premium_tax_test::test_rates()
{
    product_database db
        ("sample"
        ,mce_female
        ,mce_standard
        ,mce_nonsmoker
        ,45
        ,mce_nonmedical
        ,mce_s_CT
        );
    stratified_charges strata("sample.strata");

    // Tax state = domicile; not tiered.
    {
    // arguments: tax_state, domicile, amortize_premium_load, db, strata
    premium_tax z(mce_s_CT, mce_s_CT, false, db, strata);
    BOOST_TEST_EQUAL(z.levy_rate                (), 0.0175);
    BOOST_TEST_EQUAL(z.load_rate                (), 0.0175);
    BOOST_TEST_EQUAL(z.maximum_load_rate        (), 0.0175);
    BOOST_TEST_EQUAL(z.minimum_load_rate        (), 0.0175);
    BOOST_TEST_EQUAL(z.is_tiered                (), false );
    BOOST_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0175);
    }

    // Retaliation.
    {
    premium_tax z(mce_s_CT, mce_s_MA, false, db, strata);
    BOOST_TEST_EQUAL(z.levy_rate                (), 0.0200);
    BOOST_TEST_EQUAL(z.load_rate                (), 0.0200);
    BOOST_TEST_EQUAL(z.maximum_load_rate        (), 0.0200);
    BOOST_TEST_EQUAL(z.minimum_load_rate        (), 0.0200);
    BOOST_TEST_EQUAL(z.is_tiered                (), false );
    BOOST_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0200);
    }

    // Tiered.
    {
    premium_tax z(mce_s_AK, mce_s_CT, false, db, strata);
    BOOST_TEST_EQUAL(z.levy_rate                (), 0.0000);
    BOOST_TEST_EQUAL(z.load_rate                (), 0.0000);
    BOOST_TEST_EQUAL(z.maximum_load_rate        (), 0.0270);
    BOOST_TEST_EQUAL(z.minimum_load_rate        (), 0.0010);
    BOOST_TEST_EQUAL(z.is_tiered                (), true  );
    BOOST_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0270);
    }

    // Tiered in premium-tax state, but load uniformly zero.
    // A uniform but nonzero load would elicit a runtime error,
    // because the tiered load is not zero.
    {
    database_entity const original = DBDictionary::instance().datum("PremTaxLoad");
    database_entity const scalar(DB_PremTaxLoad, 0.0000);

    DBDictionary::instance().datum("PremTaxLoad") = scalar;

    premium_tax z(mce_s_AK, mce_s_CT, false, db, strata);
    BOOST_TEST_EQUAL(z.levy_rate                (), 0.0000);
    BOOST_TEST_EQUAL(z.load_rate                (), 0.0000);
    BOOST_TEST_EQUAL(z.maximum_load_rate        (), 0.0000);
    BOOST_TEST_EQUAL(z.minimum_load_rate        (), 0.0000);
    BOOST_TEST_EQUAL(z.is_tiered                (), true  );
    BOOST_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0000);

    DBDictionary::instance().datum("PremTaxLoad") = original;
    }

    // Amortized.
    {
    premium_tax z(mce_s_CT, mce_s_MA, true , db, strata);
    BOOST_TEST_EQUAL(z.levy_rate                (), 0.0000);
    BOOST_TEST_EQUAL(z.load_rate                (), 0.0000);
    BOOST_TEST_EQUAL(z.maximum_load_rate        (), 0.0000);
    BOOST_TEST_EQUAL(z.minimum_load_rate        (), 0.0000);
    BOOST_TEST_EQUAL(z.is_tiered                (), false );
    BOOST_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0000);
    }
}

int test_main(int, char*[])
{
    // Absolute paths require "native" name-checking policy for msw.
    initialize_filesystem();

    premium_tax_test::test();

    return EXIT_SUCCESS;
}

