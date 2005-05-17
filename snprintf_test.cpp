// Test snprintf() implementations against ISO/IEC 9899 7.19.6.5 .
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: snprintf_test.cpp,v 1.3 2005-05-17 12:29:06 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <stdio.h> // Nonstandard (in C++98) snprintf().
#include <string>

// COMPILER !! Ask como how to conditionalize this tastefully. It
// appears to be a defect in the msvc library underlying MinGW.
//#if defined __MINGW32_WITH_COMO [doesn't seem to work]
#if defined __COMO__
#   define snprintf _snprintf
#endif // Old gcc compiler.

int test_main(int, char*[])
{
    // COMPILER !! Test for this ms C runtime-library defect:
    //   http://www.gotw.ca/publications/mill19.htm
    // In 2003-03, mingw added a sprintf() function that calls
    // msvc's _vsnprintf(), which has the same defect.
    char buf[100] = "zzzzzzzzz";
    int len;

    len = snprintf(0, 0, "%4d", 1234);
    BOOST_TEST_EQUAL(4, len);

    // All tests in this group fail with the defective msvc rtl.
    len = snprintf(buf, 0, "%4d", 1234);
    BOOST_TEST_EQUAL(4, len);

    // All tests in this group fail with the defective msvc rtl.
    len = snprintf(buf, 3, "%4d", 1234);
    BOOST_TEST_EQUAL(4, len);
    // This test fails with borland C++ 5.5.1 .
    BOOST_TEST_EQUAL(std::string(buf, 9), std::string("12\0zzzzzz\0", 9));

    len = snprintf(buf, 4, "%4d", 1234);
    BOOST_TEST_EQUAL(4, len);
    // This test fails with the defective msvc rtl and also
    // with borland C++ 5.5.1 .
    BOOST_TEST_EQUAL(std::string(buf, 9), std::string("123\0zzzzz\0", 9));

    len = snprintf(buf, 5, "%4d", 1234);
    BOOST_TEST_EQUAL(4, len);
    BOOST_TEST_EQUAL(std::string(buf, 9), std::string("1234\0zzzz\0", 9));
    std::cout << "'buf' is '" << buf << "'." << std::endl;

    return 0;
}

#if defined __COMO__
#   undef snprintf
#endif // Old gcc compiler.

