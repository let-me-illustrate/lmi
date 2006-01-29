// Test material equality of floating-point values--unit test.
//
// Copyright (C) 2002, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: materially_equal_test.cpp,v 1.3 2006-01-29 13:52:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "materially_equal.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <limits>

int test_main(int, char*[])
{
    double epsilon = std::numeric_limits<double>::epsilon();

    BOOST_TEST( materially_equal(0.0,  0.0));
    BOOST_TEST( materially_equal(0.0, -0.0));
    BOOST_TEST(!materially_equal(0.0, epsilon));

    BOOST_TEST( materially_equal(1, 1));
    BOOST_TEST(!materially_equal(1, 2));

    BOOST_TEST( materially_equal(1.0, 1.0));
    BOOST_TEST(!materially_equal(1.0, 2.0));

    BOOST_TEST(!materially_equal(epsilon, -epsilon));

    BOOST_TEST(!materially_equal( 1.0,  1.0 + epsilon, 0.0));

    BOOST_TEST( materially_equal( 1.0,  1.0 + 1.0E1 * epsilon));
    BOOST_TEST( materially_equal( 1.0,  1.0 + 1.0E2 * epsilon));
    BOOST_TEST(!materially_equal( 1.0,  1.0 + 1.0E3 * epsilon));
    BOOST_TEST(!materially_equal( 1.0,  1.0 + 1.0E4 * epsilon));
    BOOST_TEST(!materially_equal( 1.0,  1.0 + 1.0E5 * epsilon));
    BOOST_TEST(!materially_equal( 1.0,  1.0 + 1.0E6 * epsilon));
    BOOST_TEST(!materially_equal( 1.0,  1.0 + 1.0E7 * epsilon));
    BOOST_TEST(!materially_equal( 1.0,  1.0 + 1.0E8 * epsilon));
    BOOST_TEST(!materially_equal( 1.0,  1.0 + 1.0E9 * epsilon));

    BOOST_TEST( materially_equal(-1.0, -1.0 + 1.0E1 * epsilon));
    BOOST_TEST( materially_equal(-1.0, -1.0 + 1.0E2 * epsilon));
    BOOST_TEST(!materially_equal(-1.0, -1.0 + 1.0E3 * epsilon));
    BOOST_TEST(!materially_equal(-1.0, -1.0 + 1.0E4 * epsilon));
    BOOST_TEST(!materially_equal(-1.0, -1.0 + 1.0E5 * epsilon));
    BOOST_TEST(!materially_equal(-1.0, -1.0 + 1.0E6 * epsilon));
    BOOST_TEST(!materially_equal(-1.0, -1.0 + 1.0E7 * epsilon));
    BOOST_TEST(!materially_equal(-1.0, -1.0 + 1.0E8 * epsilon));
    BOOST_TEST(!materially_equal(-1.0, -1.0 + 1.0E9 * epsilon));

    BOOST_TEST(!materially_equal(1000000000.0, 1000000001.0));
    BOOST_TEST(!materially_equal(1000000000L , 1000000001.0));
    BOOST_TEST(!materially_equal(1000000000.0, 1000000001L ));
    BOOST_TEST(!materially_equal(1000000000L , 1000000001L ));

    BOOST_TEST( materially_equal(1000000000.0, 1000000001.0, 1.0E-9));
    BOOST_TEST( materially_equal(1000000000L , 1000000001.0, 1.0E-9));
    BOOST_TEST( materially_equal(1000000000.0, 1000000001L , 1.0E-9));
    BOOST_TEST( materially_equal(1000000000L , 1000000001L , 1.0E-9));

    // Test material_difference().

    BOOST_TEST_EQUAL( 0.0, material_difference(0.0, 0.0));
    BOOST_TEST_EQUAL( 0.0, material_difference(1.1, 1.1));
    BOOST_TEST_EQUAL( 1.1, material_difference(1.1, 0.0));
    BOOST_TEST_EQUAL(-1.1, material_difference(0.0, 1.1));

    BOOST_TEST_EQUAL
        (0.0
        ,material_difference(1234567890.123456789, 1234567890.123456789)
        );

    BOOST_TEST_EQUAL
        (0.0
        ,material_difference(1234567890.123456789, 1234567890.1234)
        );

    BOOST_TEST_UNEQUAL
        (0.0
        ,material_difference(1234567890.123456789, 1234567890.123)
        );

    return 0;
}

