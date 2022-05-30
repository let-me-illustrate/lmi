// TAOCP 4.6.3 Right-to-left binary method for exponentiation--unit test.
//
// Copyright (C) 2022 Gregory W. Chicares.
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

#include "bin_exp.hpp"

#include "materially_equal.hpp"
#include "stl_extensions.hpp"           // nonstd::power()
#include "test_tools.hpp"
#include "timer.hpp"

#include <cfloat>                       // DECIMAL_DIG
#include <cmath>                        // pow()
#include <limits>

namespace
{
    constexpr auto inf {std::numeric_limits<double>::infinity()};
} // Unnamed namespace.

void test_systematically()
{
    // powers of zero

    LMI_TEST_EQUAL( 1.0, bin_exp( 0.0,  0));
    LMI_TEST_EQUAL( 1.0, bin_exp(-0.0,  0));
    LMI_TEST_EQUAL( 1.0, bin_exp( 0.0, -0));
    LMI_TEST_EQUAL( 1.0, bin_exp(-0.0, -0));

    LMI_TEST_EQUAL( 0.0, bin_exp( 0.0,  1));
    LMI_TEST_EQUAL( 0.0, bin_exp(-0.0,  1));
    LMI_TEST_EQUAL( inf, bin_exp( 0.0, -1));
    LMI_TEST_EQUAL(-inf, bin_exp(-0.0, -1));

    LMI_TEST_EQUAL( 0.0, bin_exp( 0.0,  9));
    LMI_TEST_EQUAL( 0.0, bin_exp(-0.0,  9));
    LMI_TEST_EQUAL( inf, bin_exp( 0.0, -9));
    LMI_TEST_EQUAL(-inf, bin_exp(-0.0, -9));

    // powers of one

    LMI_TEST_EQUAL( 1.0, bin_exp( 1.0,  0));
    LMI_TEST_EQUAL( 1.0, bin_exp(-1.0,  0));
    LMI_TEST_EQUAL( 1.0, bin_exp( 1.0, -0));
    LMI_TEST_EQUAL( 1.0, bin_exp(-1.0, -0));

    LMI_TEST_EQUAL( 1.0, bin_exp( 1.0,  1));
    LMI_TEST_EQUAL(-1.0, bin_exp(-1.0,  1));
    LMI_TEST_EQUAL( 1.0, bin_exp( 1.0, -1));
    LMI_TEST_EQUAL(-1.0, bin_exp(-1.0, -1));

    // powers of e

    constexpr double e     = 2.71828'18284'59045'23536;
    constexpr double e_sq  = 7.38905'60989'30650'22723;
    constexpr double e_101 = 7.30705'99793'68067'27265e43;

    LMI_TEST_EQUAL(     1.0, bin_exp( e,  0));
    LMI_TEST_EQUAL(     1.0, bin_exp(-e,  0));
    LMI_TEST_EQUAL(     1.0, bin_exp( e, -0));
    LMI_TEST_EQUAL(     1.0, bin_exp(-e, -0));

    LMI_TEST_EQUAL(       e, bin_exp( e,  1));
    LMI_TEST_EQUAL(      -e, bin_exp(-e,  1));
    LMI_TEST_EQUAL( 1.0 / e, bin_exp( e, -1));
    LMI_TEST_EQUAL(-1.0 / e, bin_exp(-e, -1));

    LMI_TEST(materially_equal(       e_sq , bin_exp( e,    2), 1.0e-15));
    LMI_TEST(materially_equal(       e_sq , bin_exp(-e,    2), 1.0e-15));
    LMI_TEST(materially_equal( 1.0 / e_sq , bin_exp( e,   -2), 1.0e-16));
    LMI_TEST(materially_equal( 1.0 / e_sq , bin_exp(-e,   -2), 1.0e-16));

    LMI_TEST(materially_equal(       e_101, bin_exp( e,  101), 1.0e-14));
    LMI_TEST(materially_equal(      -e_101, bin_exp(-e,  101), 1.0e-14));
    LMI_TEST(materially_equal( 1.0 / e_101, bin_exp( e, -101), 1.0e-14));
    LMI_TEST(materially_equal(-1.0 / e_101, bin_exp(-e, -101), 1.0e-14));

    // [change of sign shouldn't affect absolute value]
    LMI_TEST_EQUAL(bin_exp(-e,    2),  bin_exp( e,    2));
    LMI_TEST_EQUAL(bin_exp(-e,   -2),  bin_exp( e,   -2));
    LMI_TEST_EQUAL(bin_exp(-e,  101), -bin_exp( e,  101));
    LMI_TEST_EQUAL(bin_exp(-e, -101), -bin_exp( e, -101));

    LMI_TEST_EQUAL(     inf, bin_exp( e,  999));
    LMI_TEST_EQUAL(    -inf, bin_exp(-e,  999));
    LMI_TEST_EQUAL(     0.0, bin_exp( e, -999));
    LMI_TEST_EQUAL(    -0.0, bin_exp(-e, -999));
}

