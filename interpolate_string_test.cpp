// Interpolate string containing embedded variable references.
//
// Copyright (C) 2017 Gregory W. Chicares.
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

#include "interpolate_string.hpp"

#include "test_tools.hpp"

int test_main(int, char*[])
{
    auto const test_interpolate = [](char const* s)
        {
        return interpolate_string(s, [](std::string const& k) { return k; });
        };

    // Check that basic interpolation works.
    BOOST_TEST_EQUAL( test_interpolate(""),             "" );
    BOOST_TEST_EQUAL( test_interpolate("${foo}"),       "foo" );
    BOOST_TEST_EQUAL( test_interpolate("${foo}bar"),    "foobar" );
    BOOST_TEST_EQUAL( test_interpolate("foo${}bar"),    "foobar" );
    BOOST_TEST_EQUAL( test_interpolate("foo${bar}"),    "foobar" );
    BOOST_TEST_EQUAL( test_interpolate("${foo}${bar}"), "foobar" );

    // Should throw if the input syntax is invalid.
    BOOST_TEST_THROW
        (test_interpolate("${x")
        ,std::runtime_error
        ,lmi_test::what_regex("Unmatched opening brace")
        );
    BOOST_TEST_THROW
        (test_interpolate("${x${y}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unexpected nested interpolation")
        );

    // Or because the lookup function throws.
    BOOST_TEST_THROW
        (interpolate_string
            ("${x}"
            ,[](std::string const& s) -> std::string
                {
                throw std::runtime_error("no such variable '" + s + "'");
                }
            )
            ,std::runtime_error
        ,"no such variable 'x'"
        );


    return EXIT_SUCCESS;
}
