// Test the testing framework.
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

#include "test_tools.hpp"

#include <iostream>
#include <ostream>

volatile bool always_true  = true;
volatile bool always_false = false;

void test_function(bool a, bool b, char const* file, int line)
{
    INVOKE_BOOST_TEST(a, file, line);
    INVOKE_BOOST_TEST_EQUAL(a, b, file, line);
    INVOKE_BOOST_TEST_UNEQUAL(a, b, file, line);
}

// Throw an exception unconditionally, while preventing compilers from
// complaining that any following code is unreachable.
//
template<typename ExceptionType>
void throw_exception(ExceptionType const& e)
{
    if(always_true)
        {
        throw e;
        }
}

int test_main(int, char*[])
{
    BOOST_TEST(always_true);
    BOOST_TEST(always_false);

    BOOST_TEST_EQUAL(always_true, always_true );
    BOOST_TEST_EQUAL(always_true, always_false);

    BOOST_TEST_UNEQUAL(always_true, always_true );
    BOOST_TEST_UNEQUAL(always_true, always_false);

    test_function(always_true , always_true, __FILE__, __LINE__);
    test_function(always_false, always_true, __FILE__, __LINE__);

    try
        {
        BOOST_CRITICAL_TEST(always_true);
        BOOST_CRITICAL_TEST(always_false);
        }
    catch(lmi_test::test::test_tools_exception const&)
        {
        }

    BOOST_TEST_THROW((void)(0), std::runtime_error, "arbitrary");
    BOOST_TEST_THROW(;, std::runtime_error, "arbitrary");

    // COMPILER !! The next two tests fail with borland C++ 5.5.1 .
    // Probably this is a compiler defect, but someday this should be
    // investigated.

    BOOST_TEST_THROW
        (throw_exception(std::runtime_error("arbitrary"))
        ,std::logic_error
        ,"arbitrary"
        );

    BOOST_TEST_THROW
        (throw_exception(std::runtime_error("wrong what_arg"))
        ,std::runtime_error
        ,"right what_arg"
        );

    std::cout
        << "\n[This is a test of the testing framework's error-reporting\n"
        << "facilities. It is contrived to report simulated errors.\n"
        << "On exit, its error counter is overridden so that it reports\n"
        << "a total of zero errors.]"
        << std::endl
        ;
    lmi_test::test::test_tools_errors = 0;

    // This test, unlike the others above, should not fail. It makes
    // sure that the anticipated and actually-thrown exceptions are
    // treated as equivalent even though the latter has an extra
    // terminal substring beginning with "\n[file ", which some lmi
    // exceptions add.

    BOOST_TEST_THROW
        (throw_exception
            (std::runtime_error
                ("arbitrary"
                "\n[file <remainder of terminal substring to ignore>]"
                )
            )
        ,std::runtime_error
        ,"arbitrary"
        );

    return 0;
}

