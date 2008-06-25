// Test files for consistency with various rules.
//
// Copyright (C) 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: test_coding_rules.cpp,v 1.77 2008-06-25 23:51:30 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "assert_lmi.hpp"
#include "handle_exceptions.hpp"
#include "istream_to_string.hpp"
#include "main_common.hpp"
#include "miscellany.hpp" // lmi_array_size()
#include "obstruct_slicing.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>
#include <boost/utility.hpp>

#include <cstddef>        // std::size_t
#include <ctime>
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

std::map<std::string, bool> my_taboos();

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

enum enum_phylum
    {e_no_phylum  = 0
    ,e_binary     = 1 <<  0
    ,e_c_header   = 1 <<  1
    ,e_c_source   = 1 <<  2
    ,e_cxx_header = 1 <<  3
    ,e_cxx_source = 1 <<  4
    ,e_ephemeral  = 1 <<  5
    ,e_expungible = 1 <<  6
    ,e_gpl        = 1 <<  7
    ,e_log        = 1 <<  8
    ,e_make       = 1 <<  9
    ,e_md5        = 1 << 10
    ,e_patch      = 1 << 11
    ,e_script     = 1 << 12
    ,e_synopsis   = 1 << 13
    ,e_touchstone = 1 << 14
    ,e_xml_input  = 1 << 15
    ,e_xml_other  = 1 << 16
    ,e_xpm        = 1 << 17
    };

enum enum_kingdom
    {e_c          = e_c_header   | e_c_source
    ,e_cxx        = e_cxx_header | e_cxx_source
    ,e_header     = e_c_header   | e_cxx_header
    ,e_c_or_cxx   = e_c          | e_cxx
    };

