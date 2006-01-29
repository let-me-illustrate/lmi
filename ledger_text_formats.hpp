// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: ledger_text_formats.hpp,v 1.5 2006-01-29 13:52:00 chicares Exp $

#ifndef ledger_text_formats_hpp
#define ledger_text_formats_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

class Ledger;

void LMI_SO PrintFormTabDelimited
    (Ledger const&      ledger_values
    ,std::string const& file_name
    );

std::string LMI_SO FormatSelectedValuesAsHtml
    (Ledger const& ledger_values
    );

#endif // ledger_text_formats_hpp

