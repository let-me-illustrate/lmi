// Generate product rule and rate files.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "dbdict.hpp"
#include "fund_data.hpp"
#include "main_common.hpp"
#include "path_utility.hpp" // initialize_filesystem()
#include "product_data.hpp"
#include "rounding_rules.hpp"
#include "stratified_charges.hpp"

#include <iostream>
#include <ostream>

int try_main(int, char*[])
{
    initialize_filesystem();

    std::cout << "Generating product files." << std::endl;

    DBDictionary::instance() .WriteSampleDBFile                  ();
    product_data            ::WritePolFiles                      ();
    FundData                ::WriteFundFiles                     ();
    rounding_rules          ::write_rounding_files               ();
    stratified_charges      ::write_stratified_files             ();

    DBDictionary::instance() .WriteProprietaryDBFiles            ();
    FundData                ::WriteProprietaryFundFiles          ();
    product_data            ::WriteProprietaryPolFiles           ();
    rounding_rules          ::write_proprietary_rounding_files   ();
    stratified_charges      ::write_proprietary_stratified_files ();

    std::cout << "\nAll product files written.\n" << std::endl;

    return EXIT_SUCCESS;
}

