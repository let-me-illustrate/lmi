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

// $Id: xml_lmi.cpp,v 1.1.2.28 2006-11-08 23:10:56 etarassov Exp $

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
#include <libxml++/nodes/cdatanode.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>

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

xml_lmi::Document& xml_lmi::dom_parser::document()
{
    try
        {
        xml_lmi::Document* document = parser_->get_document();
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

/// Template implementation of xml_lmi::dom_parser::root_node methods.

template
    <class DomParserType
    ,class DocumentType
    ,class ElementType
    >
ElementType&
get_root_node_from_dom_parser
    (DomParserType& dom_parser
    ,std::string const& expected_name
    ,std::string const& error_context
    )
{
    try
        {
        DocumentType& document = dom_parser.document();
        ElementType* root = document.get_root_node();
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
        fatal_error() << error_context << e.what() << LMI_FLUSH;
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
    return get_root_node_from_dom_parser
        <xml_lmi::dom_parser const
        ,xml_lmi::Document const
        ,xml_lmi::Element const
        >
        (*this
        ,expected_name
        ,error_context_
        );
}

xml_lmi::Element& xml_lmi::dom_parser::root_node
    (std::string const& expected_name
    )
{
    return get_root_node_from_dom_parser
        <xml_lmi::dom_parser
        ,xml_lmi::Document
        ,xml_lmi::Element
        >
        (*this
        ,expected_name
        ,error_context_
        );
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

xml_lmi::ElementContainer xml_lmi::child_elements
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

std::string xml_lmi::get_content(Element const& element, enum_content content)
{
    std::ostringstream oss;
    get_content(element, oss, content);
    return oss.str();
}

// Read text content of an xml_lmi::Element.
// Include whitespaces if e_content_include_whitespace is specified.
// Read content recursivly if e_content_recursivly is specified.

void xml_lmi::get_content
    (Element const& element
    ,std::ostream& os
    ,enum_content content
    )
{
    xml_lmi::NodeContainer const direct_children = element.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = direct_children.begin()
        ;iter != direct_children.end()
        ;++iter
        )
        {
        xmlpp::ContentNode const* contentNode
            = dynamic_cast<xmlpp::ContentNode const*>(*iter);
        if(!contentNode)
            {
            xmlpp::Element const* elementNode
                = dynamic_cast<xmlpp::Element const*>(*iter);
            if
                (  elementNode
                && (content & e_content_recursivly)
                )
                {
                get_content(*elementNode, os, content);
                }
            }
        else if
            (  dynamic_cast<xmlpp::TextNode const*>(contentNode)
            || dynamic_cast<xmlpp::CdataNode const*>(contentNode)
            )
            {
            if
                (  !contentNode->is_white_space()
                || (content & e_content_include_whitespace)
                )
                {
                os << contentNode->get_content();
                }
            }
        }
}

Element& xml_lmi::get_first_element(Element& parent)
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
    transform(document, os, output_type, std::map<std::string,std::string>());
}

void xml_lmi::Stylesheet::transform
    (Document const& document
    ,std::ostream& os
    ,enum_output_type output_type
    ,std::map<std::string,std::string> const& parameters
    ) const
{
    std::string error_context = "Unable to apply xsl stylesheet to xml document: ";
    try
        {
        if(0 == stylesheet_)
            {
            throw std::runtime_error("Can't apply a NULL stylesheet.");
            }

        // parameters buffer
        static std::vector<char const*> params;
        // container for string that need to be &quote; escaped
        static std::vector<std::string> params_container;
        // parameters pointer to pass to the libxslt engine
        char const** params_ptr = NULL;

        // if there are any parameters, pass it to the transformation
        if(!parameters.empty())
            {
            params.clear();
            params.reserve(parameters.size() * 2 + 1);

            params_container.clear();
            params_container.reserve(parameters.size());

            std::map<std::string,std::string>::const_iterator ci;
            for(ci = parameters.begin(); ci != parameters.end(); ++ci)
                {
                // add parameter name
                params.push_back(ci->first.c_str());

                // parameter value has to be &quote; escaped
                std::string str(ci->second);
                std::size_t pos = 0;
                while(std::string::npos != (pos = str.find('\'', pos)))
                    {
                    str.replace(pos, 1, "&quote;");
                    }
                // add surrounding quotes
                params_container.push_back("'" + str + "'");
                // finally add parameter value
                params.push_back(params_container.back().c_str());
                }
            // parameters array passed to libxslt has to be NULL terminated
            params.push_back(NULL);
            params_ptr = &params[0];
            }

        boost::shared_ptr<xmlDoc> xml_document_ptr
            (xsltApplyStylesheet
                (stylesheet_
                ,const_cast<xmlDoc*>(document.cobj())
                ,params_ptr
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

