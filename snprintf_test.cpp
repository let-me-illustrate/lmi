// Test snprintf() implementations against ISO/IEC 9899 7.19.6.5 .
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#include "test_tools.hpp"

#include <stdio.h> // snprintf() (C99, not C++98).
#include <string>

int test_main(int, char*[])
{
    // COMPILER !! Test for this ms C runtime-library defect:
    //   http://www.gotw.ca/publications/mill19.htm
    // In 2003-03, mingw added a sprintf() function that calls
    // msvc's _vsnprintf(), which has the same defect.
    char buf[1000] = "zzzzzzzzz";
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

    long double z = 2.718281828459045L;
    len = snprintf(buf, 5, "%.5Lf", z);
    BOOST_TEST_EQUAL(7, len);
    // This should truncate to 2.71, not round to 2.72 .
    BOOST_TEST_EQUAL(std::string(buf, 9), std::string("2.71\0zzzz\0", 9));
    len = snprintf(buf, 7, "%.5Lf", z);
    BOOST_TEST_EQUAL(7, len);
    BOOST_TEST_EQUAL(std::string(buf, 9), std::string("2.7182\0zz\0", 9));
    len = snprintf(buf,       0, "%1.12Lf", z);
    BOOST_TEST_EQUAL(14, len);
    len = snprintf(buf, 1 + len, "%1.12Lf", z);
    BOOST_TEST_EQUAL(14, len);
    BOOST_TEST_EQUAL(std::string(buf, 15), std::string("2.718281828459\0", 15));

    // See:
    //   http://www.nabble.com/forum/PrintPost.jtp?post=17172711
    //     [2008-05-11T11:46Z from François-Xavier Coudert]
    double g = 39.0;
    len = snprintf(buf,       0, "%9.0e", g);
    BOOST_TEST_EQUAL(9, len);
    len = snprintf(buf, 1 + len, "%9.0e", g);
    BOOST_TEST_EQUAL(9, len);
    BOOST_TEST_EQUAL(std::string(buf, 9), std::string("    4e+01\0", 9));

    double d = 1e+161;
    len = snprintf(buf,       0, "%#.*f", 16, d);
    BOOST_TEST_EQUAL(179, len);
    len = snprintf(buf, 1 + len, "%#.*f", 16, d);
    BOOST_TEST_EQUAL(179, len);
    std::string e
        ("1"
        "00000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000"
        "00000000000"
        ".0000000000000000"
        );
    // http://lists.nongnu.org/archive/html/lmi/2010-04/msg00042.html
    int const number_of_digits = 16;
    BOOST_TEST(0 == e.compare(0, number_of_digits, buf, 0, number_of_digits));

    return 0;
}

