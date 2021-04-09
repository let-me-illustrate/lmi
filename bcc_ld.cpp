// Frontend for borland C++ linkers.
//
// Copyright (C) 2001, 2003, 2004, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// See my usenet messages sent to borland.public.whatever on
// 2000-06-06T00:05:29 -0400 and 2000-08-28T21:24:18 -0400 for a rather
// different borland linker frontend.

// borland tlink32 syntax:
//   tlink32 [options] startup_file object_files,
//     [exe_name], [mapfile_name], [libraries], [def_file], [res_file]
// The commas are not optional. Paths must be punctuated by backslashes.
// Position is significant.

// desired syntax:
//   borland_link [options] --startup-file=name -o name --map-file=name
//     --def-file=name objects libraries
// where files ending in '.rc.o' are compiled resources, other files ending
// in '.o' are objects, and files ending in '.lib' or '.a' are libraries.

#include "getopt.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "license.hpp"
#include "miscellany.hpp"               // ends_with()
#include "system_command.hpp"

#include <algorithm>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>                     // ostream_iterator
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    // The borland linker doesn't accept *nix paths with forward slashes,
    // so paths must be converted to the windows backslash convention.
    // But the leading forward slash in an option must not be converted
    // to a backslash; that's governed by an argument because the
    // desired behavior depends on context.
    std::string switch_slashes(std::string const& s, bool skip_first)
        {
        std::string t(s);
        for(std::string::size_type j = skip_first; j < t.size(); ++j)
            {
            if('/' == t[j])
                {
                t[j] = '\\';
                }
            }
        return t;
        }

    // TRICKY COMPILER !! Contrary to the borland documentation,
    // library paths *must* be given as
    //  /L$(bc_owl_path)/lib;$(bcXXXdir)/LIB
    // i.e. a single '/L' option with paths separated by a semicolon.
    // This option:
    //  /L$(bc_owl_path)/lib /L$(bcXXXdir)/LIB
    // will *not* work, even though the documentation says it will.
    // It seems that the last '/L' option *replaces* any preceding
    // '/L' options. For 'sh' compatibility, the libdirs would need
    // to be quoted with '"' to prevent the shell from interpreting
    // ';', but that's not wanted for the response file written here.

    std::string mangle_library_paths(std::vector<std::string> const& paths)
        {
        std::ostringstream oss;
        oss << "/L";
        std::ostream_iterator<std::string> osi(oss, ";");
        std::copy(paths.begin(), paths.end(), osi);
        oss << "";
        return oss.str();
        }
} // Unnamed namespace.

