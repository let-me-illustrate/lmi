// Format doubles with thousands separators: unit test.
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

#include "miscellany.hpp"               // begins_with()
#include "test_tools.hpp"

#include <limits>

int test_main(int, char*[])
{
    // Format positive numbers, with two decimals.
    LMI_TEST_EQUAL( "1,234,567,890.14", duff_fmt( 1234567890.14159, 2));
    LMI_TEST_EQUAL(   "234,567,890.14", duff_fmt(  234567890.14159, 2));
    LMI_TEST_EQUAL(    "34,567,890.14", duff_fmt(   34567890.14159, 2));
    LMI_TEST_EQUAL(     "4,567,890.14", duff_fmt(    4567890.14159, 2));
    LMI_TEST_EQUAL(       "567,890.14", duff_fmt(     567890.14159, 2));
    LMI_TEST_EQUAL(        "67,890.14", duff_fmt(      67890.14159, 2));
    LMI_TEST_EQUAL(         "7,890.14", duff_fmt(       7890.14159, 2));
    LMI_TEST_EQUAL(           "890.14", duff_fmt(        890.14159, 2));
    LMI_TEST_EQUAL(            "90.14", duff_fmt(         90.14159, 2));
    LMI_TEST_EQUAL(             "0.14", duff_fmt(          0.14159, 2));
    LMI_TEST_EQUAL( "1,234,567,890.00", duff_fmt( 1234567890.0    , 2));
    LMI_TEST_EQUAL(   "234,567,890.00", duff_fmt(  234567890.0    , 2));
    LMI_TEST_EQUAL(    "34,567,890.00", duff_fmt(   34567890.0    , 2));
    LMI_TEST_EQUAL(     "4,567,890.00", duff_fmt(    4567890.0    , 2));
    LMI_TEST_EQUAL(       "567,890.00", duff_fmt(     567890.0    , 2));
    LMI_TEST_EQUAL(        "67,890.00", duff_fmt(      67890.0    , 2));
    LMI_TEST_EQUAL(         "7,890.00", duff_fmt(       7890.0    , 2));
    LMI_TEST_EQUAL(           "890.00", duff_fmt(        890.0    , 2));
    LMI_TEST_EQUAL(            "90.00", duff_fmt(         90.0    , 2));
    LMI_TEST_EQUAL(             "1.00", duff_fmt(          1.0    , 2));
    LMI_TEST_EQUAL( "1,000,000,000.00", duff_fmt( 1000000000.0    , 2));
    LMI_TEST_EQUAL(   "100,000,000.00", duff_fmt(  100000000.0    , 2));
    LMI_TEST_EQUAL(    "10,000,000.00", duff_fmt(   10000000.0    , 2));
    LMI_TEST_EQUAL(     "1,000,000.00", duff_fmt(    1000000.0    , 2));
    LMI_TEST_EQUAL(       "100,000.00", duff_fmt(     100000.0    , 2));
    LMI_TEST_EQUAL(        "10,000.00", duff_fmt(      10000.0    , 2));
    LMI_TEST_EQUAL(         "1,000.00", duff_fmt(       1000.0    , 2));
    LMI_TEST_EQUAL(           "100.00", duff_fmt(        100.0    , 2));
    LMI_TEST_EQUAL(            "10.00", duff_fmt(         10.0    , 2));
    LMI_TEST_EQUAL(             "1.00", duff_fmt(          1.0    , 2));
    LMI_TEST_EQUAL(             "0.10", duff_fmt(          0.1    , 2));
    LMI_TEST_EQUAL(             "0.01", duff_fmt(          0.01   , 2));
    LMI_TEST_EQUAL( "2,147,483,647.00", duff_fmt( 2147483647.0    , 2));
    LMI_TEST_EQUAL(         "2,000.00", duff_fmt(       1999.999  , 2));
    LMI_TEST_EQUAL(         "2,000.00", duff_fmt(       1999.99501, 2));
    LMI_TEST_EQUAL(         "1,999.99", duff_fmt(       1999.99499, 2));
    LMI_TEST_EQUAL(         "1,900.00", duff_fmt(       1899.999  , 2));
    LMI_TEST_EQUAL(    "10,000,000.00", duff_fmt(    9999999.999  , 2));

    // Repeat with negatives.
    LMI_TEST_EQUAL("-1,234,567,890.14", duff_fmt(-1234567890.14159, 2));
    LMI_TEST_EQUAL(  "-234,567,890.14", duff_fmt( -234567890.14159, 2));
    LMI_TEST_EQUAL(   "-34,567,890.14", duff_fmt(  -34567890.14159, 2));
    LMI_TEST_EQUAL(    "-4,567,890.14", duff_fmt(   -4567890.14159, 2));
    LMI_TEST_EQUAL(      "-567,890.14", duff_fmt(    -567890.14159, 2));
    LMI_TEST_EQUAL(       "-67,890.14", duff_fmt(     -67890.14159, 2));
    LMI_TEST_EQUAL(        "-7,890.14", duff_fmt(      -7890.14159, 2));
    LMI_TEST_EQUAL(          "-890.14", duff_fmt(       -890.14159, 2));
    LMI_TEST_EQUAL(           "-90.14", duff_fmt(        -90.14159, 2));
    LMI_TEST_EQUAL(            "-0.14", duff_fmt(         -0.14159, 2));
    LMI_TEST_EQUAL("-1,234,567,890.00", duff_fmt(-1234567890.0    , 2));
    LMI_TEST_EQUAL(  "-234,567,890.00", duff_fmt( -234567890.0    , 2));
    LMI_TEST_EQUAL(   "-34,567,890.00", duff_fmt(  -34567890.0    , 2));
    LMI_TEST_EQUAL(    "-4,567,890.00", duff_fmt(   -4567890.0    , 2));
    LMI_TEST_EQUAL(      "-567,890.00", duff_fmt(    -567890.0    , 2));
    LMI_TEST_EQUAL(       "-67,890.00", duff_fmt(     -67890.0    , 2));
    LMI_TEST_EQUAL(        "-7,890.00", duff_fmt(      -7890.0    , 2));
    LMI_TEST_EQUAL(          "-890.00", duff_fmt(       -890.0    , 2));
    LMI_TEST_EQUAL(           "-90.00", duff_fmt(        -90.0    , 2));
    LMI_TEST_EQUAL(            "-1.00", duff_fmt(         -1.0    , 2));
    LMI_TEST_EQUAL("-1,000,000,000.00", duff_fmt(-1000000000.0    , 2));
    LMI_TEST_EQUAL(  "-100,000,000.00", duff_fmt( -100000000.0    , 2));
    LMI_TEST_EQUAL(   "-10,000,000.00", duff_fmt(  -10000000.0    , 2));
    LMI_TEST_EQUAL(    "-1,000,000.00", duff_fmt(   -1000000.0    , 2));
    LMI_TEST_EQUAL(      "-100,000.00", duff_fmt(    -100000.0    , 2));
    LMI_TEST_EQUAL(       "-10,000.00", duff_fmt(     -10000.0    , 2));
    LMI_TEST_EQUAL(        "-1,000.00", duff_fmt(      -1000.0    , 2));
    LMI_TEST_EQUAL(          "-100.00", duff_fmt(       -100.0    , 2));
    LMI_TEST_EQUAL(           "-10.00", duff_fmt(        -10.0    , 2));
    LMI_TEST_EQUAL(            "-1.00", duff_fmt(         -1.0    , 2));
    LMI_TEST_EQUAL(            "-0.10", duff_fmt(         -0.1    , 2));
    LMI_TEST_EQUAL(            "-0.01", duff_fmt(         -0.01   , 2));
    LMI_TEST_EQUAL("-2,147,483,647.00", duff_fmt(-2147483647.0    , 2));
    LMI_TEST_EQUAL(        "-2,000.00", duff_fmt(      -1999.999  , 2));
    LMI_TEST_EQUAL(        "-2,000.00", duff_fmt(      -1999.999  , 2));
    LMI_TEST_EQUAL(        "-2,000.00", duff_fmt(      -1999.99501, 2));
    LMI_TEST_EQUAL(        "-1,999.99", duff_fmt(      -1999.99499, 2));
    LMI_TEST_EQUAL(        "-1,900.00", duff_fmt(      -1899.999  , 2));
    LMI_TEST_EQUAL(   "-10,000,000.00", duff_fmt(   -9999999.999  , 2));

    // Same as above, with zero decimals.
    LMI_TEST_EQUAL( "1,234,567,890", duff_fmt( 1234567890.14159, 0));
    LMI_TEST_EQUAL(   "234,567,890", duff_fmt(  234567890.14159, 0));
    LMI_TEST_EQUAL(    "34,567,890", duff_fmt(   34567890.14159, 0));
    LMI_TEST_EQUAL(     "4,567,890", duff_fmt(    4567890.14159, 0));
    LMI_TEST_EQUAL(       "567,890", duff_fmt(     567890.14159, 0));
    LMI_TEST_EQUAL(        "67,890", duff_fmt(      67890.14159, 0));
    LMI_TEST_EQUAL(         "7,890", duff_fmt(       7890.14159, 0));
    LMI_TEST_EQUAL(           "890", duff_fmt(        890.14159, 0));
    LMI_TEST_EQUAL(            "90", duff_fmt(         90.14159, 0));
    LMI_TEST_EQUAL(             "0", duff_fmt(          0.14159, 0));
    LMI_TEST_EQUAL( "1,234,567,890", duff_fmt( 1234567890.0    , 0));
    LMI_TEST_EQUAL(   "234,567,890", duff_fmt(  234567890.0    , 0));
    LMI_TEST_EQUAL(    "34,567,890", duff_fmt(   34567890.0    , 0));
    LMI_TEST_EQUAL(     "4,567,890", duff_fmt(    4567890.0    , 0));
    LMI_TEST_EQUAL(       "567,890", duff_fmt(     567890.0    , 0));
    LMI_TEST_EQUAL(        "67,890", duff_fmt(      67890.0    , 0));
    LMI_TEST_EQUAL(         "7,890", duff_fmt(       7890.0    , 0));
    LMI_TEST_EQUAL(           "890", duff_fmt(        890.0    , 0));
    LMI_TEST_EQUAL(            "90", duff_fmt(         90.0    , 0));
    LMI_TEST_EQUAL(             "1", duff_fmt(          1.0    , 0));
    LMI_TEST_EQUAL( "1,000,000,000", duff_fmt( 1000000000.0    , 0));
    LMI_TEST_EQUAL(   "100,000,000", duff_fmt(  100000000.0    , 0));
    LMI_TEST_EQUAL(    "10,000,000", duff_fmt(   10000000.0    , 0));
    LMI_TEST_EQUAL(     "1,000,000", duff_fmt(    1000000.0    , 0));
    LMI_TEST_EQUAL(       "100,000", duff_fmt(     100000.0    , 0));
    LMI_TEST_EQUAL(        "10,000", duff_fmt(      10000.0    , 0));
    LMI_TEST_EQUAL(         "1,000", duff_fmt(       1000.0    , 0));
    LMI_TEST_EQUAL(           "100", duff_fmt(        100.0    , 0));
    LMI_TEST_EQUAL(            "10", duff_fmt(         10.0    , 0));
    LMI_TEST_EQUAL(             "1", duff_fmt(          1.0    , 0));
    LMI_TEST_EQUAL(             "0", duff_fmt(          0.1    , 0));
    LMI_TEST_EQUAL(             "0", duff_fmt(          0.01   , 0));
    LMI_TEST_EQUAL( "2,147,483,647", duff_fmt( 2147483647.0    , 0));
    LMI_TEST_EQUAL(         "2,000", duff_fmt(       1999.999  , 0));
    LMI_TEST_EQUAL(         "2,000", duff_fmt(       1999.99501, 0));
    LMI_TEST_EQUAL(         "2,000", duff_fmt(       1999.99499, 0));
    LMI_TEST_EQUAL(         "1,900", duff_fmt(       1899.999  , 0));
    LMI_TEST_EQUAL(    "10,000,000", duff_fmt(    9999999.999  , 0));

    // Repeat with negatives.
    LMI_TEST_EQUAL("-1,234,567,890", duff_fmt(-1234567890.14159, 0));
    LMI_TEST_EQUAL(  "-234,567,890", duff_fmt( -234567890.14159, 0));
    LMI_TEST_EQUAL(   "-34,567,890", duff_fmt(  -34567890.14159, 0));
    LMI_TEST_EQUAL(    "-4,567,890", duff_fmt(   -4567890.14159, 0));
    LMI_TEST_EQUAL(      "-567,890", duff_fmt(    -567890.14159, 0));
    LMI_TEST_EQUAL(       "-67,890", duff_fmt(     -67890.14159, 0));
    LMI_TEST_EQUAL(        "-7,890", duff_fmt(      -7890.14159, 0));
    LMI_TEST_EQUAL(          "-890", duff_fmt(       -890.14159, 0));
    LMI_TEST_EQUAL(           "-90", duff_fmt(        -90.14159, 0));
    LMI_TEST_EQUAL(            "-0", duff_fmt(         -0.14159, 0));
    LMI_TEST_EQUAL("-1,234,567,890", duff_fmt(-1234567890.0    , 0));
    LMI_TEST_EQUAL(  "-234,567,890", duff_fmt( -234567890.0    , 0));
    LMI_TEST_EQUAL(   "-34,567,890", duff_fmt(  -34567890.0    , 0));
    LMI_TEST_EQUAL(    "-4,567,890", duff_fmt(   -4567890.0    , 0));
    LMI_TEST_EQUAL(      "-567,890", duff_fmt(    -567890.0    , 0));
    LMI_TEST_EQUAL(       "-67,890", duff_fmt(     -67890.0    , 0));
    LMI_TEST_EQUAL(        "-7,890", duff_fmt(      -7890.0    , 0));
    LMI_TEST_EQUAL(          "-890", duff_fmt(       -890.0    , 0));
    LMI_TEST_EQUAL(           "-90", duff_fmt(        -90.0    , 0));
    LMI_TEST_EQUAL(            "-1", duff_fmt(         -1.0    , 0));
    LMI_TEST_EQUAL("-1,000,000,000", duff_fmt(-1000000000.0    , 0));
    LMI_TEST_EQUAL(  "-100,000,000", duff_fmt( -100000000.0    , 0));
    LMI_TEST_EQUAL(   "-10,000,000", duff_fmt(  -10000000.0    , 0));
    LMI_TEST_EQUAL(    "-1,000,000", duff_fmt(   -1000000.0    , 0));
    LMI_TEST_EQUAL(      "-100,000", duff_fmt(    -100000.0    , 0));
    LMI_TEST_EQUAL(       "-10,000", duff_fmt(     -10000.0    , 0));
    LMI_TEST_EQUAL(        "-1,000", duff_fmt(      -1000.0    , 0));
    LMI_TEST_EQUAL(          "-100", duff_fmt(       -100.0    , 0));
    LMI_TEST_EQUAL(           "-10", duff_fmt(        -10.0    , 0));
    LMI_TEST_EQUAL(            "-1", duff_fmt(         -1.0    , 0));
    LMI_TEST_EQUAL(            "-0", duff_fmt(         -0.1    , 0));
    LMI_TEST_EQUAL(            "-0", duff_fmt(         -0.01   , 0));
    LMI_TEST_EQUAL("-2,147,483,647", duff_fmt(-2147483647.0    , 0));
    LMI_TEST_EQUAL(        "-2,000", duff_fmt(      -1999.999  , 0));
    LMI_TEST_EQUAL(        "-2,000", duff_fmt(      -1999.999  , 0));
    LMI_TEST_EQUAL(        "-2,000", duff_fmt(      -1999.99501, 0));
    LMI_TEST_EQUAL(        "-2,000", duff_fmt(      -1999.99499, 0));
    LMI_TEST_EQUAL(        "-1,900", duff_fmt(      -1899.999  , 0));
    LMI_TEST_EQUAL(   "-10,000,000", duff_fmt(   -9999999.999  , 0));

    // Implementation-defined value [2.13.3/1].
    LMI_TEST
        (               "1,999.99" == duff_fmt(      1999.995     , 2)
        ||              "2,000.00" == duff_fmt(      1999.995     , 2)
        );

    // Infinities and NaNs.

    double volatile d = 0.0;
    std::string pos_inf = duff_fmt( 1.0 / d, 2);
    std::string neg_inf = duff_fmt(-1.0 / d, 2);
    LMI_TEST( "inf" == pos_inf ||  "infinity" == pos_inf);
    LMI_TEST("-inf" == neg_inf || "-infinity" == neg_inf);

    if(std::numeric_limits<double>::has_quiet_NaN)
        {
        constexpr double quiet_NaN = std::numeric_limits<double>::quiet_NaN();
        std::string qnan = duff_fmt(quiet_NaN, 2);
        // Test only "nan", disregarding any 'n-char-sequence' payload.
        // The sign of quiet_NaN() seems to be unspecified.
        LMI_TEST(begins_with(qnan, "nan") || begins_with(qnan, "-nan"));
        }

    return 0;
}
