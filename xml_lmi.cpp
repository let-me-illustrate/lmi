// libxml++ inclusion header.
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

// $Id: xml_lmi.cpp,v 1.1.2.2 2006-10-15 23:29:43 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_lmi.hpp"

#include <cstddef>
#include <ios>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <sstream>

namespace xmlpp
{

/**
   Retrieve the text content of the node.

   This function is not recursive, meaning that only the text-nodes
   directly attached to the node are taken into the account.
 */
std::string LmiHelper::get_content(const Element & node)
{
    std::ostringstream buf;
    Node::NodeList const nodeChildren = node.get_children();
    for( Node::NodeList::const_iterator iter = nodeChildren.begin();
                                        iter != nodeChildren.end();
                                        ++iter )
    {
        xmlpp::TextNode const * textNode
            = dynamic_cast<xmlpp::TextNode const *>(*iter);
        // maybe we should add CdataNode also?
        if(textNode)
        {
            buf << textNode->get_content();
        }
    }
    return buf.str();
}

Element * LmiHelper::get_first_element(Element & node)
{
    Node::NodeList const nodeList = node.get_children();
    for( Node::NodeList::const_iterator iter = nodeList.begin();
                                        iter != nodeList.end();
                                        ++iter )
    {
        Element const * pChild = dynamic_cast<Element const *>(*iter);
        if(pChild)
            return const_cast<Element*>(pChild);
    }
    return 0;
}

const Element * LmiHelper::get_first_element(const Element & node)
{
    Node::NodeList const nodeList = node.get_children();
    for( Node::NodeList::const_iterator iter = nodeList.begin();
                                               iter != nodeList.end();
                                               ++iter )
    {
        Element const * pChild = dynamic_cast<Element const *>(*iter);
        if(pChild)
            return pChild;
    }
    return 0;
}


std::ostream & operator << ( std::ostream & os, Document & document )
{
    document.write_to_stream( os, "utf-8" );
    return os;
}

} // namespace xmlpp

