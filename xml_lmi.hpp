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

// $Id: xml_lmi.hpp,v 1.1.2.9 2006-10-20 00:25:12 chicares Exp $

#ifndef xml_lmi_hpp
#define xml_lmi_hpp

#include "config.hpp"

#include <boost/scoped_ptr.hpp>

#include <iosfwd>
#include <list>
#include <string>

namespace xmlpp
{
    class Attribute;
    class Document;
    class DomParser;
    class Element;
    class Node;
} // namespace xmlpp

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

    class dom_parser
    {
        typedef xmlpp::DomParser DomParser;

      public:
        dom_parser(std::string const& filename);
        dom_parser(std::istream&);
        ~dom_parser();

        Element const& root_node(std::string const& expected_name) const;

      private:
        std::string error_context_;
        boost::scoped_ptr<DomParser> parser_;
    };

    /// Retrieve an xml element's full text-node contents.
    ///
    /// The contents of all text-node children are concatenated and
    /// returned. Only direct children are considered: children of
    /// child nodes are not.

    std::string get_content(Element const& element);

    /// Get the first non-text node element under a parent node.

    Element      * get_first_element(Element      & parent);
    Element const* get_first_element(Element const& parent);
} // namespace xml_lmi

/// Streaming operator for xml documents.
///
/// Avoid using libxml++'s write_to_stream() directly: see
///   http://lists.gnu.org/archive/html/lmi/2006-10/msg00023.html

std::ostream& operator<<(std::ostream&, xml_lmi::Document&);

#endif //xml_lmi_hpp

