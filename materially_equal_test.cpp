// Test material equality of floating-point values--unit test.
//
// Copyright (C) 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "materially_equal.hpp"

#include "test_tools.hpp"

#include <limits>

int test_main(int, char*[])
{
    double epsilon = std::numeric_limits<double>::epsilon();

    BOOST_TEST( materially_equal(0.0,  0.0));
    BOOST_TEST( materially_equal(0.0, -0.0));
    BOOST_TEST(!materially_equal(0.0, epsilon));
    // https://lists.nongnu.org/archive/html/lmi/2018-02/msg00099.html
    BOOST_TEST(!materially_equal(0.0, 1e-100));

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
    BOOST_TEST(!materially_equal(1000000000L , 1000000001L ));

    BOOST_TEST( materially_equal(1000000000.0, 1000000001.0, 1.0E-9));
    BOOST_TEST( materially_equal(1000000000L , 1000000001L , 1.0E-9));

    long double bignum = std::numeric_limits<long double>::max();
    BOOST_TEST( materially_equal(bignum, bignum, 0.0L   ));
    BOOST_TEST( materially_equal(bignum, bignum, 1.0E-9L));
    BOOST_TEST( materially_equal(bignum, bignum, 1.0L   ));
    BOOST_TEST( materially_equal(bignum, bignum, 1.0E9L ));

    long double smallnum = std::numeric_limits<long double>::min();
    BOOST_TEST( materially_equal(smallnum, smallnum, 0.0L   ));
    BOOST_TEST( materially_equal(smallnum, smallnum, 1.0E-9L));
    BOOST_TEST( materially_equal(smallnum, smallnum, 1.0L   ));
    BOOST_TEST( materially_equal(smallnum, smallnum, 1.0E9L ));

    long double infinity = std::numeric_limits<long double>::infinity();
    BOOST_TEST( materially_equal(infinity, infinity, 0.0L   ));
    BOOST_TEST( materially_equal(infinity, infinity, 1.0E-9L));
    BOOST_TEST( materially_equal(infinity, infinity, 1.0L   ));
    BOOST_TEST( materially_equal(infinity, infinity, 1.0E9L ));

    BOOST_TEST( materially_equal(bignum  , bignum  , smallnum));
    BOOST_TEST( materially_equal(infinity, infinity, smallnum));
    BOOST_TEST( materially_equal(smallnum, smallnum, bignum  ));
    BOOST_TEST( materially_equal(infinity, infinity, bignum  ));
    BOOST_TEST( materially_equal(bignum  , bignum  , infinity));
    BOOST_TEST( materially_equal(smallnum, smallnum, infinity));

    BOOST_TEST( materially_equal(0.0L    ,-0.0L    , smallnum));
    BOOST_TEST( materially_equal(smallnum,-smallnum, 3.0L    ));

    // NaNs should always compare unequal, at least for compilers
    // that claim to implement them properly.

    if(std::numeric_limits<double>::has_quiet_NaN)
        {
        double quiet_NaN = std::numeric_limits<double>::quiet_NaN();
        if(quiet_NaN == quiet_NaN)
            {
            std::cout << "Defective compiler: qNaNs compare equal" << std::endl;
            BOOST_TEST(  quiet_NaN != quiet_NaN );
            }
        else
            {
            BOOST_TEST(  quiet_NaN != quiet_NaN );
            BOOST_TEST(!(quiet_NaN == quiet_NaN));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 0.0   ));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 1.0E-9));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 1.0   ));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 1.0E9 ));
            }
        }

    if(std::numeric_limits<long double>::has_quiet_NaN)
        {
        long double quiet_NaN = std::numeric_limits<long double>::quiet_NaN();
        if(quiet_NaN == quiet_NaN)
            {
            std::cout << "Defective compiler: qNaNs compare equal" << std::endl;
            BOOST_TEST(  quiet_NaN != quiet_NaN );
            }
        else
            {
            BOOST_TEST(  quiet_NaN != quiet_NaN );
            BOOST_TEST(!(quiet_NaN == quiet_NaN));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 0.0L   ));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 1.0E-9L));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 1.0L   ));
            BOOST_TEST(!materially_equal(quiet_NaN, quiet_NaN, 1.0E9L ));
            }
        }

    // Test material_difference().

    BOOST_TEST_EQUAL( 0.0, material_difference(0.0, 0.0));
    BOOST_TEST_EQUAL( 0.0, material_difference(1.1, 1.1));
    BOOST_TEST_EQUAL( 1.1, material_difference(1.1, 0.0));
    BOOST_TEST_EQUAL(-1.1, material_difference(0.0, 1.1));

    // https://lists.nongnu.org/archive/html/lmi/2018-02/msg00099.html
    BOOST_TEST_UNEQUAL(0.0, material_difference(0.0, 1e-100));

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
