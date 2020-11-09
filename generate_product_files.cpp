// Generate product rule and rate files.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "dbdict.hpp"
#include "fund_data.hpp"
#include "lingo.hpp"
#include "main_common.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "product_data.hpp"
#include "rounding_rules.hpp"
#include "stratified_charges.hpp"

#include <iostream>
#include <ostream>

int try_main(int, char*[])
{
    initialize_filesystem();

    std::cout << "Generating product files." << std::endl;

    DBDictionary       ::write_database_files ();
    FundData           ::write_funds_files    ();
    lingo              ::write_lingo_files    ();
    product_data       ::write_policy_files   ();
    rounding_rules     ::write_rounding_files ();
    stratified_charges ::write_strata_files   ();

    DBDictionary       ::write_proprietary_database_files ();
    FundData           ::write_proprietary_funds_files    ();
    lingo              ::write_proprietary_lingo_files    ();
    product_data       ::write_proprietary_policy_files   ();
    rounding_rules     ::write_proprietary_rounding_files ();
    stratified_charges ::write_proprietary_strata_files   ();

    std::cout << "\nAll product files written.\n" << std::endl;

    return EXIT_SUCCESS;
}
