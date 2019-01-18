// Convert between types as extractors and inserters do--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

#include "stream_cast.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

/// A minimalistic clone of stream_cast<>().

template<typename To, typename From>
To cast_1(From from, To = To())
{
    std::stringstream interpreter;
    interpreter.imbue(blank_is_not_whitespace_locale());

    To result = To();
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

/// Like cast_1<>(), but with static 'interpreter'.

template<typename To, typename From>
To cast_2(From from, To = To())
{
    static std::stringstream interpreter;
    interpreter.imbue(blank_is_not_whitespace_locale());
    interpreter.str(std::string{});
    interpreter.clear();

    To result = To();
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

std::stringstream imbued()
{
    std::stringstream interpreter;
    interpreter.imbue(blank_is_not_whitespace_locale());
    return interpreter;
}

/// Like cast_2<>(), but with statically imbued static 'interpreter'.

template<typename To, typename From>
To cast_3(From from, To = To())
{
    static std::stringstream interpreter {imbued()};
    interpreter.str(std::string{});
    interpreter.clear();
    To result = To();
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
    auto f1 = [] {for(int n = 0; n < 1000; ++n) cast_1     <std::string>(e);};
    auto f2 = [] {for(int n = 0; n < 1000; ++n) cast_2     <std::string>(e);};
    auto f3 = [] {for(int n = 0; n < 1000; ++n) cast_3     <std::string>(e);};
    std::cout
        << "\n  Speed tests..."
        << "\n  stream_cast     : " << TimeAnAliquot(f0)
        << "\n  minimalistic    : " << TimeAnAliquot(f1)
        << "\n  static stream   : " << TimeAnAliquot(f2)
        << "\n  static facet too: " << TimeAnAliquot(f3)
        << std::endl
        ;
}

int test_main(int, char*[])
{
    std::string s;

    // Conversion from an empty std::string to another std::string
    // works only because a specialization is used in that case.
    // This would fail if the type converted to were of a different
    // string-like type: the stream inserter would set failbit, and
    // that's an essential feature of the technique. In general,
    // prefer function template value_cast, and specialize it for
    // string-like classes.

    s = stream_cast<std::string>(s);
    BOOST_TEST(s.empty());

    BOOST_TEST_EQUAL("", stream_cast<std::string>(""));

    s = "Not empty.";
    std::string empty("");
    s = stream_cast<std::string>(empty);
    BOOST_TEST_EQUAL("", s);

    BOOST_TEST_EQUAL( "Z" , stream_cast<std::string>( "Z" ));
    BOOST_TEST_EQUAL(" Z" , stream_cast<std::string>(" Z" ));
    BOOST_TEST_EQUAL( "Z ", stream_cast<std::string>( "Z "));
    BOOST_TEST_EQUAL(" Z ", stream_cast<std::string>(" Z "));

    char* c0 = const_cast<char*>("as df"); // Cast avoids 4.2/2 warning.
    s = stream_cast<std::string>(c0);
    BOOST_TEST_EQUAL("as df", s);
    char const* c1 = "jk l;";
    s = stream_cast(c1, s);
    BOOST_TEST_EQUAL("jk l;", s);

    std::string s0 = " !@ #$% ";
    s = stream_cast<std::string>(s0);
    BOOST_TEST_EQUAL(" !@ #$% ", s);
    std::string const s1 = "  ^&  *()  ";
    s = stream_cast(s1, s);
    BOOST_TEST_EQUAL("  ^&  *()  ", s);

    std::string const& r0(s0);
    s = stream_cast<std::string>(r0);
    BOOST_TEST_EQUAL(" !@ #$% ", s);
    std::string const& r1(s1);
    s = stream_cast(r1, s);
    BOOST_TEST_EQUAL("  ^&  *()  ", s);

    s = stream_cast(r0, s);
    BOOST_TEST_EQUAL(" !@ #$% ", s);
    s = stream_cast(r1, r1);
    BOOST_TEST_EQUAL("  ^&  *()  ", s);

    // Attempting to construct a std::string from a null pointer to
    // char or char const elicits undefined behavior. The volatile
    // qualifier need not be tested because 27.6.2.5.4/3 does not
    // apply to stream inserters for volatile pointers to char; as
    // this message
    //   http://groups.google.com/group/comp.lang.c++.moderated/msg/6022d0bc84207ff1
    // explains, a conversion to bool is used instead.

    BOOST_TEST_THROW
        (stream_cast<std::string>((char*)nullptr)
        ,std::runtime_error
        ,"Cannot convert (char*)(0) to std::string."
        );

    BOOST_TEST_THROW
        (stream_cast<std::string>((char const*)nullptr)
        ,std::runtime_error
        ,"Cannot convert (char const*)(0) to std::string."
        );

    assay_speed();

    return 0;
}

