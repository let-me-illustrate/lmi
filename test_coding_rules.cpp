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

// $Id: test_coding_rules.cpp,v 1.18 2007-12-18 02:19:38 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "assert_lmi.hpp"
#include "handle_exceptions.hpp"
#include "istream_to_string.hpp"
#include "main_common.hpp"
#include "obstruct_slicing.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>
#include <boost/utility.hpp>

#include <ctime>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
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

class file
    :private boost::noncopyable
    ,virtual private obstruct_slicing<file>
{
  public:
    explicit file(std::string const& name);
    ~file() {}

    std::string const& name() const {return name_;}
    fs::path    const& path() const {return path_;}
    std::string const& ext () const {return ext_ ;}
    std::string const& data() const {return data_;}

  private:
    std::string name_;
    fs::path    path_;
    std::string ext_ ;
    std::string data_;
};

file::file(std::string const& name)
    :name_(name)
    ,path_(name)
    ,ext_ (fs::extension(path_))
{
    if(!fs::exists(path_))
        {
        throw std::runtime_error("File not found.");
        }

    if(fs::is_directory(path_))
        {
        throw std::runtime_error("Argument is a directory.");
        }

    fs::ifstream ifs(path_, std::ios_base::binary);
    istream_to_string(ifs, data_);
    if(!ifs)
        {
        throw std::runtime_error("Failure in file input stream.");
        }
}

void complain(file const& f, std::string const& complaint)
{
    std::cout << "File '" << f.name() << "' " << complaint << '\n';
}

bool contains_regex(file const& f, std::string const& regex)
{
    boost::regex const r(regex, boost::regex::sed);
    std::istringstream iss(f.data());
    std::string line;
    while(std::getline(iss, line))
        {
        if(boost::regex_search(line, r))
            {
            return true;
            }
        }
    return false;
}

void require
    (file const&        f
    ,std::string const& regex
    ,std::string const& complaint
    )
{
    if(!contains_regex(f, regex))
        {
        complain(f, complaint);
        }
}

void forbid
    (file const&        f
    ,std::string const& regex
    ,std::string const& complaint
    )
{
    if(contains_regex(f, regex))
        {
        complain(f, complaint);
        }
}

void check_copyright(file const& f)
{
    std::time_t const t0 = fs::last_write_time(f.path());
    std::tm const*const t1 = std::gmtime(&t0);
    LMI_ASSERT(NULL != t1);
    std::ostringstream oss;
    oss << "Copyright.*" << 1900 + t1->tm_year;
    require(f, oss.str(), "lacks current copyright.");
}

void check_include_guards(file const& f)
{
    std::string guard = f.path().leaf();
    std::string::size_type position = guard.find('.');
    while(position != std::string::npos)
        {
        guard.replace(position, 1, std::string("_"));
        position = guard.find('.', 1 + position);
        }

    if
        (   std::string::npos == f.data().find("\n#ifndef "   + guard + "\n")
        ||  std::string::npos == f.data().find("\n#define "   + guard + "\n")
        ||  std::string::npos == f.data().find("\n#endif // " + guard + "\n")
        )
        {
        complain(f, "has noncanonical header guards.");
        }
}

void check_xpm(file const& f)
{
    if(std::string::npos == f.data().find("\nstatic char const*"))
        {
        complain(f, "lacks /^static char const\\*/.");
        }
}

void process_file(std::string const& filename)
{
    file f(filename);

    if(std::string::npos != f.data().find('\r'))
        {
        complain(f, "contains '\\r'.");
        }

    if(std::string::npos != f.data().find("\n\n\n"))
        {
        complain(f, "contains '\\n\\n\\n'.");
        }

    if(std::string::npos != f.data().find(" \n"))
        {
        complain(f, "contains ' \\n'.");
        }

    if(".hpp" == f.ext())
        {
        check_include_guards(f);
        }

    if(".xpm" == f.ext())
        {
        check_xpm(f);
        }
    else
        {
        check_copyright(f);
        }
}

int try_main(int argc, char* argv[])
{
    bool error_flag = false;
    for(int j = 1; j < argc; ++j)
        {
        try
            {
            process_file(argv[j]);
            }
        catch(...)
            {
            error_flag = true;
            std::cerr << "Exception--file '" << argv[j] << "': " << std::flush;
            report_exception();
            }
        }
    return error_flag ? EXIT_FAILURE : EXIT_SUCCESS;
}

