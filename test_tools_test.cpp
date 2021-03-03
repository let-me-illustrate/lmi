// Test the testing framework.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "test_tools.hpp"

#include <iostream>
#include <ostream>

bool volatile always_true  = true;
bool volatile always_false = false;

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
    // Tests in this special section may be designed to fail. Their
    // failures are reported with a distinctive prefix so that they
    // don't look like real errors.

    lmi_test::error_prefix = "\n#### ";

    BOOST_TEST(always_true);
    BOOST_TEST(always_false);

    BOOST_TEST_EQUAL(always_true, always_true );
    BOOST_TEST_EQUAL(always_true, always_false);

    BOOST_TEST_UNEQUAL(always_true, always_true );
    BOOST_TEST_UNEQUAL(always_true, always_false);

    test_function(always_true , always_true, __FILE__, __LINE__);
    test_function(always_false, always_true, __FILE__, __LINE__);


    BOOST_TEST_THROW((void)(0), std::runtime_error, "arbitrary");
    BOOST_TEST_THROW(;, std::runtime_error, "arbitrary");

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
        << "\n[This is a test of the testing framework's error-reporting"
        << "\nfacilities. It is contrived to report simulated errors,"
        << "\nwhich are marked thus:"
        << lmi_test::error_prefix << "(simulation of simulated error)"
        << "\nto distinguish them from real errors and are excluded from"
        << "\nthe count of real errors reported upon exit.]"
        << std::endl
        ;
    lmi_test::test::test_tools_errors = 0;

    // The following tests, unlike those above, should not fail.

    lmi_test::error_prefix = lmi_test::default_error_prefix;

    // Ensure that the anticipated and actually-thrown exceptions are
    // treated as equivalent even though the latter has an extra
    // terminal substring beginning with "\n[file ", which some lmi
    // exceptions add.

    BOOST_TEST_THROW
        (throw_exception
            (std::runtime_error
                ("arbitrary"
                "\n[file <remainder of terminal substring to ignore>"
                )
            )
        ,std::runtime_error
        ,"arbitrary"
        );

    // Test exception::what() against a regular expression.

    BOOST_TEST_THROW
        (throw_exception(std::runtime_error("Iteration 31: failure."))
        ,std::runtime_error
        ,lmi_test::what_regex("^Iteration [0-9]*: failure\\.$")
        );

    // Test whats_what().

    // [Here, '.*$' means what it would mean if this were a regex.]
    std::string const observed = "xyzzy\n[file .*$";
    // An expectation given as "" means that the what-string is not to
    // be tested at all, because it was impossible, difficult, or just
    // unimportant to specify an actual expectation when the test was
    // written. It doesn't mean that an empty what-string is expected;
    // it only means that any what-string is accepted.
    BOOST_TEST( lmi_test::whats_what(observed, ""));
    // A full exact match is accepted [and here '.*$' is no regex]:
    BOOST_TEST( lmi_test::whats_what(observed, "xyzzy\n[file .*$"));
    // Alternatively, discard any portion of the what-string that
    // begins with "\n[file " (presumably appended by LMI_FLUSH) and
    // test that truncated what-string. An exact match is accepted:
    BOOST_TEST( lmi_test::whats_what(observed, "xyzzy"));
    // However, partial matches are rejected:
    BOOST_TEST(!lmi_test::whats_what(observed, "xyzz"));
    BOOST_TEST(!lmi_test::whats_what(observed, "xyzzy!"));
    // The expectation must exactly equal either the untruncated or
    // the truncated what-string; an exact match to a "partially
    // truncated" what-string is rejected:
    BOOST_TEST(!lmi_test::whats_what(observed, "xyzzy\n"));
    BOOST_TEST(!lmi_test::whats_what(observed, "xyzzy\n[file .*"));

    return 0;
}
