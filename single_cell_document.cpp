// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: single_cell_document.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "single_cell_document.hpp"

#include "inputillus.hpp"
#include "xmlwrapp_ex.hpp"

#include <xmlwrapp/init.h>
#include <xmlwrapp/tree_parser.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

//============================================================================
single_cell_document::single_cell_document()
    :input_data(new IllusInputParms())
{
}

//============================================================================
single_cell_document::single_cell_document(IllusInputParms const& parms)
    :input_data(new IllusInputParms(parms))
{
}

//============================================================================
single_cell_document::single_cell_document(std::string const& filename)
    :input_data(new IllusInputParms())
{
    xml::init init;
// XMLWRAPP !! See comment on parse() in header.
//    parse(xml::tree_parser(filename.c_str()));
    xml::tree_parser parser(filename.c_str());
    parse(parser);
}

//============================================================================
single_cell_document::~single_cell_document()
{
}

//============================================================================
std::string single_cell_document::xml_root_name() const
{
    return "single_cell_document";
}

//============================================================================
void single_cell_document::parse(xml::tree_parser& parser)
{
    if(!parser)
        {
        throw std::runtime_error("Error parsing XML file.");
        }

    xml::node& root = parser.get_root_node();
    if(xml_root_name() != root.get_name())
        {
        std::ostringstream msg;
        msg
            << "XML node name is '"
            << root.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            ;
        throw std::runtime_error(msg.str());
        }

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

// TODO ?? Seems like this should be a const iterator. OTOH, maybe
// the streaming operators should take a node iterator rather than
// a node as argument?
    xml::node::iterator child = root.begin();
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
        ++child;
        }
    if(!child->is_text())
        {
        *child >> *input_data;
        }
}

//============================================================================
void single_cell_document::read(std::istream& is)
{
    // XMLWRAPP !! xmlwrapp-0.2.0 doesn't know about istreams yet, so
    // read the istream into a std::string via a std::ostringstream
    // and pass that to the xml::tree_parser ctor that takes a char* .

    std::string s(istream_to_string(is));
    xml::init init;
// XMLWRAPP !! See comment on parse() in header.
//    parse(xml::tree_parser(s.c_str(), 1 + s.size()));
    xml::tree_parser parser(s.c_str(), 1 + s.size());
    parse(parser);
}

//============================================================================
void single_cell_document::write(std::ostream& os)
{
    xml::init init;
    xml::node root(xml_root_name().c_str());
    root << *input_data;
    os << root;
}

