// Interface to libxml++ .
//
// Copyright (C) 2006 Gregory W. Chicares.
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

// $Id: xml_lmi.cpp,v 1.1.2.9 2006-10-19 00:59:26 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_lmi.hpp"

#include "alert.hpp"

#include <ostream>
#include <sstream>
#include <stdexcept>

namespace xml_lmi
{

xml_lmi::dom_parser::dom_parser(std::string const& filename)
{
    try
        {
        error_context_ = "Error--xml file '" + filename + "': ";
        if(filename.empty())
            {
            throw std::runtime_error("File name is empty.");
            }
        parser_.parse_file(filename);
        if(!parser_)
            {
            throw std::runtime_error("Parser failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error()
            << error_context_
            << e.what()
            << LMI_FLUSH
            ;
        }
}

xml_lmi::dom_parser::dom_parser(std::istream& is)
{
    try
        {
        error_context_ = "Error--xml stream: ";
        parser_.parse_stream(is);
        if(!parser_)
            {
            throw std::runtime_error("Parser failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error()
            << error_context_
            << e.what()
            << LMI_FLUSH
            ;
        }
}

xml_lmi::dom_parser::~dom_parser()
{}

xml_lmi::DomParser const& xml_lmi::dom_parser::parser() const
{
    return parser_;
}

xml_lmi::Element const& xml_lmi::dom_parser::root_node
    (std::string const& expected_name
    ) const
{
    xmlpp::Document const* document = parser_.get_document();
    if(!document)
        {
        fatal_error()
            << error_context_
            << "Document is null."
            << LMI_FLUSH
            ;
        }
    xml_lmi::Element const* root = document->get_root_node();
    if(!root)
        {
        fatal_error()
            << error_context_
            << "Document has no root node."
            << LMI_FLUSH
            ;
        }
    if(!expected_name.empty() && expected_name != root->get_name())
        {
        fatal_error()
            << error_context_
            << "Root-node name is '"
            << root->get_name()
            << "', but '"
            << expected_name
            << "' was expected."
            << LMI_FLUSH
            ;
        }

    return *root;
}

std::string xml_lmi::get_content(Element const& element)
{
    std::ostringstream oss;
    xml_lmi::NodeContainer const direct_children = element.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = direct_children.begin()
        ;iter != direct_children.end()
        ;++iter
        )
        {
        xmlpp::TextNode const* t = dynamic_cast<xmlpp::TextNode const*>(*iter);
        // maybe we should add CdataNode also?
        if(t)
            {
            oss << t->get_content();
            }
        }
    return oss.str();
}

Element* xml_lmi::get_first_element(Element& parent)
{
    xml_lmi::NodeContainer const direct_children = parent.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = direct_children.begin()
        ;iter != direct_children.end()
        ;++iter
        )
        {
        Element const* e = dynamic_cast<Element const*>(*iter);
        if(e)
            {
            return const_cast<Element*>(e);
            }
        }
    return 0;
}

Element const* xml_lmi::get_first_element(Element const& parent)
{
    xml_lmi::NodeContainer const direct_children = parent.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = direct_children.begin()
        ;iter != direct_children.end()
        ;++iter
        )
        {
        Element const* e = dynamic_cast<Element const*>(*iter);
        if(e)
            {
            return e;
            }
        }
    return 0;
}

} // Namespace xml_lmi.

std::ostream& operator<<(std::ostream& os, xml_lmi::Document& document)
{
    document.write_to_stream(os, "utf-8");
    return os;
}

