// Path utilities--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "miscellany.hpp"
#include "path.hpp"
#include "platform_dependent.hpp"       // access()
#include "test_tools.hpp"
#include "wine_workarounds.hpp"         // running_under_wine()

#include <cstdio>                       // remove()
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
/// To make sure a file exists, it's necessary to write something to
/// it: C99 7.19.3/4, last sentence.

    void write_dummy_file(char const* s)
    {
        std::ofstream ofs(s);
        ofs << s;
    }

    void write_dummy_file(fs::path p)
    {
        fs::ofstream ofs(p);
        ofs << p;
    }
} // Unnamed namespace.

void test_modify_directory()
{
    // Motivating case:
    LMI_TEST_EQUAL("/usr/bin/sh"  , modify_directory("/bin/sh", "/usr/bin").string());

    // First argument: "/bin/sh"

    fs::path old_path ("/bin/sh");
    fs::path newdir   ("/usr/bin");
    fs::path expected ("/usr/bin/sh");

    fs::path f = fs::absolute(old_path);
    fs::path d = fs::absolute(newdir);
    fs::path x = fs::absolute(expected);

    // this result is expected...
    LMI_TEST_EQUAL(x, modify_directory(f, d).string());
    // ...but these are surprising:
    LMI_TEST_EQUAL(f, (d / f).string());
    LMI_TEST_EQUAL(old_path, (newdir / old_path).string());

    // First argument: just "sh"

    fs::path const file("sh");
    fs::path const dir0("/bin");
    fs::path const dir1("/usr/bin/");

    LMI_TEST_EQUAL("/bin/sh"      , modify_directory(file     , dir0      ).string());
    LMI_TEST_EQUAL("/bin/sh"      , modify_directory("sh"     , "/bin/"   ).string());
    LMI_TEST_EQUAL("/usr/bin/wish", modify_directory("wish"   , dir1      ).string());
    LMI_TEST_EQUAL("/usr/bin/wish", modify_directory("wish"   , "/usr/bin").string());

    // An empty directory may be supplied.
    LMI_TEST_EQUAL("sh"           , modify_directory("sh"     , ""        ).string());
    LMI_TEST_EQUAL("sh"           , modify_directory("/bin/sh", ""        ).string());

    // A dirname can have a trailing slash, but a basename cannot:
    //   $ls /bin/sh/
    //   ls: cannot access '/bin/sh/': Not a directory
    //
    // The original boost implementation nevertheless allowed this:
    //   modify_directory("sh/", "/bin/")
    // even though 'has_leaf()' was asserted for its first argument,
    // because its path ctor silently discarded the trailing slash.
    //
    // Appropriately, std::filesystem doesn't discard a trailing '/':
    LMI_TEST_EQUAL('/', fs::path("sh/").string().back());
    // so 'has_filename()' returns false:
    LMI_TEST(!fs::path("sh/").has_filename());
    // and the assertion fires as intended:
    LMI_TEST_THROW
        (modify_directory("sh/", "/bin/")
        ,std::runtime_error
        ,"Assertion 'original_filepath.has_filename()' failed."
        );

    LMI_TEST_THROW
        (modify_directory("", "/bin")
        ,std::runtime_error
        ,"Assertion 'original_filepath.has_filename()' failed."
        );
}

void test_portable_filename()
{
    LMI_TEST_THROW
        (portable_filename("")
        ,std::runtime_error
        ,"Assertion '!original_filename.empty()' failed."
        );

    LMI_TEST_EQUAL("Z"     , portable_filename("Z"));
    LMI_TEST_EQUAL("_"     , portable_filename("."));
    LMI_TEST_EQUAL("_"     , portable_filename("#"));

    LMI_TEST_EQUAL("AZ"    , portable_filename("AZ"));
    LMI_TEST_EQUAL("A_"    , portable_filename("A."));
    LMI_TEST_EQUAL("_Z"    , portable_filename(".Z"));
    LMI_TEST_EQUAL("__"    , portable_filename(".."));
    LMI_TEST_EQUAL("__"    , portable_filename("##"));

    LMI_TEST_EQUAL("A.Z"   , portable_filename("A.Z"));
    LMI_TEST_EQUAL("A-Z"   , portable_filename("A-Z"));

    LMI_TEST_EQUAL("_xyz_" , portable_filename(".xyz."));
    LMI_TEST_EQUAL("_xyz_" , portable_filename("-xyz-"));

    LMI_TEST_EQUAL("and_or", portable_filename("and/or"));

    LMI_TEST_EQUAL
        (                  "Crime_and_or_Punishment.text"
        ,portable_filename("Crime and/or Punishment.text")
        );

    LMI_TEST_EQUAL
        (                  "_Fyodor_Dostoyevskiy_Crime_and_Punishment.text"
        ,portable_filename("/Fyodor Dostoyevskiy/Crime and Punishment.text")
        );
}

