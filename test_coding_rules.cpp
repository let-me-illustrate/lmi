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

// $Id: test_coding_rules.cpp,v 1.23 2007-12-20 12:31:21 chicares Exp $

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
    explicit file(std::string const& file_path);
    ~file() {}

    fs::path    const& path     () const {return path_;     }
    std::string const& full_name() const {return full_name_;}
    std::string const& leaf_name() const {return leaf_name_;}
    std::string const& extension() const {return extension_;}
    std::string const& data     () const {return data_;     }

  private:
    fs::path    path_;
    std::string full_name_;
    std::string leaf_name_;
    std::string extension_;
    std::string data_;
};

/// Read file contents into a string.
///
/// Require a '\n' at the end of every file, extending the C++98
/// [2.1/1/2] requirement to all files as an lmi standard. C++98 makes
/// an exception for empty files, but there's no reason for lmi to
/// have any.
///
/// Add a '\n' sentry at the beginning of the string for the reason
/// explained in 'regex_test.cpp'.

file::file(std::string const& file_path)
    :path_     (file_path)
    ,full_name_(file_path)
    ,leaf_name_(path_.leaf())
    ,extension_(fs::extension(path_))
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

    data_ = '\n' + data_;
    if(0 == data_.size() || '\n' != data_.at(data_.size() - 1))
        {
        throw std::runtime_error("File does not end in '\\n'.");
        }
}

void complain(file const& f, std::string const& complaint)
{
    std::cout << "File '" << f.full_name() << "' " << complaint << '\n';
}

void require
    (file const&        f
    ,std::string const& regex
    ,std::string const& complaint
    )
{
    if(!boost::regex_search(f.data(), boost::regex(regex)))
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
    if(boost::regex_search(f.data(), boost::regex(regex)))
        {
        complain(f, complaint);
        }
}

// SOMEDAY !! This test could be liberalized to permit copyright
// notices to span multiple lines.

// SOMEDAY !! Move the 'GNUmakefile' logic to exclude certain other
// files hither.

void check_copyright(file const& f)
{
    if(".xpm" == f.extension())
        {
        return;
        }

    std::time_t const t0 = fs::last_write_time(f.path());
    std::tm const*const t1 = std::gmtime(&t0);
    LMI_ASSERT(NULL != t1);
    std::ostringstream oss;
    oss << "Copyright[^\\n]*" << 1900 + t1->tm_year;
    require(f, oss.str(), "lacks current copyright.");
}

void check_include_guards(file const& f)
{
    if(".hpp" != f.extension())
        {
        return;
        }

    std::string guard = f.leaf_name();
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
    if(".xpm" != f.extension())
        {
        return;
        }

    if(std::string::npos == f.data().find("\nstatic char const*"))
        {
        complain(f, "lacks /^static char const\\*/.");
        }
}

void process_file(std::string const& file_path)
{
    file f(file_path);

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

    check_copyright      (f);
    check_include_guards (f);
    check_xpm            (f);
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

