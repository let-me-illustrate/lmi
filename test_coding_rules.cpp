// Test files for consistency with various rules.
//
// Copyright (C) 2006, 2007 Gregory W. Chicares.
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

// $Id: test_coding_rules.cpp,v 1.13 2007-12-15 13:58:19 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "assert_lmi.hpp"
#include "handle_exceptions.hpp"
#include "istream_to_string.hpp"
#include "main_common.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>

#include <ctime>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

// Open predefined standard streams in binary mode.
//
// There is no portable way to do this. Of course, it doesn't matter
// on *nix anyway.
//
// SOMEDAY !! Consider moving this to 'main_common.cpp'. The issue is
// that there are two behaviors:
//   (1) open all files in binary mode
//   (2) open predefined streams in binary mode before main()
// and we want only (2) and not (1), but MinGW doesn't permit that.
// For portable correctness, we should take care to open all streams
// in binary mode; but if we fail to do so, then (1) masks the defect.
// Yet MinGW offers only
//   (a) '_fmode'     --> (1) only
//   (b) '_CRT_fmode' --> both (1) and (2)
// and not [(2) and not (1)]. It is not sufficient to override
// '_fmode' on the first line of main() because non-local objects can
// be constructed before main() is called.

#if defined __MINGW32__
#   include <fcntl.h> // _O_BINARY
    int _CRT_fmode = _O_BINARY;
#endif // defined __MINGW32__

void check_copyright(std::string const& filename, std::string const& s)
{
    std::time_t const t0 = fs::last_write_time(filename);
    std::tm const*const t1 = std::gmtime(&t0);
    LMI_ASSERT(NULL != t1);
    std::ostringstream oss;
    oss << "Copyright.*" << 1900 + t1->tm_year;
    boost::regex const re(oss.str(), boost::regex::sed);
    std::istringstream iss(s);
    std::string line;
    while(std::getline(iss, line))
        {
        if(boost::regex_search(line, re))
            {
            return;
            }
        }
    std::cout << "File '" << filename << "' lacks current copyright.\n";
}

void check_include_guards(std::string const& filename, std::string const& s)
{
    std::string guard = filename;
    std::string::size_type position = guard.find('.');
    while(position != std::string::npos)
        {
        guard.replace(position, 1, std::string("_"));
        position = guard.find('.', 1 + position);
        }

    if
        (   std::string::npos == s.find("\n#ifndef "   + guard + "\n")
        ||  std::string::npos == s.find("\n#define "   + guard + "\n")
        ||  std::string::npos == s.find("\n#endif // " + guard + "\n")
        )
        {
        std::cout << "Noncanonical header guards in '" << filename << "'.\n";
        }
}

void check_xpm(std::string const& filename, std::string const& s)
{
    if(std::string::npos == s.find("\nstatic char const*"))
        {
        std::cout
            << "Lacking /^static char const\\*/ in '"
            << filename
            << "'.\n"
            ;
        }
}

int process_file(std::string const& filename)
{
    fs::path filepath(filename);

    if(!fs::exists(filepath))
        {
        std::cerr << "File '" << filename << "' not found.\n";
        return 1;
        }

    if(fs::is_directory(filepath))
        {
        return 0;
        }

    fs::ifstream ifs(filepath, std::ios_base::binary);
    std::string s;
    istream_to_string(ifs, s);

    if(std::string::npos != s.find('\r'))
        {
        std::cout << "File '" << filename << "' contains '\\r'.\n";
        }

    if(std::string::npos != s.find("\n\n\n"))
        {
        std::cout << "File '" << filename << "' contains '\\n\\n\\n'.\n";
        }

    if(std::string::npos != s.find(" \n"))
        {
        std::cout << "File '" << filename << "' contains ' \\n'.\n";
        }

    if(".hpp" == fs::extension(filename))
        {
        check_include_guards(filename, s);
        }

    if(".xpm" == fs::extension(filename))
        {
        check_xpm(filename, s);
        }
    else
        {
        check_copyright(filename, s);
        }

    if(!ifs)
        {
        std::cerr << "Error processing file '" << filename << "'.\n";
        return 1;
        }

    return 0;
}

int try_main(int argc, char* argv[])
{
    int result = 0;
    for(int j = 1; j < argc; ++j)
        {
        try
            {
            result = process_file(argv[j]) || result;
            }
        catch(...)
            {
            result = 1;
            report_exception();
            }
        }
    return result ? EXIT_FAILURE : EXIT_SUCCESS;
}