void test_serial_file_path()
{
    LMI_TEST_EQUAL
        (                         "Abolitionists.John_Brown.018591203.text"
        ,serial_file_path("Abolitionists.cns", "John Brown", 18591202, "text").string()
        );

    // Serial number is zero-padded to nine positions, but can use more.
    LMI_TEST_EQUAL
        (                            "x.000012346.y"
        ,serial_file_path("x.cns", "",      12345, "y").string()
        );
    LMI_TEST_EQUAL
        (                            "x.123456790.y"
        ,serial_file_path("x.cns", "",  123456789, "y").string()
        );
    LMI_TEST_EQUAL
        (                           "x.1234567891.y"
        ,serial_file_path("x.cns", "", 1234567890, "y").string()
        );

    // Input census filepath needn't have any extension;
    // any extension it has is discarded.
    LMI_TEST_EQUAL
        (serial_file_path("x.ignored", "",      12345, "y").string()
        ,serial_file_path("x"        , "",      12345, "y").string()
        );
    LMI_TEST_EQUAL
        (                                "x.000012346.y"
        ,serial_file_path("x.ignored", "",      12345, "y").string()
        );

    // Discard path from input census filepath; use its basename only.
    LMI_TEST_EQUAL
        (serial_file_path("/path/to/x", "",      12345, "y").string()
        ,serial_file_path("x"         , "",      12345, "y").string()
        );
    LMI_TEST_EQUAL
        (                                 "x.000012346.y"
        ,serial_file_path("/path/to/x", "",      12345, "y").string()
        );

    // Make both census and personal names portable.
    LMI_TEST_EQUAL
        (                         "My_Employer.My_Name.text.1000000000.tsv"
        ,serial_file_path("My Employer.cns", "My Name.text", 999999999, "tsv").string()
        );
    LMI_TEST_EQUAL
        (                 "Fyodor_Dostoyevskiy.Crime_and_Punishment.text.000001867.xyz"
        ,serial_file_path("Fyodor Dostoyevskiy", "Crime and Punishment.text", 1866, "xyz").string()
        );
}

