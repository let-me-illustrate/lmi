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

// $Id: xml_lmi.cpp,v 1.1.2.7 2006-10-18 01:20:16 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_lmi.hpp"

#include <ostream>
#include <sstream>

namespace xml_lmi
{

std::string xml_lmi::get_content(Element const& element)
{
    std::ostringstream buf;
    xml_lmi::NodeContainer const nodeChildren = element.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = nodeChildren.begin()
        ;iter != nodeChildren.end()
        ;++iter
        )
        {
        xmlpp::TextNode const* textNode = dynamic_cast<xmlpp::TextNode const*>(*iter);
        // maybe we should add CdataNode also?
        if(textNode)
            {
            buf << textNode->get_content();
            }
        }
    return buf.str();
}

Element* xml_lmi::get_first_element(Element& parent)
{
    xml_lmi::NodeContainer const nodeList = parent.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = nodeList.begin()
        ;iter != nodeList.end()
        ;++iter
        )
        {
        Element const* pChild = dynamic_cast<Element const*>(*iter);
        if(pChild)
            {
            return const_cast<Element*>(pChild);
            }
        }
    return 0;
}

Element const* xml_lmi::get_first_element(Element const& parent)
{
    xml_lmi::NodeContainer const nodeList = parent.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = nodeList.begin()
        ;iter != nodeList.end()
        ;++iter
        )
        {
        Element const* pChild = dynamic_cast<Element const*>(*iter);
        if(pChild)
            {
            return pChild;
            }
        }
    return 0;
}

} // Namespace xml_lmi.

std::ostream& operator<<(std::ostream& os, xml_lmi::Document& document)
{
    document.write_to_stream(os, "utf-8");
    return os;
}

