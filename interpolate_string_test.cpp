// Interpolate string containing embedded variable references.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "interpolate_string.hpp"

#include "test_tools.hpp"

#include <stdexcept>

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
    LMI_TEST_EQUAL( test_interpolate(""),               ""        );
    LMI_TEST_EQUAL( test_interpolate("literal"),        "literal" );
    LMI_TEST_EQUAL( test_interpolate("{{foo}}"),        "foo"     );
    LMI_TEST_EQUAL( test_interpolate("{{foo}}bar"),     "foobar"  );
    LMI_TEST_EQUAL( test_interpolate("foo{{}}bar"),     "foobar"  );
    LMI_TEST_EQUAL( test_interpolate("foo{{bar}}"),     "foobar"  );
    LMI_TEST_EQUAL( test_interpolate("{{foo}}{{bar}}"), "foobar"  );

    // Comments should be just ignored.
    LMI_TEST_EQUAL( test_interpolate("{{! ignore me}}"), ""       );
    LMI_TEST_EQUAL( test_interpolate("{{! too}}{{x}}"),  "x"      );
    LMI_TEST_EQUAL( test_interpolate("{{x}}{{!also}}"),  "x"      );

    // Recursive interpolation should work too.
    auto const test_recursive = [](char const* s)
        {
        return interpolate_string
            (s
            ,[](std::string const& k, interpolate_lookup_kind) -> std::string
                {
                if(k == "rec1") return "1 {{rec2}}";
                if(k == "rec2") return "2 {{rec3}}";
                if(k == "rec3") return "3"         ;
                if(k == "inf" ) return "{{inf}}"   ;
                if(k == "infA") return "{{infB}}"  ;
                if(k == "infB") return "{{infA}}"  ;

                throw std::runtime_error("no such variable '" + k + "'");
                }
            );
        };

    LMI_TEST_EQUAL( test_recursive("{{rec3}}"), "3"     );
    LMI_TEST_EQUAL( test_recursive("{{rec2}}"), "2 3"   );
    LMI_TEST_EQUAL( test_recursive("{{rec1}}"), "1 2 3" );

    LMI_TEST_THROW
        (test_recursive("error due to infinite recursion in {{inf}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Nesting level too deep")
        );

    LMI_TEST_THROW
        (test_recursive("infinite co-recursion in {{infA}} is detected too")
        ,std::runtime_error
        ,lmi_test::what_regex("Nesting level too deep")
        );

    // Sections.
    auto const section_test = [](char const* str)
        {
        return interpolate_string
            (str
            ,[](std::string const& s, interpolate_lookup_kind) -> std::string
                {
                if(s == "var0") return "0";
                if(s == "var1") return "1";
                if(s == "var" ) return "" ;

                throw std::runtime_error("no such variable '" + s + "'");
                }
            );
        };

    LMI_TEST_EQUAL( section_test("x{{#var1}}y{{/var1}}z"),   "xyz"    );
    LMI_TEST_EQUAL( section_test("x{{#var0}}y{{/var0}}z"),   "xz"     );
    LMI_TEST_EQUAL( section_test("x{{^var0}}y{{/var0}}z"),   "xyz"    );
    LMI_TEST_EQUAL( section_test("x{{^var1}}y{{/var1}}z"),   "xz"     );

    LMI_TEST_EQUAL
        (section_test("a{{#var1}}b{{#var1}}c{{/var1}}d{{/var1}}e")
        ,"abcde"
        );
    LMI_TEST_EQUAL
        (section_test("a{{#var1}}b{{#var0}}c{{/var0}}d{{/var1}}e")
        ,"abde"
        );
    LMI_TEST_EQUAL
        (section_test("a{{^var1}}b{{#var0}}c{{/var0}}d{{/var1}}e")
        ,"ae"
        );
    LMI_TEST_EQUAL
        (section_test("a{{^var1}}b{{^var0}}c{{/var0}}d{{/var1}}e")
        ,"ae"
        );

    // Partials.
    auto const partial_test = [](char const* str)
        {
        return interpolate_string
            (str
            ,[](std::string const& s, interpolate_lookup_kind) -> std::string
                {
                if(s == "header")       return "[header with {{var}}]";
                if(s == "footer")       return "[footer with {{var}}]";
                if(s == "nested")       return "[header with {{>footer}}]";
                if(s == "recursive")    return "{{>recursive}}";
                if(s == "sec" )         return "1" ;
                if(s == "var" )         return "variable" ;

                throw std::runtime_error("no such variable '" + s + "'");
                }
            );
        };

    LMI_TEST_EQUAL
        (partial_test("{{>header}}")
        ,"[header with variable]"
        );

    LMI_TEST_EQUAL
        (partial_test("{{>header}}{{var}} in body{{>footer}}")
        ,"[header with variable]variable in body[footer with variable]"
        );

    LMI_TEST_EQUAL
        (partial_test("{{#sec}}{{>header}}{{/sec}}")
        ,"[header with variable]"
        );

    LMI_TEST_EQUAL
        (partial_test("only{{^sec}}{{>header}}{{/sec}}{{>footer}}")
        ,"only[footer with variable]"
        );

    LMI_TEST_EQUAL
        (partial_test("{{>nested}}")
        ,"[header with [footer with variable]]"
        );

    LMI_TEST_THROW
        (partial_test("{{>recursive}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Nesting level too deep")
        );

    LMI_TEST_EQUAL
        (partial_test("no {{^sec}}{{>recursive}}{{/sec}} problem")
        ,"no  problem"
        );

    // Check that the kind of variable being expanded is correct.
    LMI_TEST_EQUAL
        (interpolate_string
            ("{{>test}}"
             "{{#section1}}{{^section0}}{{variable}}{{/section0}}{{/section1}}"
             ,[](std::string const& s, interpolate_lookup_kind kind) -> std::string
                {
                switch(kind)
                    {
                    case interpolate_lookup_kind::variable:
                        return "value of " + s;

                    case interpolate_lookup_kind::section:
                        // Get rid of the "section" prefix.
                        return s.substr(7);

                    case interpolate_lookup_kind::partial:
                        return s + " partial included\n";
                    }

                throw std::runtime_error("invalid lookup kind");
                }
            )
        ,"test partial included\nvalue of variable"
        );

    // Should throw if the input syntax is invalid.
    LMI_TEST_THROW
        (test_interpolate("{{x")
        ,std::runtime_error
        ,lmi_test::what_regex("Unmatched opening brace")
        );
    LMI_TEST_THROW
        (test_interpolate("{{x{{y}}}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unexpected nested interpolation")
        );
    LMI_TEST_THROW
        (section_test("{{#var1}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unclosed section 'var1'")
        );
    LMI_TEST_THROW
        (section_test("{{^var0}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unclosed section 'var0'")
        );
    LMI_TEST_THROW
        (section_test("{{/var1}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unexpected end of section")
        );
    LMI_TEST_THROW
        (section_test("{{#var1}}{{/var0}}")
        ,std::runtime_error
        ,lmi_test::what_regex("Unexpected end of section")
        );

    // Or because the lookup function throws.
    LMI_TEST_THROW
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
