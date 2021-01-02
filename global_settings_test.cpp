// Global settings--unit test.
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

#include "global_settings.hpp"

#include "path_utility.hpp"             // initialize_filesystem()
#include "test_tools.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

void test_directory_exceptions()
{
    // Test an exception for each 'directory' member to make sure
    // the member context (e.g., "Data directory") is correctly
    // reported.

    // String values assigned to 'directory' data members must be
    // valid "directory paths" as defined in the boost filesystem
    // documentation. In particular:

    // Such string values must not be empty.

    // "Data directory must not be empty."
    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("")
        ,std::runtime_error
        ,""
        );

    // Such string values must be validly-formed paths.

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("?")
        ,std::runtime_error
        ,""
        );

    // Such string values must name paths that exist in the filesystem.

    // "Data directory 'UnLiKeLyNaMe' not found."
    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("UnLiKeLyNaMe")
        ,std::runtime_error
        ,""
        );

    // Such string values must name directories, not normal files.

    // "Data directory 'global_settings.o' is not a directory."
    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("global_settings.o")
        ,std::runtime_error
        ,""
        );
}

int test_main(int, char*[])
{
    // Absolute paths require "native" name-checking policy for msw.

    initialize_filesystem();

    // Initial values of 'directory' data members must be valid: the
    // operations tested here are required not to throw.

    global_settings::instance().set_data_directory
        (global_settings::instance().data_directory().string()
        );

    fs::path path(global_settings::instance().data_directory());

    fs::directory_iterator i(path);

    BOOST_TEST(exists(*i));

    // Certain other operations are required to throw.

    test_directory_exceptions();

    // Attempting to set a default name-checking policy after creating
    // an instance of class global_settings should throw. Test this in
    // order to guard against changes to the boost filesystem library.

    BOOST_TEST_THROW
        (fs::path::default_name_check(fs::native)
        ,fs::filesystem_error
        ,""
        );

    // 'ash_nazg' implies 'mellon'.
    global_settings::instance().set_mellon(false);
    BOOST_TEST(!global_settings::instance().mellon());

    global_settings::instance().set_ash_nazg(true);
    BOOST_TEST( global_settings::instance().mellon());

    return 0;
}
