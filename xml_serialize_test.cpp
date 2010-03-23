// Serialization to and from xml--unit test.
//
// Copyright (C) 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_serialize.hpp"

#include "miscellany.hpp" // lmi_array_size(), stifle_warning_for_unused_variable()
#include "test_tools.hpp"
#include "timer.hpp"

#include <string>
#include <vector>

// All /write.*/ functions save xml to this string.

std::string dom_string;

// Repetition count for /mete.*/ functions, tuned for speed and accuracy.

int const number_of_elements = 20;

// http://www.drpeppermuseum.com/Learn/10-2-4-Club.aspx

int const dr_pepper[] = {10, 2, 4};

// /[dsv]0/: constant values for /write.*/ functions.
// /[dsv]1/: variables for /read.*/ functions.

double           const d0(2.718281828459045235360);
std::string      const s0("string with ampersand & embedded spaces");
std::vector<int> const v0(dr_pepper, dr_pepper + lmi_array_size(dr_pepper));

double                 d1;
std::string            s1;
std::vector<int>       v1;

void write()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    xml_serialize::set_element(root, "d", d0);
    xml_serialize::set_element(root, "s", s0);
    xml_serialize::set_element(root, "v", v0);
    dom_string = document.str();
}

void read()
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    xml_serialize::get_element(root, "d", d1);
    xml_serialize::get_element(root, "s", s1);
    xml_serialize::get_element(root, "v", v1);
}

void write_erroneous()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    xml_serialize::set_element(root, "d", d0);
    xml_serialize::set_element(root, "d", d0); // Error: duplicate element.
}

void read_erroneous()
{
    float f1;

    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    xml_serialize::get_element(root, "d", d1);
    xml_serialize::get_element(root, "s", s1);
    xml_serialize::get_element(root, "v", v1);
    xml_serialize::get_element(root, "f", f1); // Error: no <f> element.
}

// These /mete_[write|read]/ functions are like write() and read()
// except that they don't actually do anything: they serve only to
// measure overhead.

void mete_write()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    stifle_warning_for_unused_variable(root);
    dom_string = document.str();
}

void mete_read()
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    stifle_warning_for_unused_variable(root);
}

// These /mete_._[write|read]/ functions are like write() and read()
// except that they each test a single datatype repeatedly.

void mete_s_write()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    for(int j = 0; j < number_of_elements; ++j)
        {
        root.erase("s");
        xml_serialize::set_element(root, "s", s0);
        }
    dom_string = document.str();
}

void mete_s_read()
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    for(int j = 0; j < number_of_elements; ++j)
        {
        xml_serialize::get_element(root, "s", s1);
        }
}

void mete_d_write()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    for(int j = 0; j < number_of_elements; ++j)
        {
        root.erase("d");
        xml_serialize::set_element(root, "d", d0);
        }
    dom_string = document.str();
}

void mete_d_read()
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    for(int j = 0; j < number_of_elements; ++j)
        {
        xml_serialize::get_element(root, "d", d1);
        }
}

void mete_v_write()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    for(int j = 0; j < number_of_elements; ++j)
        {
        root.erase("v");
        xml_serialize::set_element(root, "v", v0);
        }
    dom_string = document.str();
}

void mete_v_read()
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    for(int j = 0; j < number_of_elements; ++j)
        {
        xml_serialize::get_element(root, "v", v1);
        }
}

int test_main(int, char*[])
{
    write();
    read();

    // Not every floating-point number would remain invariant through
    // serialization, but the base of natural logarithms does: see the
    // documentation for value_cast<>().
    BOOST_TEST(d0 == d1);
    BOOST_TEST_EQUAL(d0, d1);
    BOOST_TEST(s0 == s1);
    BOOST_TEST_EQUAL(s0, s1);
    // BOOST_TEST_EQUAL(v0, v1); // No--couldn't display if not equal.
    BOOST_TEST(v0 == v1);
    BOOST_TEST_EQUAL(v0.size(), v1.size());

    std::string found
        ("Assertion 'parent.end() == parent.find(name.c_str())' failed."
        );
    BOOST_TEST_THROW(write_erroneous(), std::runtime_error, found);

    std::string not_found("Required element 'f' not found.");
    BOOST_TEST_THROW(read_erroneous(), std::runtime_error, not_found);

    std::cout << "  Speed tests...\n";
    std::cout << "  Write   : " << TimeAnAliquot(mete_write  ) << '\n';
    std::cout << "  Read    : " << TimeAnAliquot(mete_read   ) << '\n';
    std::cout << "  Write s : " << TimeAnAliquot(mete_s_write) << '\n';
    std::cout << "  Read  s : " << TimeAnAliquot(mete_s_read ) << '\n';
    std::cout << "  Write d : " << TimeAnAliquot(mete_d_write) << '\n';
    std::cout << "  Read  d : " << TimeAnAliquot(mete_d_read ) << '\n';
    std::cout << "  Write v : " << TimeAnAliquot(mete_v_write) << '\n';
    std::cout << "  Read  v : " << TimeAnAliquot(mete_v_read ) << '\n';
    std::cout << std::endl;

    return 0;
}

