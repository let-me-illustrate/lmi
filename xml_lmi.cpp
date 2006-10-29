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

// $Id: xml_lmi.cpp,v 1.1.2.19 2006-10-29 01:14:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_lmi.hpp"

#include "alert.hpp"

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <libxml/globals.h>
#include <libxml/HTMLtree.h>
#include <libxml/parser.h>
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>

#include <ostream>
#include <sstream>
#include <stdexcept>

// EVGENIY Has 'XmlDocSharedPtrDeleter' become
// dead code that could now be removed?
#if defined OKAY_TO_REMOVE_XmlDocSharedPtrDeleter
namespace
{
/// Custom deleter for xmlDocPtr shared_ptr
///
/// Use this class in a shared_ptr to free an xml document pointer using
/// xmlFree method.
/// The pointer to be freed should be allocated using libxml functions.
///
/// See boost::shared_ptr requirements for a deleter class:
/// >> D  must be CopyConstructible.
/// >> The copy constructor and destructor of D must not throw.
/// >> The expression d(p) must be well-formed, must not invoke undefined
/// >> behavior, and must not throw exceptions.

class XmlDocSharedPtrDeleter
{
  public:
    void operator () (xmlDocPtr xml_doc)
    {
        try
            {
            xmlFreeDoc(xml_doc);
            }
        catch(...)
            {
            warning()
                << "A call to xmlFreeDoc failed."
                << LMI_FLUSH
                ;
            }
    }
    // default ctor, dtor and copy-ctor are ok
};

} // anonymous namespace
#endif // defined OKAY_TO_REMOVE_XmlDocSharedPtrDeleter

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

Element& xml_lmi::get_first_element(Element& parent)
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
            return *const_cast<Element*>(e);
            }
        }
    fatal_error()
        << "A child element was expected, but none was found."
        << LMI_FLUSH
        ;
    throw std::logic_error("Unreachable"); // Silence compiler warning.
}

Element const& xml_lmi::get_first_element(Element const& parent)
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

xml_lmi::Stylesheet::Stylesheet(std::string const& filename)
:stylesheet_(NULL)
{
    try
        {
        error_context_ = "Unable to parse xsl stylesheet file '" + filename + "': ";
        if(filename.empty())
            {
            throw std::runtime_error("File name is empty.");
            }

        // set global options in libxml for libxslt
        int substitute_entities_copy = xmlSubstituteEntitiesDefault(1);
        int load_ext_dtd_default_copy = xmlLoadExtDtdDefaultValue;
        xmlLoadExtDtdDefaultValue = 0;

        set_stylesheet
            (xsltParseStylesheetFile(reinterpret_cast<xmlChar const*>(filename.c_str()))
            );

        // reset global options in libxml for libxslt
        xmlSubstituteEntitiesDefault(substitute_entities_copy);
        xmlLoadExtDtdDefaultValue = load_ext_dtd_default_copy;

        if(0 == stylesheet_)
            {
            throw std::runtime_error("Parser failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        }
}

xml_lmi::Stylesheet::Stylesheet(Document const& document)
:stylesheet_(NULL)
{
    try
        {
        error_context_ = "Unable to parse xsl stylesheet document from xml: ";
        if(0 == document.cobj())
            {
            throw std::runtime_error("Document is empty.");
            }
        set_stylesheet(xsltParseStylesheetDoc(const_cast<xmlDoc*>(document.cobj())));

        if(0 == stylesheet_)
            {
            throw std::runtime_error("Parsing failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        }
}

xml_lmi::Stylesheet::~Stylesheet()
{
    set_stylesheet(NULL);
}

void xml_lmi::Stylesheet::transform
    (Document const& document
    ,std::ostream& os
    ,enum_output_type output_type
    ) const
{
    std::string error_context = "Unable to apply xsl stylesheet to xml document: ";
    try
        {
        if(0 == stylesheet_)
            {
            throw std::runtime_error("Can't apply a NULL stylesheet.");
            }

        boost::shared_ptr<xmlDoc> xml_document_ptr
            (xsltApplyStylesheet
                (stylesheet_
                ,const_cast<xmlDoc*>(document.cobj())
                ,NULL
                )
            );
        if(0 == xml_document_ptr.get())
            {
            throw std::runtime_error("Failed to apply stylesheet.");
            }

        char* buffer = NULL;
        int buffer_size = 0;

        // a pointer suitable for passing it to libxslt functions
        xmlChar** buffer_ptr = reinterpret_cast<xmlChar**>(&buffer);

        if(e_output_xml == output_type)
            {
            xmlDocDumpFormatMemory
                (xml_document_ptr.get() // xml document to write
                ,buffer_ptr         // text buffer pointer, we own it
                ,&buffer_size       // text buffer size
                ,true               // indent xml output
                );
            }
        else if(e_output_html == output_type)
            {
            htmlDocDumpMemoryFormat
                (xml_document_ptr.get() // xml document to write
                ,buffer_ptr         // text buffer pointer, we own it
                ,&buffer_size       // text buffer size
                ,true               // indent html output
                );
            }
        else if(e_output_text == output_type)
            {
            htmlDocDumpMemoryFormat
                (xml_document_ptr.get() // xml document to write
                ,buffer_ptr         // text buffer pointer, we own it
                ,&buffer_size       // text buffer size
                ,false              // do _not_ indent the output
                );
            }
        else
            {
            throw std::runtime_error("Invalid output type specified.");
            }

        boost::scoped_array<char> buffer_guard(buffer);

        if(buffer_size <= 0)
            {
            throw std::runtime_error("Empty output.");
            }

        os.write(buffer, buffer_size);
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context << e.what() << LMI_FLUSH;
        }
}

void xml_lmi::Stylesheet::set_stylesheet(stylesheet_ptr_t stylesheet)
{
    if(0 != stylesheet_)
        {
        // free the current stylesheet
        xsltFreeStylesheet(stylesheet_);
        }
    stylesheet_ = stylesheet;
}

} // namespace xml_lmi

std::ostream& operator<<(std::ostream& os, xml_lmi::Document& document)
{
    document.write_to_stream(os, "utf-8");
    return os;
}

