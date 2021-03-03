// Represent a currency amount exactly as integral cents--unit test.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "currency.hpp"

#include "round_to.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <algorithm>                    // min()
#include <limits>
#include <sstream>

namespace
{
    round_to<double> const round_to_nearest_cent(2, r_to_nearest);
} // Unnamed namespace.

class currency_test
{
  public:
    static void test();

  private:
    static void test_default_ctor();
    static void test_copy_ctor();
    static void test_explicit_ctor();
    static void test_negation();
    static void test_plus_or_minus_eq();
    static void test_plus_or_minus();
    static void test_multiply_by_int();
    static void test_multiply_by_double();
    static void test_divide_by_double();
    static void test_relops();
    static void test_stream_inserter();
    static void test_dollars();
    static void test_round_double();
    static void test_round_currency();
    static void mete_humongous();
    static void mete_infinite();
    static void test_infinite();
    static void test_quodlibet();
};

void currency_test::test()
{
#if defined USE_CURRENCY_CLASS
    test_default_ctor();
    test_copy_ctor();
    test_explicit_ctor();
    test_negation();
// CURRENCY !! Most of these tests assume that the currency unit is
// cents. It's not worth adapting them to any other case because
// soon this macro will, in effect, always be defined.
#   if defined CURRENCY_UNIT_IS_CENTS
    test_plus_or_minus_eq();
    test_plus_or_minus();
    test_multiply_by_int();
    test_multiply_by_double();
    test_divide_by_double();
    test_relops();
    test_stream_inserter();
    test_dollars();
    test_round_double();
    test_round_currency();
    test_infinite();
    test_quodlibet();
#   endif // defined CURRENCY_UNIT_IS_CENTS
#endif // defined USE_CURRENCY_CLASS
}

#if defined USE_CURRENCY_CLASS
void currency_test::test_default_ctor()
{
    // default ctor
    currency const a0;
    LMI_TEST(0.00 == a0.d());
    LMI_TEST(   0 == a0.m_);
    constexpr currency zero {};
    LMI_TEST(   0 == a0.m_);
}

void currency_test::test_copy_ctor()
{
    currency const a1(325, raw_cents{});
    currency const copy0 = a1;
    LMI_TEST_EQUAL( 325, copy0.m_);
    currency const copy1 {a1};
    LMI_TEST_EQUAL( 325, copy1.m_);
}

void currency_test::test_explicit_ctor()
{
    currency const a1(325, raw_cents{});
    LMI_TEST_EQUAL( 325, a1.m_);
#if defined DETECT_NONINTEGRAL_CENTS
    // 1/64 is an exact binary constant, so 100/64 cents could be
    // converted to 1/64 dollars and back without loss of precision;
    // but that's outside the intended scope of the currency class.
    LMI_TEST_THROW
        ((currency {1.5625, raw_cents {}})
        ,std::runtime_error
        ,"Nonintegral cents."
        );
#endif // defined DETECT_NONINTEGRAL_CENTS
}

void currency_test::test_negation()
{
    currency const a1(321, raw_cents{});
    -a1;
    // make sure that didn't mutate the object
    // (making negation a nonmember makes that mistake less likely)
    LMI_TEST_EQUAL( 321, a1.m_);
    LMI_TEST_EQUAL(-321, (-a1).m_);

    currency const a2 = -a1;
    LMI_TEST_EQUAL(-321, a2.m_);
}

void currency_test::test_plus_or_minus_eq()
{
    currency a1(325, raw_cents{});
    a1 += a1;
    LMI_TEST_EQUAL( 650, a1.m_);

    a1 -= currency {123, raw_cents {}};
    LMI_TEST_EQUAL(527, a1.m_);
}

