// Converstion to and from strings--unit test.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: value_cast_test.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "value_cast_ihs.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <cmath>     // std::pow()
#include <cstring>   // std::strcmp(), std::strcpy()
#include <exception>
#include <ios>
#include <iostream>
#include <stdio.h> // snprintf()
#include <string>

std::string strip(std::string numeric_string)
{
    long double d;
    d = value_cast_ihs<long double>(numeric_string);
    return value_cast_ihs<std::string>(d);
}

int test_main(int, char*[])
{
    char const* p = "31";
    BOOST_TEST(31 == value_cast_ihs<int   >(p));
    BOOST_TEST(31 == value_cast_ihs<double>(p));

    char q[0x0100];
    std::strcpy(q, value_cast_ihs<std::string>(0).c_str());
    BOOST_TEST(0 == std::strcmp(q, "0"));
    int i = 0;
    std::strcpy(q, value_cast_ihs<std::string>(i).c_str());
    BOOST_TEST(0 == std::strcmp(q, "0"));
    std::strcpy(q, value_cast_ihs<std::string>(31.0).c_str());
    BOOST_TEST(0 == std::strcmp(q, "31"));

    BOOST_TEST("31"  == value_cast_ihs<std::string>(31));
    BOOST_TEST("310" == value_cast_ihs<std::string>(310));
    BOOST_TEST("31"  == value_cast_ihs<std::string>(31.0));
    BOOST_TEST("310" == value_cast_ihs<std::string>(310.0));
    BOOST_TEST("0"   == value_cast_ihs<std::string>(.0));

    double d;
    d =         130000000000000.0;
    BOOST_TEST("130000000000000" == value_cast_ihs<std::string>(d));
    d =         1300000000000000.0;
    BOOST_TEST("1300000000000000" == value_cast_ihs<std::string>(d));

    BOOST_TEST("1233"   == strip("1233"));
    BOOST_TEST("1230"   == strip("1230"));
    BOOST_TEST("1230"   == strip("1230."));
    BOOST_TEST("1230"   == strip("1230.0"));
    BOOST_TEST("123"    == strip("123.0"));
    BOOST_TEST("123.3"  == strip("123.30"));
    BOOST_TEST("123.3"  == strip("123.3"));
/* These tests have no particular right to succeed:
    BOOST_TEST("1.233"  == strip("1.233"));
    BOOST_TEST("0.1233" == strip(".1233"));
    BOOST_TEST("0.1233" == strip("0.1233"));
    BOOST_TEST("0.1233" == strip("0.123300"));

std::cout << strip("1.233") << '\n';
std::cout << strip(".1233") << '\n';
std::cout << strip("0.1233") << '\n';
std::cout << strip("0.123300") << '\n';

    BOOST_TEST
        (        "0.000000000000000000001233"
        == strip("0.000000000000000000001233")
        );
    BOOST_TEST
        (        "0.000000000000000000001233"
        == strip( ".00000000000000000000123300000000")
        );
*/

    BOOST_TEST("0"      == strip("0."));
    BOOST_TEST("0"      == strip(".0"));
    BOOST_TEST("0"      == strip("0.0"));
    BOOST_TEST("0"      == strip("00.00"));

    std::string a("1.2");
    std::string b("3.4 777");
    char c[256] = "This is a test.";
    d = 3.14159;

    a = value_cast_ihs<std::string>(b);
    BOOST_TEST("3.4 777" == a);

    a = value_cast_ihs<std::string>(d);
    BOOST_TEST("3.14159" == a);

    int return_value = -1;

    try
        {
        // This should throw:
        value_cast_ihs<double>(b);
        // This shouldn't be reached:
        BOOST_TEST(false);
        }
    catch(std::exception const& e)
        {
        return_value = 0;
        }

    try
        {
        // TODO ?? bc++5.02 fails here.
        // This should throw:
        value_cast_ihs<double>("");
        // This shouldn't be reached:
        BOOST_TEST(false);
        }
    catch(std::exception const& e)
        {
        // TODO ?? This idiom is silly. Remove it everywhere.
        return_value = 0;
        }

    d = value_cast_ihs<double>(a);
    BOOST_TEST(3.14159 == d);

    a = value_cast_ihs<std::string>(d);
    BOOST_TEST("3.14159" == a);

    a = value_cast_ihs<std::string>(c);
    BOOST_TEST("This is a test." == a);

    b = value_cast_ihs<std::string>(a);
    BOOST_TEST("This is a test." == b);

/* TODO ?? This blows up mingw gcc-3.2.3:
    d = 1e+161;
    a = value_cast_ihs<std::string>(d);
    BOOST_TEST(d == value_cast_ihs<double>(a));
*/

    // TODO ?? We'd like to be able to do something like this, but this
    // particular number probably is not exactly representable:
//    std::string e
//        ("1"
//        "00000000000000000000000000000000000000000000000000"
//        "00000000000000000000000000000000000000000000000000"
//        "00000000000000000000000000000000000000000000000000"
//        "00000000000"
//        );
//    BOOST_TEST(e == a);

    // TODO ?? Should we offer a 'fail' macro that just prints a string?
    BOOST_TEST(0 == std::string("MinGW gcc-3.2.3 segfaults here").size());
/* TODO ?? MinGW gcc-3.2.3 segfaults here:
    // A big number that must be representable as a finite
    // floating-point number [18.2.1.2/27].
    double big = std::pow
        (static_cast<double>(std::numeric_limits<double>::radix)
        ,std::numeric_limits<double>::max_exponent - 1
        );
    BOOST_TEST( big == value_cast_ihs<double>(value_cast_ihs<std::string>( big)));
    BOOST_TEST(-big == value_cast_ihs<double>(value_cast_ihs<std::string>(-big)));
*/

    // An empty string should be convertible to string without error.
    std::string s;
    BOOST_TEST(value_cast_ihs<std::string>(s).empty());
    std::string const& s_const_ref(s);
    BOOST_TEST(value_cast_ihs<std::string>(s_const_ref).empty());
// TODO ?? Melts your computer.
//    char const* cs ='\0';
//    BOOST_TEST(value_cast_ihs<std::string>(cs).empty());

#if 0
// TODO ?? All our compilers fail this test; is it valid?
    // A small number that must be representable as a normalized
    // floating-point number [18.2.1.2/23].
    double small = std::pow
        (std::numeric_limits<double>::radix
        ,std::numeric_limits<double>::min_exponent - 1
        );
    BOOST_TEST( small == value_cast_ihs<double>(value_cast_ihs<std::string>( small)));
    BOOST_TEST(-small == value_cast_ihs<double>(value_cast_ihs<std::string>(-small)));
std::cout << small << '\n';
std::cout << value_cast_ihs<std::string>( small) << '\n';
std::cout << value_cast_ihs<double>(value_cast_ihs<std::string>( small)) << '\n';
#endif // 0

    return return_value;
}

