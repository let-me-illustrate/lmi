// Represent a currency amount exactly as integral cents--unit test.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "test_tools.hpp"

#include <limits>

class currency_test
{
  public:
    static void test();

  private:
    static void test_something();
};

void currency_test::test()
{
    test_something();
}

void currency_test::test_something()
{
    currency const a0;
    std::cout << a0 << std::endl;
    BOOST_TEST(0.00 == a0.d());
    BOOST_TEST(   0 == a0.m_);

// Figure out what to do about this:
//  currency a1(3.14);

    currency a1(3.25);
    BOOST_TEST(3.25 == a1.d());
    BOOST_TEST( 325 == a1.m_);
    a1 += a1;
    BOOST_TEST(6.50 == a1.d());
    BOOST_TEST( 650 == a1.m_);

    currency a2 = currency(0.0) - a1;
    BOOST_TEST(-6.50 == a2.d());
    BOOST_TEST( -650 == a2.m_);

    double d0 = 123.99999999999;
    currency c0(d0);
std::cout << c0 << " converted from 123.999..." << std::endl;
    double d1 = 1.0 + std::numeric_limits<double>::epsilon();
    currency c1(d1);
std::cout << c1 << " converted from 1.0 + epsilon..." << std::endl;
    double d2 = 1.0 - std::numeric_limits<double>::epsilon();
    currency c2(d2);
std::cout << c2 << " converted from 1.0 - epsilon..." << std::endl;

    double big_num = 1.0e100;
#pragma GCC diagnostic ignored "-Wfloat-conversion"
    currency::data_type big_int0 = big_num;
std::cout << "int0: " << big_int0 << std::endl;
    currency::data_type big_int1 = 1.0 * big_num;
std::cout << "int1: " << big_int1 << std::endl;
    currency::data_type big_int2 = 10.0 * big_num;
std::cout << "int2: " << big_int2 << std::endl;
    currency::data_type big_int3 = 100.0 * big_num;
std::cout << "int3: " << big_int3 << std::endl;
    currency::data_type big_int4 = round(100.0 * big_num);
std::cout << "int4: " << big_int4 << std::endl;

    currency a3(big_num / 1000.0);
std::cout << a3 << std::endl;
std::cout << big_num << std::endl;
std::cout << "rounded: " << round(big_num) << std::endl;
    double too_big = std::numeric_limits<double>::max();
    currency a4(too_big);
std::cout << a4 << std::endl;
std::cout << too_big << std::endl;
std::cout << "rounded: " << round(too_big) << std::endl;
std::cout << 100.0 * too_big << std::endl;

    currency b0(464.180000000000006821);
    currency b1(263.01999999999998181);
    currency b2(0.0);
    b2 += b0;
    b2 += b1;
    currency b3 = b0 + b1;
    BOOST_TEST_EQUAL(b2.m(), b3.m());
    BOOST_TEST_EQUAL(b2, b3);
}

int test_main(int, char*[])
{
    currency_test::test();

    return EXIT_SUCCESS;
}