void currency_test::test_plus_or_minus()
{
    currency const a1(650, raw_cents{});
    currency a2 = currency() + a1 + a1;
    LMI_TEST_EQUAL(13.00, a2.d());
    LMI_TEST_EQUAL( 1300, a2.m_);

    a2 = currency() - a1;
    LMI_TEST_EQUAL(-6.50, a2.d());
    LMI_TEST_EQUAL( -650, a2.m_);
    a2 = C0 - a1;
    LMI_TEST_EQUAL(-6.50, a2.d());
    LMI_TEST_EQUAL( -650, a2.m_);
}

void currency_test::test_multiply_by_int()
{
    // currency * int returns currency
    currency const mult2 {3125, raw_cents {}};
    LMI_TEST_EQUAL(1000.0,  (32 * mult2).d());
    LMI_TEST_EQUAL(1000.0, dblize(32 * mult2));
    LMI_TEST_EQUAL(100000, (mult2 * 32).m_);
}

void currency_test::test_multiply_by_double()
{
    currency const mult2 {3125, raw_cents {}};
    // currency * double returns double
    LMI_TEST_EQUAL(1000.0, 32.0 * mult2);
    LMI_TEST_EQUAL(1000.0, mult2 * 32.0);
}

void currency_test::test_divide_by_double()
{
    // currency / double returns double
    currency const div2 {3300, raw_cents {}};
    LMI_TEST_EQUAL(1.0, div2 / 33);
}

void currency_test::test_relops()
{
    currency const a0;
    currency const a1(1728, raw_cents{});
    LMI_TEST(  C0 == a0);
    LMI_TEST(  a1 == a1);
    LMI_TEST(  a0 <  a1);
    LMI_TEST(  a0 <= a1);
    LMI_TEST(  a1 <= a1);
    LMI_TEST(  a1 >  a0);
    LMI_TEST(  a1 >= a0);
    LMI_TEST(  a1 >= a1);
}

void currency_test::test_stream_inserter()
{
    currency const a3 {123456, raw_cents {}};
    std::ostringstream oss;
    oss << a3;
    LMI_TEST_EQUAL("1234.56", oss.str());
}

void currency_test::test_dollars()
{
    currency const a0;
    LMI_TEST(0.00 == a0.d());

    currency const a1(325, raw_cents{});
    LMI_TEST_EQUAL( 325, a1.m_);
    LMI_TEST_EQUAL( 325, a1.cents());
    // 3.25 is an exact binary constant
    LMI_TEST_EQUAL(3.25, a1.d());
}

void currency_test::test_round_double()
{
    double d0 = 123.99999999999;
    currency c0 = round_to_nearest_cent.c(d0);
    LMI_TEST_EQUAL(12400, c0.m_);
    double d1 = 1.0 + std::numeric_limits<double>::epsilon();
    currency c1 = round_to_nearest_cent.c(d1);
    LMI_TEST_EQUAL(100, c1.m_);
    double d2 = 1.0 - std::numeric_limits<double>::epsilon();
    currency c2 = round_to_nearest_cent.c(d2);
    LMI_TEST_EQUAL(100, c2.m_);
}

void currency_test::test_round_currency()
{
}

void currency_test::mete_humongous()
{
    static constexpr double d0 = std::numeric_limits<double>::max();
    static currency const extreme = from_cents(d0);
    static currency const value   = from_cents(1234567);
    for(int i = 0; i < 100000; ++i)
        {
        currency volatile z = std::min(extreme, value);
        }
}

void currency_test::mete_infinite()
{
    static constexpr double d0 = std::numeric_limits<double>::infinity();
    static currency const extreme = from_cents(d0);
    static currency const value   = from_cents(1234567);
    for(int i = 0; i < 100000; ++i)
        {
        currency volatile z = std::min(extreme, value);
        }
}

