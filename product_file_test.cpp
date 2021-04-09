// Product files--unit test.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// Facilities offered by all of these headers are tested here.
#include "database.hpp"
#include "dbdict.hpp"
#include "fund_data.hpp"
#include "lingo.hpp"
#include "product_data.hpp"
#include "rounding_rules.hpp"
#include "stratified_charges.hpp"
// End of headers tested here.

#include "data_directory.hpp"           // AddDataDir()
#include "global_settings.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "test_tools.hpp"
#include "timer.hpp"                    // TimeAnAliquot()

#include <boost/filesystem/path.hpp>

#include <string>
#include <utility>                      // move()

class product_file_test
{
  public:
    static void test()
        {
        // Location of product files.
        global_settings::instance().set_data_directory("/opt/lmi/data");
        get_filenames();
        test_copying();
        assay_speed();
        }

  private:
    static void get_filenames();
    static void test_copying();
    static void assay_speed();
    static void read_database_file()   ;
    static void read_fund_file()       ;
    static void read_lingo_file()      ;
    static void read_policy_file()     ;
    static void read_rounding_file()   ;
    static void read_stratified_file() ;
    static void read_cached_files()    ;

    inline static fs::path database_filename_   ;
    inline static fs::path fund_filename_       ;
    inline static fs::path lingo_filename_      ;
    inline static fs::path policy_filename_     ;
    inline static fs::path rounding_filename_   ;
    inline static fs::path stratified_filename_ ;
};

void product_file_test::get_filenames()
{
    policy_filename_     = filename_from_product_name("sample");
    product_data p(policy_filename_);
    database_filename_   = AddDataDir(p.datum("DatabaseFilename"));
    fund_filename_       = AddDataDir(p.datum("FundFilename"    ));
    lingo_filename_      = AddDataDir(p.datum("LingoFilename"   ));
    rounding_filename_   = AddDataDir(p.datum("RoundingFilename"));
    stratified_filename_ = AddDataDir(p.datum("TierFilename"    ));
}

void product_file_test::test_copying()
{
    product_data p(policy_filename_);

    // Test product_data copy ctor.
    product_data q(p);
    LMI_TEST(database_filename_   == AddDataDir(q.datum("DatabaseFilename")));
    LMI_TEST(fund_filename_       == AddDataDir(q.datum("FundFilename"    )));
    LMI_TEST(lingo_filename_      == AddDataDir(q.datum("LingoFilename"   )));
    LMI_TEST(rounding_filename_   == AddDataDir(q.datum("RoundingFilename")));
    LMI_TEST(stratified_filename_ == AddDataDir(q.datum("TierFilename"    )));

    // Test product_database move ctor.
    product_database d
        ("sample"
        ,mce_female
        ,mce_standard
        ,mce_nonsmoker
        ,45
        ,mce_nonmedical
        ,mce_s_CT
        );
    LMI_TEST(mce_s_CT == d.index().state());
    LMI_TEST(      55 == d.length());
    LMI_TEST(      99 == d.query<int>(DB_MaxIncrAge));
    product_database e(std::move(d));
    LMI_TEST(mce_s_CT == e.index().state());
    LMI_TEST(      55 == e.length());
    LMI_TEST(      99 == e.query<int>(DB_MaxIncrAge));

    // Test product_database copy ctor.
    product_database f
        ("sample"
        ,mce_female
        ,mce_standard
        ,mce_nonsmoker
        ,59
        ,mce_nonmedical
        ,mce_s_CT
        );
    product_database g(f);
    LMI_TEST(mce_s_CT == f.index().state());
    LMI_TEST(      41 == f.length());
    LMI_TEST(      99 == f.query<int>(DB_MaxIncrAge));
    LMI_TEST(mce_s_CT == g.index().state());
    LMI_TEST(      41 == g.length());
    LMI_TEST(      99 == g.query<int>(DB_MaxIncrAge));
}

void product_file_test::read_database_file()
{
    DBDictionary z(database_filename_);
}

void product_file_test::read_fund_file()
{
    FundData z(fund_filename_);
}

void product_file_test::read_lingo_file()
{
    lingo z(lingo_filename_);
}

void product_file_test::read_policy_file()
{
    product_data z(policy_filename_);
}

void product_file_test::read_rounding_file()
{
    rounding_rules z(rounding_filename_);
}

void product_file_test::read_stratified_file()
{
    stratified_charges z(stratified_filename_);
}

void product_file_test::read_cached_files()
{
    DBDictionary       ::read_via_cache(database_filename_);
    FundData           ::read_via_cache(fund_filename_);
    lingo              ::read_via_cache(lingo_filename_);
    product_data       ::read_via_cache(policy_filename_);
    rounding_rules     ::read_via_cache(rounding_filename_);
    stratified_charges ::read_via_cache(stratified_filename_);
}

void product_file_test::assay_speed()
{
    std::cout
        << "  Speed tests..."
        << "\n  Read 'database'   : " << TimeAnAliquot(read_database_file  )
        << "\n  Read 'fund'       : " << TimeAnAliquot(read_fund_file      )
        << "\n  Read 'lingo'      : " << TimeAnAliquot(read_lingo_file     )
        << "\n  Read 'policy'     : " << TimeAnAliquot(read_policy_file    )
        << "\n  Read 'rounding'   : " << TimeAnAliquot(read_rounding_file  )
        << "\n  Read 'stratified' : " << TimeAnAliquot(read_stratified_file)
        << "\n  Read all, cached' : " << TimeAnAliquot(read_cached_files   )
        << '\n'
        ;
}

int test_main(int, char*[])
{
    // Absolute paths require "native" name-checking policy for msw.
    initialize_filesystem();

    product_file_test::test();
    return EXIT_SUCCESS;
}
