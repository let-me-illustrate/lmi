// Format NNNN.NNN --> "N,NNN.NN": unit test.
//
// Copyright (C) 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ncnnnpnn.hpp"

#include "test_tools.hpp"

#include <limits>

#if defined __BORLANDC__
#   include <float.h>                   // nonstandard _control87()
#endif // __BORLANDC__

int test_main(int, char*[])
{
    BOOST_TEST_EQUAL( "1,234,567,890.14", ncnnnpnn( 1234567890.14159));
    BOOST_TEST_EQUAL(   "234,567,890.14", ncnnnpnn(  234567890.14159));
    BOOST_TEST_EQUAL(    "34,567,890.14", ncnnnpnn(   34567890.14159));
    BOOST_TEST_EQUAL(     "4,567,890.14", ncnnnpnn(    4567890.14159));
    BOOST_TEST_EQUAL(       "567,890.14", ncnnnpnn(     567890.14159));
    BOOST_TEST_EQUAL(        "67,890.14", ncnnnpnn(      67890.14159));
    BOOST_TEST_EQUAL(         "7,890.14", ncnnnpnn(       7890.14159));
    BOOST_TEST_EQUAL(           "890.14", ncnnnpnn(        890.14159));
    BOOST_TEST_EQUAL(            "90.14", ncnnnpnn(         90.14159));
    BOOST_TEST_EQUAL(             "0.14", ncnnnpnn(          0.14159));
    BOOST_TEST_EQUAL( "1,234,567,890.00", ncnnnpnn( 1234567890.     ));
    BOOST_TEST_EQUAL(   "234,567,890.00", ncnnnpnn(  234567890.     ));
    BOOST_TEST_EQUAL(    "34,567,890.00", ncnnnpnn(   34567890.     ));
    BOOST_TEST_EQUAL(     "4,567,890.00", ncnnnpnn(    4567890.     ));
    BOOST_TEST_EQUAL(       "567,890.00", ncnnnpnn(     567890.     ));
    BOOST_TEST_EQUAL(        "67,890.00", ncnnnpnn(      67890.     ));
    BOOST_TEST_EQUAL(         "7,890.00", ncnnnpnn(       7890.     ));
    BOOST_TEST_EQUAL(           "890.00", ncnnnpnn(        890.     ));
    BOOST_TEST_EQUAL(            "90.00", ncnnnpnn(         90.     ));
    BOOST_TEST_EQUAL(             "1.00", ncnnnpnn(          1.     ));
    BOOST_TEST_EQUAL( "1,000,000,000.00", ncnnnpnn( 1000000000.     ));
    BOOST_TEST_EQUAL(   "100,000,000.00", ncnnnpnn(  100000000.     ));
    BOOST_TEST_EQUAL(    "10,000,000.00", ncnnnpnn(   10000000.     ));
    BOOST_TEST_EQUAL(     "1,000,000.00", ncnnnpnn(    1000000.     ));
    BOOST_TEST_EQUAL(       "100,000.00", ncnnnpnn(     100000.     ));
    BOOST_TEST_EQUAL(        "10,000.00", ncnnnpnn(      10000.     ));
    BOOST_TEST_EQUAL(         "1,000.00", ncnnnpnn(       1000.     ));
    BOOST_TEST_EQUAL(           "100.00", ncnnnpnn(        100.     ));
    BOOST_TEST_EQUAL(            "10.00", ncnnnpnn(         10.     ));
    BOOST_TEST_EQUAL(             "1.00", ncnnnpnn(          1.     ));
    BOOST_TEST_EQUAL(             "0.10", ncnnnpnn(          0.1    ));
    BOOST_TEST_EQUAL(             "0.01", ncnnnpnn(          0.01   ));
    BOOST_TEST_EQUAL( "2,147,483,647.00", ncnnnpnn( 2147483647.0    ));
    BOOST_TEST_EQUAL(         "2,000.00", ncnnnpnn(       1999.999  ));
    BOOST_TEST_EQUAL(         "2,000.00", ncnnnpnn(       1999.99501));
    BOOST_TEST_EQUAL(         "1,999.99", ncnnnpnn(       1999.99499));
    BOOST_TEST_EQUAL(         "1,900.00", ncnnnpnn(       1899.999  ));
    BOOST_TEST_EQUAL(    "10,000,000.00", ncnnnpnn(    9999999.999  ));

    // Repeat with negatives.
    BOOST_TEST_EQUAL("-1,234,567,890.14", ncnnnpnn(-1234567890.14159));
    BOOST_TEST_EQUAL(  "-234,567,890.14", ncnnnpnn( -234567890.14159));
    BOOST_TEST_EQUAL(   "-34,567,890.14", ncnnnpnn(  -34567890.14159));
    BOOST_TEST_EQUAL(    "-4,567,890.14", ncnnnpnn(   -4567890.14159));
    BOOST_TEST_EQUAL(      "-567,890.14", ncnnnpnn(    -567890.14159));
    BOOST_TEST_EQUAL(       "-67,890.14", ncnnnpnn(     -67890.14159));
    BOOST_TEST_EQUAL(        "-7,890.14", ncnnnpnn(      -7890.14159));
    BOOST_TEST_EQUAL(          "-890.14", ncnnnpnn(       -890.14159));
    BOOST_TEST_EQUAL(           "-90.14", ncnnnpnn(        -90.14159));
    BOOST_TEST_EQUAL(            "-0.14", ncnnnpnn(         -0.14159));
    BOOST_TEST_EQUAL("-1,234,567,890.00", ncnnnpnn(-1234567890.     ));
    BOOST_TEST_EQUAL(  "-234,567,890.00", ncnnnpnn( -234567890.     ));
    BOOST_TEST_EQUAL(   "-34,567,890.00", ncnnnpnn(  -34567890.     ));
    BOOST_TEST_EQUAL(    "-4,567,890.00", ncnnnpnn(   -4567890.     ));
    BOOST_TEST_EQUAL(      "-567,890.00", ncnnnpnn(    -567890.     ));
    BOOST_TEST_EQUAL(       "-67,890.00", ncnnnpnn(     -67890.     ));
    BOOST_TEST_EQUAL(        "-7,890.00", ncnnnpnn(      -7890.     ));
    BOOST_TEST_EQUAL(          "-890.00", ncnnnpnn(       -890.     ));
    BOOST_TEST_EQUAL(           "-90.00", ncnnnpnn(        -90.     ));
    BOOST_TEST_EQUAL(            "-1.00", ncnnnpnn(         -1.     ));
    BOOST_TEST_EQUAL("-1,000,000,000.00", ncnnnpnn(-1000000000.     ));
    BOOST_TEST_EQUAL(  "-100,000,000.00", ncnnnpnn( -100000000.     ));
    BOOST_TEST_EQUAL(   "-10,000,000.00", ncnnnpnn(  -10000000.     ));
    BOOST_TEST_EQUAL(    "-1,000,000.00", ncnnnpnn(   -1000000.     ));
    BOOST_TEST_EQUAL(      "-100,000.00", ncnnnpnn(    -100000.     ));
    BOOST_TEST_EQUAL(       "-10,000.00", ncnnnpnn(     -10000.     ));
    BOOST_TEST_EQUAL(        "-1,000.00", ncnnnpnn(      -1000.     ));
    BOOST_TEST_EQUAL(          "-100.00", ncnnnpnn(       -100.     ));
    BOOST_TEST_EQUAL(           "-10.00", ncnnnpnn(        -10.     ));
    BOOST_TEST_EQUAL(            "-1.00", ncnnnpnn(         -1.     ));
    BOOST_TEST_EQUAL(            "-0.10", ncnnnpnn(         -0.1    ));
    BOOST_TEST_EQUAL(            "-0.01", ncnnnpnn(         -0.01   ));
    BOOST_TEST_EQUAL("-2,147,483,647.00", ncnnnpnn(-2147483647.0    ));
    BOOST_TEST_EQUAL(        "-2,000.00", ncnnnpnn(      -1999.999  ));
    BOOST_TEST_EQUAL(        "-2,000.00", ncnnnpnn(      -1999.999  ));
    BOOST_TEST_EQUAL(        "-2,000.00", ncnnnpnn(      -1999.99501));
    BOOST_TEST_EQUAL(        "-1,999.99", ncnnnpnn(      -1999.99499));
    BOOST_TEST_EQUAL(        "-1,900.00", ncnnnpnn(      -1899.999  ));
    BOOST_TEST_EQUAL(   "-10,000,000.00", ncnnnpnn(   -9999999.999  ));

    // Implementation-defined value [2.13.3/1].
    BOOST_TEST
        (               "1,999.99" == ncnnnpnn(      1999.995   )
        ||              "2,000.00" == ncnnnpnn(      1999.995   )
        );

    // Infinities and NaNs. Apparently the C89/90 standard referenced
    // by C++98 does not specify a unique string representation
    // http://groups.google.com/groups?as_umsgid=5cj5ae%24f4e%241%40shade.twinsun.com
    // so we test only that the formatting routine executes.
    // C99 does specify the result [7.19.6.1/8], but that doesn't
    // affect the standard C++ language as this is written in 2002.

#if defined __BORLANDC__
    // By default, the borland compiler traps infinity and NaNs,
    // and signals a hardware exception; but we want to test them,
    // so we mask them from the application.
    _control87(0x00ff,  0x00ff);
#endif // __BORLANDC__

    volatile long double d = 0.0;
    ncnnnpnn( 1.0 / d  );
    ncnnnpnn(-1.0 / d  );

#if !defined __BORLANDC__
    if(std::numeric_limits<long double>::has_quiet_NaN)
        {
        ncnnnpnn(std::numeric_limits<long double>::quiet_NaN());
        }
#else // defined __BORLANDC__
    // COMPILER !! Problem with borland compiler: see
    //   http://lists.boost.org/Archives/boost/2001/05/12046.php
    //   http://lists.boost.org/Archives/boost/2001/05/12078.php
    // We choose to avoid the hardware exception here,
    // so that all our tests can run to completion
    // unattended; but it is a failure, so:
    BOOST_TEST(false);
#endif // defined __BORLANDC__

    return 0;
}

