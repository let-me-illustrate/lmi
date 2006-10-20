// Inputs for life insurance illustrations--unit test.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: input_test.cpp,v 1.7.2.7 2006-10-20 00:25:12 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputillus.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "xml_lmi.hpp"

#include <libxml++/libxml++.h>

#if defined BOOST_MSVC || defined __BORLANDC__
#   include <cfloat> // floating-point hardware control
#endif // defined BOOST_MSVC || defined __BORLANDC__
#include <cstdio> // std::remove()
#include <fstream>
#include <ios>
#include <istream>
#include <iterator>

// This function is a derived work adapted from usenet article
// <1eo2sct.ggkc9z84ko0eN%cbarron3@ix.netcom.com>. GWC rewrote it
// in 2005. Any defect should not reflect on Carl Barron's reputation.
//
// TODO ?? Test; consider making generally available; consider whether
// the std::istream version is separately useful.
//
bool streams_are_identical(std::istream& is0, std::istream& is1)
{
    std::istreambuf_iterator<char> i(is0);
    std::istreambuf_iterator<char> j(is1);
    std::istreambuf_iterator<char> end;

    for(; end != i && end != j; ++i, ++j)
        {
        if(*i != *j)
            {
            return false;
            }
        }
    return *i == *j;
}

bool files_are_identical(std::string const& file0, std::string const& file1)
{
    std::ifstream is0(file0.c_str());
    std::ifstream is1(file1.c_str());
    return streams_are_identical(is0, is1);
}

int test_main(int, char*[])
{
    // Test IllusInputParms << and >> operators.
    IllusInputParms original;
    IllusInputParms replica;

    std::ofstream os0
        ("eraseme0.xml"
        ,   std::ios_base::out
          | std::ios_base::trunc
          | std::ios_base::binary
        );
    BOOST_TEST(!!os0);

    // The obsolete first-, middle-, and last-name fields live on
    // in base class 'InputParms', but the full name now actually
    // resides in its first-name field, which is mapped to a single
    // field in derived class 'IllusInputParms'.

    original.AgentFirstName     = "Herbert Cassidy";
    original.AgentPhone         = "123-4567";
    original.InsdFirstName      = "Actually Full Name";
    original.InsdAddr1          = "addr1";
    original.InsdAddr2          = "addr2";
    original.InsdCity           = "city";
//    original.FundAllocations    = "0.4 0.3 0.2 0.1";
    original.SepAcctIntRate     = "0.03125";
/*
    original.FundAllocs[0]      = 1.0;
    original.SepAcctRate[0]     = .01234567890123456789;
    original.SepAcctRate[1]     = .12345678901234567890;
    original.SepAcctRate[2]     = .23456789012345678901;
    original.SepAcctRate[3]     = .34567890123456789012;
    original.SepAcctRate[4]     = .45678901234567890123;
    original.SepAcctRate[5]     = .56789012345678901234;
    original.SepAcctRate[6]     = .67890123456789012345;
    original.SepAcctRate[7]     = .78901234567890123456;
    original.SepAcctRate[8]     = .89012345678901234567;
    original.SepAcctRate[9]     = .90123456789012345678;
*/
    original.propagate_changes_to_base_and_finalize();

    xml_lmi::Document doc;
    xml_lmi::Element& xml_root0 = *doc.create_root_node("root");
    xml_root0 << original;
    os0 << doc;
    os0.close();

    {
        xml_lmi::Element* xml_node = xml_lmi::get_first_element(xml_root0);
        BOOST_TEST(!!xml_node);
        *xml_node >> replica;
    }
    std::ofstream os1
        ("eraseme1.xml"
        ,   std::ios_base::out
          | std::ios_base::trunc
          | std::ios_base::binary
        );
    BOOST_TEST(!!os1);
    xml_lmi::Document xml_doc1;
    xml_lmi::Element& xml_root1 = *xml_doc1.create_root_node("root");
    xml_root1 << replica;
    os1 << xml_doc1;
    os1.close();

    BOOST_TEST(original == replica);
    BOOST_TEST(files_are_identical("eraseme0.xml", "eraseme1.xml"));

    BOOST_TEST(0.03125 == original.SepAcctRate[0]);
    BOOST_TEST(0.03125 == replica.SepAcctRate[0]);

/* TODO ?? The code this tests is defective--fix it someday.
    BOOST_TEST(0.4 == original.FundAllocs[0]);
    BOOST_TEST(0.4 == replica.FundAllocs[0]);
std::cout << "original.FundAllocs[0] is " << original.FundAllocs[0] << '\n';
std::cout << "replica.FundAllocs[0] is " << replica.FundAllocs[0] << '\n';

std::cout << "original.FundAllocs.size() is " << original.FundAllocs.size() << '\n';
std::cout << "replica.FundAllocs.size() is " << replica.FundAllocs.size() << '\n';
*/

    BOOST_TEST(0 == std::remove("eraseme0.xml"));
    BOOST_TEST(0 == std::remove("eraseme1.xml"));

    BOOST_TEST(0 == original.InforceYear);
    original["InforceYear"] = std::string("3");
    BOOST_TEST(3 == original.InforceYear);

    BOOST_TEST(45 == original.Status_IssueAge);
    original["IssueAge"] = std::string("57");
    BOOST_TEST(57 == original.Status_IssueAge);
    // Unfortunately, this can't get updated automatically:
    BOOST_TEST(45 == original.Status[0].IssueAge);

    // Test copy constructor.
    original.propagate_changes_to_base_and_finalize();
    IllusInputParms copy0(original);
    copy0.propagate_changes_from_base_and_finalize();
//    BOOST_TEST(original == copy0); // TODO ?? Fails.
    copy0["InsuredName"] = "Claude Proulx";
    BOOST_TEST(std::string("Claude Proulx") == copy0   .InsdFirstName);
    BOOST_TEST(std::string("Actually Full Name") == original.InsdFirstName);

    BOOST_TEST(std::string("") == copy0   .InsdMiddleName);
    BOOST_TEST(std::string("") == original.InsdMiddleName);

    BOOST_TEST(std::string("") == copy0   .InsdLastName);
    BOOST_TEST(std::string("") == original.InsdLastName);

    // Reset this in case it changed.
    original.InsdFirstName      = "Actually Full Name";

    // Test assignment operator.
    IllusInputParms copy1;
    original.propagate_changes_to_base_and_finalize();
    copy1 = original;
    copy1.propagate_changes_from_base_and_finalize();
//    BOOST_TEST(original == copy1); // TODO ?? Fails.
    copy1["InsuredName"] = "Angela";
    BOOST_TEST(std::string("Angela") == copy1   .InsdFirstName);
    BOOST_TEST(std::string("Actually Full Name")  == original.InsdFirstName);

    return 0;
}

