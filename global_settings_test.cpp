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

// $Id: global_settings_test.cpp,v 1.2 2005-06-21 14:57:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

int test_main(int, char*[])
{
    // Test an exception from each 'directory' member to make sure
    // the member's nature (e.g., "Data directory") is correct.

    // Directory names must not be empty.

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

    // Directory names must exist in the filesystem.

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("UnLiKeLyNaMe")
        ,std::runtime_error
        ,"Data directory 'UnLiKeLyNaMe' not found."
        );

    // Directory names must be name directories, not normal files.

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("global_settings.o")
        ,std::runtime_error
        ,"Data directory 'global_settings.o' is not a directory."
        );

    // Directory names must be valid pathnames.

    BOOST_TEST_THROW
        (global_settings::instance().set_data_directory("?")
        ,std::runtime_error
        ,""
        );

    return 0;
}

