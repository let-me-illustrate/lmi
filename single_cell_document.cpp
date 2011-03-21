// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "single_cell_document.hpp"

#include "assert_lmi.hpp"
#include "input.hpp"
#include "xml_lmi.hpp"

#include <xmlwrapp/nodes_view.h>

#include <istream>
#include <ostream>

//============================================================================
single_cell_document::single_cell_document()
    :input_data_(new Input)
{
}

//============================================================================
single_cell_document::single_cell_document(Input const& parms)
    :input_data_(new Input(parms))
{
}

/// This ctor is used to read the default input file.

single_cell_document::single_cell_document(std::string const& filename)
    :input_data_(new Input)
{
    xml_lmi::dom_parser parser(filename);
    parse(parser.root_node(xml_root_name()));
}

//============================================================================
single_cell_document::~single_cell_document()
{
}

//============================================================================
std::string const& single_cell_document::xml_root_name() const
{
    static std::string const s("single_cell_document");
    return s;
}

//============================================================================
void single_cell_document::parse(xml::element const& root)
{
    xml::const_nodes_view const elements(root.elements());
    LMI_ASSERT(!elements.empty());
    xml::const_nodes_view::const_iterator i(elements.begin());
    *i >> *input_data_;
    // XMLWRAPP !! It would be better to have operator+(int) in the
    // iterator class, and to write this check above as
    //   LMI_ASSERT(elements.end() == 1 + i);
    LMI_ASSERT(elements.end() == ++i);
}

//============================================================================
void single_cell_document::read(std::istream const& is)
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

