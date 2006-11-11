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

// $Id: input_test.cpp,v 1.14 2006-11-11 14:09:29 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

// Facilities offered by all three of these headers are tested here.
#include "inputillus.hpp"
#include "multiple_cell_document.hpp"
#include "single_cell_document.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"
#include "xml_lmi.hpp"

#include <boost/bind.hpp>

#if defined BOOST_MSVC || defined __BORLANDC__
#   include <cfloat> // floating-point hardware control
#endif // defined BOOST_MSVC || defined __BORLANDC__
#include <cstdio> // std::remove()
#include <fstream>
#include <ios>
#include <istream>
#include <iterator>

#if defined USING_LIBXMLPP
#   include <boost/ref.hpp>
#   define GET_FIRST_ELEMENT(child,parent) \
    boost::reference_wrapper<xml_lmi::Element> child(xml_lmi::get_first_element((parent)));
#else  // !defined USING_LIBXMLPP
#   define GET_FIRST_ELEMENT(child,parent) \
    xml_lmi::NodeConstIterator xyzzy = (parent).begin(); \
    LMI_ASSERT(!(xyzzy)->is_text()); \
    xml_lmi::Element const& child = *(xyzzy);
#endif // !defined USING_LIBXMLPP

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

template<typename DocumentClass>
void test_document_io
    (std::string const& original_filename
    ,std::string const& replica_filename
    ,char const*        file
    ,int                line
    ,bool               test_speed_only
    )
{
    DocumentClass document(original_filename);
    std::ofstream ofs
        (replica_filename.c_str()
        ,   std::ios_base::out
          | std::ios_base::trunc
          | std::ios_base::binary
        );
    document.write(ofs);
    if(test_speed_only)
        {
        return;
        }
    ofs.close();
    bool okay = files_are_identical(original_filename, replica_filename);
    INVOKE_BOOST_TEST(okay, file, line);
    // Leave the file for analysis if it didn't match.
    if(okay)
        {
        BOOST_TEST(0 == std::remove(replica_filename.c_str()));
        }
}

void mete_cns_io()
{
    typedef multiple_cell_document M;
    test_document_io<M>("sample.cns", "replica.cns", __FILE__, __LINE__, true);
}

void mete_ill_io()
{
    typedef single_cell_document S;
    test_document_io<S>("sample.ill", "replica.ill", __FILE__, __LINE__, true);
}

void mete_overhead()
{
    static IllusInputParms raw_data;
    xml_lmi::xml_document document("root");
    xml_lmi::Element& root = document.root_node();
    (void)root;
}

void mete_vector(xml_lmi::Element& xml_data)
{
    xml_lmi::child_elements(xml_data);
}

void mete_read(xml_lmi::Element& xml_data)
{
    static IllusInputParms raw_data;
    xml_data >> raw_data;
}

void mete_write()
{
    static IllusInputParms raw_data;
    xml_lmi::xml_document document("root");
    xml_lmi::Element& root = document.root_node();
    root << raw_data;
}

void assay_speed()
{
    IllusInputParms raw_data;
    xml_lmi::xml_document document("root");
    xml_lmi::Element& root = document.root_node();
    root << raw_data;
    GET_FIRST_ELEMENT(e,root)

    std::cout
        << "  Speed tests...\n"
        << "  Overhead: " << aliquot_timer(mete_overhead              ) << '\n'
        << "  Vector  : " << aliquot_timer(boost::bind(mete_vector, e)) << '\n'
        << "  Read    : " << aliquot_timer(boost::bind(mete_read  , e)) << '\n'
        << "  Write   : " << aliquot_timer(mete_write                 ) << '\n'
        << "  'cns' io: " << aliquot_timer(mete_cns_io                ) << '\n'
        << "  'ill' io: " << aliquot_timer(mete_ill_io                ) << '\n'
        ;
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

    xml_lmi::xml_document xml_document0("root");
    xml_lmi::Element& xml_root0 = xml_document0.root_node();
    xml_root0 << original;
    os0 << xml_document0;
    os0.close();

    GET_FIRST_ELEMENT(xml_node,xml_root0)

    xml_node >> replica;
    std::ofstream os1
        ("eraseme1.xml"
        ,   std::ios_base::out
          | std::ios_base::trunc
          | std::ios_base::binary
        );
    BOOST_TEST(!!os1);

    xml_lmi::xml_document xml_document1("root");
    xml_lmi::Element& xml_root1 = xml_document1.root_node();
    xml_root1 << replica;
    os1 << xml_document1;
    os1.close();
    os1.close();

    BOOST_TEST(original == replica);
    bool okay = files_are_identical("eraseme0.xml", "eraseme1.xml");
    BOOST_TEST(okay);
    // Leave the files for analysis if they didn't match.
    if(okay)
        {
        BOOST_TEST(0 == std::remove("eraseme0.xml"));
        BOOST_TEST(0 == std::remove("eraseme1.xml"));
        }

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
    BOOST_TEST(original == copy0);
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
    BOOST_TEST(original == copy1);
    copy1["InsuredName"] = "Angela";
    BOOST_TEST(std::string("Angela") == copy1   .InsdFirstName);
    BOOST_TEST(std::string("Actually Full Name")  == original.InsdFirstName);

    assay_speed();

    // TODO ?? Errors reported here with como seem to stem from
    // innocuous differences in the way floating-point exponents are
    // formatted. For instance, 'sample.ill', generated with gcc, has
    //   <TotalSpecifiedAmount>1e+006</TotalSpecifiedAmount>
    // where a 'replica.ill' generated by como has
    //   <TotalSpecifiedAmount>1e+06</TotalSpecifiedAmount>
    // . This spurious error report should be suppressed in a way that
    // doesn't block any actual error that may later develop.

    typedef multiple_cell_document M;
    test_document_io<M>("sample.cns", "replica.cns", __FILE__, __LINE__, false);
    typedef single_cell_document S;
    test_document_io<S>("sample.ill", "replica.ill", __FILE__, __LINE__, false);

    return EXIT_SUCCESS;
}

