// Format NNNN.NNN --> "N,NNN.NN": unit test.
//
// Copyright (C) 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "duff_fmt.hpp"

#include "test_tools.hpp"

#include <limits>

#if defined __BORLANDC__
#   include <float.h>                   // nonstandard _control87()
#endif // defined __BORLANDC__

int test_main(int, char*[])
{
    LMI_TEST_EQUAL( "1,234,567,890.14", ncnnnpnn( 1234567890.14159));
    LMI_TEST_EQUAL(   "234,567,890.14", ncnnnpnn(  234567890.14159));
    LMI_TEST_EQUAL(    "34,567,890.14", ncnnnpnn(   34567890.14159));
    LMI_TEST_EQUAL(     "4,567,890.14", ncnnnpnn(    4567890.14159));
    LMI_TEST_EQUAL(       "567,890.14", ncnnnpnn(     567890.14159));
    LMI_TEST_EQUAL(        "67,890.14", ncnnnpnn(      67890.14159));
    LMI_TEST_EQUAL(         "7,890.14", ncnnnpnn(       7890.14159));
    LMI_TEST_EQUAL(           "890.14", ncnnnpnn(        890.14159));
    LMI_TEST_EQUAL(            "90.14", ncnnnpnn(         90.14159));
    LMI_TEST_EQUAL(             "0.14", ncnnnpnn(          0.14159));
    LMI_TEST_EQUAL( "1,234,567,890.00", ncnnnpnn( 1234567890.0    ));
    LMI_TEST_EQUAL(   "234,567,890.00", ncnnnpnn(  234567890.0    ));
    LMI_TEST_EQUAL(    "34,567,890.00", ncnnnpnn(   34567890.0    ));
    LMI_TEST_EQUAL(     "4,567,890.00", ncnnnpnn(    4567890.0    ));
    LMI_TEST_EQUAL(       "567,890.00", ncnnnpnn(     567890.0    ));
    LMI_TEST_EQUAL(        "67,890.00", ncnnnpnn(      67890.0    ));
    LMI_TEST_EQUAL(         "7,890.00", ncnnnpnn(       7890.0    ));
    LMI_TEST_EQUAL(           "890.00", ncnnnpnn(        890.0    ));
    LMI_TEST_EQUAL(            "90.00", ncnnnpnn(         90.0    ));
    LMI_TEST_EQUAL(             "1.00", ncnnnpnn(          1.0    ));
    LMI_TEST_EQUAL( "1,000,000,000.00", ncnnnpnn( 1000000000.0    ));
    LMI_TEST_EQUAL(   "100,000,000.00", ncnnnpnn(  100000000.0    ));
    LMI_TEST_EQUAL(    "10,000,000.00", ncnnnpnn(   10000000.0    ));
    LMI_TEST_EQUAL(     "1,000,000.00", ncnnnpnn(    1000000.0    ));
    LMI_TEST_EQUAL(       "100,000.00", ncnnnpnn(     100000.0    ));
    LMI_TEST_EQUAL(        "10,000.00", ncnnnpnn(      10000.0    ));
    LMI_TEST_EQUAL(         "1,000.00", ncnnnpnn(       1000.0    ));
    LMI_TEST_EQUAL(           "100.00", ncnnnpnn(        100.0    ));
    LMI_TEST_EQUAL(            "10.00", ncnnnpnn(         10.0    ));
    LMI_TEST_EQUAL(             "1.00", ncnnnpnn(          1.0    ));
    LMI_TEST_EQUAL(             "0.10", ncnnnpnn(          0.1    ));
    LMI_TEST_EQUAL(             "0.01", ncnnnpnn(          0.01   ));
    LMI_TEST_EQUAL( "2,147,483,647.00", ncnnnpnn( 2147483647.0    ));
    LMI_TEST_EQUAL(         "2,000.00", ncnnnpnn(       1999.999  ));
    LMI_TEST_EQUAL(         "2,000.00", ncnnnpnn(       1999.99501));
    LMI_TEST_EQUAL(         "1,999.99", ncnnnpnn(       1999.99499));
    LMI_TEST_EQUAL(         "1,900.00", ncnnnpnn(       1899.999  ));
    LMI_TEST_EQUAL(    "10,000,000.00", ncnnnpnn(    9999999.999  ));

    // Repeat with negatives.
    LMI_TEST_EQUAL("-1,234,567,890.14", ncnnnpnn(-1234567890.14159));
    LMI_TEST_EQUAL(  "-234,567,890.14", ncnnnpnn( -234567890.14159));
    LMI_TEST_EQUAL(   "-34,567,890.14", ncnnnpnn(  -34567890.14159));
    LMI_TEST_EQUAL(    "-4,567,890.14", ncnnnpnn(   -4567890.14159));
    LMI_TEST_EQUAL(      "-567,890.14", ncnnnpnn(    -567890.14159));
    LMI_TEST_EQUAL(       "-67,890.14", ncnnnpnn(     -67890.14159));
    LMI_TEST_EQUAL(        "-7,890.14", ncnnnpnn(      -7890.14159));
    LMI_TEST_EQUAL(          "-890.14", ncnnnpnn(       -890.14159));
    LMI_TEST_EQUAL(           "-90.14", ncnnnpnn(        -90.14159));
    LMI_TEST_EQUAL(            "-0.14", ncnnnpnn(         -0.14159));
    LMI_TEST_EQUAL("-1,234,567,890.00", ncnnnpnn(-1234567890.0    ));
    LMI_TEST_EQUAL(  "-234,567,890.00", ncnnnpnn( -234567890.0    ));
    LMI_TEST_EQUAL(   "-34,567,890.00", ncnnnpnn(  -34567890.0    ));
    LMI_TEST_EQUAL(    "-4,567,890.00", ncnnnpnn(   -4567890.0    ));
    LMI_TEST_EQUAL(      "-567,890.00", ncnnnpnn(    -567890.0    ));
    LMI_TEST_EQUAL(       "-67,890.00", ncnnnpnn(     -67890.0    ));
    LMI_TEST_EQUAL(        "-7,890.00", ncnnnpnn(      -7890.0    ));
    LMI_TEST_EQUAL(          "-890.00", ncnnnpnn(       -890.0    ));
    LMI_TEST_EQUAL(           "-90.00", ncnnnpnn(        -90.0    ));
    LMI_TEST_EQUAL(            "-1.00", ncnnnpnn(         -1.0    ));
    LMI_TEST_EQUAL("-1,000,000,000.00", ncnnnpnn(-1000000000.0    ));
    LMI_TEST_EQUAL(  "-100,000,000.00", ncnnnpnn( -100000000.0    ));
    LMI_TEST_EQUAL(   "-10,000,000.00", ncnnnpnn(  -10000000.0    ));
    LMI_TEST_EQUAL(    "-1,000,000.00", ncnnnpnn(   -1000000.0    ));
    LMI_TEST_EQUAL(      "-100,000.00", ncnnnpnn(    -100000.0    ));
    LMI_TEST_EQUAL(       "-10,000.00", ncnnnpnn(     -10000.0    ));
    LMI_TEST_EQUAL(        "-1,000.00", ncnnnpnn(      -1000.0    ));
    LMI_TEST_EQUAL(          "-100.00", ncnnnpnn(       -100.0    ));
    LMI_TEST_EQUAL(           "-10.00", ncnnnpnn(        -10.0    ));
    LMI_TEST_EQUAL(            "-1.00", ncnnnpnn(         -1.0    ));
    LMI_TEST_EQUAL(            "-0.10", ncnnnpnn(         -0.1    ));
    LMI_TEST_EQUAL(            "-0.01", ncnnnpnn(         -0.01   ));
    LMI_TEST_EQUAL("-2,147,483,647.00", ncnnnpnn(-2147483647.0    ));
    LMI_TEST_EQUAL(        "-2,000.00", ncnnnpnn(      -1999.999  ));
    LMI_TEST_EQUAL(        "-2,000.00", ncnnnpnn(      -1999.999  ));
    LMI_TEST_EQUAL(        "-2,000.00", ncnnnpnn(      -1999.99501));
    LMI_TEST_EQUAL(        "-1,999.99", ncnnnpnn(      -1999.99499));
    LMI_TEST_EQUAL(        "-1,900.00", ncnnnpnn(      -1899.999  ));
    LMI_TEST_EQUAL(   "-10,000,000.00", ncnnnpnn(   -9999999.999  ));

    // Implementation-defined value [2.13.3/1].
    LMI_TEST
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
#endif // defined __BORLANDC__

    long double volatile d = 0.0;
    ncnnnpnn( 1.0 / d  );
    ncnnnpnn(-1.0 / d  );

    if(std::numeric_limits<long double>::has_quiet_NaN)
        {
        ncnnnpnn(std::numeric_limits<long double>::quiet_NaN());
        }

    return 0;
}
