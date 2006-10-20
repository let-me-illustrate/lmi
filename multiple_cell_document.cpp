// xml document for multiple-cell illustrations.
//
// Copyright (C) 2002, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: multiple_cell_document.cpp,v 1.9.2.11 2006-10-20 00:25:12 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "multiple_cell_document.hpp"

#include "alert.hpp"
#include "inputillus.hpp"
#include "value_cast.hpp"

#include <libxml++/libxml++.h>

//============================================================================
multiple_cell_document::multiple_cell_document()
    :case_parms_  (1)
    ,class_parms_ (1)
    ,cell_parms_  (1)
{
}

//============================================================================
multiple_cell_document::multiple_cell_document(std::string const& filename)
{
    xml_lmi::dom_parser parser(filename);
    parse(parser.root_node(xml_root_name()));
}

//============================================================================
multiple_cell_document::~multiple_cell_document()
{
}

//============================================================================
std::string multiple_cell_document::xml_root_name() const
{
    return "multiple_cell_document";
}

//============================================================================
void multiple_cell_document::parse(xml_lmi::Element const& root)
{
// TODO ?? It doesn't seem right to depend on node order.
// See note below--perhaps do something like this:
//    int NumberOfCases;
//    is >> NumberOfCases;
//    LMI_ASSERT(1 == NumberOfCases);

    IllusInputParms temp;

    // Case default parameters.

    case_parms_.clear();

    typedef std::list<xml_lmi::Element*> XmlppElements;

    XmlppElements elements;
    {
        // fill elements list with element nodes of the root
        xml_lmi::NodeContainer const c = root.get_children();
        for
            (xml_lmi::NodeContainer::const_iterator iter = c.begin()
            ;iter != c.end()
            ;++iter
            )
            {
            xml_lmi::Element const* el = dynamic_cast<xml_lmi::Element const*>(*iter);
            if(el)
                {
                elements.push_back(const_cast<xml_lmi::Element*>(el));
                }
            }
    }

    XmlppElements::const_iterator iter = elements.begin();
    xml_lmi::Element* child = 0;

    if(iter == elements.end() || (child = *iter)->get_name() != "cell")
        {
        fatal_error()
            << "XML node name is '"
            << (iter == elements.end() ? "no node" : child->get_name())
            << "' but '"
            << "cell"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    (*child) >> temp;
    case_parms_.push_back(temp);

    // Number of classes.
    ++iter;
    if(iter == elements.end() || (child = *iter)->get_name() != "NumberOfClasses")
        {
        fatal_error()
            << "XML node name is '"
            << (iter == elements.end() ? "no node" : child->get_name())
            << "' but '"
            << "NumberOfClasses"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    unsigned int number_of_classes = value_cast<unsigned int>
        (xml_lmi::get_content(*child)
        );

    // Parameters for each class.
    class_parms_.clear();
    class_parms_.reserve(number_of_classes);

    for(; ++iter != elements.end();)
        {
        child = *iter;
        (*child) >> temp;
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
    ++iter;
    if(iter == elements.end() || (child = *iter)->get_name() != "NumberOfCells")
        {
        fatal_error()
            << "XML node name is '"
            << (iter == elements.end() ? "no node" : child->get_name())
            << "' but '"
            << "NumberOfCells"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    unsigned int number_of_cells = value_cast<unsigned int>
        (xml_lmi::get_content(*child)
        );

    // Parameters for each Cell.
    cell_parms_.clear();
    cell_parms_.reserve(number_of_cells);

    for(; ++iter != elements.end();)
        {
            child = *iter;
            (*child) >> temp;
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

    if(++iter != elements.end())
        {
        fatal_error()
            << "Read all data expected in XML document, "
            << "but more data remains."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void multiple_cell_document::read(std::istream& is)
{
    xml_lmi::dom_parser parser(is);
    parse(parser.root_node(xml_root_name()));
}

//============================================================================
void multiple_cell_document::write(std::ostream& os) const
{
    xml_lmi::Document doc;
    xml_lmi::Element& root = *doc.create_root_node(xml_root_name());

// TODO ?? Diagnostics will be cryptic if the xml doesn't follow
// the required layout. Perhaps they could be improved. Maybe it
// would be better to restructure the document so that each set
// of cells, with its cardinal number, is a distinct node.
//
//    root.add_child("NumberOfCases")->add_child_text
//        (value_cast<std::string>(case_parms_.size())
//        );
    root << case_parms_[0];

    root.add_child("NumberOfClasses")->add_child_text
        (value_cast<std::string>(class_parms_.size())
        );
    for(unsigned int j = 0; j < class_parms_.size(); j++)
        {
        root << class_parms_[j];
        }

    root.add_child("NumberOfCells")->add_child_text
        (value_cast<std::string>(cell_parms_.size())
        );
    for(unsigned int j = 0; j < cell_parms_.size(); j++)
        {
        root << cell_parms_[j];
        }

    os << doc;
}

