// Instruct the operating system to execute a command.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: system_command.cpp,v 1.8 2007-01-27 00:00:52 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "system_command.hpp"

#if !defined LMI_MSW

#include <cstdlib>

int system_command(std::string const& command_line)
{
    return std::system(command_line.c_str());
}

#else // defined LMI_MSW

#include <windows.h>

#include <cstring>

int system_command(std::string const& command_line)
{
    STARTUPINFO startup_info;
    std::memset(&startup_info, 0, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(STARTUPINFO);

    PROCESS_INFORMATION process_info;

    char* non_const_cmd_line_copy = new char[1 + command_line.size()];
    std::strcpy(non_const_cmd_line_copy, command_line.c_str());
    ::CreateProcess
        (0
        ,non_const_cmd_line_copy
        ,0
        ,0
        ,true
        ,NORMAL_PRIORITY_CLASS
        ,0
        ,0
        ,&startup_info
        ,&process_info
        );
    delete[]non_const_cmd_line_copy;

    DWORD exit_code;
    ::CloseHandle(process_info.hThread);
    ::WaitForSingleObject(process_info.hProcess, INFINITE);
    ::GetExitCodeProcess(process_info.hProcess, &exit_code);
    ::CloseHandle(process_info.hProcess);

    return exit_code;
}

#endif // defined LMI_MSW

