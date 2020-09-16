// Manage files of name-value pairs--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "name_value_pairs.hpp"

#include "miscellany.hpp"
#include "path.hpp"
#include "test_tools.hpp"

#include <cstdio>                       // remove()
#include <fstream>
#include <string>

int test_main(int, char*[])
{
    std::string const tmp = "/tmp/" + fs::path{__FILE__}.stem().string();
    fs::path const tmpdir(fs::absolute(tmp));
    fs::create_directory(tmpdir);

    std::string filename0(tmp + "/eraseme");

    {
    std::ofstream os(filename0.c_str(), ios_out_trunc_binary());
    BOOST_TEST(os.good());

    os
        << "a=a\n"
        << "No equal sign--line should be ignored.\n"
        << "\n"
        << "b=b\n"
        << "c=\n"
        << "d==\n"
        << "e=1=.\n"
        << "f= f \r\n"
        << "this=a test\n"
        << "z=2\n"
        << "y=-3.142\n"
        << "x=2.718 \n"
        << "w= 2.718\n"
        << "v= 2.718 \n"
        << "u=\n"
        << "t= \n"
        << "s=       \n"
        ;
    }

    name_value_pairs nv_pairs_0(filename0);
    std::map<std::string, std::string> m0 = nv_pairs_0.map();
    BOOST_TEST(0 == std::remove(filename0.c_str()));

    BOOST_TEST_EQUAL(15, m0.size());

    // Test string_value().

    BOOST_TEST_EQUAL("a"     , nv_pairs_0.string_value("a"));
    BOOST_TEST_EQUAL("b"     , nv_pairs_0.string_value("b"));
    BOOST_TEST_EQUAL(""      , nv_pairs_0.string_value("c"));
    BOOST_TEST_EQUAL("="     , nv_pairs_0.string_value("d"));
    BOOST_TEST_EQUAL("1=."   , nv_pairs_0.string_value("e"));
#if !defined LMI_MSW
    BOOST_TEST_EQUAL(" f \r" , nv_pairs_0.string_value("f"));
#else  // defined LMI_MSW
    BOOST_TEST_EQUAL(" f "   , nv_pairs_0.string_value("f"));
#endif // defined LMI_MSW
    BOOST_TEST_EQUAL("a test", nv_pairs_0.string_value("this"));

    // Test numeric_value().

    BOOST_TEST_EQUAL( 2.0    , nv_pairs_0.numeric_value("z"));
    BOOST_TEST_EQUAL(-3.142  , nv_pairs_0.numeric_value("y"));
    BOOST_TEST_EQUAL( 2.718  , nv_pairs_0.numeric_value("x"));
    BOOST_TEST_EQUAL( 2.718  , nv_pairs_0.numeric_value("w"));
    BOOST_TEST_EQUAL( 2.718  , nv_pairs_0.numeric_value("v"));
    BOOST_TEST_EQUAL( 0.0    , nv_pairs_0.numeric_value("u"));
    BOOST_TEST_EQUAL( 0.0    , nv_pairs_0.numeric_value("t"));
    BOOST_TEST_EQUAL( 0.0    , nv_pairs_0.numeric_value("s"));

    BOOST_TEST_EQUAL("2.718" , nv_pairs_0.string_numeric_value("v"));
    BOOST_TEST_EQUAL("0"     , nv_pairs_0.string_numeric_value("s"));

    std::string filename1(tmp + "/nonexistent_name_value_pairs_test_file");
    name_value_pairs nv_pairs_1(filename1);
    std::map<std::string, std::string> m1 = nv_pairs_1.map();
    BOOST_TEST_EQUAL(0, m1.size());

    fs::remove(tmpdir);

    return EXIT_SUCCESS;
}