void test_integral_powers_of_two()
{
    //                        00000000011111111
    //                        12345678901234567 17 == DBL_DECIMAL_DIG
    LMI_TEST_EQUAL(0.00000000000000011102230246251565 , bin_exp(2.0, -53));
    LMI_TEST_EQUAL(0.00000000000000022204460492503130 , bin_exp(2.0, -52));
    LMI_TEST_EQUAL(0.00000000000000044408920985006261 , bin_exp(2.0, -51));
    LMI_TEST_EQUAL(0.00000000000000088817841970012523 , bin_exp(2.0, -50));
    LMI_TEST_EQUAL(0.0000000000000017763568394002504  , bin_exp(2.0, -49));
    LMI_TEST_EQUAL(0.0000000000000035527136788005009  , bin_exp(2.0, -48));
    LMI_TEST_EQUAL(0.0000000000000071054273576010018  , bin_exp(2.0, -47));
    LMI_TEST_EQUAL(0.000000000000014210854715202003   , bin_exp(2.0, -46));
    LMI_TEST_EQUAL(0.000000000000028421709430404007   , bin_exp(2.0, -45));
    LMI_TEST_EQUAL(0.000000000000056843418860808015   , bin_exp(2.0, -44));
    LMI_TEST_EQUAL(0.000000000000113686837721616030   , bin_exp(2.0, -43));
    LMI_TEST_EQUAL(0.000000000000227373675443232059   , bin_exp(2.0, -42));
    LMI_TEST_EQUAL(0.000000000000454747350886464119   , bin_exp(2.0, -41));
    LMI_TEST_EQUAL(0.000000000000909494701772928238   , bin_exp(2.0, -40));
    LMI_TEST_EQUAL(0.000000000001818989403545856476   , bin_exp(2.0, -39));
    LMI_TEST_EQUAL(0.000000000003637978807091712952   , bin_exp(2.0, -38));
    LMI_TEST_EQUAL(0.000000000007275957614183425903   , bin_exp(2.0, -37));
    LMI_TEST_EQUAL(0.000000000014551915228366851807   , bin_exp(2.0, -36));
    LMI_TEST_EQUAL(0.000000000029103830456733703613   , bin_exp(2.0, -35));
    LMI_TEST_EQUAL(0.000000000058207660913467407227   , bin_exp(2.0, -34));
    LMI_TEST_EQUAL(0.000000000116415321826934814453   , bin_exp(2.0, -33));
    LMI_TEST_EQUAL(0.000000000232830643653869628906   , bin_exp(2.0, -32));
    LMI_TEST_EQUAL(0.000000000465661287307739257813   , bin_exp(2.0, -31));
    LMI_TEST_EQUAL(0.000000000931322574615478515625   , bin_exp(2.0, -30));
    LMI_TEST_EQUAL(0.00000000186264514923095703125    , bin_exp(2.0, -29));
    LMI_TEST_EQUAL(0.0000000037252902984619140625     , bin_exp(2.0, -28));
    LMI_TEST_EQUAL(0.000000007450580596923828125      , bin_exp(2.0, -27));
    LMI_TEST_EQUAL(0.00000001490116119384765625       , bin_exp(2.0, -26));
    LMI_TEST_EQUAL(0.0000000298023223876953125        , bin_exp(2.0, -25));
    LMI_TEST_EQUAL(0.000000059604644775390625         , bin_exp(2.0, -24));
    LMI_TEST_EQUAL(0.00000011920928955078125          , bin_exp(2.0, -23));
    LMI_TEST_EQUAL(0.0000002384185791015625           , bin_exp(2.0, -22));
    LMI_TEST_EQUAL(0.000000476837158203125            , bin_exp(2.0, -21));
    LMI_TEST_EQUAL(0.00000095367431640625             , bin_exp(2.0, -20));
    LMI_TEST_EQUAL(0.0000019073486328125              , bin_exp(2.0, -19));
    LMI_TEST_EQUAL(0.000003814697265625               , bin_exp(2.0, -18));
    LMI_TEST_EQUAL(0.00000762939453125                , bin_exp(2.0, -17));
    LMI_TEST_EQUAL(0.0000152587890625                 , bin_exp(2.0, -16));
    LMI_TEST_EQUAL(0.000030517578125                  , bin_exp(2.0, -15));
    LMI_TEST_EQUAL(0.00006103515625                   , bin_exp(2.0, -14));
    LMI_TEST_EQUAL(0.0001220703125                    , bin_exp(2.0, -13));
    LMI_TEST_EQUAL(0.000244140625                     , bin_exp(2.0, -12));
    LMI_TEST_EQUAL(0.00048828125                      , bin_exp(2.0, -11));
    LMI_TEST_EQUAL(0.0009765625                       , bin_exp(2.0, -10));
    LMI_TEST_EQUAL(0.001953125                        , bin_exp(2.0,  -9));
    LMI_TEST_EQUAL(0.00390625                         , bin_exp(2.0,  -8));
    LMI_TEST_EQUAL(0.0078125                          , bin_exp(2.0,  -7));
    LMI_TEST_EQUAL(0.015625                           , bin_exp(2.0,  -6));
    LMI_TEST_EQUAL(0.03125                            , bin_exp(2.0,  -5));
    LMI_TEST_EQUAL(0.0625                             , bin_exp(2.0,  -4));
    LMI_TEST_EQUAL(0.125                              , bin_exp(2.0,  -3));
    LMI_TEST_EQUAL(0.25                               , bin_exp(2.0,  -2));
    LMI_TEST_EQUAL(0.5                                , bin_exp(2.0,  -1));
    LMI_TEST_EQUAL(1                                  , bin_exp(2.0,   0));
    LMI_TEST_EQUAL(2                                  , bin_exp(2.0,   1));
    LMI_TEST_EQUAL(4                                  , bin_exp(2.0,   2));
    LMI_TEST_EQUAL(8                                  , bin_exp(2.0,   3));
    LMI_TEST_EQUAL(16                                 , bin_exp(2.0,   4));
    LMI_TEST_EQUAL(32                                 , bin_exp(2.0,   5));
    LMI_TEST_EQUAL(64                                 , bin_exp(2.0,   6));
    LMI_TEST_EQUAL(128                                , bin_exp(2.0,   7));
    LMI_TEST_EQUAL(256                                , bin_exp(2.0,   8));
    LMI_TEST_EQUAL(512                                , bin_exp(2.0,   9));
    LMI_TEST_EQUAL(1024                               , bin_exp(2.0,  10));
    LMI_TEST_EQUAL(2048                               , bin_exp(2.0,  11));
    LMI_TEST_EQUAL(4096                               , bin_exp(2.0,  12));
    LMI_TEST_EQUAL(8192                               , bin_exp(2.0,  13));
    LMI_TEST_EQUAL(16384                              , bin_exp(2.0,  14));
    LMI_TEST_EQUAL(32768                              , bin_exp(2.0,  15));
    LMI_TEST_EQUAL(65536                              , bin_exp(2.0,  16));
    LMI_TEST_EQUAL(131072                             , bin_exp(2.0,  17));
    LMI_TEST_EQUAL(262144                             , bin_exp(2.0,  18));
    LMI_TEST_EQUAL(524288                             , bin_exp(2.0,  19));
    LMI_TEST_EQUAL(1048576                            , bin_exp(2.0,  20));
    LMI_TEST_EQUAL(2097152                            , bin_exp(2.0,  21));
    LMI_TEST_EQUAL(4194304                            , bin_exp(2.0,  22));
    LMI_TEST_EQUAL(8388608                            , bin_exp(2.0,  23));
    LMI_TEST_EQUAL(16777216                           , bin_exp(2.0,  24));
    LMI_TEST_EQUAL(33554432                           , bin_exp(2.0,  25));
    LMI_TEST_EQUAL(67108864                           , bin_exp(2.0,  26));
    LMI_TEST_EQUAL(134217728                          , bin_exp(2.0,  27));
    LMI_TEST_EQUAL(268435456                          , bin_exp(2.0,  28));
    LMI_TEST_EQUAL(536870912                          , bin_exp(2.0,  29));
    LMI_TEST_EQUAL(1073741824                         , bin_exp(2.0,  30));
    LMI_TEST_EQUAL(2147483648                         , bin_exp(2.0,  31));
    LMI_TEST_EQUAL(4294967296                         , bin_exp(2.0,  32));
    LMI_TEST_EQUAL(8589934592                         , bin_exp(2.0,  33));
    LMI_TEST_EQUAL(17179869184                        , bin_exp(2.0,  34));
    LMI_TEST_EQUAL(34359738368                        , bin_exp(2.0,  35));
    LMI_TEST_EQUAL(68719476736                        , bin_exp(2.0,  36));
    LMI_TEST_EQUAL(137438953472                       , bin_exp(2.0,  37));
    LMI_TEST_EQUAL(274877906944                       , bin_exp(2.0,  38));
    LMI_TEST_EQUAL(549755813888                       , bin_exp(2.0,  39));
    LMI_TEST_EQUAL(1099511627776                      , bin_exp(2.0,  40));
    LMI_TEST_EQUAL(2199023255552                      , bin_exp(2.0,  41));
    LMI_TEST_EQUAL(4398046511104                      , bin_exp(2.0,  42));
    LMI_TEST_EQUAL(8796093022208                      , bin_exp(2.0,  43));
    LMI_TEST_EQUAL(17592186044416                     , bin_exp(2.0,  44));
    LMI_TEST_EQUAL(35184372088832                     , bin_exp(2.0,  45));
    LMI_TEST_EQUAL(70368744177664                     , bin_exp(2.0,  46));
    LMI_TEST_EQUAL(140737488355328                    , bin_exp(2.0,  47));
    LMI_TEST_EQUAL(281474976710656                    , bin_exp(2.0,  48));
    LMI_TEST_EQUAL(562949953421312                    , bin_exp(2.0,  49));
    LMI_TEST_EQUAL(1125899906842624                   , bin_exp(2.0,  50));
    LMI_TEST_EQUAL(2251799813685248                   , bin_exp(2.0,  51));
    LMI_TEST_EQUAL(4503599627370496                   , bin_exp(2.0,  52));
    LMI_TEST_EQUAL(9007199254740992                   , bin_exp(2.0,  53));
}

