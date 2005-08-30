// Path utilities.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: path_utility.cpp,v 1.5 2005-08-30 03:54:38 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "path_utility.hpp"

#include "alert.hpp"
#include "miscellany.hpp" // iso_8601_datestamp_terse()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>

#include <iomanip>
#include <sstream>

/// Prepend a serial number to a file extension. This is intended to
/// be used for creating output file names for cells in a census. The
/// input serial number is an origin-zero index into the container of
/// individual cells. The formatted serial number embedded in the
/// output string is in origin one, so that the census's composite can
/// use output serial number zero--that's more satisfying than having
/// it use one plus the number of individual cells.
///
/// The output serial number is formatted to nine places and filled
/// with zeros, so that output file names sort well. It is hardly
/// conceivable for a census to have more cells than nine place
/// accommodate (it's enough to represent all US Social Security
/// numbers), but if it does, then the file names are still unique;
/// they just don't sort as nicely.
///
/// TODO ?? Need unit tests.

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

/// TODO ?? Need documentation and unit tests.

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

/// Create a unique file path, following input as closely as possible.
///
/// Create a file path from 'original_filepath', changing its
/// extension, if any, to 'extension'. If the file path already exists,
/// then try to remove it. If that fails, then try to make the file
/// path unique by inserting a "YYYYMMDDTHHMMSSZ" timestamp before the
/// extension. If even that does not produce a unique name, then throw
/// an exception.
///
/// A try-block is necessary because fs::remove() can throw. The boost
/// documentation doesn't clearly say what happens if the file doesn't
/// actually get removed, e.g. because it's write-protected, so this
/// function checks the postcondition (file shouldn't exist) and, if
/// it's not satisfied, explicitly throws an exception of the same
/// type that the boost library function would throw if it enforced
/// the postcondition, because both these exceptional conditions are
/// handled the same way.

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

