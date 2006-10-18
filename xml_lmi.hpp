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

// $Id: xml_lmi.hpp,v 1.1.2.6 2006-10-18 01:20:16 chicares Exp $

#ifndef xml_lmi_hpp
#define xml_lmi_hpp

#include "config.hpp"

#include <libxml++/libxml++.h>

#include <iosfwd>
#include <string>

namespace xml_lmi
{
    typedef xmlpp::Attribute      Attribute;
    typedef xmlpp::Document       Document;
    typedef xmlpp::DomParser      DomParser;
    typedef xmlpp::Element        Element;
    typedef xmlpp::Node::NodeList NodeContainer;

    /// Retrieve an xml element's full text-node contents.
    ///
    /// The contents of all text-node children are concatenated and
    /// returned. Only direct children are considered: children of
    /// child nodes are not.

    std::string get_content(Element const& element);

    /// Get the first non-text node element under a parent node.

    Element      * get_first_element(Element      & parent);
    Element const* get_first_element(Element const& parent);
}

/// Streaming operator for xml documents.
///
/// Avoid using libxml++'s write_to_stream() directly: see
///   http://lists.gnu.org/archive/html/lmi/2006-10/msg00023.html

std::ostream& operator<<(std::ostream&, xml_lmi::Document&);

#endif //xml_lmi_hpp