void test_integral_powers_of_ten()
{
    LMI_TEST_EQUAL(0.0000000000000001                 , bin_exp(10.0, -16));
    LMI_TEST_EQUAL(0.000000000000001                  , bin_exp(10.0, -15));
    LMI_TEST_EQUAL(0.00000000000001                   , bin_exp(10.0, -14));
    LMI_TEST_EQUAL(0.0000000000001                    , bin_exp(10.0, -13));
    LMI_TEST_EQUAL(0.000000000001                     , bin_exp(10.0, -12));
    LMI_TEST_EQUAL(0.00000000001                      , bin_exp(10.0, -11));
    LMI_TEST_EQUAL(0.0000000001                       , bin_exp(10.0, -10));
    LMI_TEST_EQUAL(0.000000001                        , bin_exp(10.0,  -9));
    LMI_TEST_EQUAL(0.00000001                         , bin_exp(10.0,  -8));
    LMI_TEST_EQUAL(0.0000001                          , bin_exp(10.0,  -7));
    LMI_TEST_EQUAL(0.000001                           , bin_exp(10.0,  -6));
    LMI_TEST_EQUAL(0.00001                            , bin_exp(10.0,  -5));
    LMI_TEST_EQUAL(0.0001                             , bin_exp(10.0,  -4));
    LMI_TEST_EQUAL(0.001                              , bin_exp(10.0,  -3));
    LMI_TEST_EQUAL(0.01                               , bin_exp(10.0,  -2));
    LMI_TEST_EQUAL(0.1                                , bin_exp(10.0,  -1));
    LMI_TEST_EQUAL(1                                  , bin_exp(10.0,   0));
    LMI_TEST_EQUAL(10                                 , bin_exp(10.0,   1));
    LMI_TEST_EQUAL(100                                , bin_exp(10.0,   2));
    LMI_TEST_EQUAL(1000                               , bin_exp(10.0,   3));
    LMI_TEST_EQUAL(10000                              , bin_exp(10.0,   4));
    LMI_TEST_EQUAL(100000                             , bin_exp(10.0,   5));
    LMI_TEST_EQUAL(1000000                            , bin_exp(10.0,   6));
    LMI_TEST_EQUAL(10000000                           , bin_exp(10.0,   7));
    LMI_TEST_EQUAL(100000000                          , bin_exp(10.0,   8));
    LMI_TEST_EQUAL(1000000000                         , bin_exp(10.0,   9));
    LMI_TEST_EQUAL(10000000000                        , bin_exp(10.0,  10));
    LMI_TEST_EQUAL(100000000000                       , bin_exp(10.0,  11));
    LMI_TEST_EQUAL(1000000000000                      , bin_exp(10.0,  12));
    LMI_TEST_EQUAL(10000000000000                     , bin_exp(10.0,  13));
    LMI_TEST_EQUAL(100000000000000                    , bin_exp(10.0,  14));
    LMI_TEST_EQUAL(1000000000000000                   , bin_exp(10.0,  15));
    LMI_TEST_EQUAL(10000000000000000                  , bin_exp(10.0,  16));
}

