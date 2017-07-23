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
        return interpolate_string
            (s
            ,[](std::string const& k, interpolate_lookup_kind) { return k; }
            );
        };

    // Check that basic interpolation works.
    BOOST_TEST_EQUAL( test_interpolate(""),               ""        );
    BOOST_TEST_EQUAL( test_interpolate("literal"),        "literal" );
    BOOST_TEST_EQUAL( test_interpolate("{{foo}}"),        "foo"     );
    BOOST_TEST_EQUAL( test_interpolate("{{foo}}bar"),     "foobar"  );
    BOOST_TEST_EQUAL( test_interpolate("foo{{}}bar"),     "foobar"  );
    BOOST_TEST_EQUAL( test_interpolate("foo{{bar}}"),     "foobar"  );
    BOOST_TEST_EQUAL( test_interpolate("{{foo}}{{bar}}"), "foobar"  );

    // Sections.
    auto const section_test = [](char const* s)
        {
        return interpolate_string
            (s
            ,[](std::string const& s, interpolate_lookup_kind) -> std::string
                {
                if(s == "var0") return "0";
                if(s == "var1") return "1";
                if(s == "var" ) return "" ;

                throw std::runtime_error("no such variable '" + s + "'");
                }
            );
        };

    BOOST_TEST_EQUAL( section_test("x{{#var1}}y{{/var1}}z"),   "xyz"    );
    BOOST_TEST_EQUAL( section_test("x{{#var0}}y{{/var0}}z"),   "xz"     );
    BOOST_TEST_EQUAL( section_test("x{{^var0}}y{{/var0}}z"),   "xyz"    );
    BOOST_TEST_EQUAL( section_test("x{{^var1}}y{{/var1}}z"),   "xz"     );

    BOOST_TEST_EQUAL
        (section_test("a{{#var1}}b{{#var1}}c{{/var1}}d{{/var1}}e")
        ,"abcde"
        );
    BOOST_TEST_EQUAL
        (section_test("a{{#var1}}b{{#var0}}c{{/var0}}d{{/var1}}e")
        ,"abde"
        );
    BOOST_TEST_EQUAL
        (section_test("a{{^var1}}b{{#var0}}c{{/var0}}d{{/var1}}e")
        ,"ae"
        );
    BOOST_TEST_EQUAL
        (section_test("a{{^var1}}b{{^var0}}c{{/var0}}d{{/var1}}e")
        ,"ae"
        );

    // Some special cases.
    BOOST_TEST_EQUAL
        (interpolate_string
            ("{{expanded}}"
            ,[](std::string const& s, interpolate_lookup_kind) -> std::string
                {
                if(s == "expanded")
                    {
                    return "{{unexpanded}}";
                    }
                throw std::runtime_error("no such variable '" + s + "'");
                }
            )
        ,"{{unexpanded}}"
        );

    // Check that the kind of variable being expanded is correct.
    BOOST_TEST_EQUAL
        (interpolate_string
            ("{{#section1}}{{^section0}}{{variable}}{{/section0}}{{/section1}}"
            ,[](std::string const& s, interpolate_lookup_kind kind)
                {
                switch(kind)
                    {
                    case interpolate_lookup_kind::variable:
                        return "value of " + s;

                    case interpolate_lookup_kind::section:
                        // Get rid of the "section" prefix.
                        return s.substr(7);
                    }

                throw std::runtime_error("invalid lookup kind");
                }
            )
        ,"value of variable"
        );

    // Should throw if the input syntax is invalid.
    BOOST_TEST_THROW
        (test_interpolate("{{x")
        ,std::runtime_error
        ,lmi_test::what_regex("Unmatched opening brace")
        );
    BOOST_TEST_THROW
        (test_interpolate("{{x{{y}}}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unexpected nested interpolation")
        );
    BOOST_TEST_THROW
        (section_test("{{#var1}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unclosed section 'var1'")
        );
    BOOST_TEST_THROW
        (section_test("{{^var0}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unclosed section 'var0'")
        );
    BOOST_TEST_THROW
        (section_test("{{/var1}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unexpected end of section")
        );
    BOOST_TEST_THROW
        (section_test("{{#var1}}{{/var0}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unexpected end of section")
        );

    // Or because the lookup function throws.
    BOOST_TEST_THROW
        (interpolate_string
            ("{{x}}"
            ,[](std::string const& s, interpolate_lookup_kind) -> std::string
                {
                throw std::runtime_error("no such variable '" + s + "'");
                }
            )
            ,std::runtime_error
        ,"no such variable 'x'"
        );

    return EXIT_SUCCESS;
}