void test_unique_filepath_with_normal_filenames()
{
    // Although it's okay for this unit test, using unique_filepath()
    // as a substitute for the nonstandard mkstemp() is a bad idea.
    // See:
    //   https://lists.nongnu.org/archive/html/lmi/2020-08/msg00015.html
    fs::path const u = unique_filepath("/tmp/" + fs::path{__FILE__}.stem().string(), "");
    std::string const tmp = u.string();
    fs::path const tmpdir(fs::absolute(tmp));
    fs::create_directory(tmpdir);

    // These tests would fail if read-only files with the following
    // names already exist.

    std::string const p = (tmp + "/eraseme.0");
    std::string const q = (tmp + "/eraseme.xyzzy");

    // Don't test the return codes here. These files probably don't
    // exist, in which case C99 7.19.4.1 doesn't clearly prescribe
    // the semantics of std::remove().
    std::remove(p.c_str());
    std::remove(q.c_str());

    write_dummy_file(p);
    LMI_TEST_EQUAL(0, access(p.c_str(), R_OK));

    fs::path path0 = unique_filepath(fs::path(p), ".xyzzy");
    LMI_TEST_EQUAL(path0.string(), q);
    write_dummy_file(path0);
    LMI_TEST_EQUAL(0, access(path0.string().c_str(), R_OK));

    fs::path path1 = unique_filepath(fs::path(p), ".xyzzy");
    LMI_TEST_EQUAL(path1.string(), q);
    write_dummy_file(path1);
    LMI_TEST_EQUAL(0, access(path1.string().c_str(), R_OK));

#if defined LMI_MSW
    // This conditional block tests an msw "feature".

    // Open a file for writing, and leave it open, preventing it from
    // being erased and therefore forcing unique_filepath() to use a
    // different name. This behavior isn't guaranteed on toy OS's.
    fs::ofstream keep_open(path1, ios_out_app_binary());

    fs::path path2 = unique_filepath(fs::path(p), ".xyzzy");
    LMI_TEST_UNEQUAL(path2.string(), q);
    write_dummy_file(path2);
    LMI_TEST_EQUAL(0, access(path2.string().c_str(), R_OK));

    // There's no easy way to test that unique_filepath() throws the
    // exception that it should when it's reinvoked more rapidly than
    // its timestamp changes.

    keep_open.close();

    // Verify that the first function call here is redundant
    // (for a hypothetical free function
    //   fs::path alter_extension(fs::path const&, fs::path const&)
    // that doesn't alter its arguments):
    //   path_b = alter_extension(path_a, ext)
    //   path_c = unique_filepath(path_b, ext)
    // and this single function call has the same effect:
    //   path_c = unique_filepath(path_a, ext)
    // notably without reduplicating any part of 'ext' if 'ext'
    // contains a noninitial '.'.
    //
    // The inserted timestamp is unknown, so this postcondition is
    // weakly tested by examining the resulting pathname's length.
    // Even the timestamp's length is implementation dependent, so a
    // change in implementation may be discovered by a failure here.

    std::string const r = (tmp + "/eraseme.abc.def");
    std::string const s = (tmp + "/eraseme.abc-CCYYMMDDTHHMMSSZ.def");
//                   NOT:  tmp +  /eraseme.abc-CCYYMMDDTHHMMSSZ.abc.def

    fs::path path3 = unique_filepath(fs::path(p), ".abc.def");
    LMI_TEST_EQUAL(path3.string(), r);
    write_dummy_file(path3);
    LMI_TEST_EQUAL(0, access(path3.string().c_str(), R_OK));

    keep_open.open(path3, ios_out_app_binary());

    fs::path path4 = unique_filepath(fs::path(p), ".abc.def");
    LMI_TEST(path3.string().size() == std::string(r).size());
    LMI_TEST(path4.string().size() == std::string(s).size());

    keep_open.close();

    // Clean up the files created by this function.

    LMI_TEST(0 == std::remove(path3.string().c_str()));
    LMI_TEST(0 == std::remove(path2.string().c_str()));
#endif // defined LMI_MSW
    LMI_TEST(0 == std::remove(q.c_str()));
    LMI_TEST(0 == std::remove(p.c_str()));

    // Also remove the temporary directory created by this function.
    //
    // These calls to std::remove() fail, at least with 'wine':
//  LMI_TEST(0 == std::remove(tmp.c_str()));
//  LMI_TEST(0 == std::remove(tmpdir.string().c_str()));
    // They return nonzero, and do not remove the directory. (The
    // reason is that the msw C library's remove() function doesn't
    // delete directories; it sets errno to 13, which means EACCESS,
    // and _doserrno to 5, which might mean EIO.) Therefore,
    // instead of std::remove(), use std::filesystem::remove(),
    // whose effect in C++20 (N4861) [fs.op.remove] is that
    //   "the file p is removed as if by POSIX remove()".
    // Therefore, use the filesystem function to remove the directory:
    fs::remove(tmpdir);
    // For std::filesystem::remove(), this is a documented
    // postcondition. It does no harm to validate it here.
    LMI_TEST(0 != access(tmpdir.string().c_str(), R_OK));
}

void test_unique_filepath_with_ludicrous_filenames()
{
    fs::path path0 = unique_filepath(fs::path("eraseme1"), "");
    LMI_TEST_EQUAL(path0.string(), "eraseme1");

    fs::path path1 = unique_filepath(fs::path("eraseme2"), "");
    LMI_TEST_EQUAL(path1.string(), "eraseme2");

    // "" isn't likely to be a valid filename, but as a path, it's
    // valid: the boost library's default path ctor constructed an
    // empty path.

    fs::path path2 = unique_filepath(fs::path(""), "");
    LMI_TEST_EQUAL(path2.string(), "");

    // fs::replace_extension()'s argument is ".[extension]", so ".."
    // represents a '.' extension-delimiter followed by an extension
    // consisting of a single '.'. When fs::replace_extension() is
    // called by unique_filepath() here, adding that extension to ".."
    // yields "....".

    fs::path pathx = {".."};
    LMI_TEST_EQUAL("..", pathx.string());
    LMI_TEST_EQUAL(""  , pathx.extension().string());
    pathx.replace_extension(pathx);
    LMI_TEST_EQUAL("....", pathx.string());

    // Such a pathname is forbidden by msw, yet allowed (although of
    // course discouraged) by posix; but those are semantic rules,
    // which std::filesystem doesn't try to enforce. However, if
    // unique_filepath() decides that such a file exists, then it
    // tries to remove it; and if that fails, then it appends a
    // timestamp to render it unique. That exceptional behavior is
    // observed only with (certain versions of) 'wine'.

    fs::path path3 = unique_filepath(fs::path(".."), "..");

    if(running_under_wine() && "...." != path3.string())
        {
        std::cout << "\n'wine' did something extraordinary" << std::endl;
        }
    else
        {
        LMI_TEST_EQUAL("....", path3.string());
        }
}

