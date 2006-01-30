// The GPL and the notices it requires.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: license.hpp,v 1.6 2006-01-30 04:58:41 chicares Exp $

#ifndef license_hpp
#define license_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

// Provide the complete GPL and appropriate notices for display at
// startup as required by GPL(2)(c), in both flat-text and html
// formats. Of course, the GPL is provided in file 'COPYING' as well.
// Much of this information might alternatively have been read at run
// time from that external file; embedding it here in the source
// ensures that the license and its required notices are available
// unaltered.

std::string const& LMI_SO license_as_html();
std::string const& LMI_SO license_as_text();
std::string const& LMI_SO license_notices_as_html();
std::string const& LMI_SO license_notices_as_text();

#endif // license_hpp

