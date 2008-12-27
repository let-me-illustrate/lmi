// Instruct the operating system to execute a command--wx interface.
//
// Copyright (C) 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: system_command_wx.cpp,v 1.8 2008-12-27 02:56:56 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "system_command.hpp"

#include "alert.hpp"
#include "timer.hpp"

#include <wx/utils.h>

#include <cstddef>  // std::size_t
#include <ostream>

namespace
{
void assemble_console_lines
    (std::ostream&        os
    ,wxArrayString const& lines
    ,std::string const&   category
    )
{
    if(lines.IsEmpty())
        {
        return;
        }

    os << category << '\n';
    for(std::size_t j = 0; j < lines.GetCount(); ++j)
        {
        os << lines[j] << '\n';
        }
}

/// Execute a system command using wxExecute().
///
/// If wxExecute() returns 0L, then exit immediately: the command
/// succeeded.
///
/// If wxExecute() returns -1L, then the command could not be
/// executed, and wxExecute() itself displays rich diagnostics;
/// throw an exception to fulfill the system_command() contract,
/// even though doing so displays a redundant diagnostic.
///
/// Otherwise, show what would have appeared on stdout and stderr if
/// the command had been run in an interactive shell, along with the
/// exit code and the command itself.

void concrete_system_command(std::string const& command_line)
{
    Timer timer;
    wxBusyCursor wait;
    status() << "Running..." << std::flush;

    wxArrayString output;
    wxArrayString errors;
    long int exit_code = wxExecute(command_line, output, errors);
    status() << timer.stop().elapsed_msec_str() << std::flush;

    if(0L == exit_code)
        {
        return;
        }
    else if(-1L == exit_code)
        {
        fatal_error()
            << "Command '"
            << command_line
            << "' not recognized."
            << std::flush
            ;
        }
    else
        {
        fatal_error()
            << "Exit code "
            << exit_code
            << " from command '"
            << command_line
            << "'.\n"
            ;
        assemble_console_lines(fatal_error(), output, "Output:");
        assemble_console_lines(fatal_error(), errors, "Errors:");
        fatal_error() << std::flush;
        }
}

volatile bool ensure_setup = system_command_initialize(concrete_system_command);
} // Unnamed namespace.