int try_main(int argc, char* argv[])
{
    int c;
    int option_index = 0;
    // Long options are meta-options not directly supported by the borland
    // linker. TRICKY !! They are aliased to unlikely octal values. I thought
    // this would allow the borland linker options to pass through, since
    // they are preceded by a single '-' or '/' and no short options are
    // recognized (except for '-o'; I very much want '-o' to name an output
    // file so that I can use the same linker command line with borland as
    // with gnu tools, and I don't much care about borland's "import by
    // ordinal" option). The problem with this design is that borland parses
    // '-ap' as a single option, while getopt (and hence probably posix)
    // parses that as '-a -p'; if I had realized that, I might never have
    // designed this program this way. However, it works well enough as long
    // as you use '/' rather than '-' to introduce borland options.
    static Option long_options[] =
      {
        {"help",         NO_ARG,   nullptr, 001, nullptr, "display this help and exit"},
        {"license",      NO_ARG,   nullptr, 002, nullptr, "display license and exit"},
        {"accept",       NO_ARG,   nullptr, 003, nullptr, "accept license (-l to display)"},
        {"def-file",     REQD_ARG, nullptr, 004, nullptr, "module definition file"},
        {"map-file",     REQD_ARG, nullptr, 005, nullptr, "map file"},
        {"program",      REQD_ARG, nullptr, 007, nullptr, "linker's filepath"},
        {"startup-file", REQD_ARG, nullptr, 010, nullptr, "startup file"},
        {"lib-path",     REQD_ARG, nullptr, 'L', nullptr, "library path"},
        {"out-file",     REQD_ARG, nullptr, 'o', nullptr, "output executable file"},
        {"/",            REQD_ARG, nullptr, '/', nullptr, "linker option"},
        {nullptr,        NO_ARG,   nullptr, 000, nullptr, ""}
      };

    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;

    std::string def_file;
    std::string map_file;
    std::string out_file;
    std::string program;
    std::string res_file;
    std::string startup_file;

    std::vector<std::string> linker_options;
    std::vector<std::string> object_names;
    std::vector<std::string> library_names;
    std::vector<std::string> library_paths;

    GetOpt getopt_long
        (argc
        ,argv
        ,""
        ,long_options
        ,&option_index
        ,false
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
                def_file = switch_slashes(getopt_long.optarg, 0);
                }
                break;

            case 'L':
                {
                library_paths.push_back(switch_slashes(getopt_long.optarg, 0));
                }
                break;

            case 005:
                {
                map_file = switch_slashes(getopt_long.optarg, 0);
                }
                break;

            case 'o':
                {
                out_file = switch_slashes(getopt_long.optarg, 0);
                }
                break;

            case 007:
                {
                program = getopt_long.optarg;
                }
                break;

            case 010:
                {
                startup_file = switch_slashes(getopt_long.optarg, 0);
                }
                break;

            case '/':
            case '?':
                {
                // Forward any unrecognized options to the tool.
                int offset = getopt_long.optind - 1;
                std::string s;
                if(0 < offset)
                    {
                    s = getopt_long.nargv[offset];
                    }
                else
                    {
                    std::cerr << "Internal error\n";
                    }
                linker_options.push_back(s);
                }
                break;

            default:
                std::cerr << "getopt returned character code 0" << c << '\n';
            }
        }

    if((c = getopt_long.optind) < argc)
        {
        while(c < argc)
            {
            std::string s(argv[c++]);
            // It is my personal convention that compiled resource files
            // are named *.rc.o ; others often use *.res or *.res.o .
            if
                (   ends_with(s, ".rc.o")
                ||  ends_with(s, ".res")
                ||  ends_with(s, ".res.o")
                )
                {
                res_file = switch_slashes(s, 0);
                }
            // It is my personal convention that object files
            // are named *.o ; others often use *.obj .
            else if
                (   ends_with(s, ".o")
                ||  ends_with(s, ".obj")
                )
                {
                object_names.push_back(switch_slashes(s, 0));
                }
            else if
                (   ends_with(s, ".lib")
                ||  ends_with(s, ".a")
                )
                {
                library_names.push_back(switch_slashes(s, 0));
                }
            else
            // Forward any unrecognized arguments to the tool.
            //
            // Note that the borland linker has an option '/Vd.d' for digit d,
            // while we take '/dir/x.o' as an object file--so parsing for
            // initial '/' and internal '.' is not sufficient for us to decide
            // what sort of option we have.
                {
                linker_options.push_back(switch_slashes(s, 1));
                }
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
        getopt_long.usage();
        return EXIT_SUCCESS;
        }

    std::ostringstream oss;
    std::ostream_iterator<std::string> osi(oss, " ");
    std::copy(linker_options.begin(), linker_options.end(), osi);
    oss << mangle_library_paths(library_paths) << " ";
    oss << startup_file << " ";
    std::copy(object_names.begin(), object_names.end(), osi);
    oss << ",";
    oss << out_file;
    oss << ",";
    oss << map_file;
    oss << ",";
    std::copy(library_names.begin(), library_names.end(), osi);
    oss << ",";
    oss << def_file;
    oss << ",";
    oss << res_file;
    oss << "\n";

    std::string response_file_name("borland_link.rsp");
    std::ofstream ofs
        (response_file_name.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    ofs << oss.str();
    ofs << std::flush;

    if("" == program)
        {
        std::cerr << "Must specify --program=(linker name)\n";
        }

    // TRICKY COMPILER !! It is necessary to use a tlink32 response file.
    // If we try executing tlink32 with all its options on one command
    // line, even through std::system() or through a batch file, it fails.
    // Perhaps tlink32 spawn()s something else and runs afoul of msw's
    // nasty 260-character limit.
    std::string command_line = program + " @" + response_file_name;
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
            << "\nresponse file:" << oss.str()
            << '\n'
            << "\ndef_file: "     << def_file
            << "\nmap_file: "     << map_file
            << "\nout_file: "     << out_file
            << "\nprogram: "      << program
            << "\nres_file: "     << res_file
            << "\nstartup_file: " << startup_file
            << '\n'
            ;
        std::cerr << "\nlibrary_names: ";
        std::copy
            (library_names.begin()
            ,library_names.end()
            ,std::ostream_iterator<std::string>(std::cerr, " ")
            );
        std::cerr << "\nlibrary_paths: ";
        std::copy
            (library_paths.begin()
            ,library_paths.end()
            ,std::ostream_iterator<std::string>(std::cerr, " ")
            );
        std::cerr << "\noptions: ";
        std::copy
            (linker_options.begin()
            ,linker_options.end()
            ,std::ostream_iterator<std::string>(std::cerr, " ")
            );
        std::cerr << "\nobject_names: ";
        std::copy
            (object_names.begin()
            ,object_names.end()
            ,std::ostream_iterator<std::string>(std::cerr, " ")
            );
        std::cerr << "\n\n";
        }
    return 0;
}
