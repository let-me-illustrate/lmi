// Interface to xmlwrapp.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef xml_lmi_hpp
#define xml_lmi_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"
#include "xml_lmi_fwd.hpp"

#include <boost/scoped_ptr.hpp>

#include <xmlwrapp/node.h> // (for xml::element)

#include <cstddef>         // std::size_t
#include <iosfwd>
#include <string>

/// Interface to xmlwrapp.

namespace xml_lmi
{
    class dom_parser
        :private lmi::uncopyable<dom_parser>
    {
        typedef xml::tree_parser DomParser;

      public:
        dom_parser(std::string const& filename);
        dom_parser(char const* data, std::size_t length);
        dom_parser(std::istream const&);
        ~dom_parser();

        Document const& document() const;
        xml::element const& root_node(std::string const& expected_name) const;

      private:
        std::string                    error_context_;
        boost::scoped_ptr<DomParser>   parser_;
    };

    class xml_document
        :private lmi::uncopyable<xml_document>
    {
      public:
        xml_document(std::string const& root_node_name);
        ~xml_document();

        Document const& document() const {return *document_;}
        xml::element& root_node();

        void save(std::string const& filename);
        std::string str();

      private:
        std::string                    error_context_;
        boost::scoped_ptr<Document>    document_;
    };

    void add_node
        (xml::element&
        ,std::string const& name
        ,std::string const& content
        );

    /// Find an element subnode by name, throwing if it is not found.

    xml::node::const_iterator retrieve_element
        (xml::element const& parent
        ,std::string  const& name
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

