// Decimal conversion between std::string and arithmetic types--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: numeric_io_test.cpp,v 1.15 2007-09-30 18:16:23 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "numeric_io_cast.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

#if !defined __BORLANDC__
#   include <boost/lexical_cast.hpp>
#endif // ! defined __BORLANDC__

#include <cmath>

template<typename T>
void test_interconvertibility
    (T t
    ,std::string const s
    ,char const* file
    ,int line
    )
{
    T const v = numeric_io_cast<T>(s);
    volatile bool is_exact = std::numeric_limits<T>::is_exact;
    if(is_exact)
        {
        INVOKE_BOOST_TEST_EQUAL(v, t, file, line);
        }

    INVOKE_BOOST_TEST_EQUAL(v, numeric_io_cast<T>(s), file, line);

    T t0 = t;
    INVOKE_BOOST_TEST_EQUAL(s, numeric_io_cast<std::string>(t0   ), file, line);
    INVOKE_BOOST_TEST_EQUAL(s, numeric_io_cast<std::string>(t0, s), file, line);
    INVOKE_BOOST_TEST_EQUAL(v, numeric_io_cast             (s, t0), file, line);

    T const t1 = t;
    INVOKE_BOOST_TEST_EQUAL(s, numeric_io_cast<std::string>(t1   ), file, line);
    INVOKE_BOOST_TEST_EQUAL(s, numeric_io_cast<std::string>(t1, s), file, line);
    INVOKE_BOOST_TEST_EQUAL(v, numeric_io_cast             (s, t1), file, line);

    T const& t2 = t;
    INVOKE_BOOST_TEST_EQUAL(s, numeric_io_cast<std::string>(t2   ), file, line);
    INVOKE_BOOST_TEST_EQUAL(s, numeric_io_cast<std::string>(t2, s), file, line);
    INVOKE_BOOST_TEST_EQUAL(v, numeric_io_cast             (s, t2), file, line);

    std::string const s0 = s;
    INVOKE_BOOST_TEST_EQUAL(v, numeric_io_cast(s0, t), file, line);
}

// These tests generally assume IEC 60559 floating point. Hardware
// that deviates from that standard is probably so rare that it can
// reasonably be ignored, with an appropriate runtime message.
//
// COMPILER !! The borland compiler through version 5.5.1 reports that
// its double type doesn't conform to IEC 60559, but that seems wrong.
#ifndef __BORLANDC__
    BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);
#endif // ! defined __BORLANDC__

