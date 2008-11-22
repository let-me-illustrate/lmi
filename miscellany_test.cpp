// Miscellaneous functions--unit test.
//
// Copyright (C) 2008 Gregory W. Chicares.
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

// $Id: miscellany_test.cpp,v 1.3 2008-11-22 16:34:12 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "miscellany.hpp"

#include "test_tools.hpp"

void test_files_are_identical()
{
    std::string f0("unlikely_file_name_0");
    std::string f1("unlikely_file_name_1");
    BOOST_TEST_THROW
        (files_are_identical(f0, f1)
        ,std::runtime_error
        ,"Unable to open 'unlikely_file_name_0'."
        );
}

int test_main(int, char*[])
{
    test_files_are_identical();

    return 0;
}

