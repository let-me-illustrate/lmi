// Alert messages--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: alert_test.cpp,v 1.9 2008-12-27 02:56:36 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"

#include "test_tools.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

int test_main(int, char*[])
{
    safely_show_message("This message should be shown.");

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

    // Run this 'fatal_error' test twice in order to ensure that the
    // stream state is cleared after an exception is thrown; if it is
    // not, then getting a reference to the stream again, e.g., by
    // calling fatal_error(), fails with an exception inside the
    // standard library, probably in std::ios_base::clear().

    std::string s("First simulated fatal error.");
    BOOST_TEST_THROW(fatal_error() << s << std::flush, std::runtime_error, s);

    s = "Second simulated fatal error.";
    BOOST_TEST_THROW(fatal_error() << s << std::flush, std::runtime_error, s);

#if defined __GLIBCPP__ && __GLIBCPP__==20030426
    std::cout
        << "This test fails with libstdc++-v3 if ostream.tcc is\n"
        << "older than version 1.30.2.14 of 2003-12-01T19:39:49 .\n"
        ;
#endif // Defective version of libstdc++-v3.

    return 0;
}

