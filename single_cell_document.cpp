// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: single_cell_document.cpp,v 1.14 2007-01-27 00:00:52 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "single_cell_document.hpp"

#include "alert.hpp"
#include "inputillus.hpp"
#include "xml_lmi.hpp"

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
    xml_lmi::dom_parser parser(filename);
    parse(parser.root_node(xml_root_name()));
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
void single_cell_document::parse(xml::element const& root)
{
    xml_lmi::ElementContainer const elements(xml_lmi::child_elements(root));
    LMI_ASSERT(1 == elements.size());
    *elements[0] >> *input_data_;
}

//============================================================================
void single_cell_document::read(std::istream& is)
{
    xml_lmi::dom_parser parser(is);
    parse(parser.root_node(xml_root_name()));
}

//============================================================================
void single_cell_document::write(std::ostream& os)
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    root << *input_data_;
    os << document;
}

