// Manage files of name-value pairs--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "name_value_pairs.hpp"

#include "miscellany.hpp"
#include "test_tools.hpp"

#if !defined __BORLANDC__
#   include <boost/filesystem/operations.hpp>
#   include <boost/filesystem/path.hpp>
#endif // !defined __BORLANDC__

#include <cstdio> // std::remove()
#include <fstream>

int test_main(int, char*[])
{
#if !defined __BORLANDC__
    fs::path const tmpdir(fs::complete("/tmp"));
    fs::create_directory(tmpdir);
#endif // !defined __BORLANDC__

    std::string filename0("/tmp/eraseme");

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

    name_value_pairs n_v_pairs_0(filename0);
    std::map<std::string, std::string> m0 = n_v_pairs_0.map();
    BOOST_TEST(0 == std::remove(filename0.c_str()));

    BOOST_TEST_EQUAL(15, m0.size());

    // Test string_value().

    BOOST_TEST_EQUAL("a"     , n_v_pairs_0.string_value("a"));
    BOOST_TEST_EQUAL("b"     , n_v_pairs_0.string_value("b"));
    BOOST_TEST_EQUAL(""      , n_v_pairs_0.string_value("c"));
    BOOST_TEST_EQUAL("="     , n_v_pairs_0.string_value("d"));
    BOOST_TEST_EQUAL("1=."   , n_v_pairs_0.string_value("e"));
#ifndef LMI_MSW
    BOOST_TEST_EQUAL(" f \r" , n_v_pairs_0.string_value("f"));
#else  // LMI_MSW
    BOOST_TEST_EQUAL(" f "   , n_v_pairs_0.string_value("f"));
#endif // LMI_MSW
    BOOST_TEST_EQUAL("a test", n_v_pairs_0.string_value("this"));

    // Test numeric_value().

    BOOST_TEST_EQUAL( 2.0    , n_v_pairs_0.numeric_value("z"));
    BOOST_TEST_EQUAL(-3.142  , n_v_pairs_0.numeric_value("y"));
    BOOST_TEST_EQUAL( 2.718  , n_v_pairs_0.numeric_value("x"));
    BOOST_TEST_EQUAL( 2.718  , n_v_pairs_0.numeric_value("w"));
    BOOST_TEST_EQUAL( 2.718  , n_v_pairs_0.numeric_value("v"));
    BOOST_TEST_EQUAL( 0.0    , n_v_pairs_0.numeric_value("u"));
    BOOST_TEST_EQUAL( 0.0    , n_v_pairs_0.numeric_value("t"));
    BOOST_TEST_EQUAL( 0.0    , n_v_pairs_0.numeric_value("s"));

    BOOST_TEST_EQUAL("2.718" , n_v_pairs_0.string_numeric_value("v"));
    BOOST_TEST_EQUAL("0"     , n_v_pairs_0.string_numeric_value("s"));

    std::string filename1("/tmp/nonexistent_name_value_pairs_test_file");
    name_value_pairs n_v_pairs_1(filename1);
    std::map<std::string, std::string> m1 = n_v_pairs_1.map();
    BOOST_TEST_EQUAL(0, m1.size());

    return EXIT_SUCCESS;
}

