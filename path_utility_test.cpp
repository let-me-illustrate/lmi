// Path utilities--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "miscellany.hpp"
#include "platform_dependent.hpp"       // access()
#include "test_tools.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <cstdio>                       // std::remove()
#include <fstream>
#include <sstream>
#include <stdexcept>

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
}

void test_orthodox_filename()
{
    BOOST_TEST_THROW
        (orthodox_filename("")
        ,std::runtime_error
        ,"Assertion '!original_filename.empty()' failed."
        );

    BOOST_TEST_EQUAL("Z"     , orthodox_filename("Z"));
    BOOST_TEST_EQUAL("_"     , orthodox_filename("."));
    BOOST_TEST_EQUAL("_"     , orthodox_filename("#"));

    BOOST_TEST_EQUAL("AZ"    , orthodox_filename("AZ"));
    BOOST_TEST_EQUAL("A_"    , orthodox_filename("A."));
    BOOST_TEST_EQUAL("_Z"    , orthodox_filename(".Z"));
    BOOST_TEST_EQUAL("__"    , orthodox_filename(".."));
    BOOST_TEST_EQUAL("__"    , orthodox_filename("##"));

    BOOST_TEST_EQUAL("A.Z"   , orthodox_filename("A.Z"));
    BOOST_TEST_EQUAL("A-Z"   , orthodox_filename("A-Z"));

    BOOST_TEST_EQUAL("_xyz_" , orthodox_filename(".xyz."));
    BOOST_TEST_EQUAL("_xyz_" , orthodox_filename("-xyz-"));

    BOOST_TEST_EQUAL("and_or", orthodox_filename("and/or"));

    BOOST_TEST_EQUAL
        (                  "Crime_and_or_Punishment.text"
        ,orthodox_filename("Crime and/or Punishment.text")
        );

    BOOST_TEST_EQUAL
        (                  "_Fyodor_Dostoyevskiy_Crime_and_Punishment.text"
        ,orthodox_filename("/Fyodor Dostoyevskiy/Crime and Punishment.text")
        );
}

void test_serial_file_path()
{
    BOOST_TEST_EQUAL
        (                         "Abolitionists.John_Brown.018591203.text"
        ,serial_file_path("Abolitionists.cns", "John Brown", 18591202, "text").string()
        );

    // Serial number is zero-padded to nine positions, but can use more.
    BOOST_TEST_EQUAL
        (                            "x.000012346.y"
        ,serial_file_path("x.cns", "",      12345, "y").string()
        );
    BOOST_TEST_EQUAL
        (                            "x.123456790.y"
        ,serial_file_path("x.cns", "",  123456789, "y").string()
        );
    BOOST_TEST_EQUAL
        (                           "x.1234567891.y"
        ,serial_file_path("x.cns", "", 1234567890, "y").string()
        );

    // Input census filepath needn't have any extension.
    BOOST_TEST_EQUAL
        (serial_file_path("x.ignored", "",      12345, "y").string()
        ,serial_file_path("x"        , "",      12345, "y").string()
        );
    BOOST_TEST_EQUAL
        (                                "x.000012346.y"
        ,serial_file_path("x.ignored", "",      12345, "y").string()
        );

    // Discard path from input census filepath; use only leaf.
    BOOST_TEST_EQUAL
        (serial_file_path("/path/to/x", "",      12345, "y").string()
        ,serial_file_path("x"         , "",      12345, "y").string()
        );
    BOOST_TEST_EQUAL
        (                                 "x.000012346.y"
        ,serial_file_path("/path/to/x", "",      12345, "y").string()
        );
}

void test_unique_filepath_with_normal_filenames()
{
    fs::path const tmpdir(fs::absolute("/tmp", fs::initial_path()));
    fs::create_directory(tmpdir);

    // These tests would fail if read-only files with the following
    // names already exist.

    char const* p = "/tmp/eraseme.0";
    char const* q = "/tmp/eraseme.xyzzy";

    // Don't test the return codes here. These files probably don't
    // exist, in which case C99 7.19.4.1 doesn't clearly prescribe
    // the semantics of std::remove().
    std::remove(p);
    std::remove(q);

    write_dummy_file(p);
    BOOST_TEST_EQUAL(0, access(p, R_OK));

    fs::path path0 = unique_filepath(fs::path(p), ".xyzzy");
    BOOST_TEST_EQUAL(path0.string(), q);
    write_dummy_file(path0);
    BOOST_TEST_EQUAL(0, access(path0.string().c_str(), R_OK));

    fs::path path1 = unique_filepath(fs::path(p), ".xyzzy");
    BOOST_TEST_EQUAL(path1.string(), q);
    write_dummy_file(path1);
    BOOST_TEST_EQUAL(0, access(path1.string().c_str(), R_OK));

    // Open a file for writing, and leave it open, preventing it from
    // being erased and therefore forcing unique_filepath() to use a
    // different name. This behavior isn't guaranteed on toy OS's.
    fs::ofstream keep_open(path1, ios_out_app_binary());

    fs::path path2 = unique_filepath(fs::path(p), ".xyzzy");
    BOOST_TEST_UNEQUAL(path2.string(), q);
    write_dummy_file(path2);
    BOOST_TEST_EQUAL(0, access(path2.string().c_str(), R_OK));

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

    char const* r = "/tmp/eraseme.abc.def";
    char const* s = "/tmp/eraseme.abc-CCYYMMDDTHHMMSSZ.def";
//             NOT:  /tmp/eraseme.abc-CCYYMMDDTHHMMSSZ.abc.def

    fs::path path3 = unique_filepath(fs::path(p), ".abc.def");
    BOOST_TEST_EQUAL(path3.string(), r);
    write_dummy_file(path3);
    BOOST_TEST_EQUAL(0, access(path3.string().c_str(), R_OK));

    keep_open.open(path3, ios_out_app_binary());

    fs::path path4 = unique_filepath(fs::path(p), ".abc.def");
    BOOST_TEST(path3.string().size() == std::string(r).size());
    BOOST_TEST(path4.string().size() == std::string(s).size());

    keep_open.close();

    // Clean up the files created in this function.

    BOOST_TEST(0 == std::remove(p));
    BOOST_TEST(0 == std::remove(q));
    BOOST_TEST(0 == std::remove(path2.string().c_str()));
    BOOST_TEST(0 == std::remove(path3.string().c_str()));
}

void test_unique_filepath_with_ludicrous_filenames()
{
    fs::path path0 = unique_filepath(fs::path("eraseme1"), "");
    BOOST_TEST_EQUAL(path0.string(), "eraseme1");

    fs::path path1 = unique_filepath(fs::path("eraseme2"), "");
    BOOST_TEST_EQUAL(path1.string(), "eraseme2");

    // "" isn't likely to be a valid filename, but as a path, it's
    // valid: the boost library's default path ctor constructs an
    // empty path.

    fs::path path2 = unique_filepath(fs::path(""), "");
    BOOST_TEST_EQUAL(path2.string(), "");

    BOOST_TEST_THROW
        (unique_filepath(fs::path(".."), "..")
        ,fs::filesystem_error
        ,""
        );
}

void test_path_inserter()
{
    char const* z = "/opt/lmi/test/foo.bar";
    fs::path const p(z);
    std::ostringstream oss;
    oss << p;
    BOOST_TEST_EQUAL(z, oss.str());
}

int test_main(int, char*[])
{
    test_orthodox_filename();
    test_serial_file_path();
    test_unique_filepath_with_normal_filenames();
    test_unique_filepath_with_ludicrous_filenames();
    test_path_inserter();

    return EXIT_SUCCESS;
}

