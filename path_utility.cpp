// Path utilities.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "path_utility.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"               // iso_8601_datestamp_terse()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <cctype>                       // std::isalnum()
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
/// Call this function during initialization for any program that
/// could be passed a path argument, even if the argument is a
/// portable path. Motivating case: MSYS's bash translated it to a
/// nonportable path; e.g., if this function wasn't called, then
///   --data_path='/opt/lmi/data'
/// engendered this diagnostic:
///   boost::filesystem::path: [line split for readability]
///     invalid name "C:" in path: "C:/msys/1.0/opt/lmi/data"
/// Keep doing this for future-proofing even though MSYS is no longer
/// supported.
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

/// Return a filename appropriate for posix as well as msw.
///
/// Precondition: argument is not empty.
///
/// Postcondition: regex [0-9A-Za-z_][0-9A-Za-z_.-]*[0-9A-Za-z_]
/// or [0-9A-Za-z_] matches the result, '_' having replaced all
/// other characters.
///
/// Motivation: create a valid filename for apache fop, which, being
/// written in 'java', has its own notion of portability that may
/// clash with every platform it's implemented on. Thus, only pathless
/// filenames should be operated upon: hypothetically, given
///   /Fyodor Dostoyevskiy/Crime and Punishment.text
/// it would be all right to create
///   /Fyodor Dostoyevskiy/Crime_and_Punishment.pdf
/// but it would be wrong to attempt to create
///   /Fyodor_Dostoyevskiy/Crime_and_Punishment.pdf
/// in a directory that doesn't exist. However, path separators in the
/// argument are treated as any other invalid character--deliberately,
/// in case an end user types something like
///   Crime and/or Punishment
/// with no intention of denoting a path.
///
/// Although portable_filename() would be a better name, that would be
/// confusing because the boost filesystem library already provides
/// boolean predicates like portable_file_name(), where Myers
///   http://www.cantrip.org/coding-standard2.html
/// would prefer a predicate phrase like is_portable_file_name():
/// cf. std::isalnum(), std::numeric_limits::is_signed(), and even
/// boost::filesystem::is_complete().

std::string orthodox_filename(std::string const& original_filename)
{
    LMI_ASSERT(!original_filename.empty());
    std::string s(original_filename);
    for(std::string::size_type j = 0; j < s.size(); ++j)
        {
        unsigned char const c = static_cast<unsigned char>(s[j]);
        if
            (  std::isalnum(c)
            || '_' == c
            || ('.' == c || '-' == c) && !(0 == j || s.size() == 1 + j)
            )
            {
            continue;
            }
        else
            {
            s[j] = '_';
            }
        }
    return s;
}

namespace
{
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

std::string serial_extension
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
} // Unnamed namespace.

/// Create an output filename from census information.
///
/// Motivation: see
///   http://savannah.nongnu.org/support/?105907
/// The output filename is composed of:
///  - the census input filename, which identifies the case;
///  - the insured's name, if nonempty, except in regression tests;
///  - the serial number of the insured within the census; and
///  - an extension appropriate to the output type.
///
/// Excluding the insured's name when running regression tests makes
/// output filenames simpler and more regular, yet doesn't suppress
/// any information that would actually be useful.
///
/// Preconditions: census input filepath is nonempty and has a leaf.
/// It's not apparent how a nonempty path could fail to have a leaf,
/// but presumably boost has some undocumented reason.
///
/// Any extension or path is discarded from the input census filepath;
/// only the filename leaf is used.
///
/// It is necessary to call orthodox_filename() on the insured's name
/// in case it contains a character (probably whitespace) that might
/// fail a boost::filesystem name check.

fs::path serial_file_path
    (fs::path    const& exemplar
    ,std::string const& personal_name
    ,int                serial_number
    ,std::string const& extension
    )
{
    LMI_ASSERT(!exemplar.empty());
    LMI_ASSERT(exemplar.has_leaf());
    std::string s(serial_extension(serial_number, extension));
    if
        (  !personal_name.empty()
        && !global_settings::instance().regression_testing()
        )
        {
        s = '.' + orthodox_filename(personal_name) + s;
        }
    return fs::change_extension(exemplar.leaf(), s);
}

/// Create a unique file path, following input as closely as possible.
///
/// Motivating example. Suppose an illustration is created from input
/// file 'foo.in', and output is to be saved in a pdf file. A natural
/// name for the pdf file would be 'foo.pdf'. If a file with that
/// exact name already exists, it should normally be erased, and its
/// name reused: that's what an end user would expect. But that's not
/// possible if 'foo.pdf' is already open in some viewer that locks it
/// against modification; in that case, a distinct new name must be
/// devised.
///
/// Postcondition: !exists(returned_filepath).
///
/// Algorithm. Copy the given file path, changing its extension, if
/// any, to the given extension. If the resulting file path already
/// exists, then try to remove it. If that fails, then try to make the
/// file path unique by inserting a "YYYYMMDDTHHMMSSZ" timestamp right
/// before the last dot: that should suffice because an end user can
/// hardly run illustrations faster than once a second. If even that
/// fails to establish the postcondition, then throw an exception.
///
/// Implementation note.
///
/// A try-block is necessary because fs::remove() can throw. The
/// postcondition is asserted explicitly at the end of the try-block
/// because that boost function's semantics have changed between
/// versions, and its documentation is still unclear in boost-1.34:
/// apparently it mustn't fail without throwing, yet it doesn't throw
/// on an operation that must fail, like removing a file that's locked
/// by another process as in the motivating example above.

fs::path unique_filepath
    (fs::path    const& original_filepath
    ,std::string const& supplied_extension
    )
{
    fs::path filepath(original_filepath);
    filepath = fs::change_extension(filepath, supplied_extension);
    if(!fs::exists(filepath))
        {
        return filepath;
        }

    try
        {
        fs::remove(filepath);
        LMI_ASSERT(!fs::exists(filepath));
        }
    catch(std::exception const&)
        {
        std::string basename  = fs::basename (filepath);
        std::string extension = fs::extension(filepath);
        basename += '-' + iso_8601_datestamp_terse() + extension;
        filepath = filepath.branch_path() / basename;
        if(fs::exists(filepath))
            {
            fatal_error()
                << "Cannot create unique file path from file name '"
                << original_filepath
                << "' with extension '"
                << supplied_extension
                << "'."
                << LMI_FLUSH
                ;
            }
        }
    return filepath;
}

// TODO ?? CALCULATION_SUMMARY Refactor duplication:
//   validate_directory()
//   validate_filepath()
// and add unit tests for both.
// Also rename 'validate_directory' --> 'validate_directory_path',

/// Throw an informative exception if the 'directory' argument does
/// not name a valid directory.
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
            << path
            << "' not found."
            << LMI_FLUSH
            ;
        }
    if(!fs::is_directory(path))
        {
        fatal_error()
            << context
            << " '"
            << path
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
            << path
            << "' not found."
            << LMI_FLUSH
            ;
        }
    if(fs::is_directory(path))
        {
        fatal_error()
            << context
            << " '"
            << path
            << "' is a directory."
            << LMI_FLUSH
            ;
        }
}

