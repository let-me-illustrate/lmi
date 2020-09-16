// Path utilities--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

    // Arguably this should be forbidden:
    //   $ls /bin/sh/
    //   ls: cannot access '/bin/sh/': Not a directory
    LMI_TEST_EQUAL("/bin/sh"      , modify_directory("sh/"    , "/bin/"   ).string());

    LMI_TEST_THROW
        (modify_directory("", "/bin")
        ,std::runtime_error
        ,"Assertion 'original_filepath.has_leaf()' failed."
        );
}

void test_orthodox_filename()
{
    LMI_TEST_THROW
        (orthodox_filename("")
        ,std::runtime_error
        ,"Assertion '!original_filename.empty()' failed."
        );

    LMI_TEST_EQUAL("Z"     , orthodox_filename("Z"));
    LMI_TEST_EQUAL("_"     , orthodox_filename("."));
    LMI_TEST_EQUAL("_"     , orthodox_filename("#"));

    LMI_TEST_EQUAL("AZ"    , orthodox_filename("AZ"));
    LMI_TEST_EQUAL("A_"    , orthodox_filename("A."));
    LMI_TEST_EQUAL("_Z"    , orthodox_filename(".Z"));
    LMI_TEST_EQUAL("__"    , orthodox_filename(".."));
    LMI_TEST_EQUAL("__"    , orthodox_filename("##"));

    LMI_TEST_EQUAL("A.Z"   , orthodox_filename("A.Z"));
    LMI_TEST_EQUAL("A-Z"   , orthodox_filename("A-Z"));

    LMI_TEST_EQUAL("_xyz_" , orthodox_filename(".xyz."));
    LMI_TEST_EQUAL("_xyz_" , orthodox_filename("-xyz-"));

    LMI_TEST_EQUAL("and_or", orthodox_filename("and/or"));

    LMI_TEST_EQUAL
        (                  "Crime_and_or_Punishment.text"
        ,orthodox_filename("Crime and/or Punishment.text")
        );

    LMI_TEST_EQUAL
        (                  "_Fyodor_Dostoyevskiy_Crime_and_Punishment.text"
        ,orthodox_filename("/Fyodor Dostoyevskiy/Crime and Punishment.text")
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

    // Input census filepath needn't have any extension.
    LMI_TEST_EQUAL
        (serial_file_path("x.ignored", "",      12345, "y").string()
        ,serial_file_path("x"        , "",      12345, "y").string()
        );
    LMI_TEST_EQUAL
        (                                "x.000012346.y"
        ,serial_file_path("x.ignored", "",      12345, "y").string()
        );

    // Discard path from input census filepath; use only leaf.
    LMI_TEST_EQUAL
        (serial_file_path("/path/to/x", "",      12345, "y").string()
        ,serial_file_path("x"         , "",      12345, "y").string()
        );
    LMI_TEST_EQUAL
        (                                 "x.000012346.y"
        ,serial_file_path("/path/to/x", "",      12345, "y").string()
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

    // Verify that the first function call here is redundant:
    //   path_b = change_extension(path_a, ext)
    //   path_c = unique_filepath (path_b, ext)
    // and this single function call has the same effect:
    //   path_c = unique_filepath (path_a, ext)
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
    // They return nonzero, and do not remove the directory. The
    // reason is that the msw C library's remove() function doesn't
    // delete directories; it sets errno to 13, which means EACCESS,
    // and _doserrno to 5, which might mean EIO. The boost:filesystem
    // msw implementation calls RemoveDirectoryA() instead of the C
    // remove() function in this case. The std::filesystem::remove()
    // specification in C++17 (N4659) [30.10.15.30] says:
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
    // valid: the boost library's default path ctor constructs an
    // empty path.

    fs::path path2 = unique_filepath(fs::path(""), "");
    LMI_TEST_EQUAL(path2.string(), "");

#if defined LMI_MSW
    // fs::change_extension()'s argument is ".[extension]", so ".."
    // represents a '.' extension-delimiter followed by an extension
    // consisting of a single '.'. When fs::change_extension() is
    // called by unique_filepath() here, adding that extension to ".."
    // yields "...", which is forbidden by msw, but allowed (although
    // of course discouraged) by posix.
    LMI_TEST_THROW
        (unique_filepath(fs::path(".."), "..")
        ,fs::filesystem_error
        ,""
        );
#endif // defined LMI_MSW
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

    // Not well formed.
#if 0
    // Neither posix nor msw allows NUL in paths. However, the boost
    // filesystem library doesn't throw an explicit exception here;
    // instead, it aborts with:
    //   "Assertion `src.size() == std::strlen( src.c_str() )' failed."
    // Perhaps std::filesystem will trap this and throw an exception.
    std::string nulls = {'\0', '\0'};
    LMI_TEST_THROW
        (validate_filepath(nulls, context)
        ,std::runtime_error
        ,lmi_test::what_regex("invalid name \"<|>\" in path")
        );
#endif // 0

#if defined LMI_MSW
    LMI_TEST_THROW
        (validate_filepath("<|>", context)
        ,std::runtime_error
        ,lmi_test::what_regex("invalid name \"<|>\" in path")
        );
#endif // defined LMI_MSW

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

/// Demonstrate a boost::filesystem oddity.
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

void test_oddities()
{
    std::string const z0 = "/opt/lmi/data";
    std::string const z1 = "Z:/opt/lmi/data";
    std::string const z2 = remove_alien_msw_root(z1).string();
#if defined LMI_POSIX
    LMI_TEST_EQUAL  (z0, fs::absolute(z0).string());
    LMI_TEST_UNEQUAL(z0, fs::absolute(z1).string());
    LMI_TEST_EQUAL  (z0, z2);
    LMI_TEST_EQUAL  (z0, fs::absolute(z2).string());
#elif defined LMI_MSW
    LMI_TEST_EQUAL  (z1, fs::absolute(z0).string());
    LMI_TEST_EQUAL  (z1, fs::absolute(z1).string());
    LMI_TEST_EQUAL  (z1, z2);
    LMI_TEST_EQUAL  (z1, fs::absolute(z2).string());
#else  // Unknown platform.
    throw "Unrecognized platform."
#endif // Unknown platform.
}

int test_main(int, char*[])
{
    test_modify_directory();
    test_orthodox_filename();
    test_serial_file_path();
    test_unique_filepath_with_normal_filenames();
    test_unique_filepath_with_ludicrous_filenames();
    test_path_inserter();
    test_path_validation();
    test_oddities();

    return EXIT_SUCCESS;
}