int test_main(int, char*[])
{
    volatile int z = 0; // Avoid "condition always true/false" warnings.
    BOOST_TEST_EQUAL( z, floating_point_decimals( 0.0));
    BOOST_TEST_EQUAL( z, floating_point_decimals(-0.0));
    BOOST_TEST_EQUAL(15, floating_point_decimals( 1.0));
    BOOST_TEST_EQUAL(15, floating_point_decimals(-1.0));
    BOOST_TEST_EQUAL(12, floating_point_decimals( 1000.0));
    BOOST_TEST_EQUAL(12, floating_point_decimals(-1000.0));
    BOOST_TEST_EQUAL( 0, floating_point_decimals( 1000000000000000.0));
    BOOST_TEST_EQUAL( 0, floating_point_decimals(-1000000000000000.0));
    BOOST_TEST_EQUAL( 0, floating_point_decimals( 10000000000000000000.0));
    BOOST_TEST_EQUAL( 0, floating_point_decimals(-10000000000000000000.0));
    BOOST_TEST_EQUAL(35, floating_point_decimals( 0.00000000000000000001));
    BOOST_TEST_EQUAL(35, floating_point_decimals(-0.00000000000000000001));

    BOOST_TEST_EQUAL( 3, floating_point_decimals(-1000.0f));
    BOOST_TEST_EQUAL(15, floating_point_decimals(-1000.0L));

    // Consider the number of exact decimal digits in the neighborhood
    // of epsilon's reciprocal for type double, which is approximately
    // 0.450359962737049596e16 .
    //
    // Construct a decimal representation, D, of epsilon's reciprocal
    // with 1 + std::numeric_limits<double>::digits10 digits--scaled
    // by a negative power of ten so that at least one of those digits
    // is fractional, because floating_point_decimals() counts only
    // fractional digits. All D's digits are exact. A number slightly
    // lower has the same property. A number slightly higher has one
    // fewer exact (fractional) digit.
    //                                             000000000111111111
    //                                             123456789012345678
    BOOST_TEST_EQUAL(16, floating_point_decimals(0.450359962737049596));
    BOOST_TEST_EQUAL(16, floating_point_decimals(0.4503599627370495));
    BOOST_TEST_EQUAL(16, floating_point_decimals(0.4503599627370496));
    // TODO ?? Fails for como with mingw, but succeeds with 0.45036 .
    BOOST_TEST_EQUAL(15, floating_point_decimals(0.4503599627370497));

    BOOST_TEST_EQUAL(   "3.14", simplify_floating_point(    "3.14"));
    BOOST_TEST_EQUAL(   "3.14", simplify_floating_point( "3.14000"));
    BOOST_TEST_EQUAL(    "100", simplify_floating_point(    "100."));
    BOOST_TEST_EQUAL(    "100", simplify_floating_point( "100.000"));
    BOOST_TEST_EQUAL( "0.0001", simplify_floating_point(  "0.0001"));
    BOOST_TEST_EQUAL( "0.0001", simplify_floating_point( "0.00010"));
    BOOST_TEST_EQUAL(      "0", simplify_floating_point(      "0."));
    BOOST_TEST_EQUAL(     "-0", simplify_floating_point(     "-0."));
    BOOST_TEST_EQUAL(    "nan", simplify_floating_point(     "nan"));

    double volatile d;

// TODO ?? Use TimeAnAliquot() instead.
    Timer timer;
    int iterations = 100000;
    for(int j = 0; j < iterations; ++j)
        {
        std::string s = numeric_io_cast<std::string>(2.0 / 3.0);
        d = numeric_io_cast<double>(s);
        }
    std::cout
        << (1e6 * timer.stop().elapsed_usec() / iterations)
        << " usec per iteration for numeric_io_cast().\n"
        ;

#if !defined __BORLANDC__
    timer.restart();
    iterations = 1000;
    for(int j = 0; j < iterations; ++j)
        {
        std::string s = boost::lexical_cast<std::string>(2.0 / 3.0);
        d = boost::lexical_cast<double>(s);
        }
    std::cout
        << (1e6 * timer.stop().elapsed_usec() / iterations)
        << " usec per iteration for boost::lexical_cast()."
        ;
#endif // ! defined __BORLANDC__

    std::cout << std::endl;
    (void) d;

    // Interpreted as decimal, not as octal.
    BOOST_TEST_EQUAL(77, numeric_io_cast<int>( "077"));

    // Interpreted as valid decimal, not as invalid octal.
    BOOST_TEST_EQUAL(99, numeric_io_cast<int>("0099"));

    BOOST_TEST_EQUAL( "Z" , numeric_io_cast<std::string>( "Z" ));
    BOOST_TEST_EQUAL(" Z" , numeric_io_cast<std::string>(" Z" ));
    BOOST_TEST_EQUAL( "Z ", numeric_io_cast<std::string>( "Z "));
    BOOST_TEST_EQUAL(" Z ", numeric_io_cast<std::string>(" Z "));

    test_interconvertibility((         char)(   1), "1", __FILE__, __LINE__);
    test_interconvertibility((         char)('\1'), "1", __FILE__, __LINE__);

    test_interconvertibility((  signed char)(   1), "1", __FILE__, __LINE__);
    test_interconvertibility((  signed char)('\1'), "1", __FILE__, __LINE__);

    test_interconvertibility((unsigned char)(   1), "1", __FILE__, __LINE__);
    test_interconvertibility((unsigned char)('\1'), "1", __FILE__, __LINE__);

    test_interconvertibility((  signed char)(-1),  "-1", __FILE__, __LINE__);
    std::ostringstream u;
    u << static_cast<unsigned int>(std::numeric_limits<unsigned char>::max());
    test_interconvertibility((unsigned char)(-1), u.str(), __FILE__, __LINE__);

    // This test should fail because numeric_io_cast interprets
    // arithmetic types as numeric values, so the value of ' ' might
    // correspond to the string literal "32", but never to a blank
    // string.
    //
    BOOST_TEST_UNEQUAL(" ", numeric_io_cast<std::string>(' '));
    //
    // Furthermore, these expressions should throw because strtol
    // doesn't consider "A" valid.
    //
    BOOST_TEST_THROW(numeric_io_cast<char  >("A"), std::invalid_argument, "");
    BOOST_TEST_THROW(numeric_io_cast<int   >("A"), std::invalid_argument, "");
    BOOST_TEST_THROW(numeric_io_cast<double>("A"), std::invalid_argument, "");

    test_interconvertibility(bool(1), "1", __FILE__, __LINE__);
    test_interconvertibility(bool(0), "0", __FILE__, __LINE__);

    test_interconvertibility(int(    0),     "0", __FILE__, __LINE__);
    test_interconvertibility(int( 1234),  "1234", __FILE__, __LINE__);
    test_interconvertibility(int(-4321), "-4321", __FILE__, __LINE__);

    test_interconvertibility(float( 0.0f),    "0", __FILE__, __LINE__);
    test_interconvertibility(float( 1.5f),  "1.5", __FILE__, __LINE__);
    test_interconvertibility(float(-2.5f), "-2.5", __FILE__, __LINE__);

    test_interconvertibility(double( 0.0),    "0", __FILE__, __LINE__);
    test_interconvertibility(double( 1.5),  "1.5", __FILE__, __LINE__);
    test_interconvertibility(double(-2.5), "-2.5", __FILE__, __LINE__);

    test_interconvertibility( double(1.0 / 3.0), "0.3333333333333333", __FILE__, __LINE__);
    test_interconvertibility(0.3333333333333333, "0.3333333333333333", __FILE__, __LINE__);
    test_interconvertibility( double(2.0 / 3.0), "0.666666666666667" , __FILE__, __LINE__);
    test_interconvertibility( 0.666666666666667, "0.666666666666667" , __FILE__, __LINE__);

#if 0
// COMPILER !! MinGW gcc-3.2.3 doesn't support long double conversions
// because it uses the ms C runtime library; como with MinGW gcc as
// its underlying C compiler has the same problem.
//
    test_interconvertibility((long double)( 0.0L),    "0", __FILE__, __LINE__);
    test_interconvertibility((long double)( 1.5L),  "1.5", __FILE__, __LINE__);
    test_interconvertibility((long double)(-2.5L), "-2.5", __FILE__, __LINE__);
#endif // 0

    test_interconvertibility(std::string("  as  df  "), "  as  df  ", __FILE__, __LINE__);
    // The converse
    //   test_interconvertibility("  as  df  ", std::string("  as  df  "),...
    // is not supported: it seems fraudulent.

    BOOST_TEST_THROW
        (numeric_io_cast<double>(std::string("0.333 "))
        ,std::invalid_argument
        ,""
        );
    BOOST_TEST_THROW
        (numeric_io_cast<double>(std::string("0.333.777#3"))
        ,std::invalid_argument
        ,""
        );
    BOOST_TEST_THROW
        (numeric_io_cast<double>(std::string("xxx"))
        ,std::invalid_argument
        ,""
        );
    BOOST_TEST_THROW(numeric_io_cast<int>   ( "1.1"), std::invalid_argument, "");
    BOOST_TEST_THROW(numeric_io_cast<int>   ( "1e1"), std::invalid_argument, "");
    BOOST_TEST_THROW(numeric_io_cast<bool>  ("true"), std::invalid_argument, "");
    BOOST_TEST_THROW(numeric_io_cast<int>   (    ""), std::invalid_argument, "");
    BOOST_TEST_THROW(numeric_io_cast<double>(    ""), std::invalid_argument, "");
    BOOST_TEST_THROW(numeric_io_cast<double>(  "1e"), std::invalid_argument, "");

    BOOST_TEST_THROW(numeric_io_cast<long double>(    ""), std::invalid_argument, "");

#if defined __MINGW32__ && defined __GNUC__ && LMI_GCC_VERSION < 30404
    std::cerr
        << "The following test fails with MinGW prior to gcc-3.4.4: see\n"
        << "  http://sf.net/mailarchive/message.php?msg_id=10706179\n"
        ;
#endif // MinGW gcc version prior to 3.4.4 .

    BOOST_TEST_THROW(numeric_io_cast<long double>(  "1e"), std::invalid_argument, "");

    // This shouldn't even throw, because adequate compilers detect
    // the error at compile time:
//    BOOST_TEST_THROW(numeric_io_cast<double*>("0"), std::invalid_argument, "");

    BOOST_TEST_THROW
        (numeric_io_cast<std::string>((char const*)(0))
        ,std::runtime_error
        ,"Cannot convert (char const*)(0) to std::string."
        );
    BOOST_TEST_THROW
        (numeric_io_cast<unsigned int>((char const*)(0))
        ,std::runtime_error
        ,"Cannot convert (char const*)(0) to number."
        );

    BOOST_TEST_EQUAL("1", numeric_io_cast<std::string>(true));
    BOOST_TEST_EQUAL("0", numeric_io_cast<std::string>(false));
    BOOST_TEST_EQUAL(  0, numeric_io_cast<double>("0"));
    BOOST_TEST_EQUAL( "", numeric_io_cast<std::string>(""));
    BOOST_TEST_EQUAL(" ", numeric_io_cast<std::string>(" "));

    BOOST_TEST_EQUAL(  0, numeric_io_cast<double>("0."));
    BOOST_TEST_EQUAL(  1, numeric_io_cast<double>("1."));
    BOOST_TEST_EQUAL(  0, numeric_io_cast<double>(".0"));
    BOOST_TEST_EQUAL(  1, numeric_io_cast<double>("1"));
    BOOST_TEST_EQUAL(  1, numeric_io_cast<double>("1e0"));
    BOOST_TEST_EQUAL(  1, numeric_io_cast<double>("1.e0"));
    BOOST_TEST_EQUAL(  1, numeric_io_cast<double>("1.0e0"));
    BOOST_TEST_EQUAL(  1, numeric_io_cast<double>("0.1e1"));
    BOOST_TEST_EQUAL(  1, numeric_io_cast<double>(".1e1"));

    test_interconvertibility(std::exp(1.0), "2.718281828459045", __FILE__, __LINE__);

    test_interconvertibility( 1.0e-20,  "0.00000000000000000001", __FILE__, __LINE__);
    test_interconvertibility(-1.0e-20, "-0.00000000000000000001", __FILE__, __LINE__);
    test_interconvertibility( 1.0e20 ,   "100000000000000000000", __FILE__, __LINE__);
    test_interconvertibility(-1.0e20 ,  "-100000000000000000000", __FILE__, __LINE__);

    // For IEC 559 doubles, epsilon is 2.2204460492503131e-16
    // according to C99 5.2.4.2.2/13 .
    //
    // 2.2204460492503131e-16 * 1.0 / 3.0 is 7.40148683083438E-17
    // so double(1/3) has 16 accurate digits. In this case, more than
    // DBL_DIG digits are accurate.
    //
    // 2.2204460492503131e-16 * 2.0 / 3.0 is 1.48029736616688E-16
    // so double(2/3) has 15 accurate digits. In this case, only
    // DBL_DIG digits are accurate.
    //
    //                     1111111
    //            1234567890123456
    test_interconvertibility((1.0 / 3.0), "0.3333333333333333", __FILE__, __LINE__);
    test_interconvertibility((2.0 / 3.0),  "0.666666666666667", __FILE__, __LINE__);

    BOOST_TEST_EQUAL  ("1"  , numeric_io_cast<std::string>(1.0 + 2.2204460492503131e-16));
    BOOST_TEST_EQUAL  ("0.5", numeric_io_cast<std::string>(0.5 + 2.2204460492503131e-16));
    BOOST_TEST_UNEQUAL("0.1", numeric_io_cast<std::string>(0.1 + 2.2204460492503131e-16));

    // 1 +/- epsilon must be formatted as apparent unity.
    volatile bool eq = (1.0 + 2.2204460492503131e-16 == 1.00000000000000022204460492503131);
    BOOST_TEST(eq);
    BOOST_TEST_EQUAL("1", numeric_io_cast<std::string>(1.00000000000000022204460492503131));
    // Consider:
    //             1111111
    //   1 234567890123456
    //   1.00000000000000022204460492503131
    // Adding epsilon to unity changes the seventeenth digit, which is
    // not reliably accurate, because doubles in this neighborhood are
    // quantized to
    //   1.00000000000000022...
    //   1.00000000000000044...
    // and one of those representations would have to be chosen for
    //   1.0000000000000003
    // Adding three times epsilon would change the sixteenth digit,
    // however. Therefore, in the neighborhood of unity, we have a
    // resolution of two ulps but not three.
    BOOST_TEST_EQUAL  ("1", numeric_io_cast<std::string>(1.0 + 1.0 * 2.2204460492503131e-16));
    BOOST_TEST_EQUAL  ("1", numeric_io_cast<std::string>(1.0 + 2.0 * 2.2204460492503131e-16));
    BOOST_TEST_UNEQUAL("1", numeric_io_cast<std::string>(1.0 + 3.0 * 2.2204460492503131e-16));
    BOOST_TEST_UNEQUAL("1", numeric_io_cast<std::string>(1.0 - 3.0 * 2.2204460492503131e-16));
    BOOST_TEST_EQUAL  ("1", numeric_io_cast<std::string>(1.0 - 2.0 * 2.2204460492503131e-16));
    BOOST_TEST_EQUAL  ("1", numeric_io_cast<std::string>(1.0 - 1.0 * 2.2204460492503131e-16));

    return 0;
}

