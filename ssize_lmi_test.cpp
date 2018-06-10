// Reformed std::size() returning a signed integer: unit test.
//
// Copyright (C) 2018 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ssize_lmi.hpp"

#include "bourn_cast.hpp"
#include "test_tools.hpp"

#include <array>
#include <cstring>                      // strlen()
#include <iterator>                     // size()
#include <string>
#include <vector>

int test_main(int, char*[])
{
    char const c[2] = {'0'};
    BOOST_TEST_EQUAL(lmi::ssize(c), 2);
    BOOST_TEST_EQUAL(lmi::ssize(c), bourn_cast<int>(std::size(c)));

    std::array<int,3> const a{1, 2};
    BOOST_TEST_EQUAL(lmi::ssize(a), 3);
    BOOST_TEST_EQUAL(lmi::ssize(a), bourn_cast<int>(std::size(a)));

    std::vector<int> const v(5);
    BOOST_TEST_EQUAL(lmi::ssize(v), 5);
    BOOST_TEST_EQUAL(lmi::ssize(v), bourn_cast<int>(std::size(v)));

    std::string const s("abcdefg");
    BOOST_TEST_EQUAL(lmi::ssize(s), 7);
    BOOST_TEST_EQUAL(lmi::ssize(s), bourn_cast<int>(std::size(s)));

    char const* p = "ABCDEFGHIJK";
    BOOST_TEST_EQUAL(lmi::sstrlen(p), 11);
    BOOST_TEST_EQUAL(lmi::sstrlen(p), bourn_cast<int>(std::strlen(p)));

    return 0;
}

