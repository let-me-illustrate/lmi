// Frontend for borland C++ librarians.
//
// Copyright (C) 2004, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// This program makes borland's 'tlib' work somewhat like gnu 'ar'.
// The emulation is very limited. Only the following 'ar' commands are
// recognized:
//   r: Add member with replacement.
//   u: Update only newer files.
//   s: Write an index into the file.
//   v: Be verbose.
// However, they aren't by any means fully implemented. The borland
// tool apparently does only unqualified addition and removal, and,
// according to their documentation, gives an error message if a file
// being added is already in the archive, or if a file being removed
// is not already in the archive. And its 'dictionary' option is
// documented to fail with objects containing a C++ class with a
// virtual function, so that's useless. Therefore, we simply remove
// any existing library and create a new one from scratch.

#include "getopt.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "license.hpp"
#include "system_command.hpp"

#include <cstdio>                       // remove()
#include <iostream>
#include <string>

int try_main(int argc, char* argv[])
{
    int c;
    int option_index = 0;
    // Long options are meta-options not directly supported by the borland
    // librarian. TRICKY !! They are aliased to unlikely octal values.
    static Option long_options[] =
      {
        {"help",         NO_ARG,   nullptr, 001, nullptr, "display this help and exit"},
        {"license",      NO_ARG,   nullptr, 002, nullptr, "display license and exit"},
        {"accept",       NO_ARG,   nullptr, 003, nullptr, "accept license (-l to display)"},
        {"program",      REQD_ARG, nullptr, 004, nullptr, "librarian's filepath"},
        {"r",            REQD_ARG, nullptr, 'r', nullptr, "see inline documentation"},
        {"s",            REQD_ARG, nullptr, 's', nullptr, "see inline documentation"},
        {"u",            REQD_ARG, nullptr, 'u', nullptr, "see inline documentation"},
        {"v",            REQD_ARG, nullptr, 'v', nullptr, "see inline documentation"},
        {nullptr,        NO_ARG,   nullptr, 000, nullptr, ""}
      };

    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;

    std::string program;

    std::string library_filename;

    std::string ar_options;

    GetOpt getopt_long
        (argc
        ,argv
        ,""
        ,long_options
        ,&option_index
        ,true
        );
    getopt_long.opterr = false;
    while(EOF != (c = getopt_long()))
        {
        switch(c)
            {
            case 001:
                {
                show_help = true;
                }
                break;

            case 002:
                {
                show_license = true;
                }
                break;

            case 003:
                {
                license_accepted = true;
                }
                break;

            case 004:
                {
                program = getopt_long.optarg;
                }
                break;

            case 'r':
            case 's':
            case 'u':
            case 'v':
                {
                // Do nothing.
                }
                break;

            case '?':
                {
                // Forward any unrecognized options to the tool.
                int offset = getopt_long.optind - 1;
                if(0 < offset)
                    {
                    std::string s = getopt_long.nargv[offset];
                    std::cerr << "Unknown option'" << s << "'.\n";
                    }
                else
                    {
                    std::cerr << "Internal error\n";
                    }
                }
                break;

            default:
                std::cerr << "getopt returned character code 0" << c << '\n';
            }
        }

    if((c = getopt_long.optind) < argc)
        {
        library_filename = std::string(argv[c++]);
        while(c < argc)
            {
            // Forward any unrecognized arguments to the tool.
            ar_options += " +" + std::string(argv[c++]);
            }
        }

    if(!license_accepted)
        {
        std::cerr << license_notices_as_text() << "\n\n";
        }

    if(show_license)
        {
        std::cerr << license_as_text() << "\n\n";
        return EXIT_SUCCESS;
        }

    if(show_help)
        {
        std::cout
            << "This program's emulation of gnu 'ar' is very limited.\n"
            << "Read the inline documentation in file '" __FILE__ "'.\n"
            ;
        getopt_long.usage();
        return EXIT_SUCCESS;
        }

    if("" == program)
        {
        std::cerr << "Must specify --program=(librarian name)\n";
        return EXIT_FAILURE;
        }

    std::remove(library_filename.c_str());

    // The borland librarian allows up to 2^16 'pages', each of a
    // size specified on the command line. The version supplied with
    // their 5.02 compiler fails if the specified page size is not
    // sufficient. The version supplied with their 5.5.1 compiler
    // seems to adjust the page size automatically. It might be nice
    // to perform such an automatic adjustment here for the older
    // version, but we aren't very interested in breathing new life
    // into old tools that were badly designed in the first place.
    std::string command_line =
          program
        + " "
        + "/P1024"
        + " "
        + library_filename
        + " "
        + ar_options
        ;
    try
        {
        system_command(command_line);
        }
    catch(...)
        {
        report_exception();
        std::cerr
            << "Command failed."
            << "\ncommand line: " << command_line
            << '\n'
            << "\nprogram: "      << program
            << "\nlibrary: "      << library_filename
            << "\noptions: "      << ar_options
            << '\n'
            ;
        }
    return 0;
}
