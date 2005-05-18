// General conversion between types--unit test.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: value_cast_test.cpp,v 1.5 2005-05-18 23:57:25 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "value_cast.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <cmath>   // std::pow()
#include <cstring> // std::strcpy(), std::strcmp

struct X {std::string s;};
std::istream& operator>>(std::istream& is, X&       x) {is >> x.s; return is;}
std::ostream& operator<<(std::ostream& os, X const& x) {os << x.s; return os;}

template<typename To, typename From>
cast_method method(To, From)
{
    return value_cast_chooser<To,From>::method();
}

int extra_tests0();
int extra_tests1();

int test_main(int, char*[])
{
    // These could be static assertions, but any failure would prevent
    // other tests from running.

#ifndef __BORLANDC__
    BOOST_TEST(is_string<char*>::value);
    BOOST_TEST(is_string<char const*>::value);

    BOOST_TEST(is_string<std::string>::value);
    BOOST_TEST(is_string<std::string&>::value);
    BOOST_TEST(is_string<std::string const>::value);
    BOOST_TEST(is_string<std::string const&>::value);
#endif // __BORLANDC__ not defined.

// These tests fail to compile:
//    BOOST_TEST(is_string<std::string volatile>::value);
//    BOOST_TEST(is_string<std::string const volatile&>::value);

    char const* ccp = "2.71828";
    char* cp = const_cast<char*>("3.14159");
    int i(2);
    double d(1.23456);
    std::string s("test");
    X x;

#ifndef __BORLANDC__
    // Test which conversion is used for type double.

    BOOST_TEST_EQUAL(e_direct,  method(d, d));
    BOOST_TEST_EQUAL(e_direct,  method(d, i));
    BOOST_TEST_EQUAL(e_numeric, method(d, s));
    BOOST_TEST_EQUAL(e_numeric, method(d, cp));
    BOOST_TEST_EQUAL(e_numeric, method(d, ccp));
    BOOST_TEST_EQUAL(e_stream,  method(d, x));

    BOOST_TEST_EQUAL(e_direct,  method(d, d));
    BOOST_TEST_EQUAL(e_direct,  method(i, d));
    BOOST_TEST_EQUAL(e_numeric, method(s, d));
    BOOST_TEST_EQUAL(e_numeric, method(cp, d));
    BOOST_TEST_EQUAL(e_numeric, method(ccp, d));
    BOOST_TEST_EQUAL(e_stream,  method(x, d));

    // Test which conversion is used for type std::string.

    BOOST_TEST_EQUAL(e_numeric, method(s, d));
    BOOST_TEST_EQUAL(e_numeric, method(s, i));
    BOOST_TEST_EQUAL(e_direct,  method(s, s));
    BOOST_TEST_EQUAL(e_direct,  method(s, ccp));
    BOOST_TEST_EQUAL(e_stream,  method(s, x));

    BOOST_TEST_EQUAL(e_numeric, method(d, s));
    BOOST_TEST_EQUAL(e_numeric, method(i, s));
    BOOST_TEST_EQUAL(e_direct,  method(s, s));
    BOOST_TEST_EQUAL(e_stream,  method(ccp, s));
    BOOST_TEST_EQUAL(e_stream,  method(x, s));
#else // __BORLANDC__ defined.
    // Shut up compiler warnings for unused variables.
    &cp;
#endif // __BORLANDC__ defined.

// INELEGANT !! This is forbidden, but perhaps should be allowed:
//    cp = value_cast(d, cp);
//    BOOST_TEST_EQUAL(cp, std::string("3.14159"));

// TODO ?? This is allowed, but should be forbidden:
//    cp = value_cast(ccp, cp);

    x = value_cast(s, x);
    BOOST_TEST_EQUAL(x.s, "test");

    d = value_cast(ccp, d);
    BOOST_TEST_EQUAL(d, 2.71828);

    // A decent compiler would give a warning here:
//    i = value_cast<int>(d);
    // Alternatively, boost::numeric_cast might be used to convert
    // between arithmetic types.

    d = value_cast(i, d);
    BOOST_TEST_EQUAL(d, 2.0);

    s = value_cast<std::string>(double(2.0 / 3.0));
    BOOST_TEST_EQUAL(s, "0.666666666666667");

    d = value_cast<double>(s);
    BOOST_TEST_EQUAL(s, value_cast<std::string>(d));

    std::string t("This is a test.");
    BOOST_TEST_EQUAL(t, value_cast<std::string>(t));

    BOOST_TEST_EQUAL(0, extra_tests0());
    BOOST_TEST_EQUAL(0, extra_tests1());

    return 0;
}

