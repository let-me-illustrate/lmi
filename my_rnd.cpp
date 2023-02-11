// Product-specific fund data.
//
// Copyright (C) 2001, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "rounding_rules.hpp"

#include "data_directory.hpp"

void rounding_rules::write_proprietary_rounding_files()
{
    // Sample product.
    rounding_rules sample;
// Not necessary for 'sample' product only, because it's built in.
//    sample.save(AddDataDir("sample.rounding"));

#if 0
    // Template for adding another policy form.
    rounding_rules another;
    another.round_specamt_         = rounding_parameters(0, r_upward    , "");
    another.round_death_benefit_   = rounding_parameters(2, r_to_nearest, "");
    // ...
    another.save(AddDataDir("another.rounding"));
#endif // 0
}
