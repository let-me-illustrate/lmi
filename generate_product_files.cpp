// Generate product rule and rate files.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: generate_product_files.cpp,v 1.12 2008-12-27 02:56:41 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_dbdict.hpp"
#include "ihs_funddata.hpp"
#include "ihs_proddata.hpp"
#include "ihs_rnddata.hpp"
#include "main_common.hpp"
#include "stratified_charges.hpp"

#include <iostream>
#include <ostream>

int try_main(int, char*[])
{
    std::cout << "Generating product files." << std::endl;

    DBDictionary::instance() .WriteSampleDBFile                  ();
    TProductData            ::WritePolFiles                      ();
    FundData                ::WriteFundFiles                     ();
    StreamableRoundingRules ::WriteRndFiles                      ();
    stratified_charges      ::write_stratified_files             ();

    DBDictionary::instance() .WriteProprietaryDBFiles            ();
    FundData                ::WriteProprietaryFundFiles          ();
    TProductData            ::WriteProprietaryPolFiles           ();
    StreamableRoundingRules ::WriteProprietaryRndFiles           ();
    stratified_charges      ::write_proprietary_stratified_files ();

    std::cout << "\nAll product files written.\n" << std::endl;

    return EXIT_SUCCESS;
}

