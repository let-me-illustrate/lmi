// Alert messages--unit test.
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

#include "pchfile.hpp"

#include "alert.hpp"

#include "test_tools.hpp"

#include <algorithm>
#include <iterator>                     // ostream_iterator
#include <stdexcept>
#include <vector>

/// Demonstrate that alert streams can be used as arguments.

void test_stream_arg(std::ostream& os, std::string const& s)
{
    os << s << std::flush;
}

int test_main(int, char*[])
{
    safely_show_message("  This message should appear on stderr.");

    status()
        << "This should have no effect, because the handler for"
        << " the command-line interface gobbles status messages."
        << std::flush
        ;

    warning() << "This should be a single line," << std::flush;
    warning() << "and this should be another single line." << std::flush;

    warning() << "This";
    warning() << " sentence";
    warning() << " should";
    warning() << " occupy";
    warning() << " only";
    warning() << " one";
    warning() << " line." << std::flush;

    std::vector<double> v;
    v.push_back(2.718281828459045);
    v.push_back(1.0);
    warning() << "One would expect the natural logarithm of the first\n";
    warning() << " number in this list approximately to equal the second.\n";
    warning() << " list: ";
    std::copy
        (v.begin()
        ,v.end()
        ,std::ostream_iterator<double>(warning(), " ")
        );
    warning() << '\n';
    warning() << "File and line where this diagnostic arose:";
    warning() << LMI_FLUSH;

    // Run this 'alarum' test twice in order to ensure that the stream
    // state is cleared after an exception is thrown; if it is not,
    // then getting a reference to the stream again, e.g., by calling
    // alarum(), fails with an exception inside the standard library,
    // probably in std::ios_base::clear().

    std::string s("First simulated alarum.");
    LMI_TEST_THROW(alarum() << s << std::flush, std::runtime_error, s);

    s = "Second simulated alarum.";
    LMI_TEST_THROW(alarum() << s << std::flush, std::runtime_error, s);

    // The CLI handler should gobble this message.
    test_stream_arg(status(), "This should not be printed.");

    test_stream_arg(warning(), "This message should appear on stdout.");

    LMI_TEST_THROW(test_stream_arg(alarum(), "X"), std::runtime_error, "X");

    return 0;
}
