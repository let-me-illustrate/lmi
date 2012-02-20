// xml document for multiple-cell illustrations.
//
// Copyright (C) 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#include "multiple_cell_document.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <xmlwrapp/nodes_view.h>

#include <istream>
#include <ostream>
#include <stdexcept>

//============================================================================
multiple_cell_document::multiple_cell_document()
    :case_parms_  (1)
    ,class_parms_ (1)
    ,cell_parms_  (1)
{
    assert_vector_sizes_are_sane();
}

//============================================================================
multiple_cell_document::multiple_cell_document(std::string const& filename)
{
    xml_lmi::dom_parser parser(filename);
    parse(parser.root_node(xml_root_name()));
    assert_vector_sizes_are_sane();
}

//============================================================================
multiple_cell_document::~multiple_cell_document()
{
}

/// Verify invariants.
///
/// Throws if any asserted invariant does not hold.

void multiple_cell_document::assert_vector_sizes_are_sane() const
{
    LMI_ASSERT(1 == case_parms_.size());
    LMI_ASSERT(    !class_parms_.empty());
    LMI_ASSERT(    !cell_parms_ .empty());
}

/// Backward-compatibility serial number of this class's xml version.
///
/// What is now called version 0 had no "version" attribute.
///
/// version 0: [prior to the lmi epoch]
/// version 1: 20120220T0158Z

int multiple_cell_document::class_version() const
{
    return 1;
}

//============================================================================
std::string const& multiple_cell_document::xml_root_name() const
{
    static std::string const s("multiple_cell_document");
    return s;
}

namespace
{
/// Throw an exception while pretending to return an lvalue.
///
/// Motivating example:
///   X& x(boolean ? x1 : throw("oops"));
/// It is handy to write a throw-expression as the last operand in a
/// cascaded conditional-expression. However, that doesn't work when
/// an lvalue must be returned: according to C++2003, [15/1] a throw-
/// expression is of type void, so [5.16/2] the conditional-expression
/// above returns an lvalue. The motivating example works correctly
/// when rewritten this way:
///   X& x(boolean ? x1 : invalid<X>("oops"));

template<typename T>
T& hurl(std::string const& s)
{
    throw std::runtime_error(s.c_str());
}
} // Unnamed namespace.

/// Read xml into vectors of class Input.
///
/// The optional "size_hint" attributes improve speed remarkably.

void multiple_cell_document::parse(xml::element const& root)
{
    int file_version = 0;
    if(!xml_lmi::get_attr(root, "version", file_version))
        {
        parse_v0(root);
        return;
        }

    // Version 0 should have been handled above.
    LMI_ASSERT(0 < file_version);
    if(class_version() < file_version)
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        }

    case_parms_ .clear();
    class_parms_.clear();
    cell_parms_ .clear();

    xml::const_nodes_view const elements(root.elements());
    typedef xml::const_nodes_view::const_iterator cnvi;
    Input cell;
    int counter = 0;
    for(cnvi i = elements.begin(); i != elements.end(); ++i)
        {
        std::string const tag(i->get_name());
        std::vector<Input>& v
            ( ("case_default"     == tag) ? case_parms_
            : ("class_defaults"   == tag) ? class_parms_
            : ("particular_cells" == tag) ? cell_parms_
            : hurl<std::vector<Input> >("Unexpected element '" + tag + "'.")
            );
        int size_hint = 0;
        if(xml_lmi::get_attr(*i, "size_hint", size_hint))
            {
            v.reserve(size_hint);
            }
        xml::const_nodes_view const subelements(i->elements());
        for(cnvi j = subelements.begin(); j != subelements.end(); ++j)
            {
            *j >> cell;
            v.push_back(cell);
            status() << "Read " << ++counter << " cells." << std::flush;
            }
        }

    assert_vector_sizes_are_sane();
}

/// Parse obsolete version 0 xml (for backward compatibility).

