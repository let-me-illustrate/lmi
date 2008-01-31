// Path utilities.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: path_utility.cpp,v 1.14 2008-01-31 05:07:42 chicares Exp $

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

#include <exception>
#include <iomanip>
#include <sstream>

/// Preserve initial path and set "native" name-checking policy for
/// boost filesystem library.
///
/// Applications that end users would normally run should call this
/// function during initialization--before using this boost library
/// in any other way, to ensure uniform name checking (which enables
/// them to use nonportable paths, as some demand), and to make it
/// potentially possible to protect them somewhat from the strange
/// effects of inadvertent changes to the current working directory.
/// As boost's documentation notes, msw may resolve relative paths as
///   "complete( path, kinky ), where kinky is the current directory
///   for the [path's] drive. This will be the current directory of
///   that drive the last time it was set, and thus may well be
///   residue left over from some prior program run by the command
///   processor! Although these semantics are often useful, they are
///   also very error-prone, and certainly deserve to be called
///   'kinky'."
/// although it's unclear whether there's any way to get msw to do
/// this exactly when end users desire it and not otherwise.
///
/// MSYS !! Call this function during initialization for any program
/// that could be passed a path argument, even if the argument is a
/// portable path, because MSYS's bash translates it to a nonportable
/// one. For example, if this function isn't called, then
///   --data_path='/opt/lmi/data'
/// engenders this diagnostic:
///   boost::filesystem::path: [line split for readability]
///     invalid name "C:" in path: "C:/msys/1.0/opt/lmi/data"
///
/// This function is not called in the initialization routine used by
/// all programs, because simple command-line tools should not be
/// forced to depend on this boost library.
///
/// Resist the urge to write its simple implementation inline because
/// that may fail with gcc on msw--see:
///   http://article.gmane.org/gmane.comp.gnu.mingw.user/18633
///     [2006-01-14T11:55:49Z from Greg Chicares]
///
/// The boost documentation says:
///   "The preferred implementation would be to call initial_path()
///   during program initialization, before the call to main().
///   This is, however, not possible with changing the C++ runtime
///   library."
/// One could wish that they had expressed that in code instead of
/// commentary: std::cout manages to work this way by using the
/// so-called "nifty counter" technique, which perhaps ought to be
/// used here.

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

/// validate_directory() throws an informative exception if its
/// 'directory' argument does not name a valid directory.
///
/// 'directory': directory-name to be validated.
///
/// 'context': semantic description of the directory to be named;
/// used in the exception report.
///
/// Although a std::invalid_argument exception would seem more
/// fitting in the context of this function, in the global context
/// 'd' may be specified by users, so std::runtime_error is
/// preferable.
///
/// Exceptions thrown from the boost filesystem library on path
/// assignment are caught in order to rethrow with 'context'
/// prepended.
///
/// TODO ?? Need unit tests.

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
/// conceivable for a census to have more cells than nine places
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
/// Implementation note.
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
        // Assert this postcondition explicitly because the conditions
        // under which boost's remove() function throws have changed
        // between versions. The boost documentation doesn't seem to
        // allow it to fail unless it throws, yet it's not documented
        // to throw on an impermissible operation like removing a file
        // that's locked by another process.
        LMI_ASSERT(!fs::exists(filepath));
        }
    catch(std::exception const&)
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

