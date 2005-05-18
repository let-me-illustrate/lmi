// Convert between types as extractors and inserters do--unit test.
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

// $Id: stream_cast_test.cpp,v 1.3 2005-05-18 23:57:25 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "stream_cast.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

// TODO ?? Consider adding tests in
// http://www.boost.org/libs/conversion/lexical_cast_test.cpp

int test_main(int, char*[])
{
    std::string s;

    // Conversion from an empty std::string to another std::string
    // works only because a specialization is used in that case.
    // This would fail if the type converted to were of a different
    // string-like type: the stream inserter would set failbit, and
    // that's an essential feature of the technique. In general,
    // prefer template function value_cast, and specialize it for
    // string-like classes.
    BOOST_TEST_EQUAL("", stream_cast<std::string>(""));
    s = "Not empty.";
    std::string empty("");
    s = stream_cast<std::string>(empty);
    BOOST_TEST_EQUAL("", s);

    BOOST_TEST_EQUAL( "Z" , stream_cast<std::string>( "Z" ));
    BOOST_TEST_EQUAL(" Z" , stream_cast<std::string>(" Z" ));
    BOOST_TEST_EQUAL( "Z ", stream_cast<std::string>( "Z "));
    BOOST_TEST_EQUAL(" Z ", stream_cast<std::string>(" Z "));

#if 0
    // These tests have been suppressed because function template
    // numeric_io_cast is to be preferred for arithmetic types.
    // They are left here to assist anyone who desires to use them
    // that way anyway after carefully reading the numeric_io_cast
    // documentation.

    // For this test, numeric_io_cast() behaves differently: it treats
    // the char argument as a number.
    BOOST_TEST_EQUAL(" ", stream_cast<std::string>(' '));

    bool b0 = true;
    s = stream_cast<std::string>(b0);
    BOOST_TEST_EQUAL("1", s);
    bool const b1 = false;
    s = stream_cast(b1, s);
    BOOST_TEST_EQUAL("0", s);

    int i0 = 1234;
    s = stream_cast<std::string>(i0);
    BOOST_TEST_EQUAL("1234", s);
    int const i1 = -4321;
    s = stream_cast(i1, s);
    BOOST_TEST_EQUAL("-4321", s);

    double d0 = 1.5;
    s = stream_cast<std::string>(d0);
    BOOST_TEST_EQUAL("1.5", s);
    double const d1 = -2.5;
    s = stream_cast(d1, s);
    BOOST_TEST_EQUAL("-2.5", s);
#endif // 0

    char* c0 = const_cast<char*>("as df"); // Cast avoids 4.2/2 warning.
    s = stream_cast<std::string>(c0);
    BOOST_TEST_EQUAL("as df", s);
    char const* c1 = "jk l;";
    s = stream_cast(c1, s);
    BOOST_TEST_EQUAL("jk l;", s);

    std::string s0 = " !@ #$% ";
    s = stream_cast<std::string>(s0);
    BOOST_TEST_EQUAL(" !@ #$% ", s);
    std::string const s1 = "  ^&  *()  ";
    s = stream_cast(s1, s);
    BOOST_TEST_EQUAL("  ^&  *()  ", s);

    std::string const& r0(s0);
    s = stream_cast<std::string>(r0);
    BOOST_TEST_EQUAL(" !@ #$% ", s);
    std::string const& r1(s1);
    s = stream_cast(r1, s);
    BOOST_TEST_EQUAL("  ^&  *()  ", s);

    s = stream_cast(r0, s);
    BOOST_TEST_EQUAL(" !@ #$% ", s);
    s = stream_cast(r1, r1);
    BOOST_TEST_EQUAL("  ^&  *()  ", s);

    return 0;
}