std::string strip(std::string numeric_string)
{
    double d;
    d = value_cast<double>(numeric_string);
    return value_cast<std::string>(d);
}

int extra_tests0()
{
    char const* p = "31";
    BOOST_TEST_EQUAL(31, value_cast<int   >(p));
    BOOST_TEST_EQUAL(31, value_cast<double>(p));

    char q[0x0100];
    std::strcpy(q, value_cast<std::string>(0).c_str());
    BOOST_TEST_EQUAL(0, std::strcmp(q, "0"));
    int i = 0;
    std::strcpy(q, value_cast<std::string>(i).c_str());
    BOOST_TEST_EQUAL(0, std::strcmp(q, "0"));
    std::strcpy(q, value_cast<std::string>(31.0).c_str());
    BOOST_TEST_EQUAL(0, std::strcmp(q, "31"));

    BOOST_TEST_EQUAL("31" , value_cast<std::string>(31));
    BOOST_TEST_EQUAL("310", value_cast<std::string>(310));
    BOOST_TEST_EQUAL("31" , value_cast<std::string>(31.0));
    BOOST_TEST_EQUAL("310", value_cast<std::string>(310.0));
    BOOST_TEST_EQUAL("0"  , value_cast<std::string>(.0));

    double d;
    d =  130000000000000.0;
    BOOST_TEST_EQUAL( "130000000000000", value_cast<std::string>(d));
    d = 1300000000000000.0;
    BOOST_TEST_EQUAL("1300000000000000", value_cast<std::string>(d));

    BOOST_TEST_EQUAL("1233"  , strip("1233"));
    BOOST_TEST_EQUAL("1230"  , strip("1230"));
    BOOST_TEST_EQUAL("1230"  , strip("1230."));
    BOOST_TEST_EQUAL("1230"  , strip("1230.0"));
    BOOST_TEST_EQUAL("123"   , strip("123.0"));
    BOOST_TEST_EQUAL("123.3" , strip("123.30"));
    BOOST_TEST_EQUAL("123.3" , strip("123.3"));
    BOOST_TEST_EQUAL("1.233" , strip("1.233"));
    BOOST_TEST_EQUAL("0.1233", strip(".1233"));
    BOOST_TEST_EQUAL("0.1233", strip("0.1233"));
    BOOST_TEST_EQUAL("0.1233", strip("0.123300"));

    BOOST_TEST_EQUAL
        (      "0.000000000000000000001233"
        ,strip("0.000000000000000000001233")
        );
    BOOST_TEST_EQUAL
        (      "0.000000000000000000001233"
        ,strip( ".00000000000000000000123300000000")
        );

    char const* nptr = "0.";
//    char rendptr[100] = {'\0'};
//    char** endptr = &rendptr;
//    std::strtod(nptr, endptr);

    char rendptr[100] = {'\0'};
    char* endptr = rendptr;
    std::strtod(nptr, &endptr);
    BOOST_TEST_EQUAL('\0', *endptr);
    BOOST_TEST_UNEQUAL(nptr, endptr);

    BOOST_TEST_EQUAL("0", strip("00."  ));
    BOOST_TEST_EQUAL("0", strip( "0."  ));
    BOOST_TEST_EQUAL("0", strip(  ".0" ));
    BOOST_TEST_EQUAL("0", strip( "0.0" ));
    BOOST_TEST_EQUAL("0", strip("00.00"));

    std::string a("1.2");
    std::string b("3.4 777");
    char c[256] = "This is a test.";
    d = 3.14159;

    a = value_cast<std::string>(b);
    BOOST_TEST_EQUAL("3.4 777", a);

    a = value_cast<std::string>(d);
    BOOST_TEST_EQUAL("3.14159", a);

    BOOST_TEST_THROW(value_cast<double>(b) , std::invalid_argument, "");
    BOOST_TEST_THROW(value_cast<double>(""), std::invalid_argument, "");

    d = value_cast<double>(a);
    BOOST_TEST_EQUAL(3.14159, d);

    a = value_cast<std::string>(d);
    BOOST_TEST_EQUAL("3.14159", a);

    a = value_cast<std::string>(c);
    BOOST_TEST_EQUAL("This is a test.", a);

    b = value_cast<std::string>(a);
    BOOST_TEST_EQUAL("This is a test.", b);

    d = 1e+161;
    a = value_cast<std::string>(d);
    BOOST_TEST_EQUAL(d, value_cast<double>(a));

    std::string e
        ("1"
        "00000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000"
        "00000000000"
        );
    BOOST_TEST_EQUAL(e, a);

    // An empty string should be convertible to string without error.
    std::string s;
    BOOST_TEST(value_cast<std::string>(s).empty());
    std::string const& s_const_ref(s);
    BOOST_TEST(value_cast<std::string>(s_const_ref).empty());

    // A big number that must be representable as a finite
    // floating-point number [18.2.1.2/27].
    double big = std::pow
        (static_cast<double>(std::numeric_limits<double>::radix)
        ,static_cast<double>(std::numeric_limits<double>::max_exponent - 1)
        );
    BOOST_TEST_EQUAL( big, value_cast<double>(value_cast<std::string>( big)));
    BOOST_TEST_EQUAL(-big, value_cast<double>(value_cast<std::string>(-big)));

    // A small number that must be representable as a normalized
    // floating-point number [18.2.1.2/23].
    double small = std::pow
        (static_cast<double>(std::numeric_limits<double>::radix)
        ,static_cast<double>(std::numeric_limits<double>::min_exponent)
// TODO ?? Why doesn't this work with '- 1' appended?
//        ,static_cast<double>(std::numeric_limits<double>::min_exponent - 1)
        );
    BOOST_TEST_EQUAL( small, value_cast<double>(value_cast<std::string>( small)));
    BOOST_TEST_EQUAL(-small, value_cast<double>(value_cast<std::string>(-small)));

    return 0;
}

/// These extra tests were originally used for template function
/// stream_cast(), but represent cases for which value_cast() seems
/// preferable.

int extra_tests1()
{
    // A char is treated as an arithmetic type, not as an element of a
    // string literal: here, stream_cast() behaves differently.
    BOOST_TEST_UNEQUAL(" ", value_cast<std::string>(' '));

    std::string s;

    bool b0 = true;
    s = value_cast<std::string>(b0);
    BOOST_TEST_EQUAL("1", s);
    bool const b1 = false;
    s = value_cast(b1, s);
    BOOST_TEST_EQUAL("0", s);

    int i0 = 1234;
    s = value_cast<std::string>(i0);
    BOOST_TEST_EQUAL("1234", s);
    int const i1 = -4321;
    s = value_cast(i1, s);
    BOOST_TEST_EQUAL("-4321", s);

    double d0 = 1.5;
    s = value_cast<std::string>(d0);
    BOOST_TEST_EQUAL("1.5", s);
    double const d1 = -2.5;
    s = value_cast(d1, s);
    BOOST_TEST_EQUAL("-2.5", s);

    return 0;
}

