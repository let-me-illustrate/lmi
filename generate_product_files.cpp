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

// $Id: generate_product_files.cpp,v 1.3 2005-02-14 04:37:51 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_dbdict.hpp"
#include "ihs_funddata.hpp"
#include "ihs_proddata.hpp"
#include "ihs_resetfpu.hpp"
#include "ihs_rnddata.hpp"
#include "tiered_charges.hpp"

#include <iostream>
#include <ostream>

int main()
{
    SetIntelDefaultNdpControlWord();

    std::cout << "Writing files: " << std::flush;

    DBDictionary::WriteDBFiles();
    std::cout << ".db4 " << std::flush;

    TProductData::WritePolFiles();
    std::cout << ".pol " << std::flush;

    FundData::WriteFundFiles();
    std::cout << ".fnd " << std::flush;

    StreamableRoundingRules::WriteRndFiles();
    std::cout << ".rnd " << std::flush;

    tiered_charges::write_tier_files();
    std::cout << ".tir " << std::flush;

    std::cout << "\nAll product files written." << std::endl;

    volatile unsigned short cw = NdpControlWord();
    if(IntelDefaultNdpControlWord() != cw)
        {
        std::cerr <<
            "Product data files are probably invalid because the "
            "floating point precision changed while they were being "
            "written. Probably some other program changed this "
            "important setting. Rerun this program and do no work "
            "in any other program until this one finishes. This "
            "should take only a few seconds."
            ;
        }
    return IntelDefaultNdpControlWord() != cw;
}

