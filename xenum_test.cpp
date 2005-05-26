// Extended enumeration type--unit test.
//
// Copyright (C) 2001, 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: xenum_test.cpp,v 1.2 2005-05-26 22:01:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xenum_sample.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <algorithm>
#include <exception>
#include <ios>
#include <iterator>
#include <sstream>

e_123 higher_of_0(e_123 x, e_123 y) {return std::max(x, y);}
e_123 higher_of_1(e_123 const& x, e_123 const& y) {return std::max(x, y);}

#define TEST_GLOBALS

#ifdef TEST_GLOBALS
    e_123 xyz;
    std::string xyz_string = xyz.str();
    std::vector<std::string> xyz_all_strings = xyz.all_strings();
#endif // TEST_GLOBALS

int test_main(int, char*[])
{
#ifndef TEST_GLOBALS
    e_123 xyz;
    std::string xyz_string = xyz.str();
    std::vector<std::string> xyz_all_strings = xyz.all_strings();
#endif // not TEST_GLOBALS

    BOOST_TEST("one" != xyz_string);
    BOOST_TEST("1 One" == xyz_string);
    BOOST_TEST
        ( "1 One 2 Two 3 Three" ==
          xyz_all_strings[0] + " "
        + xyz_all_strings[1] + " "
        + xyz_all_strings[2]
        );
    BOOST_TEST
        ("1 One,2 Two,3 Three" ==
          e_123::all_strings()[0] + ","
        + e_123::all_strings()[1] + ","
        + e_123::all_strings()[2]
        );
    e_123 a("1 One");
    BOOST_TEST("1 One" == a.str());
    e_123 b("2 Two");
    BOOST_TEST("2 Two" == b.str());
    e_123 c(a);
    BOOST_TEST("1 One" == c.str());
    e_123 d(e_one);
    BOOST_TEST("1 One" == d.str());
    e_123 e;
    e = e_one;
    BOOST_TEST("1 One" == e.str());
    BOOST_TEST
        ("1 One,2 Two,3 Three" ==
          e.all_strings()[0] + ","
        + e.all_strings()[1] + ","
        + e.all_strings()[2]
        );
    std::stringstream ss;
    e_123 f(e_two);
    e_123 g;
    ss << f;
    ss >> g;
    BOOST_TEST("2 Two" == g.str());

    e_123 m(e_one);
    e_123 n(e_two);

    BOOST_TEST(m == m);
    BOOST_TEST(m != n);

    BOOST_TEST(n == higher_of_0(m, n));
// This
//   BOOST_TEST(n == higher_of_0(m, e_two));
// doesn't work, by design, because of 'explicit'.
// See section 14.2 of the boost coding guidelines.
    BOOST_TEST(n == higher_of_0(m, e_123(e_two)));

    BOOST_TEST(n == higher_of_1(m, n));
// This
//    BOOST_TEST(n == higher_of_1(m, e_two));
// doesn't work, by design, because of 'explicit'.
// See section 14.2 of the boost coding guidelines.
    BOOST_TEST(n == higher_of_1(m, e_123(e_two)));

    int return_value = -1;

// COMPILER !! With bc++5.02, when TEST_GLOBALS is defined, this
// fails unless the '-xd-' flag is used to prevent calling dtors
// when the stack is unwound due to the (intentional) exception.
// There is a well-known problem with unwinding with this compiler;
// see the borland newsgroups.
#ifdef RESUMPTION_SEMANTICS_SUPPRESSED_AND_TERMINATION_SEMANTICS_RESTORED
    try
        {
        // This should throw:
        e_123 x("ABC");
        // This shouldn't be reached (it serves only to prevent a diagnostic):
        ss << x;
        // This shouldn't be reached:
        BOOST_TEST(false);
        }
    catch(std::exception const& e)
        {
        return_value = 0;
        }
#else // not RESUMPTION_SEMANTICS_SUPPRESSED_AND_TERMINATION_SEMANTICS_RESTORED
    return_value = 0;
#endif // not RESUMPTION_SEMANTICS_SUPPRESSED_AND_TERMINATION_SEMANTICS_RESTORED

// This should not compile:
//  e_foobar s;
//  e_123 g(s);

// This should not compile:
//  e_empty z;

    return return_value;
}

