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

// $Id: test_coding_rules.cpp,v 1.9 2007-08-03 21:32:37 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "handle_exceptions.hpp"
#include "istream_to_string.hpp"
#include "main_common.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <ios>
#include <iostream>
#include <ostream>
#include <string>

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
            result = result || process_file(argv[j]);
            }
        catch(...)
            {
            report_exception();
            }
        }
    return result ? EXIT_FAILURE : EXIT_SUCCESS;
}

