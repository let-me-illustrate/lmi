// Unit-test framework based on Beman Dawes's boost library.
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

// This is a derived work based on Beman Dawes's boost test library
// that bears the following copyright and license statement:
// [Beman Dawes's copyright and license statement begins]
// (C) Copyright Beman Dawes 1995-2001. Permission to copy, use, modify, sell
// and distribute this software is granted provided this copyright notice
// appears in all copies. This software is provided "as is" without express or
// implied warranty, and with no claim as to its suitability for any purpose.
//
// See http://www.boost.org for updates, documentation, and revision history.
// [end Beman Dawes's copyright and license statement]
//
// Gregory W. Chicares modified it in 2004 to make it independent of
// the boost directory structure, and in any other ways indicated
// below, and in any later years shown above. Any defect in it should
// not reflect on Beman Dawes's reputation.

// The original boost.org test library upon which this derived work is
// based was later replaced by a very different library. That new test
// library has more features but is less transparent; it is not
// strictly compatible with tests written for the original library;
// and, for boost-1.31.0 at least, it didn't work out of the box with
// the latest como compiler. The extra features don't seem to be worth
// the cost.
//
// It seems unwise to mix boost versions, and better to extract this
// library from boost-1.23.0 and remove its dependencies on other
// parts of boost, which is easy to do because, as Beman Dawes says:
// Header dependencies are deliberately restricted to reduce coupling.

// boost/test_main.cpp (header or not, as you like it) ----------------------//
//
// This file may be included as a header file, or may be compiled and placed
// in a library for traditional linking. It is unusual for non-template
// non-inline implementation code to be used as a header file, but the user
// may elect to do so because header-only implementation requires no library
// build support. (Suggested by Ed Brey)

#include "pchfile.hpp"

#include "exit_codes.hpp"
#include "fenv_lmi.hpp"
#include "miscellany.hpp"               // stifle_warning_for_unused_value()
#include "test_tools.hpp"

#include <atomic>
#include <iostream>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>

// GWC changed namespace 'boost' to prevent any conflict with code in
// a later version of boost.
namespace lmi_test
{
  namespace test
  {
    std::atomic<int> test_tools_errors    {0}; // Count of errors detected.
    std::atomic<int> test_tools_successes {0}; // Count of successful tests.

    class test_tools_exception : public std::runtime_error
    {
      public:
        test_tools_exception() : std::runtime_error("fatal test error") {}
    };
  } // namespace test

    // Rationale for writing these prefixen as concatenations of short
    // string literals: during pre-release testing, an error in the
    // following statement created an appearance that every test had
    // succeeded when actually none could be built, because gcc emitted
    // the (not-then-obfuscated) success prefix once for each test as
    // part of an error message...and therefore 'grep --count' found
    // exactly the expected number of apparent success markers.

    std::string success_prefix             = "\n!""!""!""! ";

    // There is no 'default_success_prefix' because there is no need to
    // override 'success_prefix'.

    std::string const default_error_prefix = "\n?""?""?""? ";

    // Change this to test this testing library's facilities without
    // emitting this actual prefix, e.g., to force simulated errors.
    // Change it back to perform tests that are intended to pass,
    // e.g., tests to validate internal helpers such as whats_what().
    std::string error_prefix = default_error_prefix;

    std::ostream& error_stream()
    {
        return std::cout << error_prefix << "test failed: ";
    }

    void record_error()
    {
        ++test::test_tools_errors;
    }

    void record_success()
    {
        ++test::test_tools_successes;
    }

    /// Preserve regex ctor argument so stream inserter can write it.
    ///
    /// The sole motivation for this simple std::regex wrapper is to
    /// let BOOST_TEST_THROW print the regex in diagnostics like:
    ///   "Caught 'XYZ' but expected '[0-9]*'."

    class what_regex
    {
      public:
        what_regex(std::string const& s) : s_(s) {}

        std::string const& str() const {return s_;}

      private:
        std::string s_;
    };

    std::ostream& operator<<(std::ostream& os, what_regex const& z)
    {
        return os << z.str();
    }

    bool whats_what(std::string const& observed, what_regex const& expected)
    {
        return std::regex_search(observed, std::regex(expected.str()));
    }
} // namespace lmi_test

// cpp_main()  --------------------------------------------------------------//

// See the cpp_main.cpp comments; they apply to cpp_main and test_main.

int test_main(int argc, char* argv[]);  // Prototype for user's test_main().

int cpp_main(int argc, char* argv[])
{
    fenv_initialize();

    int result = 0;
    stifle_warning_for_unused_value(result);

    try
        {
        result = test_main( argc, argv );
        }

    // The rules for catch & arguments are a bit different from function
    // arguments (ISO 15.3 paragraphs 18 & 19). Apparently const isn't
    // required, but it doesn't hurt and some programmers ask for it.

    catch(lmi_test::test::test_tools_exception const&)
        {
        std::cout
            << lmi_test::error_prefix
            << "previous test error is fatal"
            << std::endl
            ;
        // Reset so we don't get two messages.
        lmi_test::test::test_tools_errors = 0;
        result = lmi_test::exit_test_failure;
        }

    if(lmi_test::test::test_tools_errors)
        {
        std::cout
            << lmi_test::error_prefix
            << lmi_test::test::test_tools_errors
            << " test errors detected; "
            << lmi_test::test::test_tools_successes
            << " tests succeeded"
            << std::endl
            ;
        result = lmi_test::exit_test_failure;
        }
    else
        {
        std::cout
            << "\n.... "
            << lmi_test::test::test_tools_successes
            << " tests succeeded"
            << std::endl
            ;
        }

    return result;
}
