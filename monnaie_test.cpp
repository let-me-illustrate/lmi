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

#include "monnaie.hpp"

#include "bourn_cast.hpp"
#include "materially_equal.hpp"
#include "miscellany.hpp"               // stifle_warning_for_unused_value()
#include "test_tools.hpp"
#include "timer.hpp"

#include <cmath>                        // floor()
#include <limits>
#include <sstream>
#include <stdexcept>

class monnaie_test
{
  public:
    static void test();

    /// An arbitrary amount that is quasi-volatile.
    ///
    /// The return type is 'T', not 'T volatile'. Instantiations are
    /// written so that the value returned cannot be computed at
    /// compile time and calculations involving it therefore cannot
    /// be eliminated by optimization.
    ///
    /// This function template is a member so that its specialization
    /// for class monnaie can call a private constructor.

    template<typename T>
    static T arbitrary_amount();

  private:
    static void test_ctors();
    static void test_accessors();
    static void test_comparison();
    static void test_arithmetic();
    static void test_double();
    static void test_streams();
    static void test_speed();
};

void monnaie_test::test()
{
    test_ctors();
    test_accessors();
    test_comparison();
    test_arithmetic();
    test_double();
    test_streams();
    test_speed();
}

void monnaie_test::test_ctors()
{
    LMI_TEST_EQUAL(monnaie(     ).total_cents(),   0);
    LMI_TEST_EQUAL(monnaie(0, 99).total_cents(),  99);
    LMI_TEST_EQUAL(monnaie(1, 99).total_cents(), 199);

    monnaie const c(4, 56);
    LMI_TEST_EQUAL(monnaie(c).total_cents(), 456);

    static char const* const overflow_msg = "Currency amount out of range.";
    LMI_TEST_THROW(monnaie(-1,   0), std::overflow_error, overflow_msg);
    LMI_TEST_THROW(monnaie(-1,  99), std::overflow_error, overflow_msg);
    LMI_TEST_THROW(monnaie(-1, -99), std::overflow_error, overflow_msg);
    LMI_TEST_THROW
        (monnaie(std::numeric_limits<monnaie::amount_type>::max(), 0)
        ,std::overflow_error
        ,overflow_msg
        );
    // Make sure min() means lowest().
    static_assert
        (  std::numeric_limits<monnaie::amount_type>::min()
        == std::numeric_limits<monnaie::amount_type>::lowest()
        );
    LMI_TEST_THROW
        (monnaie(std::numeric_limits<monnaie::amount_type>::min(), 0)
        ,std::overflow_error
        ,overflow_msg
        );

    static char const* const cents_msg = "Invalid number of cents.";
    LMI_TEST_THROW(monnaie(1, 100), std::runtime_error, cents_msg);
    LMI_TEST_THROW(monnaie(1, 101), std::runtime_error, cents_msg);
    LMI_TEST_THROW(monnaie(1,  -1), std::runtime_error, cents_msg);
}

void monnaie_test::test_accessors()
{
    auto c = monnaie(1234, 56);
    LMI_TEST_EQUAL(c.dollars(), 1234);
    LMI_TEST_EQUAL(c.cents()  , 56);

    c = -monnaie(9876543, 21);
    LMI_TEST_EQUAL(c.dollars(), -9876543);
    LMI_TEST_EQUAL(c.cents()  , -21);

    c = -monnaie(0, 99);
    LMI_TEST_EQUAL(c.dollars(), 0);
    LMI_TEST_EQUAL(c.cents()  , -99);

    c = -c;
    LMI_TEST_EQUAL(c.dollars(), 0);
    LMI_TEST_EQUAL(c.cents()  , 99);
}

void monnaie_test::test_comparison()
{
    LMI_TEST( monnaie(1, 23) <  monnaie(1, 24));
    LMI_TEST(-monnaie(1, 23) > -monnaie(1, 24));

    LMI_TEST( monnaie(1, 23) <= monnaie(1, 23));
    LMI_TEST( monnaie(1, 23) == monnaie(1, 23));
    LMI_TEST( monnaie(1, 23) != monnaie(1, 24));
    LMI_TEST( monnaie(1, 23) >= monnaie(1, 23));
}

void monnaie_test::test_arithmetic()
{
    auto c = monnaie(1, 23) + monnaie(4, 77);
    LMI_TEST_EQUAL(c.total_cents(), 600);

    c *= 12;
    LMI_TEST_EQUAL(c.total_cents(), 7200);

    // $72.00 - $80.10 = $8.10
    auto d = c - monnaie(80, 10);
    LMI_TEST_EQUAL(d.total_cents(), -810);
}

