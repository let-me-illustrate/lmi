// A custom interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: custom_io_0.hpp,v 1.5 2006-01-29 13:52:00 chicares Exp $

#ifndef custom_io_0_hpp
#define custom_io_0_hpp

#include "config.hpp"

#include "so_attributes.hpp"

class IllusInputParms;
class Ledger;

bool LMI_SO DoesSpecialInputFileExist();

void LMI_SO PrintFormSpecial
    (Ledger const& ledger_values
    ,char const*   overridden_filename = 0
    );

bool LMI_SO SetSpecialInput
    (IllusInputParms& ip
    ,char const*      overridden_filename = 0
    );

#endif  // custom_io_0_hpp

