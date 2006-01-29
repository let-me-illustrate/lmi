// Ledger xsl operations.
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

// $Id: ledger_xsl.hpp,v 1.4 2006-01-29 13:52:00 chicares Exp $

#ifndef ledger_xsl_hpp
#define ledger_xsl_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

class Ledger;

std::string LMI_SO write_ledger_to_pdf
    (Ledger const&      ledger
    ,std::string const& filename
    );

#endif // ledger_xsl_hpp

