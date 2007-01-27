// Display monthly values.
//
// Copyright (C) 1998, 2001, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: avdebug.cpp,v 1.4 2007-01-27 00:00:51 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include <ostream>

// Print monthly detail to a tab-delimited text file.

void AccountValue::DebugPrint(std::ostream& os) const
{
    if(0 == Year)
        {
        os << "Monthly detail\n";
        os << "\n";
        os << "Year\tMonth\tPayment\tAV\tDB\n";
        os << "\n";
        }

    os
        << Year << '\t'
        << Month << '\t'
        << GrossPmts[Month] << '\t'
        << (AVUnloaned + AVRegLn + AVPrfLn) << '\t'
        << deathbft << '\t'
        << '\n';
}

