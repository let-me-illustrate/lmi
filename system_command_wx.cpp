// Instruct the operating system to execute a command--wx interface.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "system_command.hpp"

#include "alert.hpp"
#include "force_linking.hpp"
#include "null_stream.hpp"
#include "timer.hpp"

#include <wx/app.h>                     // wxTheApp
#include <wx/frame.h>
#include <wx/utils.h>

#include <ostream>

LMI_FORCE_LINKING_IN_SITU(system_command_wx)

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
    for(auto const& j : lines)
        {
        os << j << '\n';
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
///
/// Show elapsed time on statusbar iff statusbar is available.
///
/// See:
///   https://lists.nongnu.org/archive/html/lmi/2013-11/msg00017.html
/// for the wxEXEC_NODISABLE rationale. This is potentially dangerous,
/// and could be inhibited (by an extra argument) if ever needed.

void concrete_system_command(std::string const& cmd_line)
{
    Timer timer;
    wxBusyCursor reverie;

    wxFrame const* f =
                                 wxTheApp
        ? dynamic_cast<wxFrame*>(wxTheApp->GetTopWindow())
        : nullptr;
        ;
    bool const b = f && f->GetStatusBar();
    std::ostream& statusbar_if_available = b ? status() : null_stream();

    statusbar_if_available << "Running..." << std::flush;
    wxArrayString output;
    wxArrayString errors;
    long int exit_code = wxExecute(cmd_line, output, errors, wxEXEC_NODISABLE);
    statusbar_if_available << timer.stop().elapsed_msec_str() << std::flush;

    if(0L == exit_code)
        {
        return;
        }
    else if(-1L == exit_code)
        {
        alarum()
            << "Command '"
            << cmd_line
            << "' not recognized."
            << std::flush
            ;
        }
    else
        {
        alarum()
            << "Exit code "
            << exit_code
            << " from command '"
            << cmd_line
            << "'.\n"
            ;
        assemble_console_lines(alarum(), output, "Output:");
        assemble_console_lines(alarum(), errors, "Errors:");
        alarum() << std::flush;
        }
}

bool volatile ensure_setup = system_command_initialize(concrete_system_command);
} // Unnamed namespace.