void monnaie_test::test_double()
{
    LMI_TEST_EQUAL(monnaie::from_value( 1.23).total_cents(),  123);
    LMI_TEST_EQUAL(monnaie::from_value(-1.23).total_cents(), -123);

    LMI_TEST_EQUAL(monnaie::from_value( 0.005).total_cents(),  1);
    LMI_TEST_EQUAL(monnaie::from_value(-0.005).total_cents(), -1);

    auto c = monnaie::from_value(          14857345.859999999404);
    LMI_TEST_EQUAL(c.total_cents()     , 1485734586);
    LMI_TEST(materially_equal(c.value(), 14857345.86));
}

void test_stream_roundtrip
    (monnaie c0
    ,std::string const& str
    ,char const* file
    ,int line
    )
{
    std::stringstream ss;
    monnaie c;

    ss << c0;
    INVOKE_LMI_TEST_EQUAL(ss.str(), str, file, line);
    ss >> c;
    INVOKE_LMI_TEST( ss.eof (), file, line);
    INVOKE_LMI_TEST(!ss.fail(), file, line);
    INVOKE_LMI_TEST(!ss.bad (), file, line);
    INVOKE_LMI_TEST_EQUAL(c, c0, file, line);
}

void monnaie_test::test_streams()
{
#define TEST_ROUNDTRIP(c, str) test_stream_roundtrip(c, str, __FILE__, __LINE__)
    TEST_ROUNDTRIP( monnaie(123, 45),  "123.45");
    TEST_ROUNDTRIP( monnaie(  0,  0),    "0.00");
    TEST_ROUNDTRIP( monnaie(  0,  1),    "0.01");
    TEST_ROUNDTRIP( monnaie(  0, 99),    "0.99");
    TEST_ROUNDTRIP(-monnaie(123, 45), "-123.45");
    TEST_ROUNDTRIP(-monnaie(  0,  1),   "-0.01");
    TEST_ROUNDTRIP(-monnaie(  0, 99),   "-0.99");
#undef TEST_ROUNDTRIP
}

template<>
double monnaie_test::arbitrary_amount<double>()
{
    double volatile z(1.23);
    return z;
}

template<>
monnaie::amount_type monnaie_test::arbitrary_amount<monnaie::amount_type>()
{
    monnaie::amount_type volatile z(123);
    return z;
}

/// An arbitrary monnaie amount that is quasi-volatile.
///
/// The local monnaie::amount_type variable is volatile, but the
/// monnaie object returned is not volatile. It can't be, because
/// class monnaie is not "volatile-correct", and there's no present
/// need to make it so.
///
/// However, the monnaie value represented by the object returned is
/// "volatile" in the sense that the compiler cannot presume to know
/// it, so it can't be precomputed at compile time, and calculations
/// involving it cannot be optimized into oblivion.

template<>
monnaie monnaie_test::arbitrary_amount<monnaie>()
{
    monnaie::amount_type volatile z(123);
    return monnaie(z);
}

template<typename T>
inline double convert_to_double(T t)
{
    return bourn_cast<double>(t);
}

template<>
inline double convert_to_double(monnaie c)
{
    return c.value();
}

template<typename T>
inline T convert_from_double(double d)
{
    return bourn_cast<T>(d);
}

template<>
inline monnaie convert_from_double<monnaie>(double d)
{
    return monnaie::from_value(d);
}

template<typename T>
void do_some_arithmetic(T t)
{
    T a(monnaie_test::arbitrary_amount<T>());
    T b(monnaie_test::arbitrary_amount<T>());
    T c(monnaie_test::arbitrary_amount<T>());
    T d(monnaie_test::arbitrary_amount<T>());
    T e(monnaie_test::arbitrary_amount<T>());
    T f(monnaie_test::arbitrary_amount<T>());

    for(int j = 0; j < 1000; ++j)
        {
        T u(a + b);
        t += u;
        u += u;
        t += u - c + d - e;
        u += t - f;
        u = convert_from_double<T>(std::floor(convert_to_double(u) * 1.03));
        T volatile v(u);
        stifle_warning_for_unused_value(v);
        }

    T volatile w(t);
    stifle_warning_for_unused_value(w);
}

void mete_double()
{
    double d(12345.67);
    for(int j = 0; j < 1000; ++j)
        {
        do_some_arithmetic(d);
        }
}

void mete_amount_type()
{
    monnaie::amount_type a(1234567);
    for(int j = 0; j < 1000; ++j)
        {
        do_some_arithmetic(a);
        }
}

void mete_monnaie()
{
    monnaie c(12345, 67);
    for(int j = 0; j < 1000; ++j)
        {
        do_some_arithmetic(c);
        }
}

void monnaie_test::test_speed()
{
    std::cout
        << "  Speed tests..."
        << "\n  double     : " << TimeAnAliquot(mete_double)
        << "\n  amount_type: " << TimeAnAliquot(mete_amount_type)
        << "\n  monnaie    : " << TimeAnAliquot(mete_monnaie)
        << std::endl
        ;
}

int test_main(int, char*[])
{
    monnaie_test::test();

    return EXIT_SUCCESS;
}