void currency_test::test_infinite()
{
    double const d0 = std::numeric_limits<double>::infinity();

    // Negative infinity.
    currency const c0(-d0, raw_cents{});
    LMI_TEST_EQUAL(-d0, c0.d());
    LMI_TEST_EQUAL(-d0, dblize(c0));

    // Test with from_cents(arg), which asserts that arg==rint(arg).
    // Pedantically speaking, that assertion depends on rint(INF),
    // which is implementation-defined, but what other result can
    // rint(INF) reasonably return than INF?
    currency const c1 = from_cents(-d0);
    LMI_TEST( c0 ==  c1);
    LMI_TEST(c1 < from_cents(1.0e100));

    // Positive infinity.
    currency const c2 = from_cents(d0);
    LMI_TEST_EQUAL(d0, c2.d());
    LMI_TEST_EQUAL(d0, dblize(c2));

    LMI_TEST(-c0 ==  c2);
    LMI_TEST( c0 == -c2);
    LMI_TEST(from_cents(1.0e100) < c2);

    std::ostringstream oss;
    oss << c1;
    LMI_TEST_EQUAL("-inf", oss.str());
    oss.str("");
    oss.clear();
    oss << c2;
    LMI_TEST_EQUAL("inf", oss.str());

    // Often lmi uses an identity element for std::min or std::max.
    // For example, a monthly charge might apply only to amounts up
    // to a given limit, thus:
    //   charge = std::min(amount, limit);
    // In cases where no limit applies, it is convenient to set that
    // limit to an enormous value, so that the expression above does
    // the right thing; but for that purpose, is the largest finite
    // representable value faster than infinity? At least with 32-
    // and 64-bit gcc-8 and -10, this test says "no".
    std::cout
        << "\n  Speed tests..."
        << "\n  humongous: " << TimeAnAliquot(mete_humongous)
        << "\n  infinite : " << TimeAnAliquot(mete_infinite )
        << std::endl
        ;
}

void currency_test::test_quodlibet()
{
    currency const a0(325, raw_cents{});
    LMI_TEST_EQUAL(3.25, a0.d());
    LMI_TEST_EQUAL(3.25, dblize(a0));
    currency       a1(475, raw_cents{});
    LMI_TEST_EQUAL(4.75, a1.d());
    LMI_TEST_EQUAL(4.75, dblize(a1));
    currency const a2 = from_cents(125);
    LMI_TEST_EQUAL(1.25, dblize(a2));

    currency b0 = round_to_nearest_cent.c(464.180000000000006821);
    currency b1 = round_to_nearest_cent.c(263.01999999999998181);
    currency b2 = round_to_nearest_cent.c(0.0);
    b2 += b0;
    b2 += b1;
    currency b3 = b0 + b1;
    LMI_TEST_EQUAL(b2.cents(), b3.cents());
    LMI_TEST_EQUAL(b2, b3);
}

// CURRENCY !! Ideas for testing overflow or underflow.
#if 0
//  double big_num = nonstd::power(2.0, 53);
    double big_num = 1.0e100;
    currency::data_type big_int1 =   1.0 * big_num;
    LMI_TEST_EQUAL(1.0e100, big_int1);
    currency::data_type big_int2 =  10.0 * big_num;
    LMI_TEST_EQUAL(1.0e101, big_int2);
    currency::data_type big_int3 = 100.0 * big_num;
    LMI_TEST_EQUAL(1.0e102, big_int3);
    round_to_nearest_cent.c(d0);
    std::cout << std::fixed;
std::cout << big_int3 << '\n' << 1.0e102 << '\n' << big_int3 - 1.0e102 << std::endl;

    LMI_TEST_THROW
        (round_to_nearest_cent.c(big_num / 1000.0)
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    double too_big = std::numeric_limits<double>::max();
    LMI_TEST_THROW
        (round_to_nearest_cent.c(too_big)
        ,std::runtime_error
//      ,"Cast would transgress upper limit."
        ,"Cannot cast infinite to integral."
        );
#endif // 0

#endif // defined USE_CURRENCY_CLASS

int test_main(int, char*[])
{
    currency_test::test();

    return EXIT_SUCCESS;
}
