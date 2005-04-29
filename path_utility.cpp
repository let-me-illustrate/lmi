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

// $Id: path_utility.cpp,v 1.2 2005-04-29 18:53:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "path_utility.hpp"

#include "alert.hpp"
#include "miscellany.hpp" // iso_8601_datestamp_terse()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>

/// Create a unique file path, following input as closely as possible.
///
/// Compose a file path from 'directory' and 'filename', changing its
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

