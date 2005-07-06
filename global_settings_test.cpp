// Global settings--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: global_settings_test.cpp,v 1.4 2005-07-06 00:48:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

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

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("")
        ,std::runtime_error
        ,"Data directory must not be empty."
        );

    BOOST_TEST_THROW
        (global_settings::instance().set_regression_test_directory("")
        ,std::runtime_error
        ,"Regression-test directory must not be empty."
        );

    // Such string values must be validly-formed paths.

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("?")
        ,std::runtime_error
        ,""
        );

    // Such string values must name paths that exist in the filesystem.

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("UnLiKeLyNaMe")
        ,std::runtime_error
        ,"Data directory 'UnLiKeLyNaMe' not found."
        );

    // Such string values must name directories, not normal files.

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("global_settings.o")
        ,std::runtime_error
        ,"Data directory 'global_settings.o' is not a directory."
        );
}

int test_main(int, char*[])
{
    // Initial values of 'directory' data members must be valid: the
    // operations tested here must not throw.

    global_settings::instance().set_data_directory
        (global_settings::instance().data_directory().string()
        );

    fs::path path(global_settings::instance().data_directory());

    fs::directory_iterator i(path);

    BOOST_TEST(exists(*i));

    // Certain other operations are required to throw.

    test_directory_exceptions();

    // 'ash_nazg' implies 'mellon'.
    global_settings::instance().set_mellon(false);
    BOOST_TEST(!global_settings::instance().mellon());

    global_settings::instance().set_ash_nazg(true);
    BOOST_TEST( global_settings::instance().mellon());

    return 0;
}

