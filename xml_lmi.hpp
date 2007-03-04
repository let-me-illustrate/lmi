// Interface to xmlwrapp.
//
// Copyright (C) 2006, 2007 Gregory W. Chicares.
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

// $Id: xml_lmi.hpp,v 1.19 2007-03-04 16:33:31 chicares Exp $

#ifndef xml_lmi_hpp
#define xml_lmi_hpp

#include "config.hpp"

#include "xml_lmi_fwd.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <xmlwrapp/node.h> // xml::node::const_iterator

#include <iosfwd>
#include <string>
#include <vector>

/// Interface to xmlwrapp.

namespace xml_lmi
{
    typedef std::vector<xml::node::const_iterator> ElementContainer;

    class dom_parser
        :private boost::noncopyable
    {
        typedef xml::tree_parser DomParser;
        typedef xml::init        Initializer;

      public:
        dom_parser(std::string const& filename);
        dom_parser(std::istream&);
        ~dom_parser();

        Document const& document() const;
        xml::element const& root_node(std::string const& expected_name) const;

      private:
        std::string                    error_context_;
        boost::scoped_ptr<Initializer> initializer_;
        boost::scoped_ptr<DomParser>   parser_;
    };

    class xml_document
        :private boost::noncopyable
    {
        typedef xml::init Initializer;

      public:
        xml_document(std::string const& root_node_name);
        ~xml_document();

        Document const& document() const {return *document_;}
        xml::element& root_node();
        std::string str();

      private:
        std::string                    error_context_;
        boost::scoped_ptr<Initializer> initializer_;
        boost::scoped_ptr<Document>    document_;
    };

    void add_node
        (xml::element&
        ,std::string const& name
        ,std::string const& content
        );

    /// Create a container of pointers to an element's child elements.
    /// The contents are notionally pointers, but actually iterators,
    /// because that's the abstraction xmlwrapp provides; they seem to
    /// function as incrementable smart pointers.
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
        (xml::element const&
        ,std::string const& name = std::string()
        );

    /// Retrieve an xml element's full text-node contents.
    ///
    /// The contents of all text-node children are concatenated.
    ///
    /// Only direct children are considered: children of child nodes
    /// are not.

    std::string get_content(xml::element const&);

    /// Retrieve an xml element's name.

    std::string get_name(xml::element const&);

    // Attribute functions.

    /// Get a given attribute of an xml element.
    ///
    /// If the element has no such attribute, then return false and
    /// guarantee not to modify 'value'.

    bool get_attr
        (xml::element const&
        ,std::string const& name
        ,std::string&       value
        );

    /// Set a given attribute of an xml element.

    void set_attr
        (xml::element&
        ,std::string const& name
        ,std::string const& value
        );
} // namespace xml_lmi

std::ostream& operator<<(std::ostream&, xml_lmi::xml_document const&);

#endif // xml_lmi_hpp

