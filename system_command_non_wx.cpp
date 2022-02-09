// Instruct the operating system to execute a command--non-wx interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "system_command.hpp"

#include "alert.hpp"

#if !defined LMI_MSW
#   include <cstdlib>                   // system()
#else  // defined LMI_MSW
#   include <windows.h>
#endif // defined LMI_MSW

namespace
{
#if !defined LMI_MSW

void concrete_system_command(std::string const& command_line)
{
    int exit_code = std::system(command_line.c_str());
    if(EXIT_SUCCESS != exit_code)
        {
        alarum()
            << "Exit code "
            << exit_code
            << " from command '"
            << command_line
            << "'."
            << std::flush
            ;
        }
}

#else  // defined LMI_MSW

void concrete_system_command(std::string const& command_line)
{
    STARTUPINFO startup_info = {};
    startup_info.cb = sizeof(STARTUPINFO);

    PROCESS_INFORMATION process_info;

    // For 'wine' at least, this argument cannot be const, even though
    // this authority:
    //   https://blogs.msdn.microsoft.com/oldnewthing/20090601-00/?p=18083
    // says that requirement affects "only the Unicode version". It
    // would seem wrong to change this wrapper's argument type (for
    // POSIX too) because of this msw implementation detail.
    std::string non_const_cmd_line_copy = command_line;
    ::CreateProcessA
        (nullptr
        ,non_const_cmd_line_copy.data()
        ,nullptr
        ,nullptr
        ,true
        ,NORMAL_PRIORITY_CLASS
        ,nullptr
        ,nullptr
        ,&startup_info
        ,&process_info
        );

    DWORD exit_code = 12345;
    ::CloseHandle(process_info.hThread);
    ::WaitForSingleObject(process_info.hProcess, INFINITE);
    ::GetExitCodeProcess(process_info.hProcess, &exit_code);
    ::CloseHandle(process_info.hProcess);

    if(0 != exit_code)
        {
        alarum()
            << "Exit code "
            << exit_code
            << " from command '"
            << command_line
            << "'."
            << std::flush
            ;
        }
}

#endif // defined LMI_MSW

bool volatile ensure_setup = system_command_initialize(concrete_system_command);
} // Unnamed namespace.
