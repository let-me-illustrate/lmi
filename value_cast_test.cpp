// General conversion between types--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "value_cast.hpp"

#include "miscellany.hpp"
#include "test_tools.hpp"

#if !defined __BORLANDC__
#   include <boost/lexical_cast.hpp>
#endif // !defined __BORLANDC__

#include <cmath>   // std::pow()
#include <cstring> // std::strcpy(), std::strcmp
#include <istream>
#include <limits>
#include <ostream>

class NotDefaultConstructible
{
  public:
    NotDefaultConstructible(std::istream const&) {}
};

struct X {std::string s;};
std::istream& operator>>(std::istream& is, X&       x) {is >> x.s; return is;}
std::ostream& operator<<(std::ostream& os, X const& x) {os << x.s; return os;}

#if !defined __BORLANDC__
template<typename To, typename From>
cast_method method(From, To)
{
    return value_cast_chooser<To,From>::method();
}
#endif // !defined __BORLANDC__

int extra_tests0();
int extra_tests1();
int boost_tests();

int test_main(int, char*[])
{
    // These could be static assertions, but any failure would prevent
    // other tests from running.

#if !defined __BORLANDC__
    BOOST_TEST( is_string<char               *>::value);
    BOOST_TEST( is_string<char const         *>::value);
    BOOST_TEST(!is_string<char       volatile*>::value);
    BOOST_TEST(!is_string<char const volatile*>::value);

    BOOST_TEST(is_string<std::string       >::value);
    BOOST_TEST(is_string<std::string      &>::value);
    BOOST_TEST(is_string<std::string const >::value);
    BOOST_TEST(is_string<std::string const&>::value);
#endif // !defined __BORLANDC__

// These tests fail to compile:
//    BOOST_TEST(is_string<std::string volatile>::value);
//    BOOST_TEST(is_string<std::string const volatile&>::value);

    char const* ccp = "2.71828";
    char* cp = const_cast<char*>("3.14159");
    int i(2);
    double d(1.23456);
    std::string s("test");
    X x;
    NotDefaultConstructible n_d_c(std::cin);

#if !defined __BORLANDC__
    // Test which conversion is used for type double.

    BOOST_TEST_EQUAL(e_both_numeric ,method(d   ,d  ));
    BOOST_TEST_EQUAL(e_both_numeric ,method(d   ,i  ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(d   ,s  ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(d   ,cp ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(d   ,ccp));
    BOOST_TEST_EQUAL(e_stream       ,method(d   ,x  ));

    BOOST_TEST_EQUAL(e_both_numeric ,method(d   ,d  ));
    BOOST_TEST_EQUAL(e_both_numeric ,method(i   ,d  ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(s   ,d  ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(cp  ,d  ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(ccp ,d  ));
    BOOST_TEST_EQUAL(e_stream       ,method(x   ,d  ));

    // Test which conversion is used for type std::string.

    BOOST_TEST_EQUAL(e_numeric_io   ,method(s   ,d  ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(s   ,i  ));
    BOOST_TEST_EQUAL(e_direct       ,method(s   ,s  ));
    BOOST_TEST_EQUAL(e_stream       ,method(s   ,ccp));
    BOOST_TEST_EQUAL(e_stream       ,method(s   ,x  ));

    BOOST_TEST_EQUAL(e_numeric_io   ,method(d   ,s  ));
    BOOST_TEST_EQUAL(e_numeric_io   ,method(i   ,s  ));
    BOOST_TEST_EQUAL(e_direct       ,method(s   ,s  ));
    BOOST_TEST_EQUAL(e_direct       ,method(ccp ,s  ));
    BOOST_TEST_EQUAL(e_stream       ,method(x   ,s  ));

    // Not convertible: stream_cast() forbids conversion to pointer.
    BOOST_TEST_EQUAL(e_stream       ,method(ccp ,cp ));

    // Not convertible: value_cast() forbids conversion to pointer.
    BOOST_TEST_EQUAL(e_direct       ,method(cp  ,ccp));

    // Not convertible: value_cast() forbids conversion to pointer.
    BOOST_TEST_EQUAL(e_stream       ,method(s, (char volatile*)(0)   ));

    BOOST_TEST_EQUAL(e_stream       ,method(   (char volatile*)(0), s));

    n_d_c = value_cast<NotDefaultConstructible>(n_d_c);
    n_d_c = value_cast(n_d_c, n_d_c);
#else  // defined __BORLANDC__
    stifle_warning_for_unused_variable(cp);
#endif // defined __BORLANDC__

    // Forbidden conversions to pointer, detected at compile time.
//    cp = value_cast(d, cp);
//    cp = value_cast(ccp, cp);
//    ccp = value_cast(cp, ccp);

    x = value_cast(s, x);
    BOOST_TEST_EQUAL(x.s, "test");

    d = value_cast(ccp, d);
    BOOST_TEST_EQUAL(d, 2.71828);

    // Forbidden narrowing conversions.

    // A good compiler should warn about this conversion.
//    value_cast<unsigned int>(-1);

    // The boost-1.31.0 documentation says:
    //   "An exception is thrown when a runtime value-preservation
    //   check fails."
    // yet this does not throw, even in boost-1.33.1, whose new
    // numeric_cast delegates to boost::numeric::converter:
    i = boost::numeric_cast<int>(2.71828);
    // but these do:
#if !defined __BORLANDC__
    BOOST_TEST_THROW
        (i = value_cast<int>(d)
        ,std::runtime_error
        ,"Value not preserved converting 2.71828 to 2 ."
        );
    BOOST_TEST_THROW
        (numeric_value_cast<int>(2.71828)
        ,std::runtime_error
        ,"Value not preserved converting 2.71828 to 2 ."
        );
#endif // !defined __BORLANDC__

    // This conversion should work: value is exactly preserved.

    i = value_cast<int>(2.0);
    BOOST_TEST_EQUAL(i, 2);

    d = value_cast(i, d);
    BOOST_TEST_EQUAL(d, 2.0);

    s = value_cast<std::string>(double(2.0 / 3.0));
    BOOST_TEST_EQUAL(s, "0.666666666666667");

    d = value_cast<double>(s);
    BOOST_TEST_EQUAL(s, value_cast<std::string>(d));

    d = value_cast<double>("123.456");
    BOOST_TEST_EQUAL(d, 123.456);

    std::string t("This is a test.");
    BOOST_TEST_EQUAL(t, value_cast<std::string>(t));

    BOOST_TEST_EQUAL(0, extra_tests0());
    BOOST_TEST_EQUAL(0, extra_tests1());
    BOOST_TEST_EQUAL(0, boost_tests());

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

#if !defined __BORLANDC__
    BOOST_TEST_THROW(value_cast<double>(b) , std::invalid_argument, "");
    BOOST_TEST_THROW(value_cast<double>(""), std::invalid_argument, "");

    BOOST_TEST_THROW
        (value_cast<std::string>((char*)(0))
        ,std::runtime_error
        ,"Null pointer holds no value to convert."
        );

    BOOST_TEST_THROW
        (value_cast<std::string>((char const*)(0))
        ,std::runtime_error
        ,"Null pointer holds no value to convert."
        );

    BOOST_TEST_EQUAL("0", value_cast<std::string>((char volatile*)(0)));
    BOOST_TEST_EQUAL("0", value_cast<std::string>((char const volatile*)(0)));

    // Numeric casts from
    //   (char               *)(0)
    //   (char       volatile*)(0)
    //   (char const volatile*)(0)
    // are forbidden by a compile-time assertion.
    BOOST_TEST_THROW
        (value_cast<unsigned int>((char const*)(0))
        ,std::runtime_error
        ,"Cannot convert (char const*)(0) to number."
        );
#endif // !defined __BORLANDC__

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

    std::cout
        << "Using libmingwex's strtod() to convert a really big number"
        << " fails with mpatrol."
        << std::endl
        ;
    BOOST_TEST_EQUAL( big, value_cast<double>(value_cast<std::string>( big)));
    BOOST_TEST_EQUAL(-big, value_cast<double>(value_cast<std::string>(-big)));
    // MPATROL !! This minimal, self-contained testcase produces an
    // "ILLMEM" error with mpatrol when built with MinGW gcc-3.4.5 and
    // 'mingwrt-3.15.2-mingw32-dev.tar.gz'. The value of 'nptr' is the
    // string representation of 'big', copied from the output of:
    //   std::cout << value_cast<std::string>(big) << std::endl;
    {
    // Initialize 'nptr' to a string representation of
    //   FLT_RADIX^(DBL_MAX_EXP-1)
    // produced by snprintf(), verifiable thus:
    //    double big = pow(FLT_RADIX, DBL_MAX_EXP - 1.0);
    //    snprintf(buffer, buffer_length, "%.*f", 0, big);
    char const* nptr =
    //   12345678901234567890123456789012345678901234567890 <-- 50 digits/line
        "89884656743115795386465259539451236680898848947115"
        "32863671504057886633790275048156635423866120376801"
        "05600569399356966788293948844072083112464237153197"
        "37062188883946712432742638151109800623047059726541"
        "47604250288441907534117123144073695655527041361858"
        "16752553422931491199736229692398581524176781648121"
        "12068608";
    // Pointer to which strtoT()'s 'endptr' argument refers.
    char* rendptr;
    std::strtod(nptr, &rendptr); // mpatrol illegal access here.
    }

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
/// stream_cast(), but represent cases for which value_cast() is
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

/// Function boost_tests() performs tests similar to those boost
/// uses for its lexical_cast library. It is a derived work based on
///   http://www.boost.org/libs/conversion/lexical_cast_test.cpp
/// which bears the following copyright and permissions notice:
/// [boost 'lexical_cast_test.cpp' notice begins]
///  Copyright Terje Sletteb� and Kevlin Henney, 2005.
///
///  Distributed under the Boost
///  Software License, Version 1.0. (See accompanying file
///  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).
/// [boost 'lexical_cast_test.cpp' notice ends]
///
/// According to
///   http://www.gnu.org/philosophy/license-list.html
///   "This is a simple, permissive non-copyleft free software
///   license, compatible with the GNU GPL."
///
/// Modified by Gregory W. Chicares in 2006 to
///  - adapt to lmi's unit-testing framework
///  - reflect value_cast()'s treatment of char types
/// and in the same or any later year shown above as described in
/// 'ChangeLog'); any defect here should not reflect on Terje
/// Sletteb�'s or Kevlin Henney's reputation.
///
/// Important note on char types.
///
/// value_cast() deliberately differs from boost::lexical_cast in its
/// treatment of char types. See the explanation and rationale in the
/// accompanying header. The boost::lexical_cast tests below have been
/// changed to reflect this difference. For instance, original test
///   BOOST_TEST_EQUAL('1', boost::lexical_cast<char>(1));
/// is replaced by
///   BOOST_TEST_EQUAL('\1', value_cast<char>(1));
/// which asserts the correct postcondition for value_cast.
///
/// Important note on narrowing conversions.
///
/// Some of the boost tests involve narrowing conversions, e.g.:
///   BOOST_TEST_EQUAL('1', value_cast<char>(1.0));
/// With value_cast(), some compilers give a warning about these
/// tests, as is proper. Such tests are suppressed here in order
/// order to get a clean compile.
///
/// Define TEST_BOOST_IMPLEMENTATION_TOO to DWISOTT. This macro is not
/// defined by default because it's been measured to double the RAM
/// and quintuple the time needed to build this module.
///
/// TODO ?? Many of the boost tests remain to be adapted.

#if !defined __BORLANDC__
int boost_tests()
{
    // Original boost::lexical_cast test suite, with wchar_t tests
    // omitted. These tests might profitably be used for lmi's
    // stream_cast function template.

#if defined TEST_BOOST_IMPLEMENTATION_TOO
    using boost::lexical_cast;

    // void test_conversion_to_char()

    BOOST_TEST_EQUAL('A', lexical_cast<char>('A'));
    BOOST_TEST_EQUAL(' ', lexical_cast<char>(' '));
    BOOST_TEST_EQUAL('1', lexical_cast<char>(1));
    BOOST_TEST_EQUAL('0', lexical_cast<char>(0));
    BOOST_TEST_THROW(lexical_cast<char>(123), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL('1', lexical_cast<char>(1.0));
    BOOST_TEST_EQUAL('1', lexical_cast<char>(true));
    BOOST_TEST_EQUAL('0', lexical_cast<char>(false));
    BOOST_TEST_EQUAL('A', lexical_cast<char>("A"));
    BOOST_TEST_EQUAL(' ', lexical_cast<char>(" "));
    BOOST_TEST_THROW(lexical_cast<char>(""), boost::bad_lexical_cast, "");
    BOOST_TEST_THROW(lexical_cast<char>("Test"), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL('A', lexical_cast<char>(std::string("A")));
    BOOST_TEST_EQUAL(' ', lexical_cast<char>(std::string(" ")));
    BOOST_TEST_THROW
        (lexical_cast<char>(std::string(""))
        ,boost::bad_lexical_cast
        ,""
        );
    BOOST_TEST_THROW
        (lexical_cast<char>(std::string("Test"))
        ,boost::bad_lexical_cast
        ,""
        );

    // void test_conversion_to_int()

    BOOST_TEST_EQUAL(1,lexical_cast<int>('1'));
    BOOST_TEST_EQUAL(0,lexical_cast<int>('0'));
    BOOST_TEST_THROW(lexical_cast<int>('A'),boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(1,lexical_cast<int>(1));
    BOOST_TEST_EQUAL
        (std::numeric_limits<int>::max()
        ,lexical_cast<int>(std::numeric_limits<int>::max())
        );
    BOOST_TEST_EQUAL(1,lexical_cast<int>(1.0));

    BOOST_TEST_THROW(lexical_cast<int>(1.23), boost::bad_lexical_cast, "");

    BOOST_TEST_THROW(lexical_cast<int>(1e20), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(1, lexical_cast<int>(true));
    BOOST_TEST_EQUAL(0, lexical_cast<int>(false));
    BOOST_TEST_EQUAL(123, lexical_cast<int>("123"));
    BOOST_TEST_THROW
        (lexical_cast<int>(" 123")
        ,boost::bad_lexical_cast
        ,""
        );
    BOOST_TEST_THROW(lexical_cast<int>(""), boost::bad_lexical_cast, "");
    BOOST_TEST_THROW(lexical_cast<int>("Test"), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(123, lexical_cast<int>("123"));
    BOOST_TEST_EQUAL(123,lexical_cast<int>(std::string("123")));
    BOOST_TEST_THROW
        (lexical_cast<int>(std::string(" 123"))
        ,boost::bad_lexical_cast
        ,""
        );
    BOOST_TEST_THROW
        (lexical_cast<int>(std::string(""))
        ,boost::bad_lexical_cast
        ,""
        );
    BOOST_TEST_THROW
        (lexical_cast<int>(std::string("Test"))
        ,boost::bad_lexical_cast
        ,""
        );

    // void test_conversion_to_double()

    BOOST_TEST_EQUAL(1.0, lexical_cast<double>('1'));
    BOOST_TEST_THROW(lexical_cast<double>('A'), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(1.0, lexical_cast<double>(1));
    BOOST_TEST_EQUAL(1.23, lexical_cast<double>(1.23));
//    BOOST_CHECK_CLOSE(
//        std::numeric_limits<double>::max() / 2,
//        lexical_cast<double>(std::numeric_limits<double>::max() / 2),
//        std::numeric_limits<double>::epsilon());
    BOOST_TEST_EQUAL(1.0, lexical_cast<double>(true));
    BOOST_TEST_EQUAL(0.0, lexical_cast<double>(false));
    BOOST_TEST_EQUAL(1.23, lexical_cast<double>("1.23"));
    BOOST_TEST_THROW(lexical_cast<double>(""), boost::bad_lexical_cast, "");
    BOOST_TEST_THROW(lexical_cast<double>("Test"), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(1.23, lexical_cast<double>(std::string("1.23")));
    BOOST_TEST_THROW
        (lexical_cast<double>(std::string(""))
        ,boost::bad_lexical_cast
        ,""
        );
    BOOST_TEST_THROW
        (lexical_cast<double>(std::string("Test"))
        ,boost::bad_lexical_cast
        ,""
        );

    // void test_conversion_to_bool()

    BOOST_TEST_EQUAL(true, lexical_cast<bool>('1'));
    BOOST_TEST_EQUAL(false, lexical_cast<bool>('0'));
    BOOST_TEST_THROW(lexical_cast<bool>('A'), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(true, lexical_cast<bool>(1));
    BOOST_TEST_EQUAL(false, lexical_cast<bool>(0));
    BOOST_TEST_THROW(lexical_cast<bool>(123), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(true, lexical_cast<bool>(1.0));
    BOOST_TEST_EQUAL(false, lexical_cast<bool>(0.0));
    BOOST_TEST_EQUAL(true, lexical_cast<bool>(true));
    BOOST_TEST_EQUAL(false, lexical_cast<bool>(false));
    BOOST_TEST_EQUAL(true, lexical_cast<bool>("1"));
    BOOST_TEST_EQUAL(false, lexical_cast<bool>("0"));
    BOOST_TEST_THROW(lexical_cast<bool>(""), boost::bad_lexical_cast, "");
    BOOST_TEST_THROW(lexical_cast<bool>("Test"), boost::bad_lexical_cast, "");
    BOOST_TEST_EQUAL(true, lexical_cast<bool>("1"));
    BOOST_TEST_EQUAL(false, lexical_cast<bool>("0"));
    BOOST_TEST_EQUAL(true, lexical_cast<bool>(std::string("1")));
    BOOST_TEST_EQUAL(false, lexical_cast<bool>(std::string("0")));
    BOOST_TEST_THROW
        (lexical_cast<bool>(std::string(""))
        ,boost::bad_lexical_cast
        ,""
        );
    BOOST_TEST_THROW
        (lexical_cast<bool>(std::string("Test"))
        ,boost::bad_lexical_cast
        ,""
        );

    // void test_conversion_to_string()

    BOOST_TEST_EQUAL("A", lexical_cast<std::string>('A'));
    BOOST_TEST_EQUAL(" ", lexical_cast<std::string>(' '));
    BOOST_TEST_EQUAL("123", lexical_cast<std::string>(123));
    BOOST_TEST_EQUAL("1.23", lexical_cast<std::string>(1.23));
    BOOST_TEST_EQUAL("1.111111111", lexical_cast<std::string>(1.111111111));
    BOOST_TEST_EQUAL("1",lexical_cast<std::string>(true));
    BOOST_TEST_EQUAL("0",lexical_cast<std::string>(false));
    BOOST_TEST_EQUAL("Test", lexical_cast<std::string>("Test"));
    BOOST_TEST_EQUAL(" ", lexical_cast<std::string>(" "));
    BOOST_TEST_EQUAL("", lexical_cast<std::string>(""));
    BOOST_TEST_EQUAL("Test", lexical_cast<std::string>(std::string("Test")));
    BOOST_TEST_EQUAL(" ", lexical_cast<std::string>(std::string(" ")));
    BOOST_TEST_EQUAL("", lexical_cast<std::string>(std::string("")));

    // void test_conversion_from_to_wchar_t_alias()

    BOOST_TEST_EQUAL(123u, lexical_cast<unsigned short int>("123"));
    BOOST_TEST_EQUAL(123u, lexical_cast<unsigned       int>("123"));
    BOOST_TEST_EQUAL(123u, lexical_cast<unsigned long  int>("123"));
    BOOST_TEST_EQUAL
        (std::string("123")
        ,lexical_cast<std::string>(static_cast<unsigned short int>(123))
        );
    BOOST_TEST_EQUAL(std::string("123"), lexical_cast<std::string>(123u));
    BOOST_TEST_EQUAL(std::string("123"), lexical_cast<std::string>(123ul));

    // void test_conversion_to_pointer()

    BOOST_TEST_THROW(lexical_cast<char *>("Test"), boost::bad_lexical_cast, "");
#endif // defined TEST_BOOST_IMPLEMENTATION_TOO

    // A roughly-equivalent value_cast test suite.

    BOOST_TEST_EQUAL('A', value_cast<char>('A'));
    BOOST_TEST_EQUAL(' ', value_cast<char>(' '));

    // See 'Important note on char types' above.
    BOOST_TEST_EQUAL('\1', value_cast<char>(1));
    BOOST_TEST_EQUAL('\0', value_cast<char>(0));

    // See 'Important note on char types' above.
    // boost::lexical_cast<char>(123) throws an exception.
    BOOST_TEST_EQUAL('\123', value_cast<char>(0123));

    // See 'Important note on narrowing conversions' above.
//    BOOST_TEST_EQUAL('1', value_cast<char>(1.0)); // Suppressed.

    // See 'Important note on char types' above.
    BOOST_TEST_EQUAL('\1', value_cast<char>(true));
    BOOST_TEST_EQUAL('\0', value_cast<char>(false));

////    BOOST_TEST_EQUAL('A', value_cast<char>("A"));

////    BOOST_TEST_EQUAL(' ', value_cast<char>(" "));

//    BOOST_TEST_THROW(value_cast<char>(""), boost::bad_value_cast);
//    BOOST_TEST_THROW(value_cast<char>("Test"), boost::bad_value_cast);

////    BOOST_TEST_EQUAL('A', value_cast<char>(std::string("A")));
////    BOOST_TEST_EQUAL(' ', value_cast<char>(std::string(" ")));

//    BOOST_TEST_THROW(
//        value_cast<char>(std::string("")), boost::bad_value_cast);
//    BOOST_TEST_THROW(
//        value_cast<char>(std::string("Test")), boost::bad_value_cast);

    // See 'Important note on char types' above.
    BOOST_TEST_EQUAL(1, value_cast<int>('\1'));
    BOOST_TEST_EQUAL(0, value_cast<int>('\0'));

//    BOOST_TEST_THROW(value_cast<int>('A'),boost::bad_value_cast);
    BOOST_TEST_EQUAL(1, value_cast<int>(1));

    int const max_int = std::numeric_limits<int>::max();
    BOOST_TEST_EQUAL(max_int, value_cast<int>(max_int));

//    BOOST_TEST_EQUAL(1,value_cast<int>(1.0));

//    BOOST_TEST_THROW(value_cast<int>(1.23), boost::bad_value_cast);

//    BOOST_TEST_THROW(value_cast<int>(1e20), boost::bad_value_cast);
    BOOST_TEST_EQUAL(1, value_cast<int>(true));
    BOOST_TEST_EQUAL(0, value_cast<int>(false));
    BOOST_TEST_EQUAL(123, value_cast<int>("123"));
//    BOOST_TEST_THROW(
//        value_cast<int>(" 123"), boost::bad_value_cast);
//    BOOST_TEST_THROW(value_cast<int>(""), boost::bad_value_cast);
//    BOOST_TEST_THROW(value_cast<int>("Test"), boost::bad_value_cast);
    BOOST_TEST_EQUAL(123, value_cast<int>("123"));
    BOOST_TEST_EQUAL(123,value_cast<int>(std::string("123")));
//    BOOST_TEST_THROW(
//        value_cast<int>(std::string(" 123")), boost::bad_value_cast);
//    BOOST_TEST_THROW(
//        value_cast<int>(std::string("")), boost::bad_value_cast);
//    BOOST_TEST_THROW(
//        value_cast<int>(std::string("Test")), boost::bad_value_cast);

    // See 'Important note on char types' above.
    BOOST_TEST_EQUAL(1.0, value_cast<double>('\1'));

//    BOOST_TEST_THROW(value_cast<double>('A'), boost::bad_value_cast);
    BOOST_TEST_EQUAL(1.0, value_cast<double>(1));
    BOOST_TEST_EQUAL(1.23, value_cast<double>(1.23));

    // The boost test affirms only that the original and converted
    // values are materially equal. Should something similar not be
    // done here, using lmi's 'materially_equal.hpp'?
    double const max_double = std::numeric_limits<double>::max();
    BOOST_TEST_EQUAL(max_double / 2, value_cast<double>(max_double / 2));

    BOOST_TEST_EQUAL(1.0, value_cast<double>(true));
    BOOST_TEST_EQUAL(0.0, value_cast<double>(false));
    BOOST_TEST_EQUAL(1.23, value_cast<double>("1.23"));
//    BOOST_TEST_THROW(value_cast<double>(""), boost::bad_value_cast);
//    BOOST_TEST_THROW(value_cast<double>("Test"), boost::bad_value_cast);
    BOOST_TEST_EQUAL(1.23, value_cast<double>(std::string("1.23")));
//    BOOST_TEST_THROW(
//        value_cast<double>(std::string("")), boost::bad_value_cast);
//    BOOST_TEST_THROW(
//        value_cast<double>(std::string("Test")), boost::bad_value_cast);

    // See 'Important note on char types' above.
    BOOST_TEST_EQUAL(true, value_cast<bool>('\1'));
    BOOST_TEST_EQUAL(false, value_cast<bool>('\0'));

//    BOOST_TEST_THROW(value_cast<bool>('A'), boost::bad_value_cast);
    BOOST_TEST_EQUAL(true, value_cast<bool>(1));
    BOOST_TEST_EQUAL(false, value_cast<bool>(0));
//    BOOST_TEST_THROW(value_cast<bool>(123), boost::bad_value_cast);
    BOOST_TEST_EQUAL(true, value_cast<bool>(1.0));
    BOOST_TEST_EQUAL(false, value_cast<bool>(0.0));
    BOOST_TEST_EQUAL(true, value_cast<bool>(true));
    BOOST_TEST_EQUAL(false, value_cast<bool>(false));

    // COMPILER !! Suppress this test for gcc-4.1.2, which refuses,
    // defectively it would seem, to compile it. See:
    //   http://lists.nongnu.org/archive/html/lmi/2008-06/msg00010.html
#if !(defined __GNUC__ && 40102 == LMI_GCC_VERSION)
    BOOST_TEST_EQUAL(true, value_cast<bool>("1"));
#endif // !(defined __GNUC__ && 40102 == LMI_GCC_VERSION)

    // This fails; should it?
//    BOOST_TEST_EQUAL(false, value_cast<bool>("0"));
//    BOOST_TEST_THROW(value_cast<bool>(""), boost::bad_value_cast);
//    BOOST_TEST_THROW(value_cast<bool>("Test"), boost::bad_value_cast);

    BOOST_TEST_EQUAL(true, value_cast<bool>(std::string("1")));
    BOOST_TEST_EQUAL(false, value_cast<bool>(std::string("0")));
//    BOOST_TEST_THROW(
//        value_cast<bool>(std::string("")), boost::bad_value_cast);
//    BOOST_TEST_THROW(
//        value_cast<bool>(std::string("Test")), boost::bad_value_cast);

//    BOOST_TEST_EQUAL("A", value_cast<std::string>('A'));
//    BOOST_TEST_EQUAL(" ", value_cast<std::string>(' '));
    BOOST_TEST_EQUAL("123", value_cast<std::string>(123));
    BOOST_TEST_EQUAL("1.23", value_cast<std::string>(1.23));
    BOOST_TEST_EQUAL("1.111111111", value_cast<std::string>(1.111111111));
    BOOST_TEST_EQUAL("1",value_cast<std::string>(true));
    BOOST_TEST_EQUAL("0",value_cast<std::string>(false));
    BOOST_TEST_EQUAL("Test", value_cast<std::string>("Test"));
    BOOST_TEST_EQUAL(" ", value_cast<std::string>(" "));
    BOOST_TEST_EQUAL("", value_cast<std::string>(""));
    BOOST_TEST_EQUAL("Test", value_cast<std::string>(std::string("Test")));
    BOOST_TEST_EQUAL(" ", value_cast<std::string>(std::string(" ")));
    BOOST_TEST_EQUAL("", value_cast<std::string>(std::string("")));

    BOOST_TEST_EQUAL(123u, value_cast<unsigned short int>("123"));
    BOOST_TEST_EQUAL(123u, value_cast<unsigned       int>("123"));
    BOOST_TEST_EQUAL(123u, value_cast<unsigned long  int>("123"));
    BOOST_TEST_EQUAL
        (std::string("123")
        ,value_cast<std::string>(static_cast<unsigned short int>(123))
        );
    BOOST_TEST_EQUAL(std::string("123"), value_cast<std::string>(123u));
    BOOST_TEST_EQUAL(std::string("123"), value_cast<std::string>(123ul));

//    BOOST_TEST_THROW(value_cast<char *>("Test"), boost::bad_value_cast);

    return 0;
}
#else  // defined __BORLANDC__
int boost_tests()
{
    return 0;
}
#endif // defined __BORLANDC__

