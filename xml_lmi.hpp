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

// $Id: xml_lmi.hpp,v 1.1.2.19 2006-11-02 18:24:50 etarassov Exp $

#ifndef xml_lmi_hpp
#define xml_lmi_hpp

#include "config.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <iosfwd>
#include <list>
#include <map>
#include <string>
#include <vector>

// We need to generate different variants of xml:
// - a light version, when speed is crucial (calculation summary)
// - a full version, when we need all the information
enum enum_xml_version
    {e_xml_light
    ,e_xml_full
    };

namespace xmlpp
{
    class Attribute;
    class Document;
    class DomParser;
    class Element;
    class Node;
} // namespace xmlpp

/// Forward declaration of a libxslt struct. The name is reserved,
/// but that's libxslt's fault.

struct _xsltStylesheet;

/// Interface to libxml++ .

namespace xml_lmi
{
    typedef xmlpp::Attribute Attribute;
    typedef xmlpp::Document  Document;
    typedef xmlpp::Element   Element;

    /// LIBXMLPP !! Type xmlpp::Node::NodeList is used in libxml++'s
    /// interface, but cannot be forward declared because it is a
    /// typedef inside a class. This is the typedef declaration as of
    /// libxml++-2.14.0 . Presumably the compiler will warn if the
    /// libxml++ maintainers ever change it, but it would be better to
    /// persuade them to provide a forwarding header themselves.

    typedef std::list<xmlpp::Node*> NodeContainer;

    typedef std::vector<Element*> ElementContainer;

    class dom_parser
        :private boost::noncopyable
    {
        typedef xmlpp::DomParser DomParser;

      public:
        dom_parser(std::string const& filename);
        dom_parser(std::istream&);
        ~dom_parser();

        Document const& document() const;
        Element const& root_node(std::string const& expected_name) const;

      private:
        std::string error_context_;
        boost::scoped_ptr<DomParser> parser_;

        void create_xml_dom_parser();
    };

    /// Create a container of pointers to an element's child elements.
    ///
    /// If the second argument is specified, then only elements having
    /// the given name are placed in the container.
    ///
    /// Only direct children are considered: children of child nodes
    /// are not. Only child nodes that are elements are placed in the
    /// container; other types of nodes are not.
    ///
    /// Precondition: No child element pointer returned by xml-library
    /// calls is null.
    ///
    /// Postcondition: The container holds no null pointers.
    ///
    /// Throws: an exception, via fatal_error(), if a precondition is
    /// violated, or if xml-library calls throw an exception derived
    /// from std::exception.

    ElementContainer child_elements
        (Element const&
        ,std::string const& name = std::string()
        );

    /// Retrieve an xml element's full text-node contents.
    ///
    /// The contents of all text-node children are concatenated and
    /// returned. Only direct children are considered: children of
    /// child nodes are not.

    std::string get_content(Element const&);

    /// Return an element node's first child element.
    ///
    /// Throws: std::runtime_error, via fatal_error(), if no child
    /// element exists.

    Element      & get_first_element(Element      &);
    Element const& get_first_element(Element const&);

    class Stylesheet
        :private boost::noncopyable
    {
        typedef _xsltStylesheet * stylesheet_ptr_t;

      public:
        Stylesheet(std::string const& filename);
        Stylesheet(Document const&);
        ~Stylesheet();

        enum enum_output_type
            {e_output_xml
            ,e_output_html
            ,e_output_text
            };

        void transform
            (Document const&
            ,std::ostream&
            ,enum_output_type
            ) const;

        void transform
            (Document const&
            ,std::ostream&
            ,enum_output_type
            ,std::map<std::string,std::string> const& parameters
            ) const;

      private:
        std::string error_context_;
        stylesheet_ptr_t stylesheet_;

        void set_stylesheet(stylesheet_ptr_t stylesheet);
    };
} // namespace xml_lmi

std::ostream& operator<<(std::ostream&, xml_lmi::Document&);

#endif //xml_lmi_hpp