void test_quodlibet()
{
    // Rust issue 73420:
    //             0000 0000011111111
    //             1234 5678901234567 17 == DBL_DECIMAL_DIG
    // Wolfram:    1748.219590818327062731185606025974266231060028076171875
    LMI_TEST_EQUAL(1748.2195908183271, bin_exp(12.04662322998046875, 3));

    // Compare bin_exp() to std::pow() and nonstd::power().
    double a0 = bin_exp
        (static_cast<double>(std::numeric_limits<double>::radix)
        ,                    std::numeric_limits<double>::digits
        );
    double a1 = std::pow
        (static_cast<double>(std::numeric_limits<double>::radix)
        ,static_cast<double>(std::numeric_limits<double>::digits)
        );
    double a2 = nonstd::power
        (static_cast<double>(std::numeric_limits<double>::radix)
        ,                    std::numeric_limits<double>::digits
        );
    // This compiles, but its behavior is undefined unless an int
    // is at least 54 bits (53, + 1 for sign). Otherwise it cannot
    // return the hoped-for answer, and may return zero.
    auto a3 = nonstd::power
        (                    std::numeric_limits<double>::radix
        ,                    std::numeric_limits<double>::digits
        );
    auto a4 = nonstd::power
        (static_cast<long int>(std::numeric_limits<double>::radix)
        ,static_cast<long int>(std::numeric_limits<double>::digits)
        );
    LMI_TEST_EQUAL(9007199254740992, a0);
    LMI_TEST_EQUAL(9007199254740992, a1);
    LMI_TEST_EQUAL(9007199254740992, a2);
    stifle_unused_warning(a3);
    stifle_unused_warning(a4);
}