class file
    :private boost::noncopyable
    ,virtual private obstruct_slicing<file>
{
  public:
    explicit file(std::string const& file_path);
    ~file() {}

    bool is_of_phylum(enum_phylum ) const;
    bool is_of_phylum(enum_kingdom) const;
    bool phyloanalyze(std::string const&) const;

    fs::path    const& path     () const {return path_;     }
    std::string const& full_name() const {return full_name_;}
    std::string const& leaf_name() const {return leaf_name_;}
    std::string const& extension() const {return extension_;}
    enum_phylum        phylum   () const {return phylum_;   }
    std::string const& data     () const {return data_;     }

  private:
    fs::path    path_;
    std::string full_name_;
    std::string leaf_name_;
    std::string extension_;
    enum_phylum phylum_;
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
///
/// Phylum 'e_ephemeral' is used for this program's unit test, so
/// assign files to that phylum last, and only if they fit no other.

file::file(std::string const& file_path)
    :path_     (file_path)
    ,full_name_(file_path)
    ,leaf_name_(path_.leaf())
    ,extension_(fs::extension(path_))
    ,phylum_   (e_no_phylum)
{
    if(!fs::exists(path_))
        {
        throw std::runtime_error("File not found.");
        }

    if(fs::is_directory(path_))
        {
        return;
        }

    fs::ifstream ifs(path_, std::ios_base::binary);
    istream_to_string(ifs, data_);
    if(!ifs)
        {
        throw std::runtime_error("Failure in file input stream.");
        }

    phylum_ =
          ".ico"        == extension() ? e_binary
        : ".png"        == extension() ? e_binary
        : ".h"          == extension() ? e_c_header
        : ".c"          == extension() ? e_c_source
        : ".hpp"        == extension() ? e_cxx_header
        : ".cpp"        == extension() ? e_cxx_source
        : ".tpp"        == extension() ? e_cxx_source
        : ".xpp"        == extension() ? e_cxx_source
        : ".bak"        == extension() ? e_expungible
        : ".make"       == extension() ? e_make
        : ".md5sums"    == extension() ? e_md5
        : ".patch"      == extension() ? e_patch
        : ".ac"         == extension() ? e_script
        : ".hhp"        == extension() ? e_script
        : ".rc"         == extension() ? e_script
        : ".sed"        == extension() ? e_script
        : ".sh"         == extension() ? e_script
        : ".touchstone" == extension() ? e_touchstone
        : ".cns"        == extension() ? e_xml_input
        : ".ill"        == extension() ? e_xml_input
        : ".hhc"        == extension() ? e_xml_other
        : ".html"       == extension() ? e_xml_other
        : ".xml"        == extension() ? e_xml_other
        : ".xrc"        == extension() ? e_xml_other
        : ".xsd"        == extension() ? e_xml_other
        : ".xsl"        == extension() ? e_xml_other
        : ".xpm"        == extension() ? e_xpm
        : phyloanalyze("^COPYING$")    ? e_gpl
        : phyloanalyze("^quoted_gpl")  ? e_gpl
        : phyloanalyze("Log$")         ? e_log
        : phyloanalyze("GNUmakefile$") ? e_make
        : phyloanalyze("^Makefile")    ? e_make
        : phyloanalyze("^INSTALL$")    ? e_synopsis
        : phyloanalyze("^README")      ? e_synopsis
        : phyloanalyze("^eraseme")     ? e_ephemeral
        : throw std::runtime_error("File is unexpectedly uncategorizable.")
        ;

    if(is_of_phylum(e_binary) || is_of_phylum(e_expungible))
        {
        return;
        }

    data_ = '\n' + data();
    if('\n' != data().at(data().size() - 1))
        {
        throw std::runtime_error("File does not end in '\\n'.");
        }
}

/// Ascertain whether a file appertains to the given category.

bool file::is_of_phylum(enum_phylum z) const
{
    return z & phylum();
}

/// Ascertain whether a file appertains to the given category.
///
/// This relation may be read as "has the X-nature". For example,
/// 'foo.h' has the header-nature as well as the C-nature.

bool file::is_of_phylum(enum_kingdom z) const
{
    return z & phylum();
}

/// Analyze a file's name to determine its phylum.

bool file::phyloanalyze(std::string const& s) const
{
    return boost::regex_search(leaf_name(), boost::regex(s));
}

void complain(file const& f, std::string const& complaint)
{
    std::cout << "File '" << f.full_name() << "' " << complaint << std::endl;
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

void taboo
    (file const&             f
    ,std::string const&      regex
    ,boost::regex::flag_type flags = boost::regex::ECMAScript
    )
{
    boost::regex::flag_type syntax = flags | boost::regex::ECMAScript;
    if(boost::regex_search(f.data(), boost::regex(regex, syntax)))
        {
        std::ostringstream oss;
        oss << "breaks taboo '" << regex << "'.";
        complain(f, oss.str());
        }
}

/// Validate whitespace.
///
/// Throw if the file contains '\f', '\r', '\t', or '\v', except in
/// certain narrow circumstances. Relying on this precondition,
/// regexen downstream can more readably denote space by " " instead
/// of by "\\s" or "[:space:]".
///
/// Diagnose various other whitespace defects without throwing.

void assay_whitespace(file const& f)
{
    if
        (   std::string::npos != f.data().find('\r')
        ||  std::string::npos != f.data().find('\v')
        )
        {
        throw std::runtime_error("File contains '\\r' or '\\v'.");
        }

    if
        (   !f.is_of_phylum(e_gpl)
        &&  !f.is_of_phylum(e_touchstone)
        &&  std::string::npos != f.data().find('\f')
        )
        {
        throw std::runtime_error("File contains '\\f'.");
        }

    if
        (   !f.is_of_phylum(e_gpl)
        &&  !f.is_of_phylum(e_make)
        &&  !f.is_of_phylum(e_patch)
        &&  !f.is_of_phylum(e_xpm)
        &&  std::string::npos != f.data().find('\t')
        )
        {
        throw std::runtime_error("File contains '\\t'.");
        }

    static boost::regex const postinitial_tab("[^\\n]\\t");
    if(f.is_of_phylum(e_make) && boost::regex_search(f.data(), postinitial_tab))
        {
        throw std::runtime_error("File contains postinitial '\\t'.");
        }

    if
        (   !f.is_of_phylum(e_gpl)
        &&  !f.is_of_phylum(e_touchstone)
        &&  std::string::npos != f.data().find("\n\n\n")
        )
        {
        complain(f, "contains '\\n\\n\\n'.");
        }

    if
        (   !f.is_of_phylum(e_patch)
        &&  std::string::npos != f.data().find(" \n")
        )
        {
        complain(f, "contains ' \\n'.");
        }
}

/// 'config.hpp' must be included exactly when and as required.
///
/// Except as noted below, it must be included in every header, but in
/// no other file. Where required, the include directive must take the
/// canonical form '#include "unutterable"' (its name can't be uttered
/// here because this file is not a header) on a line by itself, and
/// must precede all other include directives.
///
/// Exceptions are necessarily made for
///  - this program's test script;
///  - 'GNUmakefile' and log files; and
///  - 'config.hpp' and its related 'config_*.hpp' headers.

void check_config_hpp(file const& f)
{
    static std::string const loose ("# *include *[<\"]config.hpp[>\"]");
    static std::string const strict("\\n(#include \"config.hpp\")\\n");

    if
        (   f.is_of_phylum(e_log)
        ||  f.phyloanalyze("^test_coding_rules_test.sh$")
        ||  f.phyloanalyze("^GNUmakefile$")
        )
        {
        return;
        }
    else if(f.is_of_phylum(e_header) && !f.phyloanalyze("^config(_.*)?\\.hpp$"))
        {
        require(f, loose , "must include 'config.hpp'.");
        require(f, strict, "lacks line '#include \"config.hpp\"'.");
        boost::smatch match;
        static boost::regex const first_include("(# *include[^\\n]*)");
        boost::regex_search(f.data(), match, first_include);
        if("#include \"config.hpp\"" != match[1])
            {
            complain(f, "must include 'config.hpp' first.");
            }
        }
    else
        {
        forbid(f, loose, "must not include 'config.hpp'.");
        }
}

// SOMEDAY !! This test could be liberalized to permit copyright
// notices to span multiple lines. For now, it is assumed that the
// year appears on the same line as the word "Copyright".

// SOMEDAY !! Move the 'GNUmakefile' logic to exclude certain other
// files hither.

void check_copyright(file const& f)
{
    if
        (   f.is_of_phylum(e_gpl)
        ||  f.is_of_phylum(e_md5)
        ||  f.is_of_phylum(e_patch)
        ||  f.is_of_phylum(e_touchstone)
        ||  f.is_of_phylum(e_xml_input)
        ||  f.is_of_phylum(e_xpm)
        )
        {
        return;
        }

    std::time_t const t0 = fs::last_write_time(f.path());
    std::tm const*const t1 = std::gmtime(&t0);
    LMI_ASSERT(NULL != t1);
    std::ostringstream oss;
    oss << "Copyright \\(C\\)[^\\n]*" << 1900 + t1->tm_year;
    require(f, oss.str(), "lacks current copyright.");
}

void check_cxx(file const& f)
{
    // Remove this once these hopeless files have been expunged.
    if(f.phyloanalyze("^ihs_f?pios.[ch]pp$"))
        {
        return;
        }

    // Remove this once these files have been rewritten.
    if(f.phyloanalyze("^md5.[ch]pp$"))
        {
        return;
        }

    if(!f.is_of_phylum(e_c_or_cxx))
        {
        return;
        }

    {
    static boost::regex const r("(\\w+)( +)([*&])(\\w+\\b)([*;]?)([^\\n]*)");
    boost::sregex_iterator i(f.data().begin(), f.data().end(), r);
    boost::sregex_iterator const omega;
    for(; i != omega; ++i)
        {
        boost::smatch const& z(*i);
        if
            (   "return"    != z[1]           // 'return *p'
            &&  "nix"       != z[4]           // '*nix'
            &&  !('*' == z[3] && '*' == z[5]) // '*emphasis*' in comment
            &&  !('&' == z[3] && ';' == z[5]) // '&nbsp;'
            )
            {
            std::ostringstream oss;
            oss << "should fuse '" << z[3] << "' with type: '" << z[0] << "'.";
            complain(f, oss.str());
            }
        }
    }

    {
    static boost::regex const r("\\bconst +([A-Za-z][A-Za-z0-9_:]*) *[*&]");
    boost::sregex_iterator i(f.data().begin(), f.data().end(), r);
    boost::sregex_iterator const omega;
    for(; i != omega; ++i)
        {
        boost::smatch const& z(*i);
        if
            (   "volatile"  != z[1]           // 'const volatile'
            )
            {
            std::ostringstream oss;
            oss
                << "should write 'const' after the type it modifies: '"
                << z[0]
                << "'."
                ;
            complain(f, oss.str());
            }
        }
    }
}

/// Check defect markers, which contain a doubled '!' or '?'.
///
/// SOMEDAY !! Replace doubled '!' with something else. In retrospect,
/// doubled '!' was a poor choice, because '!!' is a legitimate idiom
/// for conversion to bool. Perhaps tripling both '!' and '?' would be
/// best.

void check_defect_markers(file const& f)
{
    if(f.is_of_phylum(e_xpm) || f.phyloanalyze("^test_coding_rules_test.sh$"))
        {
        return;
        }

    {
    static boost::regex const r("(\\b\\w+\\b\\W*)\\?\\?(.)");
    boost::sregex_iterator i(f.data().begin(), f.data().end(), r);
    boost::sregex_iterator const omega;
    for(; i != omega; ++i)
        {
        boost::smatch const& z(*i);
        bool const error_preceding = "TODO " != z[1];
        bool const error_following = " " != z[2] && "\n" != z[2];
        if(error_preceding || error_following)
            {
            std::ostringstream oss;
            oss << "has irregular defect marker '" << z[0] << "'.";
            complain(f, oss.str());
            }
        }
    }

    {
    static boost::regex const r("(\\b\\w+\\b\\W?)!!(.)");
    boost::sregex_iterator i(f.data().begin(), f.data().end(), r);
    boost::sregex_iterator const omega;
    for(; i != omega; ++i)
        {
        boost::smatch const& z(*i);
        bool const error_preceding =
                true
            &&  "APACHE "      != z[1]
            &&  "BOOST "       != z[1]
            &&  "COMPILER "    != z[1]
            &&  "CYGWIN "      != z[1]
            &&  "DATABASE "    != z[1]
            &&  "ET "          != z[1]
            &&  "EVGENIY "     != z[1]
            &&  "INELEGANT "   != z[1]
            &&  "INPUT "       != z[1]
            &&  "MPATROL "     != z[1]
            &&  "MSYS "        != z[1]
            &&  "PORT "        != z[1]
            &&  "SOMEDAY "     != z[1]
            &&  "THIRD_PARTY " != z[1]
            &&  "TRICKY "      != z[1]
            &&  "USER "        != z[1]
            &&  "WX "          != z[1]
            &&  "XMLWRAPP "    != z[1]
            ;
        bool const error_following = " " != z[2] && "\n" != z[2];
        if(error_preceding || error_following)
            {
            std::ostringstream oss;
            oss << "has irregular defect marker '" << z[0] << "'.";
            complain(f, oss.str());
            }
        }
    }
}

void check_include_guards(file const& f)
{
    if(!f.is_of_phylum(e_cxx_header))
        {
        return;
        }

    std::string const guard = boost::regex_replace
        (f.leaf_name()
        ,boost::regex("\\.hpp$")
        ,"_hpp"
        );
    std::string const guards =
            "\\n#ifndef "   + guard
        +   "\\n#define "   + guard + "\\n"
        +   ".*"
        +   "\\n#endif // " + guard + "\\n+$"
        ;
    require(f, guards, "lacks canonical header guards.");
}

void check_label_indentation(file const& f)
{
    // Remove this once these hopeless files have been expunged.
    if(f.phyloanalyze("^ihs_f?pios.hpp$"))
        {
        return;
        }

    if(!f.is_of_phylum(e_c_or_cxx))
        {
        return;
        }

    static boost::regex const r("\\n( *)([A-Za-z][A-Za-z0-9_]*)( *:)(?!:)");
    boost::sregex_iterator i(f.data().begin(), f.data().end(), r);
    boost::sregex_iterator const omega;
    for(; i != omega; ++i)
        {
        boost::smatch const& z(*i);
        if
            (   "default" != z[2]
            &&  "  "      != z[1]
            &&  "      "  != z[1]
            )
            {
            std::ostringstream oss;
            oss << "has misindented label '" << z[1] << z[2] << z[3] << "'.";
            complain(f, oss.str());
            }
        }
}

/// Forbid long lines in logs, which are often quoted in email.
///
/// Tolerate certain lines that are historically slightly over the
/// limit in the preamble, which in lmi logs is separated from actual
/// log entries by the word "MAINTENANCE" on a line by itself.
///
/// Tolerate lines quoted with an initial '|' (so that 'DefectLog' can
/// include code snippets), and http URLs optionally indented with
/// blank spaces.

void check_logs(file const& f)
{
    if(!f.is_of_phylum(e_log))
        {
        return;
        }

    std::string entries = f.data();
    entries.erase(0, entries.find("\nMAINTENANCE\n"));
    if(entries.empty())
        {
        complain(f, "lacks expected 'MAINTENANCE' line.");
        entries = f.data();
        }

    static boost::regex const r("\\n(?!\\|)(?! *http:)([^\\n]{71,})(?=\\n)");
    boost::sregex_iterator i(entries.begin(), entries.end(), r);
    boost::sregex_iterator const omega;
    if(omega == i)
        {
        return;
        }

    std::ostringstream oss;
    oss
        << "violates seventy-character limit:\n"
        << "0000000001111111111222222222233333333334444444444555555555566666666667\n"
        << "1234567890123456789012345678901234567890123456789012345678901234567890"
        ;
    for(; i != omega; ++i)
        {
        boost::smatch const& z(*i);
        oss << '\n' << z[1];
        }
    complain(f, oss.str());
}

/// Check boilerplate at the beginning of each file.
///
/// Strings that would otherwise be subject to RCS keyword
/// substitution are split across multiple lines.

void check_preamble(file const& f)
{
    if
        (   f.is_of_phylum(e_gpl)
        ||  f.is_of_phylum(e_md5)
        ||  f.is_of_phylum(e_patch)
        ||  f.is_of_phylum(e_touchstone)
        ||  f.is_of_phylum(e_xml_input)
        ||  f.is_of_phylum(e_xpm)
        )
        {
        return;
        }

    static std::string const url("http://savannah.nongnu.org/projects/lmi");
    require(f, url, "lacks lmi URL.");

    static std::string const good_rcs_id =
        "\\$"
        "Id(:[^\\n]*)*\\$"
        ;
    require(f, good_rcs_id, "lacks a well-formed RCS Id.");

    static std::string const pleonastic_rcs_id =
        "\\$"
        "Id"
        ".*"
        "\\$"
        "Id"
        ;
    forbid(f, pleonastic_rcs_id, "contains more than one RCS Id.");

    static std::string const bad_rcs_id =
        "\\$"
        "Id[^\\$\\n]*\\n"
        ;
    forbid(f, bad_rcs_id, "contains a malformed RCS Id.");
}

/// Deem a reserved name permissible or not.

bool check_reserved_name_exception(std::string const& s)
{
    static char const*const y[] =
    // Taboo, and therefore uglified here.
        {"D""__""W""IN32""__"
        ,"_""W""IN32"
        ,"__""W""IN32""__"
    // Standard (including TR1).
        ,"_1"
        ,"_2"
        ,"_IOFBF"
        ,"_IOLBF"
        ,"_IONBF"
        ,"__FILE__"
        ,"__LINE__"
        ,"__STDC_IEC_559__"
        ,"__STDC__"
        ,"__cplusplus"
    // Platform identification.
        ,"_M_IX86"
        ,"_M_X64"
        ,"_X86_"
        ,"__X__"
        ,"__amd64"
        ,"__amd64__"
        ,"__i386"
        ,"__unix"
        ,"__unix__"
        ,"__x86_64"
        ,"__x86_64__"
        ,"_mingw"
    // Platform specific.
        ,"__declspec"
        ,"__int64"
        ,"__stdcall"
        ,"_control87"
        ,"_snprintf"
        ,"_vsnprintf"
        ,"_wcsdup"
    // Compiler specific: gcc.
        ,"__GLIBCPP__"
        ,"__GNUC_MINOR__"
        ,"__GNUC_PATCHLEVEL__"
        ,"__GNUC__"
        ,"__GNUG__"
        ,"__STRICT_ANSI__"
        ,"__asm__"
        ,"__attribute__"
        ,"__cxa_demangle"
    // Compiler specific: gcc, Cygwin.
        ,"__CYGWIN__"
    // Compiler specific: gcc, MinGW.
        ,"_CRT_fmode"
        ,"__MINGW32_MAJOR_VERSION"
        ,"__MINGW32_MINOR_VERSION"
        ,"__MINGW32_VERSION"
        ,"__MINGW32__"
        ,"__MINGW_H"
        ,"_fmode"
    // Compiler specific: glibc.
        ,"_LIBC"
        ,"__BIG_ENDIAN"
        ,"__BYTE_ORDER"
    // Compiler specific: como.
        ,"__COMO__"
    // Compiler specific: borland.
        ,"_CatcherPTR"
        ,"__BORLANDC__"
        ,"__FLAT__"
        ,"__emit__"
        ,"_max_dble"
        ,"_streams"
    // Compiler specific: ms.
        ,"_MCW_EM"
        ,"_MCW_IC"
        ,"_MCW_PC"
        ,"_MCW_RC"
        ,"_MSC_VER"
        ,"_O_APPEND"
        ,"_O_BINARY"
        ,"_O_CREAT"
        ,"_O_EXCL"
        ,"_O_RDONLY"
        ,"_O_TRUNC"
        ,"_O_WRONLY"
        ,"_PC_64"
        ,"_RC_NEAR"
        ,"_fileno"
        ,"_setmode"
    // Library specific.
        ,"D__WXDEBUG__" // Hapax legomenon.
        ,"__WXGTK__"
        ,"__WXMSW__"
        };
    static int const n = lmi_array_size(y);
    static std::set<std::string> const z(y, y + n);
    return z.end() != z.find(s);
}

/// Check names reserved by C++2003 [17.4.3.1.2].
///
/// A name that could be reserved in any namespace is preferably
/// avoided in every namespace: simple style rules are better.
///
/// The regex iterated for is deliberately overbroad. Measurement
/// shows that it is far more efficient to cast the net widely and
/// then filter the matches: there's a lot more sea than fish.
///
/// TODO ?? Also test '_[A-Za-z0-9]', e.g. thus:
///   "(\\b\\w*__\\w*\\b)|(\\b\\_\\w+\\b)"

void check_reserved_names(file const& f)
{
    // Remove this once these hopeless files have been expunged.
    if
        (   f.phyloanalyze("^ledger_formats.xml$")
        ||  f.phyloanalyze("^ledger_xml_io.cpp$")
        )
        {
        return;
        }

    if(f.is_of_phylum(e_log))
        {
        return;
        }

    static boost::regex const r("(\\b\\w*__\\w*\\b)");
    boost::sregex_iterator i(f.data().begin(), f.data().end(), r);
    boost::sregex_iterator const omega;
    for(; i != omega; ++i)
        {
        boost::smatch const& z(*i);
        std::string const s = z[0];
        static boost::regex const not_all_underscore("[A-Za-z0-9]");
        if
            (   !check_reserved_name_exception(s)
            &&  boost::regex_search(s, not_all_underscore)
            )
            {
            std::ostringstream oss;
            oss << "contains reserved name '" << s << "'.";
            complain(f, oss.str());
            }
        }
}

void check_xpm(file const& f)
{
    if(!f.is_of_phylum(e_xpm))
        {
        return;
        }

    std::string const name = boost::regex_replace
        (f.leaf_name()
        ,boost::regex("[.-]")
        ,"_"
        );
    std::string const z = "static char const\\* " + name + "\\[\\] = \\{";
    require(f, z, "lacks proper variable assignment.");
}

void enforce_taboos(file const& f)
{
    if(f.phyloanalyze("test_coding_rules"))
        {
        return;
        }

    // ASCII copyright symbol requires upper-case 'C'.
    taboo(f, "\\(c\\) *[0-9]");
    // Former addresses of the Free Software Foundation.
    taboo(f, "Cambridge");
    taboo(f, "Temple");
    // Patented.
    taboo(f, "\\.gif", boost::regex::icase);
    // Obsolete email address.
    taboo(f, "chicares@mindspring.com");
    // Certain proprietary libraries.
    taboo(f, "\\bowl\\b", boost::regex::icase);
    taboo(f, "vtss", boost::regex::icase);
    // Suspiciously specific to msw.
    taboo(f, "Microsoft");
    taboo(f, "Visual [A-Z]");
    taboo(f, "\\bWIN\\b");
    taboo(f, "\\bBAT\\b", boost::regex::icase);
    taboo(f, "\\bExcel\\b");
    taboo(f, "xls|xl4", boost::regex::icase);
    // Insinuated by certain msw tools.
    taboo(f, "Microsoft Word");
    taboo(f, "Stylus Studio");
    taboo(f, "Sonic Software");
    taboo(f, "windows-1252");
    taboo(f, "Arial");

    if
        (   !f.is_of_phylum(e_log)
        &&  !f.is_of_phylum(e_make)
        )
        {
        taboo(f, "\\bexe\\b", boost::regex::icase);
        }

    if
        (   !f.is_of_phylum(e_make)
        &&  !f.is_of_phylum(e_patch)
        &&  !f.phyloanalyze("config.hpp")
        )
        {
        taboo(f, "WIN32", boost::regex::icase);
        }

    // Unspeakable private taboos.
    std::map<std::string, bool> const z = my_taboos();
    typedef std::map<std::string, bool>::const_iterator mci;
    for(mci i = z.begin(); i != z.end(); ++i)
        {
        boost::regex::flag_type syntax =
            i->second
            ? boost::regex::ECMAScript | boost::regex::icase
            : boost::regex::ECMAScript
            ;
        taboo(f, i->first, syntax);
        }
}

/// Implicitly-declared special member functions do the right thing.

class statistics
{
  public:
    statistics() : files_(0), lines_(0), defects_(0) {}
    ~statistics() {}

    statistics& operator+=(statistics const&);

    static statistics analyze_file(file const&);

    void print_summary() const;

  private:
    std::size_t files_;
    std::size_t lines_;
    std::size_t defects_;
};

statistics& statistics::operator+=(statistics const& z)
{
    files_   += z.files_  ;
    lines_   += z.lines_  ;
    defects_ += z.defects_;

    return *this;
}

/// Calculate summary statistics.
///
/// The loop counter starts at one, not zero, to disregard the leading
/// '\n' sentry.

statistics statistics::analyze_file(file const& f)
{
    statistics z;
    if
        (   f.is_of_phylum(e_binary)
        ||  f.is_of_phylum(e_expungible)
        ||  f.is_of_phylum(e_gpl)
        ||  f.is_of_phylum(e_log)
        ||  f.is_of_phylum(e_md5)
        ||  f.is_of_phylum(e_patch)
        ||  f.is_of_phylum(e_touchstone)
        ||  f.is_of_phylum(e_xml_input)
        ||  f.is_of_phylum(e_xpm)
        ||  f.phyloanalyze("^INSTALL")
        ||  f.phyloanalyze("^README")
        )
        {
        return z;
        }

    ++z.files_;

    std::string const& s = f.data();
    for(std::string::size_type i = 1; i < s.size(); ++i)
        {
        if('\n' == s[i])
            {
            ++z.lines_;
            }
        if('?' == s[i - 1] && '?' == s[i])
            {
            ++z.defects_;
            }
        }

    return z;
}

void statistics::print_summary() const
{
    std::cout
        << std::setw(9) << files_   << " source files\n"
        << std::setw(9) << lines_   << " source lines\n"
        << std::setw(9) << defects_ << " marked defects\n"
        << std::flush
        ;
}

statistics process_file(std::string const& file_path)
{
    file f(file_path);
    if(f.is_of_phylum(e_expungible))
        {
        complain(f, "ignored as being expungible.");
        return statistics();
        }
    if(f.is_of_phylum(e_binary) || fs::is_directory(f.path()))
        {
        return statistics();
        }

    if(31 < f.leaf_name().size())
        {
        complain(f, "exceeds 31-character file-name limit.");
        }

    assay_whitespace        (f);

    check_config_hpp        (f);
    check_copyright         (f);
    check_cxx               (f);
    check_defect_markers    (f);
    check_include_guards    (f);
    check_label_indentation (f);
    check_logs              (f);
    check_preamble          (f);
    check_reserved_names    (f);
    check_xpm               (f);

    enforce_taboos          (f);

    return statistics::analyze_file(f);
}

int try_main(int argc, char* argv[])
{
    bool error_flag = false;
    statistics z;
    for(int j = 1; j < argc; ++j)
        {
        try
            {
            z += process_file(argv[j]);
            }
        catch(...)
            {
            error_flag = true;
            std::cerr << "Exception--file '" << argv[j] << "': " << std::flush;
            report_exception();
            }
        }
    z.print_summary();
    return error_flag ? EXIT_FAILURE : EXIT_SUCCESS;
}

