// Product files--unit test.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "dbdict.hpp"
#include "fund_data.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "product_data.hpp"
#include "rounding_rules.hpp"
#include "stratified_charges.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

#include <string>

class product_file_test
{
  public:
    static void test()
        {
        initialize_filesystem();
        write_all_files();
        assay_speed();
        }

  private:
    static void write_all_files();
    static void assay_speed();
    static void read_database_file()   ;
    static void read_fund_file()       ;
    static void read_policy_file()     ;
    static void read_rounding_file()   ;
    static void read_stratified_file() ;

    static std::string database_filename_   ;
    static std::string fund_filename_       ;
    static std::string policy_filename_     ;
    static std::string rounding_filename_   ;
    static std::string stratified_filename_ ;
};

std::string product_file_test::database_filename_   ;
std::string product_file_test::fund_filename_       ;
std::string product_file_test::policy_filename_     ;
std::string product_file_test::rounding_filename_   ;
std::string product_file_test::stratified_filename_ ;

void product_file_test::write_all_files()
{
    DBDictionary()      .WriteSampleDBFile      ();
    product_data       ::WritePolFiles          ();
    FundData           ::WriteFundFiles         ();
    rounding_rules     ::write_rounding_files   ();
    stratified_charges ::write_stratified_files ();

    policy_filename_     = "sample";
    product_data p(policy_filename_);
    database_filename_   = p.datum("DatabaseFilename");
    fund_filename_       = p.datum("FundFilename"    );
    rounding_filename_   = p.datum("RoundingFilename");
    stratified_filename_ = p.datum("TierFilename"    );
}

void product_file_test::read_database_file()
{
    DBDictionary().Init(database_filename_);
}

void product_file_test::read_fund_file()
{
    FundData f(fund_filename_);
}

void product_file_test::read_policy_file()
{
    product_data p(policy_filename_);
}

void product_file_test::read_rounding_file()
{
    rounding_rules r(rounding_filename_);
}

void product_file_test::read_stratified_file()
{
    stratified_charges s(stratified_filename_);
}

void product_file_test::assay_speed()
{
    std::cout
        << "  Speed tests..."
        << "\n  Read 'database'   : " << TimeAnAliquot(read_database_file  )
        << "\n  Read 'fund'       : " << TimeAnAliquot(read_fund_file      )
        << "\n  Read 'policy'     : " << TimeAnAliquot(read_policy_file    )
        << "\n  Read 'rounding'   : " << TimeAnAliquot(read_rounding_file  )
        << "\n  Read 'stratified' : " << TimeAnAliquot(read_stratified_file)
        << '\n'
        ;
}

int test_main(int, char*[])
{
    product_file_test::test();
    return EXIT_SUCCESS;
}

