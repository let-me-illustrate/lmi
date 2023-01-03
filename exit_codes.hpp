// Unit-test framework based on Beman Dawes's boost library.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
// (C) Copyright Beman Dawes 2001. Permission to copy, use, modify, sell
// and distribute this software is granted provided this copyright notice
// appears in all copies. This software is provided "as is" without express or
// implied warranty, and with no claim as to its suitability for any purpose.
//
// See http://www.boost.org for updates and documentation.
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

#ifndef exit_codes_hpp
#define exit_codes_hpp

#include "config.hpp"

#include <cstdlib>
#include <string>

// GWC changed namespace 'boost' to prevent any conflict with code in
// a later version of boost.

namespace lmi_test
{
    // The intent is to propose the following for addition to namespace std
    // in the C++ Standard Library, and to then deprecate EXIT_SUCCESS and
    // EXIT_FAILURE.  As an implementation detail, this header defines the
    // new constants in terms of EXIT_SUCCESS and EXIT_FAILURE.  In a new
    // standard, the constants would be implementation-defined, although it
    // might be worthwhile to "suggest" (which a standard is allowed to do)
    // values of 0 and 1 respectively.

    // Rationale for having multiple failure values: some environments may
    // wish to distinguish between different classes of errors.
    // Rationale for choice of values: programs often use values < 100 for
    // their own error reporting.  Values > 255 are sometimes reserved for
    // system detected errors.  200/201 were suggested to minimize conflict.

    int const exit_success  = EXIT_SUCCESS; // implementation-defined value
    int const exit_failure  = EXIT_FAILURE; // implementation-defined value
    int const exit_exception_failure = 200; // otherwise uncaught exception
    int const exit_test_failure      = 201; // report_error or
                                            // report_critical_error called.

    // Decoration of test results. This goes slightly beyond the goal of the
    // original boost implementation, which was merely to propose non-macro
    // exit codes for standardization, yet it's not illogical (because the
    // purpose is to report success or failure at exit), and it's convenient
    // (because this is the one header included by all unit-test files) to
    // write such declarations here.

    extern std::string             success_prefix;
    extern std::string const default_error_prefix;
    extern std::string               error_prefix;
} // namespace lmi_test

#endif // exit_codes_hpp
