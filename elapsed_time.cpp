// Time program execution.
//
// Copyright (C) 2001, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: elapsed_time.cpp,v 1.7 2007-06-26 00:29:25 chicares Exp $

#include "handle_exceptions.hpp"
#include "main_common.hpp"
#include "system_command.hpp"
#include "timer.hpp"

#include <cstdlib>  // EXIT_FAILURE
#include <iostream>
#include <string>

int try_main(int argc, char* argv[])
{
    if(argc < 2)
        {
        std::cerr << "elapsed_time: at least one argument required.\n";
        return EXIT_FAILURE;
        }

    std::string command_line;
    for(int j = 1; j < argc; ++j)
        {
        command_line += argv[j];
        if(argc != 1 + j)
            {
            command_line += " ";
            }
        }

    int return_value = EXIT_FAILURE;
    Timer timer;
    try
        {
        system_command(command_line);
        return_value = EXIT_SUCCESS;
        }
    catch(...)
        {
        report_exception();
        }
    std::cout << "Elapsed time: " << timer.stop().elapsed_msec_str() << '\n';
    return return_value;
}

