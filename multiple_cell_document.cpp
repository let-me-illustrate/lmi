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

// $Id: multiple_cell_document.cpp,v 1.2 2005-02-12 12:59:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "multiple_cell_document.hpp"

#include "alert.hpp"
#include "inputillus.hpp"
#include "value_cast_ihs.hpp"
#include "xmlwrapp_ex.hpp"

#include <xmlwrapp/init.h>
#include <xmlwrapp/tree_parser.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

//============================================================================
multiple_cell_document::multiple_cell_document()
    :CaseParms(1)
    ,ClassParms(1)
    ,IndividualParms(1)
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

// TODO ?? Could this ever be useful? If not, then expunge.
//    int NumberOfCases;
//    is >> NumberOfCases;
//    LMI_ASSERT(1 == NumberOfCases);

// TODO ?? It doesn't seem right to depend on node order.

    IllusInputParms temp;

    // Case default parameters.

    CaseParms.clear();
    xml::node::iterator child = root.begin();
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
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
    CaseParms.push_back(temp);

    // Number of Classes.
    ++child;
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
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
    unsigned int NumberOfClasses = value_cast_ihs<int>(child->get_content());

    // Parameters for each Class.
    ClassParms.clear();
    ClassParms.reserve(NumberOfClasses);

    ++child;
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
        ++child;
        }
    for(; child != root.end(); ++child)
        {
        if(!child->is_text())
            {
            (*child) >> temp;
            ClassParms.push_back(temp);
            }
        if(ClassParms.size() == NumberOfClasses)
            {
            break;
            }
        }
    if(ClassParms.size() != NumberOfClasses)
        {
        std::ostringstream msg;
        msg
            << "Number of classes read is "
            << ClassParms.size()
            << " but should have been "
            << NumberOfClasses
            << "."
            ;
        throw std::runtime_error(msg.str());
        }

    // Number of Cells.
    ++child;
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
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
    unsigned int NumberOfCells = value_cast_ihs<int>(child->get_content());

    // Parameters for each Cell.
    IndividualParms.clear();
    IndividualParms.reserve(NumberOfCells);

    ++child;
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
        ++child;
        }
    for(; child != root.end(); ++child)
        {
        if(!child->is_text())
            {
            (*child) >> temp;
            IndividualParms.push_back(temp);
            status()
                << "Read "
                << IndividualParms.size()
                << " of "
                << NumberOfCells
                << " lives."
                << std::flush
                ;
            }
        if(IndividualParms.size() == NumberOfCells)
            {
            break;
            }
        }
    if(IndividualParms.size() != NumberOfCells)
        {
        std::ostringstream msg;
        msg
            << "Number of individuals read is "
            << IndividualParms.size()
            << " but should have been "
            << NumberOfCells
            << "."
            ;
        throw std::runtime_error(msg.str());
        }

    ++child;
    if(child->is_text())
        {
        // TODO ?? Explain what this does.
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

// TODO ?? Could this ever be useful?
//    root.push_back
//        (xml::node
//            ("NumberOfCases"
//            ,value_cast_ihs<std::string>(CaseParms.size()).c_str()
//            )
//        );
    root << CaseParms[0];

// TODO ?? Diagnostics will be cryptic if the xml doesn't follow
// the required layout. Perhaps they could be improved. Maybe it
// would be better to restructure the document so that each set
// of cells, with its cardinal number, is a distinct node.

    root.push_back
        (xml::node
            ("NumberOfClasses"
            ,value_cast_ihs<std::string>(ClassParms.size()).c_str()
            )
        );
    for(unsigned int j = 0; j < ClassParms.size(); j++)
        {
        root << ClassParms[j];
        }

    root.push_back
        (xml::node
            ("NumberOfCells"
            ,value_cast_ihs<std::string>(IndividualParms.size()).c_str()
            )
        );
    for(unsigned int j = 0; j < IndividualParms.size(); j++)
        {
        root << IndividualParms[j];
        }

    os << root;
}

