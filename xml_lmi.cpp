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

// $Id: xml_lmi.cpp,v 1.1.2.13 2006-10-20 03:24:30 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_lmi.hpp"

#include "alert.hpp"

#include <libxml++/libxml++.h>

#include <ostream>
#include <sstream>
#include <stdexcept>

namespace xml_lmi
{

/// Parse an xml file.
///
/// Precondition: argument names an accessible xml file.
///
/// Postconditions: member parser_ is a non-null pointer; the object
/// it points to is valid in that its operator bool() returns true.
///
/// Throws: std::runtime_error, via fatal_error(), if a precondition
/// is violated, or if xml-library calls throw an exception derived
/// from std::exception.

xml_lmi::dom_parser::dom_parser(std::string const& filename)
{
    try
        {
        error_context_ = "Unable to parse xml file '" + filename + "': ";
        if(filename.empty())
            {
            throw std::runtime_error("File name is empty.");
            }
        parser_.reset(new DomParser);
        if(0 == parser_.get())
            {
            throw std::runtime_error("Parser not initialized.");
            }
        parser_->parse_file(filename);
        if(0 == parser_->operator bool())
            {
            throw std::runtime_error("Parser failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        }
}

/// Parse an xml stream.
///
/// Precondition: argument is an xml stream for which 0 == rdstate().
///
/// Postconditions: member parser_ is a non-null pointer; the object
/// it points to is valid in that its operator bool() returns true.
///
/// Throws: std::runtime_error, via fatal_error(), if a precondition
/// is violated, or if xml-library calls throw an exception derived
/// from std::exception.

xml_lmi::dom_parser::dom_parser(std::istream& is)
{
    try
        {
        error_context_ = "Unable to parse xml stream: ";
        if(0 != is.rdstate())
            {
            throw std::runtime_error("Stream state is not 'good'.");
            }
        parser_.reset(new DomParser);
        if(0 == parser_.get())
            {
            throw std::runtime_error("Parser not initialized.");
            }
        parser_->parse_stream(is);
        if(0 == parser_->operator bool())
            {
            throw std::runtime_error("Parser failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        }
}

/// Throws: nothing unless member parser_'s destructor does.

xml_lmi::dom_parser::~dom_parser()
{}

/// Return the parsed document's root node.
///
/// Preconditions: member parser_ has a document that is not null and
/// has a root node; the argument, if not empty, matches the name of
/// that root node.
///
/// Throws: std::runtime_error, via fatal_error(), if a precondition
/// is violated, or if xml-library calls throw an exception derived
/// from std::exception. Ctor postconditions are assumed to have been
/// satisfied and are not tested.

xml_lmi::Element const& xml_lmi::dom_parser::root_node
    (std::string const& expected_name
    ) const
{
    try
        {
        xml_lmi::Document const* document = parser_->get_document();
        if(!document)
            {
            throw std::runtime_error("Parsed document is null.");
            }
        xml_lmi::Element const* root = document->get_root_node();
        if(!root)
            {
            throw std::runtime_error("Document has no root node.");
            }
        if(!expected_name.empty() && expected_name != root->get_name())
            {
            std::ostringstream oss;
            oss
                << "Root-node name is '"
                << root->get_name()
                << "', but '"
                << expected_name
                << "' was expected."
                ;
            throw std::runtime_error(oss.str());
            }

        return *root;
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
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
        // TODO ?? Resolve this issue:
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

} // namespace xml_lmi

std::ostream& operator<<(std::ostream& os, xml_lmi::Document& document)
{
    document.write_to_stream(os, "utf-8");
    return os;
}

