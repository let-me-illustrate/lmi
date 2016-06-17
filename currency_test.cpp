// Currency amounts--unit test.
//
// Copyright (C) 2016 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "currency.hpp"

#include "test_tools.hpp"

#include <limits>
#include <sstream>
#include <stdexcept>

void test_ctors()
{
    BOOST_TEST_EQUAL(currency().total_subunits(), 0);
    BOOST_TEST_EQUAL(currency(0, 99).total_subunits(), 99);
    BOOST_TEST_EQUAL(currency(1, 99).total_subunits(), 199);

    currency const c(4, 56);
    BOOST_TEST_EQUAL(currency(c).total_subunits(), 456);

    static char const* const range_error = "Currency amount out of range.";
    BOOST_TEST_THROW(currency(-1, 0), std::overflow_error, range_error);
    BOOST_TEST_THROW(currency(-1, 99), std::overflow_error, range_error);
    BOOST_TEST_THROW(currency(-1, -99), std::overflow_error, range_error);
    BOOST_TEST_THROW
        (currency(std::numeric_limits<currency::amount_type>::max(), 0)
        ,std::overflow_error
        ,range_error
        );
    BOOST_TEST_THROW
        (currency(std::numeric_limits<currency::amount_type>::min(), 0)
        ,std::overflow_error
        ,range_error
        );

    static char const* const subunits_error = "Invalid amount of currency subunits.";
    BOOST_TEST_THROW(currency(1, 100), std::runtime_error, subunits_error);
    BOOST_TEST_THROW(currency(1, 101), std::runtime_error, subunits_error);
    BOOST_TEST_THROW(currency(1, -1), std::runtime_error, subunits_error);
}

void test_accessors()
{
    auto c = currency(1234, 56);
    BOOST_TEST_EQUAL(c.units(), 1234);
    BOOST_TEST_EQUAL(c.subunits(), 56);

    c = -currency(9876543, 21);
    BOOST_TEST_EQUAL(c.units(), -9876543);
    BOOST_TEST_EQUAL(c.subunits(), -21);

    c = -currency(0, 99);
    BOOST_TEST_EQUAL(c.units(), 0);
    BOOST_TEST_EQUAL(c.subunits(), -99);

    c = -c;
    BOOST_TEST_EQUAL(c.units(), 0);
    BOOST_TEST_EQUAL(c.subunits(), 99);
}

void test_comparison()
{
    BOOST_TEST( currency(1, 23) < currency(1, 24) );
    BOOST_TEST( -currency(1, 23) > -currency(1, 24) );

    BOOST_TEST( currency(1, 23) <= currency(1, 23) );
    BOOST_TEST( currency(1, 23) == currency(1, 23) );
    BOOST_TEST( currency(1, 23) != currency(1, 24) );
    BOOST_TEST( currency(1, 23) >= currency(1, 23) );
}

void test_arithmetic()
{
    auto c = currency(1, 23) + currency(4, 77);
    BOOST_TEST_EQUAL(c.total_subunits(), 600);

    c *= 12;
    BOOST_TEST_EQUAL(c.total_subunits(), 7200);

    auto d = c - currency(80, 10);
    BOOST_TEST_EQUAL(d.total_subunits(), -810);
}

void test_double()
{
    BOOST_TEST_EQUAL(currency::from_value(1.23).total_subunits(), 123);
    BOOST_TEST_EQUAL(currency::from_value(-1.23).total_subunits(), -123);

    BOOST_TEST_EQUAL(currency::from_value(0.005).total_subunits(), 1);
    BOOST_TEST_EQUAL(currency::from_value(-0.005).total_subunits(), -1);

    auto c = currency::from_value(14857345.859999999404);
    BOOST_TEST_EQUAL(c.total_subunits() ,1485734586);
    BOOST_TEST_EQUAL(c.value(), 14857345.86);
}

void test_stream_roundtrip
    (currency c0
    ,std::string const& str
    ,char const* file
    ,int line
    )
{
    std::stringstream ss;
    currency c;

    ss << c0;
    INVOKE_BOOST_TEST_EQUAL(ss.str(), str, file, line);
    ss >> c;
    INVOKE_BOOST_TEST( ss.eof (), file, line);
    INVOKE_BOOST_TEST(!ss.fail(), file, line);
    INVOKE_BOOST_TEST(!ss.bad (), file, line);
    INVOKE_BOOST_TEST_EQUAL(c, c0, file, line);
}

void test_streams()
{
    #define TEST_ROUNDTRIP(c, str) \
        test_stream_roundtrip(c, str, __FILE__, __LINE__)

    TEST_ROUNDTRIP( currency(123, 45),  "123.45");
    TEST_ROUNDTRIP( currency(  0,  0),    "0.00");
    TEST_ROUNDTRIP( currency(  0,  1),    "0.01");
    TEST_ROUNDTRIP( currency(  0, 99),    "0.99");
    TEST_ROUNDTRIP(-currency(123, 45), "-123.45");
    TEST_ROUNDTRIP(-currency(  0,  1),   "-0.01");
    TEST_ROUNDTRIP(-currency(  0, 99),   "-0.99");

    #undef TEST_ROUNDTRIP
}

int test_main(int, char*[])
{
    test_ctors();
    test_accessors();
    test_comparison();
    test_arithmetic();
    test_double();
    test_streams();

    return EXIT_SUCCESS;
}
