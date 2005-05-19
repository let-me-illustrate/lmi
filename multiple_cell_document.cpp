// xml document for multiple-cell illustrations.
//
// Copyright (C) 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: multiple_cell_document.cpp,v 1.5 2005-05-19 12:30:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "multiple_cell_document.hpp"

#include "alert.hpp"
#include "inputillus.hpp"
#include "value_cast.hpp"
#include "xmlwrapp_ex.hpp"

#ifdef USING_CURRENT_XMLWRAPP
#   include <xmlwrapp/document.h>
#endif // USING_CURRENT_XMLWRAPP defined.
#include <xmlwrapp/init.h>
#include <xmlwrapp/tree_parser.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

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
    xml::init init;
// XMLWRAPP !! See comment on parse() in header.
//    parse(xml::tree_parser(filename.c_str()));
    xml::tree_parser parser(filename.c_str());
    parse(parser);
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
void multiple_cell_document::parse(xml::tree_parser& parser)
{
    if(!parser)
        {
        throw std::runtime_error("Error parsing XML file.");
        }

#ifdef USING_CURRENT_XMLWRAPP
    xml::node& root = parser.get_document().get_root_node();
#else // USING_CURRENT_XMLWRAPP not defined.
    xml::node& root = parser.get_root_node();
#endif // USING_CURRENT_XMLWRAPP not defined.
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

// TODO ?? It doesn't seem right to depend on node order.
// See note below--perhaps do something like this:
//    int NumberOfCases;
//    is >> NumberOfCases;
//    LMI_ASSERT(1 == NumberOfCases);

    IllusInputParms temp;

    // Case default parameters.

    case_parms_.clear();
    xml::node::iterator child = root.begin();
    if(child->is_text())
        {
        // TODO ?? Explain what this does (passim).
        ++child;
        }
    if(std::string("cell") != child->get_name())
        {
        std::ostringstream msg;
        msg
            << "XML node name is '"
            << child->get_name()
            << "' but '"
            << "cell"
            << "' was expected."
            ;
        throw std::runtime_error(msg.str());
        }
    (*child) >> temp;
    case_parms_.push_back(temp);

    // Number of classes.
    ++child;
    if(child->is_text())
        {
        ++child;
        }
    if(std::string("NumberOfClasses") != child->get_name())
        {
        std::ostringstream msg;
        msg
            << "XML node name is '"
            << child->get_name()
            << "' but '"
            << "NumberOfClasses"
            << "' was expected."
            ;
        throw std::runtime_error(msg.str());
        }
    unsigned int number_of_classes = value_cast<unsigned int>
        (child->get_content()
        );

    // Parameters for each class.
    class_parms_.clear();
    class_parms_.reserve(number_of_classes);

    ++child;
    if(child->is_text())
        {
        ++child;
        }
    for(; child != root.end(); ++child)
        {
        if(!child->is_text())
            {
            (*child) >> temp;
            class_parms_.push_back(temp);
            }
        if(class_parms_.size() == number_of_classes)
            {
            break;
            }
        }
    if(class_parms_.size() != number_of_classes)
        {
        std::ostringstream msg;
        msg
            << "Number of classes read is "
            << class_parms_.size()
            << " but should have been "
            << number_of_classes
            << "."
            ;
        throw std::runtime_error(msg.str());
        }

    // Number of cells.
    ++child;
    if(child->is_text())
        {
        ++child;
        }
    if(std::string("NumberOfCells") != child->get_name())
        {
        std::ostringstream msg;
        msg
            << "XML node name is '"
            << child->get_name()
            << "' but '"
            << "NumberOfCells"
            << "' was expected."
            ;
        throw std::runtime_error(msg.str());
        }
    unsigned int number_of_cells = value_cast<unsigned int>
        (child->get_content()
        );

    // Parameters for each Cell.
    cell_parms_.clear();
    cell_parms_.reserve(number_of_cells);

    ++child;
    if(child->is_text())
        {
        ++child;
        }
    for(; child != root.end(); ++child)
        {
        if(!child->is_text())
            {
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
            }
        if(cell_parms_.size() == number_of_cells)
            {
            break;
            }
        }
    if(cell_parms_.size() != number_of_cells)
        {
        std::ostringstream msg;
        msg
            << "Number of individuals read is "
            << cell_parms_.size()
            << " but should have been "
            << number_of_cells
            << "."
            ;
        throw std::runtime_error(msg.str());
        }

    ++child;
    if(child->is_text())
        {
        ++child;
        }
    if(child != root.end())
        {
        std::ostringstream msg;
        msg
            << "Read all data expected in XML document, "
            << "but more data remains."
            ;
        throw std::runtime_error(msg.str());
        }
}

//============================================================================
void multiple_cell_document::read(std::istream& is)
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
void multiple_cell_document::write(std::ostream& os) const
{
    xml::init init;
    xml::node root(xml_root_name().c_str());

// TODO ?? Diagnostics will be cryptic if the xml doesn't follow
// the required layout. Perhaps they could be improved. Maybe it
// would be better to restructure the document so that each set
// of cells, with its cardinal number, is a distinct node.
//
//    root.push_back
//        (xml::node
//            ("NumberOfCases"
//            ,value_cast<std::string>(case_parms_.size()).c_str()
//            )
//        );
    root << case_parms_[0];

    root.push_back
        (xml::node
            ("NumberOfClasses"
            ,value_cast<std::string>(class_parms_.size()).c_str()
            )
        );
    for(unsigned int j = 0; j < class_parms_.size(); j++)
        {
        root << class_parms_[j];
        }

    root.push_back
        (xml::node
            ("NumberOfCells"
            ,value_cast<std::string>(cell_parms_.size()).c_str()
            )
        );
    for(unsigned int j = 0; j < cell_parms_.size(); j++)
        {
        root << cell_parms_[j];
        }

    os << root;
}

