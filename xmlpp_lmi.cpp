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

// $Id: xmlpp_lmi.cpp,v 1.1 2006-11-07 03:23:21 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_lmi.hpp"

#if !defined USING_LIBXMLPP
#   error Use this header only with libxml++.
#endif // !defined USING_LIBXMLPP

#include "alert.hpp"

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

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
        create_xml_dom_parser();
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
        create_xml_dom_parser();
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

/// Return the parsed document.
///
/// Preconditions: member parser_ has a document that is not null.
///
/// Throws: std::runtime_error, via fatal_error(), if a precondition
/// is violated, or if xml-library calls throw an exception derived
/// from std::exception. Ctor postconditions are assumed to have been
/// satisfied and are not tested.

xml_lmi::Document const& xml_lmi::dom_parser::document() const
{
    try
        {
        xml_lmi::Document const* document = parser_->get_document();
        if(!document)
            {
            throw std::runtime_error("Parsed document is null.");
            }
        return *document;
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}

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

/// Create and initialize a new DomParser object.
///
/// Created parser does not validate documents against any DTD, it also
/// substitutes entities in the xml document to make the output as simple as possible.
///
/// Postconditions: member parser_ is a non-null pointer to a new DomParser.
///
/// Throws: std::runtime_error, if failed to allocate a DomParser.

void xml_lmi::dom_parser::create_xml_dom_parser()
{
    parser_.reset();
    parser_.reset(new DomParser);
    if(0 == parser_.get())
        {
        throw std::runtime_error("Parser not initialized.");
        }
    parser_->set_validate(false);
    parser_->set_substitute_entities(true);
}

xml_lmi::xml_document::xml_document(std::string const& root_node_name)
    :document_   (new xml_lmi::Document)
{
    document_->create_root_node(root_node_name.c_str());
}

xml_lmi::xml_document::~xml_document()
{}

xml_lmi::Element& xml_lmi::xml_document::root_node()
{
    LMI_ASSERT(document_->get_root_node());
    return *document_->get_root_node();
}

std::string xml_lmi::xml_document::str()
{
    return document_->write_to_string_formatted();
}

void add_node
    (Element&           element
    ,std::string const& name
    ,std::string const& content
    )
{
    element.add_child(name)->add_child_text(content);
}

xml_lmi::ElementContainer child_elements
    (xml_lmi::Element const& parent
    ,std::string const& name
    )
{
    try
        {
        xml_lmi::ElementContainer z;
        typedef xml_lmi::NodeContainer const& nccr;
        nccr direct_children = parent.get_children(name);
        typedef xml_lmi::NodeContainer::const_iterator nci;
        for(nci i = direct_children.begin(); i != direct_children.end(); ++i)
            {
            if(!*i)
                {
                throw std::runtime_error("Null child element.");
                }
            xml_lmi::Element* e = dynamic_cast<xml_lmi::Element*>(*i);
            if(e)
                {
                z.push_back(e);
                }
            }
        return z;
        }
    catch(std::exception const& e)
        {
        fatal_error() << e.what() << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}

std::string get_content(Element const& element)
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
        if(t)
            {
            oss << t->get_content();
            }
        }
    return oss.str();
}

Element& get_first_element(Element& parent)
{
    xml_lmi::NodeContainer direct_children = parent.get_children();
    for
        (xml_lmi::NodeContainer::iterator iter = direct_children.begin()
        ;iter != direct_children.end()
        ;++iter
        )
        {
        Element* e = dynamic_cast<Element*>(*iter);
        if(e)
            {
            return *e;
            }
        }
    fatal_error()
        << "A child element was expected, but none was found."
        << LMI_FLUSH
        ;
    throw std::logic_error("Unreachable"); // Silence compiler warning.
}

Element const& get_first_element(Element const& parent)
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
            return *e;
            }
        }
    fatal_error()
        << "A child element was expected, but none was found."
        << LMI_FLUSH
        ;
    throw std::logic_error("Unreachable"); // Silence compiler warning.
}

std::string get_name(Element const& element)
{
    try
        {
        return element.get_name();
        }
    catch(std::exception const& e)
        {
        fatal_error() << e.what() << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}

bool get_attr
    (Element const&     element
    ,std::string const& name
    ,std::string&       value
    )
{
    try
        {
        xml_lmi::Attribute const* a = element.get_attribute(name);
        if(a)
            {
            value = a->get_value();
            return true;
            }
        else
            {
            return false;
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << e.what() << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}

void set_attr
    (Element&           element
    ,std::string const& name
    ,std::string const& value
    )
{
    try
        {
        xml_lmi::Attribute const* a = element.set_attribute(name, value);
        if(!a)
            {
            throw std::runtime_error("Failed to set attribute.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << e.what() << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}
} // namespace xml_lmi

/// Streaming operator for xml documents.
///
/// Formerly, this alternative was used:
///   document.write_to_stream(os, "utf-8");
/// and care taken to specify an encoding because libxml++'s default
/// is incorrect--see:
///   http://lists.gnu.org/archive/html/lmi/2006-10/msg00023.html
/// EVGENIY This isn't worth patching ourselves, but should we at
/// least report it to the maintainers, as it appears to be a
/// libxml++ defect?
///
/// The libxml++ function now used instead has no such defect, and
/// measurements show it to be as fast. Formatted output is preferred
/// because it is readable by humans. The libxml++ documentation warns
/// that it "may insert unwanted significant whitespaces", but the
/// same libxml2 function has been used for years through xmlwrapp and
/// no such problem has been observed.

std::ostream& operator<<(std::ostream& os, xml_lmi::Document& document)
{
    os << document.write_to_string_formatted();
    return os;
}

std::ostream& operator<<(std::ostream& os, xml_lmi::xml_document& d)
{
    os << d.document().write_to_string_formatted();
    return os;
}

