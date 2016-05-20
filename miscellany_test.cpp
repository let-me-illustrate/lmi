// Miscellaneous functions--unit test.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "miscellany.hpp"

#include "test_tools.hpp"

#include <cstdio>                       // std::remove()
#include <fstream>

void test_files_are_identical()
{
    char const* f0("unlikely_file_name_0");
    char const* f1("unlikely_file_name_1");

    std::remove(f0);
    std::remove(f1);

    // Nonexistent files.
    BOOST_TEST_THROW
        (files_are_identical(f0, f1)
        ,std::runtime_error
        ,"Unable to open 'unlikely_file_name_0'."
        );

    // Identical empty files.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    }
    BOOST_TEST(files_are_identical(f0, f1));

    // Identical nonempty files.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    ofs0 << "Test";
    ofs1 << "Test";
    }
    BOOST_TEST(files_are_identical(f0, f1));

    // Files whose contents might be identical in text mode but differ
    // in binary mode are nonidentical.
    {
    std::ofstream ofs0(f0, ios_out_trunc_binary());
    std::ofstream ofs1(f1, ios_out_trunc_binary());
    ofs0 << "Test\r\n";
    ofs1 << "Test\n";
    }
    BOOST_TEST(!files_are_identical(f0, f1));
}

void test_minmax()
{
    double const zero = 0.0;
    double const one  = 1.0;

    std::vector<double> w;
    w.push_back(one );
    w.push_back(zero);

    // Test const-correctness.
    std::vector<double> const v = w;
    minmax<double> const m(v);
    BOOST_TEST(zero == m.minimum());
    BOOST_TEST(one  == m.maximum());

    // Motivation for relational operators: to write this...
    BOOST_TEST(zero <= m.minimum() && m.maximum() <= one);
    // ...more compactly:
#if !defined __BORLANDC__
    BOOST_TEST(  zero <= m && m <= one );
    BOOST_TEST(!(zero <  m || m <  one));
#endif // !defined __BORLANDC__
}

void test_trimming()
{
    char const*const superfluous = " ;";

    std::string s = "";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "");

    s = " ";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "");

    s = " ;; ";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "");

    s = "a";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "a");

    s = "; ;a; ;";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "a");

    s = "a; ;";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "a");

    s = "; ;a";
    ltrim(s, superfluous); rtrim(s, superfluous);
    BOOST_TEST_EQUAL(s, "a");
}

int test_main(int, char*[])
{
    test_files_are_identical();
    test_minmax();
    test_trimming();

    return 0;
}

