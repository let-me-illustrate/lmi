// Punctuate numbers with commas: unit test.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: comma_punct_test.cpp,v 1.4 2006-01-29 13:52:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "comma_punct.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

// TODO ?? More tests should be added, but the one test written here
// suffices to show a gcc defect that's fixed in a later version.

int test_main(int, char*[])
{
#if defined __GNUC__ && LMI_GCC_VERSION <= 40001
    std::cerr
        << "This test fails with gcc versions before 4.0.1: see\n"
        << "  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=20914\n"
        ;
#endif // gcc version less than 4.0.1 .

    std::ostringstream oss;

    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    oss.imbue(new_loc);
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

    oss << std::setprecision(2) << -999;
    BOOST_TEST_EQUAL("-999", oss.str());

    return EXIT_SUCCESS;
}

