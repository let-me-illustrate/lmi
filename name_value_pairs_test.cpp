// Manage files of name-value pairs--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: name_value_pairs_test.cpp,v 1.1 2005-06-08 16:03:04 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "name_value_pairs.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <cstdio> // std::remove()

int test_main(int, char*[])
{
    std::string filename0("/tmp/eraseme");

    {
    std::ofstream os
        (filename0.c_str()
        ,   std::ios_base::out
          | std::ios_base::trunc
          | std::ios_base::binary
        );
    BOOST_TEST(!!os);

    os
        << "a=a\n"
        << "No equal sign--line should be ignored.\n"
        << "\n"
        << "b=b\n"
        << "c=\n"
        << "d==\n"
        << "e=1=.\n"
        << "f= f \r\n"
        ;
    }

    std::map<std::string, std::string> m0 = read_name_value_pairs(filename0);
    BOOST_TEST_EQUAL("a"  , m0["a"]);
    BOOST_TEST_EQUAL("b"  , m0["b"]);
    BOOST_TEST_EQUAL(""   , m0["c"]);
    BOOST_TEST_EQUAL("="  , m0["d"]);
    BOOST_TEST_EQUAL("1=.", m0["e"]);
#ifndef LMI_MSW
    BOOST_TEST_EQUAL(" f \r", m0["f"]);
#else  // LMI_MSW
    BOOST_TEST_EQUAL(" f ", m0["f"]);
#endif // LMI_MSW

    BOOST_TEST_EQUAL(6, m0.size());

    BOOST_TEST(0 == std::remove(filename0.c_str()));

    std::string filename1("/tmp/nonexistent_name_value_pairs_test_file");
    std::map<std::string, std::string> m1 = read_name_value_pairs(filename1);
    BOOST_TEST_EQUAL(0, m1.size());

// TODO ?? expunge
// Test with a particular file:
//    read_name_value_pairs("/opt/lmi/test/ini00000.test");

    return EXIT_SUCCESS;
}

