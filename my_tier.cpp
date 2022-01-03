// Product-specific fund data.
//
// Copyright (C) 2001, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "stratified_charges.hpp"

#include "data_directory.hpp"
#include "ieee754.hpp"                  // infinity<>()

//============================================================================
void stratified_charges::write_proprietary_strata_files()
{
    // Sample policy form.
    stratified_charges foo;

    // Example only.
    foo.datum("CurrSepAcctLoadTieredByAssets").values_.push_back(0.0);
    foo.datum("CurrSepAcctLoadTieredByAssets").limits_.push_back(infinity<double>());

// Not necessary for 'sample' product only, because it's built in.
//    foo.write(AddDataDir("sample.strata"));

    // Another policy form....
}
