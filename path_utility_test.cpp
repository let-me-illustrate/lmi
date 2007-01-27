// Path utilities--unit test.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: path_utility_test.cpp,v 1.6 2007-01-27 00:00:52 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "path_utility.hpp"

#include "miscellany.hpp"
#include "platform_dependent.hpp" // access()
#include "test_tools.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>

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
        ofs << p.string();
    }
}

void test_normal_operation()
{
    // These tests would fail if read-only files with the following
    // names already exist.

    char const* p = "/tmp/eraseme.0";
    char const* q = "/tmp/eraseme.xyzzy";

    // Assume directory /tmp/ exists: the makefile provided ensures
    // that, even on platforms that don't always provide it.

    // Don't test the return codes here. These files probably don't
    // exist, in which case C99 7.19.4.1 doesn't clearly prescribe
    // the semantics of remove().
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
    // intended exception if it's reinvoked more rapidly than its
    // timestamp changes.

    keep_open.close();

    BOOST_TEST(0 == std::remove(p));
    BOOST_TEST(0 == std::remove(q));
    BOOST_TEST(0 == std::remove(path2.string().c_str()));
}

void test_ludicrous_filenames()
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
        ,fs::filesystem_error, ""
        );
}

int test_main(int, char*[])
{
    test_normal_operation();
    test_ludicrous_filenames();

    return EXIT_SUCCESS;
}

