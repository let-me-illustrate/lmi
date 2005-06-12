// Generate product rule and rate files.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: generate_product_files.cpp,v 1.6 2005-06-12 16:58:36 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_dbdict.hpp"
#include "ihs_funddata.hpp"
#include "ihs_proddata.hpp"
#include "ihs_rnddata.hpp"
#include "main_common.hpp"
#include "tiered_charges.hpp"

#include <iostream>
#include <ostream>

int try_main(int, char*[])
{
    std::cout << "Generating product files." << std::endl;

    DBDictionary::instance().WriteSampleDBFile();
    TProductData            ::WritePolFiles();
    FundData                ::WriteFundFiles();
    StreamableRoundingRules ::WriteRndFiles();
    tiered_charges          ::write_tier_files();

    DBDictionary::instance().WriteProprietaryDBFiles();
    FundData                ::WriteProprietaryFundFiles    ();
    TProductData            ::WriteProprietaryPolFiles     ();
    StreamableRoundingRules ::WriteProprietaryRndFiles     ();
    tiered_charges          ::write_proprietary_tier_files ();

    std::cout << "\nAll product files written.\n" << std::endl;

    return EXIT_SUCCESS;
}

