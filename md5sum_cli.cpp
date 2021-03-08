// Limited functionality variant of GNU 'md5sum' program.
//
// Copyright (C) 2020, 2021 Gregory W. Chicares.
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

#include "getopt.hpp"
#include "main_common.hpp"
#include "md5sum.hpp"

#include <cstdio>                       // fflush()
#include <iostream>
#include <stdexcept>
#include <string>

int usage(int status)
{
    if(status != EXIT_SUCCESS)
        {
        std::cerr << "Try 'lmi_md5sum --help' for more information.\n";
        }
    else
        {
        std::cout
            <<
1 + R"--cut-here--(
Usage: lmi_md5sum [OPTION]... [FILE]...
Print or check MD5 (128-bit) checksums.

  -b, --binary   read in binary mode (default)
  -c, --check    read MD5 sums from the FILEs and check them
  -t, --text     read in text mode

The following two options are useful only when verifying checksums:
      --quiet    don't print OK for each successfully verified file
      --status   don't output anything, status code shows success

      --help     display this help and exit
      --version  output version information and exit

The sums are computed as described in RFC 1321.  When checking, the input
should be a former output of this program. The default mode is to print a
line with checksum, a space, a character indicating input mode ('*' for binary
' ' for text or where binary is insignificant), and name for each FILE.

This program is a limited functionality variant of GNU 'md5sum' utility
and is part of the 'Let Me Illustrate' project.
)--cut-here--";
        }
    return status;
}

void version()
{
    std::cout
        <<
1 + R"--cut-here--(
lmi_md5sum 0.9

This program is a limited functionality variant of GNU 'md5sum' utility
and is part of the 'Let Me Illustrate' project.
)--cut-here--";
}

// For long options that have no equivalent short option, use a
// non-character as a pseudo short option, starting with CHAR_MAX + 1.
enum
{
    STATUS_OPTION = CHAR_MAX + 1,
    QUIET_OPTION,
    HELP_OPTION,
    VERSION_OPTION
};

int try_main(int argc, char* argv[])
{
    bool all_ok = true;
    int c;
    int option_index = 0;
    struct Option long_options[] =
        {
            {"binary"      ,NO_ARG   ,nullptr ,'b'            ,nullptr ,""},
            {"check"       ,NO_ARG   ,nullptr ,'c'            ,nullptr ,""},
            {"quiet"       ,NO_ARG   ,nullptr ,QUIET_OPTION   ,nullptr ,""},
            {"status"      ,NO_ARG   ,nullptr ,STATUS_OPTION  ,nullptr ,""},
            {"text"        ,NO_ARG   ,nullptr ,'t'            ,nullptr ,""},
            {"help"        ,NO_ARG   ,nullptr ,HELP_OPTION    ,nullptr ,""},
            {"version"     ,NO_ARG   ,nullptr ,VERSION_OPTION ,nullptr ,""},
            {nullptr       ,NO_ARG   ,nullptr ,000            ,nullptr ,""}
        };

    bool show_help = false;
    bool show_version = false;
    bool binary = true;
    bool have_input_mode_option = false;
    bool do_check = false;
    bool command_line_syntax_error = false;

    // With --check, don't generate any output.
    // The exit code indicates success or failure.
    bool status_only = false;

    // With --check, suppress the "OK" printed for each verified file.
    bool quiet = false;

    GetOpt getopt_long
        (argc
        ,argv
        ,"chv"
        ,long_options
        ,&option_index
        ,1
        );

    while(EOF != (c = getopt_long ()))
        {
        switch(c)
            {
            case 'b':
                have_input_mode_option = true;
                break;
            case 'c':
                do_check = true;
                break;
            case QUIET_OPTION:
                quiet = true;
                break;
            case STATUS_OPTION:
                status_only = true;
                break;
            case 't':
                binary = false;
                have_input_mode_option = true;
                break;
            case HELP_OPTION:
                show_help = true;
                break;
            case VERSION_OPTION:
                show_version = true;
                break;
            default:
                // Error message was already given from getopt() code, so no need
                // to output anything else here, but do flush its output so that it
                // appears before the usage message.
                std::fflush(stderr);

                command_line_syntax_error = true;
            }

        if(command_line_syntax_error)
            break;
        }

    if(command_line_syntax_error)
        {
        std::cerr << "Try 'lmi_md5sum --help' for more information." << std::endl;
        return EXIT_FAILURE;
        }
    if(have_input_mode_option && do_check)
        {
        std::cerr
            << "The --binary and --text options are meaningless when "
            << "verifying checksums."
            << std::endl
            ;
        return usage(EXIT_FAILURE);
        }
    if(status_only && !do_check)
        {
        std::cerr
            << "The --status option is meaningful only when verifying checksums."
            << std::endl
            ;
        return usage(EXIT_FAILURE);
        }
    if(quiet && !do_check)
        {
        std::cerr
            << "The --quiet option is meaningful only when verifying checksums."
            << std::endl
            ;
        return usage(EXIT_FAILURE);
        }
    if(show_help)
        {
        return usage(EXIT_SUCCESS);
        }

    if(show_version)
        {
        version();
        return EXIT_SUCCESS;
        }

    try
        {
        for(int i = getopt_long.optind; i < argc; ++i)
            {
            char const* filename = argv[i];
            std::string md5;

            if(do_check)
                {
                auto const sums = md5_read_checksum_file(filename);

                for(auto const& s : sums)
                    {
                    md5 = md5_calculate_file_checksum(s.filename, s.file_mode);

                    bool const current_ok = md5 == s.md5sum;
                    if(!status_only)
                        {
                        if(!current_ok || !quiet)
                            {
                            std::cout
                                << s.filename.string()
                                << ": "
                                << (current_ok ? "OK" : "FAILED")
                                << std::endl
                                ;
                            }
                        }
                    all_ok &= current_ok;
                    }
                }
            else
                {
                md5 = md5_calculate_file_checksum
                    (filename
                    ,binary ? md5_file_mode::binary : md5_file_mode::text
                    );

                std::cout << md5 << " " << (binary ? "*" : " ") << filename << std::endl;
                }
            }
        }
    catch(std::runtime_error const& e)
        {
        if(!status_only)
            std::cerr << "lmi_md5sum: " << e.what() << std::endl;
        return EXIT_FAILURE;
        }

    return all_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
