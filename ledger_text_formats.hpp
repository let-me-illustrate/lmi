// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger_text_formats.hpp,v 1.1 2005-04-30 18:15:33 chicares Exp $

#ifndef ledger_text_formats_hpp
#define ledger_text_formats_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <string>

class Ledger;

void LMI_EXPIMP PrintFormTabDelimited
    (Ledger const&      a_LedgerValues
    ,std::string const& file_name
    );

void LMI_EXPIMP PrintFormSpecial(Ledger const& a_LedgerValues);

std::string LMI_EXPIMP FormatSelectedValuesAsHtml(Ledger const& ledger_values);

#endif // ledger_text_formats_hpp

