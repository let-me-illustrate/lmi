// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2005, 2006 Gregory W. Chicares.
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

// $Id: single_cell_document.cpp,v 1.6.2.1 2006-01-28 01:41:59 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "single_cell_document.hpp"

#include "alert.hpp"
#include "inputillus.hpp"
#include "istream_to_string.hpp"

#include "xmlpp.hpp"

#include <fstream>

//============================================================================
single_cell_document::single_cell_document()
    :input_data_(new IllusInputParms())
{
}

//============================================================================
single_cell_document::single_cell_document(IllusInputParms const& parms)
    :input_data_(new IllusInputParms(parms))
{
}

/// This ctor doesn't use class IllusInputParms's defaults.
/// It needn't, because it overrides those defaults anyway.
/// It mustn't, because it's used to read those defaults.

single_cell_document::single_cell_document(std::string const& filename)
    :input_data_(new IllusInputParms(false))
{
    xmlpp::DomParser parser;
    parser.parse_file(filename);
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
void single_cell_document::parse(const xmlpp::DomParser & parser)
{
    if(!parser)
        {
        throw std::runtime_error("Error parsing XML file.");
        }

    xmlpp::Element& root = *parser.get_document()->get_root_node();
    if(xml_root_name() != root.get_name())
        {
        fatal_error()
            << "XML node name is '"
            << root.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            << LMI_FLUSH
            ;
        }

    // read from the first non textual node all the information
    // in other words this 'single cell' document xml representation
    // should contain one and only one xml node
    xmlpp::Node::NodeList children = root.get_children();
    for( xmlpp::Node::NodeList::iterator iter = children.begin();
                                         iter != children.end();
                                         ++iter )
    {
        xmlpp::Element* child = dynamic_cast< xmlpp::Element * >( *iter );
        if( child )
        {
        *child >> *input_data_;
        break;
        }
    }
}

//============================================================================
void single_cell_document::read(std::istream& is)
{
    xmlpp::DomParser parser;
    parser.parse_stream(is);
    parse(parser);
}

//============================================================================
void single_cell_document::write(std::ostream& os)
{
    xmlpp::Document document;
    xmlpp::Element & root = *document.create_root_node(xml_root_name());
    root << *input_data_;
    document.write_to_stream(os);
}

