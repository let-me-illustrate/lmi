// Product-specific fund data.
//
// Copyright (C) 2001, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: my_fund.cpp,v 1.2 2006-01-29 13:52:00 chicares Exp $

// This file is a template for embedding product-specific data. Doing
// that creates a derived work covered by the GPL. But you may prefer
// not to publish your data, for instance because it is proprietary.
// In that case, the GPL does not permit you to distribute the derived
// work at all. But read the second paragraph of section 0 of the GPL
// carefully: it permits you to run your modified version of the
// program--and to distribute its output, which is not a derived work
// because it's merely your data, trivially cast in a format suitable
// for use with lmi. You can therefore distribute the files created by
// your modified version of this program, but not that program itself.
// Those files are all you need: distributing the program itself isn't
// necessary anyway.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_funddata.hpp"

#include "data_directory.hpp" // AddDataDir()

//============================================================================
void FundData::WriteProprietaryFundFiles()
{
    {
    // Sample product.
    FundData foo;
// Not necessary for 'sample' product only, because it's built in.
//    foo->Write(AddDataDir("sample.fnd"));
    }
#if 0 // Template for other products.
    {
    // Another product.
    FundData foo;
    foo.FundInfo_.push_back(FundInfo(70, "Short Name", "Long Fund Name"));
    foo.Write(AddDataDir("another_product.fnd"));
    }
#endif // 0
}

