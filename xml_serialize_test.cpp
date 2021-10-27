// Serialization to and from xml--unit test.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "xml_serialize.hpp"

#include "miscellany.hpp"               // stifle_unused_warning()
#include "test_tools.hpp"
#include "timer.hpp"

#include <map>
#include <string>
#include <unordered_map>
#include <utility>                      // pair
#include <vector>

// All /write.*/ functions save xml to this string, and
// all /read.*/ functions read from it.

std::string dom_string;

// Repetition count for /mete.*/ functions, tuned for speed and accuracy.

int const number_of_iterations = 20;

using omap = std::          map<std::string, float>;
using umap = std::unordered_map<int        , float>;

// /[dspvou]0/: constant values for /write.*/ functions.
// /[dspvou]1/: variables for /read.*/ functions.

double             const d0(2.718281828459045235360);
std::string        const s0("string with ampersand & embedded spaces");
std::pair<int,int> const p0 {17, 19};
std::vector<int>   const v0 {10, 2, 4}; // Be a pepper...
omap               const o0 {{"a",  1.1f}, {"b",  2.2f}, {"c",  3.3f}};
umap               const u0 {{1, 101.01f}, {2, 202.02f}, {3, 303.03f}};

double                   d1;
std::string              s1;
std::pair<int,int>       p1;
std::vector<int>         v1;
omap                     o1;
umap                     u1;

void write()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    xml_serialize::set_element(root, "d", d0);
    xml_serialize::set_element(root, "s", s0);
    xml_serialize::set_element(root, "p", p0);
    xml_serialize::set_element(root, "v", v0);
    xml_serialize::set_element(root, "o", o0);
    xml_serialize::set_element(root, "u", u0);
    dom_string = document.str();
}

void read()
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    xml_serialize::get_element(root, "d", d1);
    xml_serialize::get_element(root, "s", s1);
    xml_serialize::get_element(root, "p", p1);
    xml_serialize::get_element(root, "v", v1);
    xml_serialize::get_element(root, "o", o1);
    xml_serialize::get_element(root, "u", u1);
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
    xml_serialize::get_element(root, "p", p1);
    xml_serialize::get_element(root, "v", v1);
    xml_serialize::get_element(root, "o", o1);
    xml_serialize::get_element(root, "u", u1);
    xml_serialize::get_element(root, "f", f1); // Error: no <f> element.
}

// These /mete_[write|read]/ functions are like write() and read()
// except that they don't actually do anything: they serve only to
// measure overhead.

void mete_write_0()
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    stifle_unused_warning(root);
    dom_string = document.str();
}

void mete_read_0()
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    stifle_unused_warning(root);
}

// These /mete_._[write|read]/ functions are like write() and read()
// except that they each test a single datatype repeatedly.

template<typename T>
void mete_write(char const* name, T const& data)
{
    xml_lmi::xml_document document("eraseme");
    xml::element& root = document.root_node();
    for(int j = 0; j < number_of_iterations; ++j)
        {
        root.erase(name);
        xml_serialize::set_element(root, name, data);
        }
    dom_string = document.str();
}

template<typename T>
void mete_read(char const* name, T& data)
{
    xml_lmi::dom_parser parser(dom_string.c_str(), dom_string.size());
    xml::element const& root = parser.root_node("eraseme");
    for(int j = 0; j < number_of_iterations; ++j)
        {
        xml_serialize::get_element(root, name, data);
        }
}

void mete_write_d() {mete_write("d", d0);}
void mete_read_d()  {mete_read ("d", d1);}

void mete_write_s() {mete_write("s", s0);}
void mete_read_s()  {mete_read ("s", s1);}

void mete_write_p() {mete_write("p", p0);}
void mete_read_p()  {mete_read ("p", p1);}

void mete_write_v() {mete_write("v", v0);}
void mete_read_v()  {mete_read ("v", v1);}

void mete_write_o() {mete_write("o", o0);}
void mete_read_o()  {mete_read ("o", o1);}

void mete_write_u() {mete_write("u", u0);}
void mete_read_u()  {mete_read ("u", u1);}

int test_main(int, char*[])
{
    write();
    read();

    // Not every floating-point number would remain invariant through
    // serialization, but the base of natural logarithms does: see the
    // documentation for value_cast<>().
    LMI_TEST(d0 == d1);
    LMI_TEST_EQUAL(d0, d1);

    LMI_TEST(s0 == s1);
    LMI_TEST_EQUAL(s0, s1);

    // LMI_TEST_EQUAL() inserts unequal values into an ostream, so
    // it can only be used with streamable types (as above).

    LMI_TEST(p0 == p1);

    // For Containers, test both
    //   P: c0 == c1
    //   Q: c0.size() == c1.size()
    // even though P implies Q, because Q AND ~P is easy to detect.

    LMI_TEST(v0 == v1);
    LMI_TEST_EQUAL(v0.size(), v1.size());

    LMI_TEST(o0 == o1);
    LMI_TEST_EQUAL(o0.size(), o1.size());

    LMI_TEST(u0 == u1);
    LMI_TEST_EQUAL(u0.size(), u1.size());

    std::string found
        ("Assertion 'parent.end() == parent.find(name.c_str())' failed."
        );
    LMI_TEST_THROW(write_erroneous(), std::runtime_error, found);

    std::string not_found("Required element 'f' not found.");
    LMI_TEST_THROW(read_erroneous(), std::runtime_error, not_found);

    std::cout << "  Speed tests...\n";
    std::cout << "  Write empty : " << TimeAnAliquot(mete_write_0) << '\n';
    std::cout << "  Read  empty : " << TimeAnAliquot(mete_read_0 ) << '\n';
    std::cout << "  Write d     : " << TimeAnAliquot(mete_write_d) << '\n';
    std::cout << "  Read  d     : " << TimeAnAliquot(mete_read_d ) << '\n';
    std::cout << "  Write s     : " << TimeAnAliquot(mete_write_s) << '\n';
    std::cout << "  Read  s     : " << TimeAnAliquot(mete_read_s ) << '\n';
    std::cout << "  Write p     : " << TimeAnAliquot(mete_write_p) << '\n';
    std::cout << "  Read  p     : " << TimeAnAliquot(mete_read_p ) << '\n';
    std::cout << "  Write v     : " << TimeAnAliquot(mete_write_v) << '\n';
    std::cout << "  Read  v     : " << TimeAnAliquot(mete_read_v ) << '\n';
    std::cout << "  Write o     : " << TimeAnAliquot(mete_write_o) << '\n';
    std::cout << "  Read  o     : " << TimeAnAliquot(mete_read_o ) << '\n';
    std::cout << "  Write u     : " << TimeAnAliquot(mete_write_u) << '\n';
    std::cout << "  Read  u     : " << TimeAnAliquot(mete_read_u ) << '\n';
    std::cout << std::endl;

    return 0;
}
