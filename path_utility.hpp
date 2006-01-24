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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: path_utility.hpp,v 1.5 2006-01-24 07:11:47 chicares Exp $

#ifndef path_utility_hpp
#define path_utility_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

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
///   http://sourceforge.net/mailarchive/message.php?msg_id=14476898
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

void LMI_SO initialize_filesystem();

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

void LMI_SO validate_directory
    (std::string const& directory
    ,std::string const& context
    );

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

std::string LMI_SO serialize_extension
    (int                serial_number
    ,std::string const& extension
    );

/// TODO ?? Need documentation and unit tests.

fs::path LMI_SO serialized_file_path
    (fs::path const&    exemplar
    ,int                serial_number
    ,std::string const& extension
    );

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

fs::path LMI_SO unique_filepath
    (fs::path const&    original_filepath
    ,std::string const& extension
    );

#endif // path_utility_hpp

