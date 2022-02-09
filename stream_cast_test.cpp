// Convert between types as extractors and inserters do--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "stream_cast.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

/// A streamlined clone of stream_cast<>() with simpler diagnostics.

template<typename To, typename From>
To streamlined(From from, To = To())
{
    To result = To();
    static std::stringstream interpreter = []
        {
        std::stringstream ss {};
        ss.imbue(blank_is_not_whitespace_locale());
        return ss;
        } ();
    interpreter.str(std::string{});
    interpreter.clear();

    if
        (  !(interpreter << from)
        || !(interpreter >> result)
        || !(interpreter >> std::ws).eof()
        )
        {
        throw std::runtime_error("Oops!");
        }

    return result;
}

void assay_speed()
{
    static double const e {2.718281828459045};
    auto f0 = [] {for(int n = 0; n < 1000; ++n) stream_cast<std::string>(e);};
    auto f1 = [] {for(int n = 0; n < 1000; ++n) streamlined<std::string>(e);};
    std::cout
        << "\n  Speed tests..."
        << "\n  stream_cast: " << TimeAnAliquot(f0)
        << "\n  streamlined: " << TimeAnAliquot(f1)
        << std::endl
        ;
}

int test_main(int, char*[])
{
    std::string s;

    // First, test each exception coded in the primary template.

    // Induce failure in ostream inserter:
    std::stringstream ss0;
    ss0 << static_cast<std::streambuf*>(nullptr);
    LMI_TEST(!ss0);
    LMI_TEST_THROW
        (stream_cast<std::string>(static_cast<std::streambuf*>(nullptr))
        ,std::runtime_error
        ,lmi_test::what_regex("^Failure in ostream inserter")
        );

    // Induce failure in istream extractor:
    std::stringstream ss1;
    ss1 << "3";
    LMI_TEST(! !ss1);
    bool b {0};
    ss1 >> b;
    LMI_TEST(!ss1);
    LMI_TEST_THROW
        (stream_cast<bool>("3")
        ,std::runtime_error
        ,lmi_test::what_regex("^Failure in istream extractor")
        );

    // Throw if any trailing input remains...
    LMI_TEST_THROW
        (stream_cast<double>("3.14 59")
        ,std::runtime_error
        ,lmi_test::what_regex("^Unconverted data remains")
        );
    LMI_TEST_THROW
        (stream_cast<double>("3.14\r59")
        ,std::runtime_error
        ,lmi_test::what_regex("^Unconverted data remains")
        );
    // ...unless it's all whitespace...
    LMI_TEST_EQUAL(2, stream_cast<int>("2\r"));
    // ...as designated by blank_is_not_whitespace_locale()
    LMI_TEST_THROW
        (stream_cast<double>("3.14 ")
        ,std::runtime_error
        ,lmi_test::what_regex("^Unconverted data remains")
        );

    // Conversion from an empty std::string to another std::string
    // works only because a specialization is used in that case.
    // This would fail if the type converted to were of a different
    // string-like type: the stream inserter would set failbit, and
    // that's an essential feature of the technique. In general,
    // prefer function template value_cast, and specialize it for
    // string-like classes.

    s = stream_cast<std::string>(s);
    LMI_TEST(s.empty());

    LMI_TEST_EQUAL("", stream_cast<std::string>(""));

    s = "Not empty.";
    std::string empty("");
    s = stream_cast<std::string>(empty);
    LMI_TEST_EQUAL("", s);

    LMI_TEST_EQUAL( "Z" , stream_cast<std::string>( "Z" ));
    LMI_TEST_EQUAL(" Z" , stream_cast<std::string>(" Z" ));
    LMI_TEST_EQUAL( "Z ", stream_cast<std::string>( "Z "));
    LMI_TEST_EQUAL(" Z ", stream_cast<std::string>(" Z "));

    char* c0 = const_cast<char*>("as df"); // Cast avoids 4.2/2 warning.
    s = stream_cast<std::string>(c0);
    LMI_TEST_EQUAL("as df", s);
    char const* c1 = "jk l;";
    s = stream_cast(c1, s);
    LMI_TEST_EQUAL("jk l;", s);

    std::string s0 = " !@ #$% ";
    s = stream_cast<std::string>(s0);
    LMI_TEST_EQUAL(" !@ #$% ", s);
    std::string const s1 = "  ^&  *()  ";
    s = stream_cast(s1, s);
    LMI_TEST_EQUAL("  ^&  *()  ", s);

    std::string const& r0(s0);
    s = stream_cast<std::string>(r0);
    LMI_TEST_EQUAL(" !@ #$% ", s);
    std::string const& r1(s1);
    s = stream_cast(r1, s);
    LMI_TEST_EQUAL("  ^&  *()  ", s);

    s = stream_cast(r0, s);
    LMI_TEST_EQUAL(" !@ #$% ", s);
    s = stream_cast(r1, r1);
    LMI_TEST_EQUAL("  ^&  *()  ", s);

    // Attempting to construct a std::string from a null pointer to
    // char or char const elicits undefined behavior. The volatile
    // qualifier need not be tested because 27.6.2.5.4/3 does not
    // apply to stream inserters for volatile pointers to char; as
    // this message
    //   http://groups.google.com/group/comp.lang.c++.moderated/msg/6022d0bc84207ff1
    // explains, a conversion to bool is used instead.

    LMI_TEST_THROW
        (stream_cast<std::string>(static_cast<char*>(nullptr))
        ,std::runtime_error
        ,"Cannot convert (char*)(0) to std::string."
        );

    LMI_TEST_THROW
        (stream_cast<std::string>(static_cast<char const*>(nullptr))
        ,std::runtime_error
        ,"Cannot convert (char const*)(0) to std::string."
        );

    assay_speed();

    return 0;
}
