// Premium tax--unit test.
//
// Copyright (C) 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "premium_tax.hpp"

#include "data_directory.hpp"           // AddDataDir()
#include "database.hpp"
#include "dbdict.hpp"
#include "global_settings.hpp"
#include "product_data.hpp"
#include "stratified_charges.hpp"
#include "test_tools.hpp"

class premium_tax_test
{
  public:
    static void test()
        {
        // Location of product files.
        global_settings::instance().set_data_directory("/opt/lmi/data");
        test_rates();
        }

  private:
    static void test_rates();
};

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
    stratified_charges strata(AddDataDir("sample.strata"));

    // Tax state = domicile; not tiered.
    {
    // arguments: tax_state, domicile, amortize_premium_load, db, strata
    premium_tax z(mce_s_CT, mce_s_CT, false, db, strata);
    LMI_TEST_EQUAL(z.levy_rate                (), 0.0175);
    LMI_TEST_EQUAL(z.load_rate                (), 0.0175);
    LMI_TEST_EQUAL(z.maximum_load_rate        (), 0.0175);
    LMI_TEST_EQUAL(z.minimum_load_rate        (), 0.0175);
    LMI_TEST_EQUAL(z.is_tiered                (), false );
    LMI_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0175);
    }

    // Retaliation.
    {
    premium_tax z(mce_s_CT, mce_s_MA, false, db, strata);
    LMI_TEST_EQUAL(z.levy_rate                (), 0.0200);
    LMI_TEST_EQUAL(z.load_rate                (), 0.0200);
    LMI_TEST_EQUAL(z.maximum_load_rate        (), 0.0200);
    LMI_TEST_EQUAL(z.minimum_load_rate        (), 0.0200);
    LMI_TEST_EQUAL(z.is_tiered                (), false );
    LMI_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0200);
    }

    // Tiered.
    {
    premium_tax z(mce_s_AK, mce_s_CT, false, db, strata);
    LMI_TEST_EQUAL(z.levy_rate                (), 0.0000);
    LMI_TEST_EQUAL(z.load_rate                (), 0.0000);
    LMI_TEST_EQUAL(z.maximum_load_rate        (), 0.0270);
    LMI_TEST_EQUAL(z.minimum_load_rate        (), 0.0008);
    LMI_TEST_EQUAL(z.is_tiered                (), true  );
    LMI_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0270);
    }

    // Tiered in premium-tax state, but load uniformly zero.
    // A uniform but nonzero load would elicit a runtime error,
    // because the tiered load is not zero.
    {
    auto x = std::make_shared<DBDictionary>();
    db.db_ = x;
    DBDictionary& dictionary = *x;

    database_entity const original = dictionary.datum("PremTaxLoad");
    database_entity const scalar(DB_PremTaxLoad, 0.0000);

    dictionary.datum("PremTaxLoad") = scalar;

    premium_tax z(mce_s_AK, mce_s_CT, false, db, strata);
    LMI_TEST_EQUAL(z.levy_rate                (), 0.0000);
    LMI_TEST_EQUAL(z.load_rate                (), 0.0000);
    LMI_TEST_EQUAL(z.maximum_load_rate        (), 0.0000);
    LMI_TEST_EQUAL(z.minimum_load_rate        (), 0.0000);
    LMI_TEST_EQUAL(z.is_tiered                (), true  );
    LMI_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0000);

    dictionary.datum("PremTaxLoad") = original;
    }

    // Amortized.
    {
    premium_tax z(mce_s_CT, mce_s_MA, true , db, strata);
    LMI_TEST_EQUAL(z.levy_rate                (), 0.0000);
    LMI_TEST_EQUAL(z.load_rate                (), 0.0000);
    LMI_TEST_EQUAL(z.maximum_load_rate        (), 0.0000);
    LMI_TEST_EQUAL(z.minimum_load_rate        (), 0.0000);
    LMI_TEST_EQUAL(z.is_tiered                (), false );
    LMI_TEST_EQUAL(z.calculate_load(1.0, strata), 0.0000);
    }
}

int test_main(int, char*[])
{
    premium_tax_test::test();

    return EXIT_SUCCESS;
}