void multiple_cell_document::parse_v0(xml::element const& root)
{
    Input temp;

    xml::const_nodes_view const elements(root.elements());
    typedef xml::const_nodes_view::const_iterator cnvi;
    cnvi i = elements.begin();

    // Case default parameters.

    case_parms_.clear();

    LMI_ASSERT(i != elements.end());
    if("cell" != xml_lmi::get_name(*i))
        {
        fatal_error()
            << "XML node name is '"
            << xml_lmi::get_name(*i)
            << "' but '"
            << "cell"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    *i >> temp;
    case_parms_.push_back(temp);

    // Number of classes.
    ++i;
    LMI_ASSERT(i != elements.end());
    if("NumberOfClasses" != xml_lmi::get_name(*i))
        {
        fatal_error()
            << "XML node name is '"
            << xml_lmi::get_name(*i)
            << "' but '"
            << "NumberOfClasses"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    std::string const n_classes = xml_lmi::get_content(*i);
    LMI_ASSERT(!n_classes.empty());
    unsigned int number_of_classes = value_cast<unsigned int>(n_classes);

    // Parameters for each class.
    class_parms_.clear();
    class_parms_.reserve(number_of_classes);

    ++i;
    for(; i != elements.end(); ++i)
        {
        *i >> temp;
        class_parms_.push_back(temp);
        if(class_parms_.size() == number_of_classes)
            {
            break;
            }
        }
    if(class_parms_.size() != number_of_classes)
        {
        fatal_error()
            << "Number of classes read is "
            << class_parms_.size()
            << " but should have been "
            << number_of_classes
            << "."
            << LMI_FLUSH
            ;
        }

    // Number of cells.
    LMI_ASSERT(i != elements.end());
    ++i;
    LMI_ASSERT(i != elements.end());
    if("NumberOfCells" != xml_lmi::get_name(*i))
        {
        fatal_error()
            << "XML node name is '"
            << xml_lmi::get_name(*i)
            << "' but '"
            << "NumberOfCells"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    std::string const n_cells = xml_lmi::get_content(*i);
    LMI_ASSERT(!n_cells.empty());
    unsigned int number_of_cells = value_cast<unsigned int>(n_cells);

    // Parameters for each cell.
    cell_parms_.clear();
    cell_parms_.reserve(number_of_cells);

    ++i;
    for(; i != elements.end(); ++i)
        {
        *i >> temp;
        cell_parms_.push_back(temp);
        status()
            << "Read "
            << cell_parms_.size()
            << " of "
            << number_of_cells
            << " lives."
            << std::flush
            ;
        if(cell_parms_.size() == number_of_cells)
            {
            break;
            }
        }
    if(cell_parms_.size() != number_of_cells)
        {
        fatal_error()
            << "Number of individuals read is "
            << cell_parms_.size()
            << " but should have been "
            << number_of_cells
            << "."
            << LMI_FLUSH
            ;
        }

    LMI_ASSERT(i != elements.end());
    ++i;
    if(i != elements.end())
        {
        fatal_error()
            << "Read all data expected in XML document, "
            << "but more data remains."
            << LMI_FLUSH
            ;
        }

    assert_vector_sizes_are_sane();
}

//============================================================================
void multiple_cell_document::read(std::istream const& is)
{
    xml_lmi::dom_parser parser(is);
    parse(parser.root_node(xml_root_name()));
}

//============================================================================
void multiple_cell_document::write(std::ostream& os) const
{
    assert_vector_sizes_are_sane();

    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    xml_lmi::set_attr(root, "version", class_version());

    xml::element case_default("case_default");
    xml::node::iterator case_i = root.insert(case_default);
    case_parms_[0].write(*case_i);

    typedef std::vector<Input>::const_iterator svii;

    xml::element class_defaults("class_defaults");
    xml::node::iterator classes_i = root.insert(class_defaults);
    xml_lmi::set_attr(*classes_i, "size_hint", class_parms_.size());
    for(svii i = class_parms_.begin(); i != class_parms_.end(); ++i)
        {
        i->write(*classes_i);
        }

    xml::element particular_cells("particular_cells");
    xml::node::iterator cells_i = root.insert(particular_cells);
    xml_lmi::set_attr(*cells_i, "size_hint", cell_parms_.size());
    for(svii i = cell_parms_.begin(); i != cell_parms_.end(); ++i)
        {
        i->write(*cells_i);
        }

    os << document;
}

