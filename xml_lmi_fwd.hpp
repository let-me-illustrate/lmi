// Interface to xmlwrapp: forward declarations.
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

// $Id: xml_lmi_fwd.hpp,v 1.3 2006-11-26 20:51:05 chicares Exp $

#ifndef xml_lmi_fwd_hpp
#define xml_lmi_fwd_hpp

#include "config.hpp"

#if defined USING_LIBXMLPP
#   include "xmlpp_lmi_fwd.hpp"
#else // !defined USING_LIBXMLPP

namespace xml // This is xmlwrapp's namespace.
{
#if defined USING_CURRENT_XMLWRAPP
    class attributes;
    class document;
#endif // defined USING_CURRENT_XMLWRAPP
    class init;
    class node;
    class tree_parser;
} // Namespace xml.

/// Interface to xmlwrapp.

namespace xml_lmi
{
    class dom_parser;
    class xml_document;

#if defined USING_CURRENT_XMLWRAPP
    typedef xml::attributes Attribute;
    typedef xml::document   Document;
#endif // defined USING_CURRENT_XMLWRAPP
    typedef xml::node       Element;

// Something like this might be useful:
//    typedef std::vector<xml::node::const_iterator> ElementContainer;
// but a nested class can't be forward declared.
} // namespace xml_lmi

#endif // !defined USING_LIBXMLPP

#endif //xml_lmi_fwd_hpp

