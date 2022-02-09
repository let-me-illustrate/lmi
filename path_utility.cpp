// Path utilities.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "path_utility.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"               // iso_8601_datestamp_terse()
#include "path.hpp"

#include <cctype>                       // isalnum()
#include <exception>
#include <iomanip>
#include <sstream>

/// Change '/path/to/file' to '/some/other/place/file'.
///
/// Motivation: lmi must provide this function because the standard
/// library doesn't.
///
/// Given
///   fs::path file("/bin/sh";
///   fs::path dir ("/usr/bin");
/// what should 'dir / file' yield? Oddly, boost returned:
///   /usr/bin/bin/sh
/// even on posix, where
///   true == file.is_complete() // old boost function name
/// It is odd, in a different way, that std::filesystem returns:
///   /bin/sh
/// both on posix (where true == file.is_absolute()), and also on
/// msw (where, without a 'root-name', it isn't "absolute").
///
/// Arguably the arguments should be given in the opposite order:
///   modify_directory("sh", "/usr/bin") // present order
///   modify_directory("/usr/bin", "sh") // opposite order
/// because the dirname precedes the basename in canonical form.
/// However, consider a nondegenerate case--this:
///   modify_directory("/bin/sh", "/usr/bin") // present order
/// naturally means "change the directory of /bin/sh to /usr/bin"
/// (yielding "/usr/bin/sh"), whereas this:
///   modify_directory("/usr/bin", "/bin/sh") // opposite order
/// seems less natural.
///
/// "change_directory" might be a more natural name, except that it
/// evokes chdir(2) and cd(1).
///
/// Asserted precondition:
///   - argument 'original_filepath' has a nonempty basename
///     (i.e., true == has_filename())
/// It is notably not required that 'supplied_directory' name an
/// actual existing directory.
///
/// std::filesystem provides no way to test whether a path has the
/// form of a directory. Its fs::is_directory() asks the operating
/// system whether or not a directory exists, so
///   is_directory("/usr/lib")
/// returns 'true' iff the OS reports that such a directory exists;
/// but the same function call would return 'false' after
///   rm -rf /usr/lib ; touch /usr/lib
/// It allows both fs::path("/bin/") and fs::path("/bin"): posix
/// would say the first must be a dirname, while the second could be
/// a basename, but fs::path includes both those concepts.

fs::path modify_directory
    (fs::path const& original_filepath
    ,fs::path const& supplied_directory
    )
{
    LMI_ASSERT(original_filepath.has_filename());
    return supplied_directory / original_filepath.filename();
}

/// Return a filename appropriate for posix as well as msw.
///
/// Precondition: argument is not empty.
///
/// Postcondition: regex [0-9A-Za-z_][0-9A-Za-z_.-]*[0-9A-Za-z_]
/// or [0-9A-Za-z_] matches the result, '_' having replaced all
/// other characters.
///
/// Original motivation: create a valid filename for apache fop, which
/// used the 'java' notion of portability that may clash with every
/// platform it's implemented on. Thus, only pathless filenames should
/// be operated upon: hypothetically, given
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
/// Even though lmi no longer uses fop, it is still good to impose
/// some rationality on output filenames that reflect end-user input,
/// and the rules above are reasonable.

std::string portable_filename(std::string const& original_filename)
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

/// Remove an msw root /^.*:/ from path iff system is not msw.
///
/// Motivation: Prevent the ghastly outcome demonstrated in the unit
/// test when an msw-native path is used on a posix system.
///
/// On an msw system, return the path unaltered: it may contain a
/// 'root-name', but that 'root-name' is native, not alien.

