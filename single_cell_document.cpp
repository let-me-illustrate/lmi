// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "single_cell_document.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "handle_exceptions.hpp"
#include "xml_lmi.hpp"

#include <xmlwrapp/document.h>
#include <xmlwrapp/nodes_view.h>
#include <xmlwrapp/schema.h>
#include <xsltwrapp/stylesheet.h>

#include <istream>
#include <ostream>

//============================================================================
single_cell_document::single_cell_document()
    :input_data_()
{
}

//============================================================================
single_cell_document::single_cell_document(Input const& z)
    :input_data_(z)
{
}

/// This ctor is used to read the default input file.

single_cell_document::single_cell_document(std::string const& filename)
    :input_data_()
{
    xml_lmi::dom_parser parser(filename);
    parse(parser);
}

//============================================================================
single_cell_document::~single_cell_document()
{
}

/// Backward-compatibility serial number of this class's xml version.
///
/// What is now called version 0 had no "version" attribute.
///
/// version 0: [prior to the lmi epoch]
/// version 1: 20130428T1828Z

int single_cell_document::class_version() const
{
    return 1;
}

//============================================================================
std::string const& single_cell_document::xml_root_name() const
{
    static std::string const s("single_cell_document");
    return s;
}

//============================================================================
void single_cell_document::parse(xml_lmi::dom_parser const& parser)
{
    if(data_source_is_external(parser.document()))
        {
        validate_with_xsd_schema(parser.document());
        }

    xml::element const& root(parser.root_node(xml_root_name()));

    xml::const_nodes_view const elements(root.elements());
    LMI_ASSERT(!elements.empty());
    xml::const_nodes_view::const_iterator i(elements.begin());
    *i >> input_data_;
    // XMLWRAPP !! It would be better to have operator+(int) in the
    // iterator class, and to write this check above as
    //   LMI_ASSERT(elements.end() == 1 + i);
    LMI_ASSERT(elements.end() == ++i);
}

//============================================================================
bool single_cell_document::data_source_is_external(xml::document const&) const
{
    return false; // Actual implementation coming soon.
}

//============================================================================
void single_cell_document::validate_with_xsd_schema(xml::document const& d) const
{
    try
        {
        xsd_schema().validate(cell_sorter().apply(d));
        }
    catch(...)
        {
        warning() << "Schema validation failed--diagnostics follow." << std::flush;
        report_exception();
        }
}

/// Stylesheet to sort <cell> elements.
///
/// This is needed for an external system that cannot economically
/// provide xml with alphabetically-sorted elements.

xslt::stylesheet& single_cell_document::cell_sorter() const
{
    static std::string const f("sort_cell_subelements.xsl");
    static xslt::stylesheet z(xml_lmi::dom_parser(AddDataDir(f)).document());
    return z;
}

//============================================================================
xml::schema const& single_cell_document::xsd_schema() const
{
    static std::string const f("single_cell_document.xsd");
    static xml::schema const z(xml_lmi::dom_parser(AddDataDir(f)).document());
    return z;
}

//============================================================================
void single_cell_document::read(std::istream const& is)
{
    xml_lmi::dom_parser parser(is);
    parse(parser);
}

//============================================================================
void single_cell_document::write(std::ostream& os) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    xml_lmi::set_attr(root, "version", class_version());
    xml_lmi::set_attr(root, "data_source", 1); // "1" means lmi.
    root << input_data_;
    os << document;
}

