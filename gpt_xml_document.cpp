// xml document for guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "gpt_xml_document.hpp"

#include "assert_lmi.hpp"
#include "xml_lmi.hpp"

#include <xmlwrapp/nodes_view.h>

#include <istream>
#include <ostream>

//============================================================================
gpt_xml_document::gpt_xml_document(gpt_input const& z)
    :input_data_ {z}
{
}

//============================================================================
gpt_xml_document::gpt_xml_document(std::string const& filename)
    :input_data_ {}
{
    xml_lmi::dom_parser parser(filename);
    parse(parser);
}

//============================================================================
std::string const& gpt_xml_document::xml_root_name() const
{
    static std::string const s("gpt_xml_document");
    return s;
}

//============================================================================
void gpt_xml_document::parse(xml_lmi::dom_parser const& parser)
{
    xml::element const& root(parser.root_node(xml_root_name()));
    xml::const_nodes_view const elements(root.elements());
    // A '.gpt' document's root contains only one child element.
    LMI_ASSERT(1 == elements.size());
    // "*elements.begin()" because there is no front():
    *elements.begin() >> input_data_;
}

//============================================================================
void gpt_xml_document::read(std::istream const& is)
{
    xml_lmi::dom_parser parser(is);
    parse(parser);
}

//============================================================================
void gpt_xml_document::write(std::ostream& os) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    root << input_data_;
    os << document;
}
