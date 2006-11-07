// Interface to libxml++ : forward declarations.
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

// $Id: xmlpp_lmi_fwd.hpp,v 1.1 2006-11-07 03:23:21 chicares Exp $

#ifndef xmlpp_lmi_fwd_hpp
#define xmlpp_lmi_fwd_hpp

#include "config.hpp"

#if defined USING_LIBXMLPP

namespace xmlpp
{
    class Attribute;
    class Document;
    class DomParser;
    class Element;
    class Node;
} // namespace xmlpp

namespace xml_lmi
{
    typedef xmlpp::Attribute Attribute;
    typedef xmlpp::Document  Document;
    typedef xmlpp::Element   Element;
} // namespace xml_lmi

#endif // defined USING_LIBXMLPP

#endif //xmlpp_lmi_fwd_hpp