void test_path_inserter()
{
    {
    char const* z = "/opt/lmi/test/foo.bar";
    fs::path const p(z);
    std::ostringstream oss;
    oss << p;
    LMI_TEST_EQUAL(z, oss.str());
    }

    // Ensure that operator<<() works with spaces in path.
    {
    char const* z = "/My Opt/lmi/My Tests/My Foo.My Bar";
    fs::path const p(z);
    std::ostringstream oss;
    oss << p;
    LMI_TEST_EQUAL(z, oss.str());
    }
}

void test_path_validation()
{
    std::string context("Unit test file");

    // Create a file and a directory to test.
    //
    // Another test that calls fs::create_directory() uses an absolute
    // path that's uniquified and canonicalized with fs::absolute().
    // This call uses a relative path, with no such safeguards; this
    // being a unit test, it is appropriate to retain some fragility.
    // If one user runs this test, and the directory created here
    // somehow doesn't get deleted, then the test might fail for
    // another user; that's interesting enough to report.

    fs::create_directory("path_utility_test_dir");
    write_dummy_file("path_utility_test_file");

    // All right.
    validate_directory("path_utility_test_dir", context);
    validate_filepath("path_utility_test_file", context);
    validate_filepath("./path_utility_test_file", context);

    // SOMEDAY !! With boost::filesystem, a path was not well-formed
    // if it contained forbidden characters. With std::filesystem,
    // semantic validity is not considered, and any syntactically
    // valid path is well formed. Therefore, shouldn't the lmi
    // 'validate_*' functions test semantic validity? If not, the
    // next three tests are senseless (the first is already suppressed
    // because it seems to indicate an 'alarum' problem).

#if 0
    // Neither posix nor msw allows a path to consist solely of NUL.
    // SOMEDAY !! 'alarum' truncates, treating arguments as ASCIIZ.
    std::string nulls = {'\0', '\0'};
    LMI_TEST_THROW
        (validate_filepath(nulls, context)
        ,std::runtime_error
        ,"Unit test file '' not found."
        );
#endif // 0

    // Neither posix nor msw allows NUL in paths.
    std::string with_nulls = {'x', '\0', 'y', '\0', 'z'};

    // Note that we can't test for the full error message here because it is
    // truncated at the first NUL, due to using std::runtime_error::what(),
    // which returns "char*" string terminated by the first NUL occurring in
    // it, when constructing this message, so just check that it starts with
    // the expected part. SOMEDAY !! Of course, that's just an expedient
    // workaround; the ASCIIZ defect (above) in 'alarum' needs to be fixed.
    LMI_TEST_THROW
        (validate_filepath(with_nulls, context)
        ,std::runtime_error
        ,lmi_test::what_regex("^Unit test file 'x")
        );

    // Posix doesn't forbid these characters, though msw does.
    // But of course the test only made sense for the boost
    // implementation, which treated "<|>" as ill-formed; it's
    // senseless now because std::filesystem::path treats any
    // path as well-formed.
    LMI_TEST_THROW
        (validate_filepath("<|>", context)
        ,std::runtime_error
        ,"Unit test file '<|>' not found."
        );

    // Posix doesn't forbid filenames with more than two consecutive
    // dots; msw-nt forbids them; and, worse, msw-95 allows them and
    // defines them to mean the {grandparent, great grandparent, ...}
    // of a directory.
    //
    // This conditional block of tests is pointless today, but if
    // semantic validity in the OS context is to be enforced by lmi's
    // 'validate_' functions, they will become valuable.
#if defined LMI_POSIX
    LMI_TEST_THROW
        (validate_filepath("...", context)
        ,std::runtime_error
        ,"Unit test file '...' not found."
        );
#elif defined LMI_MSW
    if(!running_under_wine())
        {
        // At least some versions of 'wine' don't throw here.
        LMI_TEST_THROW
            (validate_directory("...", context)
            ,std::runtime_error
            ,"Unit test file '...' not found."
            );
        }
#else  // Unknown platform.
    throw "Unrecognized platform."
#endif // Unknown platform.

    // Not empty.
    LMI_TEST_THROW
        (validate_filepath("", context)
        ,std::runtime_error
        ,"Unit test file must not be empty."
        );

    // Must exist.
    LMI_TEST_THROW
        (validate_filepath("no_such_file", context)
        ,std::runtime_error
        ,"Unit test file 'no_such_file' not found."
        );

    // Must be a directory.
    LMI_TEST_THROW
        (validate_directory("path_utility_test_file", context)
        ,std::runtime_error
        ,"Unit test file 'path_utility_test_file' is not a directory."
        );

    // Must not be a directory.
    LMI_TEST_THROW
        (validate_filepath("path_utility_test_dir", context)
        ,std::runtime_error
        ,"Unit test file 'path_utility_test_dir' is a directory."
        );

    // Remove file and directory created for this test.
    fs::remove("path_utility_test_file");
    fs::remove("path_utility_test_dir");
}

