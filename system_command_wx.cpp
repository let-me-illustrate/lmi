// Instruct the operating system to execute a command--wx interface.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: system_command_wx.cpp,v 1.4 2007-06-14 18:09:36 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "system_command.hpp"

#include "alert.hpp"
#include "timer.hpp"

#include <wx/app.h> // wxTheApp
#include <wx/msgdlg.h>
#include <wx/utils.h>

#include <cstddef>  // std::size_t
#include <sstream>

namespace
{
void assemble_console_lines
    (std::ostringstream&  oss
    ,wxArrayString const& lines
    ,std::string const&   category
    )
{
    if(lines.IsEmpty())
        {
        return;
        }

    oss << category << '\n';
    for(std::size_t j = 0; j < lines.GetCount(); ++j)
        {
        oss << lines[j] << '\n';
        }
}

/// Execute a system command using wxExecute().
///
/// If wxExecute() returns -1L, then exit immediately: the command
/// could not be run, and wxExecute() itself pops up a messagebox
/// explaining why.
///
/// If wxExecute() returns 0L, then exit immediately: the command
/// succeeded.
///
/// Otherwise, show what would have appeared on stdout and stderr if
/// the command had been run in an interactive shell, along with the
/// exit code and the command itself.
///
/// In all cases, return wxExecute()'s exit code, truncated to 'int',
/// provided that truncation does not change its boolean sense; else
/// return -13.

int concrete_system_command(std::string const& command_line)
{
    Timer timer;
    wxBusyCursor wait;
    status() << "Running..." << std::flush;

    wxArrayString output;
    wxArrayString errors;
    long int exit_code = wxExecute(command_line, output, errors);
    status() << timer.stop().elapsed_msec_str() << std::flush;

    if(-1L != exit_code && 0L != exit_code)
        {
        std::ostringstream oss;
        assemble_console_lines(oss, output, "Output:");
        assemble_console_lines(oss, errors, "Errors:");
        oss
            << "Exit code "
            << exit_code
            << " from command '"
            << command_line
            << "'."
            ;
        wxMessageBox
            (oss.str()
            ,"Problem executing command"
            ,wxICON_ERROR
            ,wxTheApp->GetTopWindow()
            );
        }

    int return_value = static_cast<int>(exit_code);
    if(0 == return_value && 0 != exit_code)
        {
        return -13;
        }
    else
        {
        return return_value;
        }
}

volatile bool ensure_setup = system_command_initialize(concrete_system_command);
} // Unnamed namespace.

