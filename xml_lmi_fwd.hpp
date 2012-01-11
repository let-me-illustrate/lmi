// Interface to xmlwrapp: forward declarations.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#ifndef xml_lmi_fwd_hpp
#define xml_lmi_fwd_hpp

#include "config.hpp"

namespace xml // This is xmlwrapp's namespace.
{
    class attributes;
    class document;
    class init;
    class node;
    class tree_parser;

    /// XMLWRAPP !! It is useful to distinguish elements from DOM
    /// nodes that are not elements; xmlwrapp doesn't make this
    /// distinction, but a future replacement might.
    typedef xml::node element;
} // namespace xml

/// Interface to xmlwrapp.

namespace xml_lmi
{
    class dom_parser;
    class xml_document;

    typedef xml::attributes Attribute;
    typedef xml::document   Document;
} // namespace xml_lmi

#endif // xml_lmi_fwd_hpp

