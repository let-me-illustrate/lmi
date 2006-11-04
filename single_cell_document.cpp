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
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: single_cell_document.cpp,v 1.10 2006-11-04 14:27:06 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "single_cell_document.hpp"

#include "alert.hpp"
#include "inputillus.hpp"
#include "istream_to_string.hpp"
#include "xml_lmi.hpp"

#ifdef USING_CURRENT_XMLWRAPP
#   include <xmlwrapp/document.h>
#endif // USING_CURRENT_XMLWRAPP defined.
#include <xmlwrapp/init.h>
#include <xmlwrapp/tree_parser.h>

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
    xml::init init;
    xml::tree_parser parser(filename.c_str());
    if(!parser)
        {
        throw std::runtime_error("Error parsing XML file.");
        }

#ifdef USING_CURRENT_XMLWRAPP
    xml_lmi::Element& root = parser.get_document().get_root_node();
#else // USING_CURRENT_XMLWRAPP not defined.
    xml_lmi::Element& root = parser.get_root_node();
#endif // USING_CURRENT_XMLWRAPP not defined.
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
    parse(root);
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
void single_cell_document::parse(xml_lmi::Element const& root)
{
// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

// TODO ?? Seems like this should be a const iterator. OTOH, maybe
// the streaming operators should take a node iterator rather than
// a node as argument?
    xml::node::const_iterator child = root.begin();
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
        ++child;
        }
    if(!child->is_text())
        {
        *child >> *input_data_;
        }
}

//============================================================================
void single_cell_document::read(std::istream& is)
{
    // XMLWRAPP !! xmlwrapp-0.2.0 doesn't know about istreams yet, so
    // read the istream into a std::string via a std::ostringstream
    // and pass that to the xml::tree_parser ctor that takes a char* .

    std::string s;
    istream_to_string(is, s);
    xml::init init;
    xml::tree_parser parser(s.c_str(), 1 + s.size());
    if(!parser)
        {
        throw std::runtime_error("Error parsing XML file.");
        }

#ifdef USING_CURRENT_XMLWRAPP
    xml_lmi::Element& root = parser.get_document().get_root_node();
#else // USING_CURRENT_XMLWRAPP not defined.
    xml_lmi::Element& root = parser.get_root_node();
#endif // USING_CURRENT_XMLWRAPP not defined.
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
    parse(root);
}

//============================================================================
void single_cell_document::write(std::ostream& os)
{
    xml::init init;
    xml_lmi::Element root(xml_root_name().c_str());
    root << *input_data_;
    os << root;
}