void mete0()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        for(int k = 0; k < 32; ++k)
            {x = bin_exp(2.0, 1 + k);}
    stifle_unused_warning(x);
}

void mete1()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        for(int k = 0; k < 32; ++k)
            {x = nonstd::power(2.0, 1 + k);}
    stifle_unused_warning(x);
}

void mete2()
{
    double volatile x;
    for(int j = 0; j < 100000; ++j)
        for(int k = 0; k < 32; ++k)
            {x = std::pow(2.0, 1 + k);}
    stifle_unused_warning(x);
}

void assay_speed()
{
    std::cout << "Speed tests ['power' limits the domain]:\n";
    std::cout << "  bin_exp " << TimeAnAliquot(mete0) << '\n';
    std::cout << "  power   " << TimeAnAliquot(mete1) << '\n';
    std::cout << "  pow     " << TimeAnAliquot(mete2) << '\n';
    std::cout << std::flush;
}

int test_main(int, char*[])
{
    // This affects diagnostics shown when LMI_TEST_EQUAL() fails.
    std::cout.precision(DECIMAL_DIG);

    test_systematically();
    test_integral_powers_of_two();
    test_integral_powers_of_ten();
    test_quodlibet();

    // Optional demonstration.
    //
    // A reviewer asked whether the example on page 442 is right:
    // shouldn't the "Z" column go from x^4 to x^8 in the last
    // row (rather than from x^4 to x^16 as shown), since squaring
    // x^4 yields x^8? This explains why the book is correct.

    LMI_TEST(8388608 == Algorithm_A(2.0, 23));
    LMI_TEST(8388608 == bin_exp    (2.0, 23));

    assay_speed();

    return 0;
}