fs::path remove_alien_msw_root(fs::path const& original_filepath)
{
#if defined LMI_POSIX
    std::string s {original_filepath.string()};
    std::string::size_type p = s.find_last_of(':');
    if(std::string::npos != p)
        {
        s.erase(0, 1 + p);
        }
    return s;
#elif defined LMI_MSW
    return original_filepath;
#else  // Unknown platform.
    throw "Unrecognized platform."
#endif // Unknown platform.
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
///   https://savannah.nongnu.org/support/?105907
/// The output filename is composed of:
///  - the census input basename, which identifies the case;
///  - the insured's name, if nonempty, except in regression tests;
///  - the serial number of the insured within the census; and
///  - an extension appropriate to the output type.
///
/// Excluding the insured's name when running regression tests makes
/// output filenames simpler and more regular, yet doesn't suppress
/// any information that would actually be useful.
///
/// Precondition: census input filepath has a nonempty basename.
///
/// Any extension or path is discarded from the input census filepath;
/// only the basename is used. In std::filesystem terms, that would
/// seem to suggest calling stem(), but stem() is not called; instead,
/// filename() is extracted, and then replace_extension() is called.
///
/// Apply portable_filename() to the census filename and the insured's
/// name: because they're under end-user control, they may contain
/// characters that would render the resulting filename nonportable.

fs::path serial_file_path
    (fs::path    const& exemplar
    ,std::string const& personal_name
    ,int                serial_number
    ,std::string const& extension
    )
{
    LMI_ASSERT(exemplar.has_filename());
    fs::path f(portable_filename(exemplar.filename().string()));
    std::string s(serial_extension(serial_number, extension));
    if
        (  !personal_name.empty()
        && !global_settings::instance().regression_testing()
        )
        {
        s = '.' + portable_filename(personal_name) + s;
        }
    return f.replace_extension(s);
}

/// Create a unique file path, following input as closely as possible.
///
/// Motivating example. Suppose an illustration is created from input
/// file 'foo.in', and output is to be saved in a pdf file. A natural
/// name for the pdf file would be 'foo.pdf'. If a file with that
/// exact name already exists, it should normally be erased, and its
/// name reused: that's what an end user would expect, and it's how
/// *nix naturally works. But that's not possible on msw if 'foo.pdf'
/// is already open in some viewer that locks it against modification;
/// in that case, a distinct new name must be devised.
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
/// Implementation notes.
///
/// A try-block is necessary because fs::remove() can throw. The
/// postcondition is asserted explicitly at the end of the try-block
/// because the fs::remove() documentation is still unclear:
///   SOMEDAY !! Is this true of std::filesystem as it was of boost?
/// apparently it mustn't fail without throwing, yet it doesn't throw
/// on an operation that must fail, like removing a file that's locked
/// by another process as in the motivating example above.
///
/// For *nix, the catch-clause is not normally expected to be reached,
/// and the alternative filename it devises might work no better than
/// the original. However, it doesn't hurt to try it, so there's no
/// reason to restrict it to msw.

fs::path unique_filepath
    (fs::path    const& original_filepath
    ,std::string const& supplied_extension
    )
{
    fs::path filepath(original_filepath);
    filepath.replace_extension(supplied_extension);
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
        std::string basename  = filepath.stem().string();
        std::string const extension = filepath.extension().string();
        basename += '-' + iso_8601_datestamp_terse() + extension;
        filepath = filepath.parent_path() / basename;
        if(fs::exists(filepath))
            {
            alarum()
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

namespace
{
/// Throw if 'a_path' does not name a valid path.
///
/// "Valid" means the path is well formed, not an empty string, and
/// names an existing file or directory.
///
/// 'a_path': path to be validated.
///
/// 'context': semantic description of the path's usage,
/// to be shown in the exception report.
///
/// Although a std::invalid_argument exception might at first seem
/// appropriate here, std::runtime_error (via alarum()) is chosen
/// because the 'a_path' argument may be specified by users.
///
/// Design alternative: instead of calling this function from
/// validate_directory() and validate_filepath(), eliminate those
/// functions and call this directly with an 'is_directory' argument.
/// This alternative would make it easy to add other requirements such
/// as 'is_readable' or 'is_writable'.

void validate_path
    (std::string const& a_path
    ,std::string const& context
    )
{
    fs::path const path{a_path};

    // SOMEDAY !! This is where well-formedness with respect to OS
    // rules ought to be tested, if that is indeed desirable.

    if(path.empty())
        {
        alarum() << context << " must not be empty." << LMI_FLUSH;
        }
    if(!fs::exists(path))
        {
        alarum() << context << " '" << path << "' not found." << LMI_FLUSH;
        }
}
} // Unnamed namespace.

/// Throw if 'directory' does not name a valid directory.
///
/// 'directory': directory-name to be validated.
///
/// Implemented in terms of validate_path(), q.v.

void validate_directory
    (std::string const& directory
    ,std::string const& context
    )
{
    validate_path(directory, context);
    if(!fs::is_directory(directory))
        {
        alarum()
            << context
            << " '"
            << directory
            << "' is not a directory."
            << LMI_FLUSH
            ;
        }
}

/// Throw if 'filepath' does not name a valid filepath.
///
/// 'filepath': filepath to be validated.
///
/// Implemented in terms of validate_path(), q.v.

void validate_filepath
    (std::string const& filepath
    ,std::string const& context
    )
{
    validate_path(filepath, context);
    if(fs::is_directory(filepath))
        {
        alarum()
            << context
            << " '"
            << filepath
            << "' is a directory."
            << LMI_FLUSH
            ;
        }
}
