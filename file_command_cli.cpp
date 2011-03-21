// Run file command--command-line interface.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "file_command.hpp"

#include "alert.hpp"

namespace
{
void concrete_file_command
    (std::string const&
    ,std::string const&
    )
{
    fatal_error()
        << "Class 'file_command' not implemented for command-line interface."
        << LMI_FLUSH
        ;
}

volatile bool ensure_setup = file_command_initialize(concrete_file_command);
} // Unnamed namespace.

