// Time program execution.
//
// Copyright (C) 2001, 2005 Gregory W. Chicares.
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

#include "system_command.hpp"
#include "timer.hpp"

#include <cstdlib>  // EXIT_FAILURE
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if(argc < 2)
        {
        std::cerr << "elapsed_time: at least one argument required.\n";
        return EXIT_FAILURE;
        }

    std::string command_line;
    std::string space(" ");
    for(int j = 1; j < argc; ++j)
        {
        command_line += argv[j] + space;
        }

    Timer timer;
    int rc = system_command(command_line);
    if(rc)
        {
        std::cerr
            << "elapsed_time: CreateProcess() failed."
            << "\nreturn code: "  << rc
            << "\ncommand line: " << command_line << '\n'
            ;
        }
    std::cout
        << "Elapsed time: "
        << static_cast<int>(1000.0 * timer.Stop().Result())
        << " milliseconds\n"
        ;
    return rc;
}