/// Demonstrate a filepath nonportability concern.
///
/// A print directory is specified in 'configurable_settings.xml', and
/// managed by 'preferences_model.cpp'. Using an msw build of lmi to
/// change its value endues it with a 'root-name'. Subsequently using
/// a posix build of lmi does not remove the 'root-name'; instead, it
/// does something bizarre, viz.:
///   fs::absolute(/opt/lmi/data) returns:
///   /opt/lmi/data
/// as expected, but
///   fs::absolute(Z:/opt/lmi/data) bizarrely returns:
///   /opt/lmi/gcc_x86_64-pc-linux-gnu/build/ship/Z:/opt/lmi/data
/// or something like that, depending on the build directory.
/// Use remove_alien_msw_root() to prevent this.
///
/// The effects of loading a stored directory path using an msw build
/// of lmi are also tested, for completeness only, though they're
/// beside the point. The purpose of this function is to demonstrate
/// the anomaly that arises when a posix build loads an msw directory
/// path without removing the msw 'root-name'.

void test_oddities()
{
    std::string const z0 = "/opt/lmi/data";
    std::string const z1 = "Z:/opt/lmi/data";
    std::string const z2 = remove_alien_msw_root(z1).string();
#if defined LMI_POSIX
    LMI_TEST(!fs::path{z1}.has_root_name());
    LMI_TEST_EQUAL  (z0, fs::absolute(z0).string());
    LMI_TEST_UNEQUAL(z0, fs::absolute(z1).string());
    std::cout
        << "\nThis test demonstrates that a bad filename such as:"
        << "\n  " << fs::absolute(z1).string()
        << "\nmay result from failure to call"
        << " remove_alien_msw_root() where needed."
        << std::endl
        ;
    LMI_TEST_EQUAL  (z0, z2);
    LMI_TEST_EQUAL  (z0, fs::absolute(z2).string());
#elif defined LMI_MSW
    LMI_TEST( fs::path{z1}.has_root_name());
    std::string const current_drive = fs::current_path().root_name().string();
    if(current_drive == fs::path{z1}.root_name())
        {
        LMI_TEST_EQUAL  (z1, fs::absolute(z0).string());
        LMI_TEST_EQUAL  (z1, fs::absolute(z1).string());
        LMI_TEST_EQUAL  (z1, z2);
        LMI_TEST_EQUAL  (z1, fs::absolute(z2).string());
        }
    else
        {
        LMI_TEST_UNEQUAL(z1, fs::absolute(z0).string());
        std::cout
            << "This test demonstrates that if an msw directory path such as:"
            << "\n  " << z1
            << "\nis saved as a posix directory path:"
            << "\n  " << z0
            << "\nand subsequently reloaded when the msw current drive is:"
            << "\n  " << current_drive
            << "\nthen a possibly nonexistent:"
            << "\n  " << fs::absolute(z0).string()
            << "\ndirectory path might be addressed."
            << std::endl
            ;
        LMI_TEST_EQUAL  (z1, fs::absolute(z1).string());
        LMI_TEST_EQUAL  (z1, z2);
        LMI_TEST_EQUAL  (z1, fs::absolute(z2).string());
        }
#else  // Unknown platform.
    throw "Unrecognized platform."
#endif // Unknown platform.
}

int test_main(int, char*[])
{
    test_modify_directory();
    test_portable_filename();
    test_serial_file_path();
    test_unique_filepath_with_normal_filenames();
    test_unique_filepath_with_ludicrous_filenames();
    test_path_inserter();
    test_path_validation();
    test_oddities();

    return EXIT_SUCCESS;
}
