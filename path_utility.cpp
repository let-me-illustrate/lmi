// Path utilities.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: path_utility.cpp,v 1.10 2006-12-12 11:16:19 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "path_utility.hpp"

#include "alert.hpp"
#include "miscellany.hpp" // iso_8601_datestamp_terse()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <iomanip>
#include <sstream>

void initialize_filesystem()
{
    fs::path::default_name_check(fs::native);
    fs::initial_path();
}

// TODO ?? CALCULATION_SUMMARY Refactor duplication:
//   validate_directory()
//   validate_filepath()
// Also rename 'validate_directory' --> 'validate_directory_path',
// and add unit tests.

void validate_directory
    (std::string const& directory
    ,std::string const& context
    )
{
    fs::path path;
    try
        {
        path = directory;
        }
    catch(fs::filesystem_error const& e)
        {
        fatal_error()
            << context
            << ": "
            << e.what()
            << LMI_FLUSH
            ;
        }

    if(path.empty())
        {
        fatal_error()
            << context
            << " must not be empty."
            << LMI_FLUSH
            ;
        }
    if(!fs::exists(path))
        {
        fatal_error()
            << context
            << " '"
            << path.string()
            << "' not found."
            << LMI_FLUSH
            ;
        }
    if(!fs::is_directory(path))
        {
        fatal_error()
            << context
            << " '"
            << path.string()
            << "' is not a directory."
            << LMI_FLUSH
            ;
        }
}

void validate_filepath
    (std::string const& filepath
    ,std::string const& context
    )
{
    fs::path path;
    try
        {
        path = filepath;
        }
    catch(fs::filesystem_error const& e)
        {
        fatal_error()
            << context
            << ": "
            << e.what()
            << LMI_FLUSH
            ;
        }

    if(path.empty())
        {
        fatal_error()
            << context
            << " must not be empty."
            << LMI_FLUSH
            ;
        }
    if(!fs::exists(path))
        {
        fatal_error()
            << context
            << " '"
            << path.string()
            << "' not found."
            << LMI_FLUSH
            ;
        }
    if(fs::is_directory(path))
        {
        fatal_error()
            << context
            << " '"
            << path.string()
            << "' is a directory."
            << LMI_FLUSH
            ;
        }
}

std::string serialize_extension
    (int                serial_number
    ,std::string const& extension
    )
{
    std::ostringstream oss;
    oss
        << '.'
        << std::setfill('0') << std::setw(9) << 1 + serial_number
        << '.'
        << extension
        ;
    return oss.str();
}

fs::path serialized_file_path
    (fs::path const&    exemplar
    ,int                serial_number
    ,std::string const& extension
    )
{
    return fs::change_extension
        (exemplar
        ,serialize_extension(serial_number, extension)
        );
}

fs::path unique_filepath
    (fs::path const&    original_filepath
    ,std::string const& extension
    )
{
    fs::path filepath(original_filepath);
    filepath = fs::change_extension(filepath, extension);
    if(!fs::exists(filepath))
        {
        return filepath;
        }

    try
        {
        fs::remove(filepath);
        if(fs::exists(filepath))
            {
            throw fs::filesystem_error("", "Removal failed");
            }
        }
    catch(fs::filesystem_error const& e)
        {
        std::string basename = fs::basename(filepath);
        basename += '-' + iso_8601_datestamp_terse() + extension;
        filepath = filepath.branch_path() / basename;
        if(fs::exists(filepath))
            {
            fatal_error()
                << "Cannot create unique file path from file name '"
                << original_filepath.string()
                << "' with extension '"
                << extension
                << "'."
                << LMI_FLUSH
                ;
            }
        }
    return filepath;
}

