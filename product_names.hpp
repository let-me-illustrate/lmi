// List of available products.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#ifndef product_names_hpp
#define product_names_hpp

#include "config.hpp"

#include <string>
#include <vector>

/// Base names of '.policy' files found in data directory.
/// They are read only once and cached: see documentation for
/// class ce_product_name.

std::vector<std::string> const& product_names();

/// Default product name is "sample" if that product is available,
/// else the name of the first product found.
///
/// Rationale: It is always possible to specify a different default
/// product by using a default-input file. If none is used, then the
/// first product found is not necessarily a better default than
/// "sample". Defaults hardcoded in the input class are designed to
/// be generally suitable, but might be inappropriate for some exotic
/// product. If a user creates an invalid product that appears first
/// in the list, then the system will still work in default cases
/// with "sample".

std::string const& default_product_name();

#endif // product_names_hpp

